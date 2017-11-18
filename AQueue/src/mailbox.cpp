
#include "mailbox.hpp"
#include "err.hpp"

aqueue::mailbox_t::mailbox_t ()
{

	//在创建的时候清空队列，设置状态为非活动
    bool ok = cpipe.read (NULL);
    aqueue_assert (!ok);
    active = false;
}

aqueue::mailbox_t::~mailbox_t ()
{
    //  TODO: Retrieve and deallocate commands inside the cpipe.
}

aqueue::fd_t aqueue::mailbox_t::get_fd ()
{
	//获取socketpair的可读文件描述符
	//用于注册到poller中去，以便监听是否有
	//其他线程有消息发送过来了
    return signaler.get_fd ();
}

void aqueue::mailbox_t::send (const command_t &cmd_, bool exigence)
{
	//向队列中写入数据
	sync.lock ();
	cpipe.write (cmd_, false);
	bool ok = cpipe.flush ();
	sync.unlock ();
	if (!ok)
		//写入成功则发送消息
		signaler.send ();
}

int aqueue::mailbox_t::recv (command_t *cmd_, int timeout_)
{
    //  如果mailbox状态设置为活动则从队列中读取数据
    if (active) {
        bool ok = cpipe.read (cmd_);
        if (ok)
            return 0;

        //如果没有获取到数据则设置状态为非活动状态
        active = false;
        signaler.recv ();
    }

    // 等待发送端的数据
    int rc = signaler.wait (timeout_);
    if (rc != 0 && (errno == EAGAIN || errno == EINTR))
        return -1;

    //获取到了消息信号则设置状态为活动状态
    active = true;

    //从队列中获取数据
    errno_assert (rc == 0);
    bool ok = cpipe.read (cmd_);
    aqueue_assert (ok);
    return 0;
}

