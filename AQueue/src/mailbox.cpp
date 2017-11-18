
#include "mailbox.hpp"
#include "err.hpp"

aqueue::mailbox_t::mailbox_t ()
{

	//�ڴ�����ʱ����ն��У�����״̬Ϊ�ǻ
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
	//��ȡsocketpair�Ŀɶ��ļ�������
	//����ע�ᵽpoller��ȥ���Ա�����Ƿ���
	//�����߳�����Ϣ���͹�����
    return signaler.get_fd ();
}

void aqueue::mailbox_t::send (const command_t &cmd_, bool exigence)
{
	//�������д������
	sync.lock ();
	cpipe.write (cmd_, false);
	bool ok = cpipe.flush ();
	sync.unlock ();
	if (!ok)
		//д��ɹ�������Ϣ
		signaler.send ();
}

int aqueue::mailbox_t::recv (command_t *cmd_, int timeout_)
{
    //  ���mailbox״̬����Ϊ���Ӷ����ж�ȡ����
    if (active) {
        bool ok = cpipe.read (cmd_);
        if (ok)
            return 0;

        //���û�л�ȡ������������״̬Ϊ�ǻ״̬
        active = false;
        signaler.recv ();
    }

    // �ȴ����Ͷ˵�����
    int rc = signaler.wait (timeout_);
    if (rc != 0 && (errno == EAGAIN || errno == EINTR))
        return -1;

    //��ȡ������Ϣ�ź�������״̬Ϊ�״̬
    active = true;

    //�Ӷ����л�ȡ����
    errno_assert (rc == 0);
    bool ok = cpipe.read (cmd_);
    aqueue_assert (ok);
    return 0;
}

