

#include <string.h>
#include <stdarg.h>

#include "a_object.hpp"
#include "ctx.hpp"
#include "err.hpp"
#include "io_thread.hpp"


aqueue::a_object_t::a_object_t (ctx_t *ctx_, uint32_t tid_, i_asynobj_events* evt_callback) :
    ctx (ctx_),
    tid (tid_),
	eventsink(evt_callback)
{
	poller = 0;
	iothread = 0;
}

aqueue::a_object_t::a_object_t (io_thread_t *io_thread_, i_asynobj_events* evt_callback) :
    ctx (io_thread_->ctx),
    tid (io_thread_->tid),
	eventsink(evt_callback)
{
	poller = io_thread_->get_poller();
	iothread = io_thread_;
}

aqueue::a_object_t::~a_object_t ()
{
}

uint32_t aqueue::a_object_t::get_tid ()
{
    return tid;
}

aqueue::ctx_t *aqueue::a_object_t::get_ctx ()
{
    return ctx;
}

void aqueue::a_object_t::send_stop ()
{
	command_t cmd;
	cmd.destination = this;
	cmd.type = command_t::stop;
	ctx->send_command (tid, cmd);
}

void aqueue::a_object_t::send_plug()
{
	command_t cmd;
	cmd.destination = this;
	cmd.type = command_t::plug;
	send_command (cmd);
}

//根据不同的命令类型进行操作
void aqueue::a_object_t::process_command (command_t &cmd_)
{
    switch (cmd_.type) {

    case command_t::stop:
        process_stop ();
        break;

    case command_t::plug:
        process_plug ();
        break;
	case command_t::msg:
		process_msg (&(cmd_.args.msg._msg));
		break;

	case command_t::timer_add:
		process_timer_add (cmd_.args.timer_add.timeout, cmd_.args.timer_add.id);
		break;

	case command_t::timer_cancel:
		process_timer_cancel (cmd_.args.timer_cancel.id);
		break;
    default:
        aqueue_assert (false);
    }
}

void aqueue::a_object_t::process_stop ()
{
	if (eventsink != NULL)
	{
		//触发对象的销毁事件，自己处理(外部注册自己的对象自己处理)
		eventsink->dettach_event();
	}

	//清除本对象的所有定时器
	poller->clear_timer(this);

	//删除自己
	delete this;
}

//自己初始化完成则告知外部注册对象
void aqueue::a_object_t::process_plug ()
{
	if (eventsink != NULL)
	{
		eventsink->attach_event();
	}
}

//在poller中增加定时对象
void aqueue::a_object_t::process_timer_add( int timeout, int id)
{
	if (0 == poller)
	{
		return;
	}
	poller->add_timer(timeout, this, id);
}

//在poller中删除指定定时对象
void aqueue::a_object_t::process_timer_cancel(int id)
{
	if (0 == poller)
	{
		return;
	}

	poller->cancel_timer(this, id);
}

//处理消息
void aqueue::a_object_t::process_msg(aqueue_msg_t* msg)
{
	if (eventsink != NULL)
	{
		eventsink->msg_event(msg);
	}
}

//定时事件的处理
void aqueue::a_object_t::timer_event (int id_)
{
	if (eventsink != NULL)
	{
		eventsink->timer_event(id_);
	}
}

//发送命令
void aqueue::a_object_t::send_command (command_t &cmd_)
{
    ctx->send_command (cmd_.destination->get_tid (), cmd_);
}

aqueue::io_thread_t* aqueue::a_object_t::get_thread()
{
	return iothread;
}
