

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

//���ݲ�ͬ���������ͽ��в���
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
		//��������������¼����Լ�����(�ⲿע���Լ��Ķ����Լ�����)
		eventsink->dettach_event();
	}

	//�������������ж�ʱ��
	poller->clear_timer(this);

	//ɾ���Լ�
	delete this;
}

//�Լ���ʼ��������֪�ⲿע�����
void aqueue::a_object_t::process_plug ()
{
	if (eventsink != NULL)
	{
		eventsink->attach_event();
	}
}

//��poller�����Ӷ�ʱ����
void aqueue::a_object_t::process_timer_add( int timeout, int id)
{
	if (0 == poller)
	{
		return;
	}
	poller->add_timer(timeout, this, id);
}

//��poller��ɾ��ָ����ʱ����
void aqueue::a_object_t::process_timer_cancel(int id)
{
	if (0 == poller)
	{
		return;
	}

	poller->cancel_timer(this, id);
}

//������Ϣ
void aqueue::a_object_t::process_msg(aqueue_msg_t* msg)
{
	if (eventsink != NULL)
	{
		eventsink->msg_event(msg);
	}
}

//��ʱ�¼��Ĵ���
void aqueue::a_object_t::timer_event (int id_)
{
	if (eventsink != NULL)
	{
		eventsink->timer_event(id_);
	}
}

//��������
void aqueue::a_object_t::send_command (command_t &cmd_)
{
    ctx->send_command (cmd_.destination->get_tid (), cmd_);
}

aqueue::io_thread_t* aqueue::a_object_t::get_thread()
{
	return iothread;
}
