

#ifndef __AQUEUE_OBJECT_HPP_INCLUDED__
#define __AQUEUE_OBJECT_HPP_INCLUDED__

#include "stdint.hpp"
#include "aqueue.h"
#include "i_poll_events.hpp"
#include "poller.hpp"
namespace aqueue
{
    struct command_t;
    struct aqueue_msg_t;
    class  ctx_t;
    class  io_thread_t;
	//��̨������첽����
    class a_object_t: public i_poll_events
    {
    public:
        a_object_t (aqueue::ctx_t *ctx_, uint32_t tid_, i_asynobj_events* evt_callback = NULL);
        a_object_t (io_thread_t *io_thread_, i_asynobj_events* evt_callback = NULL);
        virtual ~a_object_t ();

		//��ȡ
        uint32_t get_tid ();

		//��ȡ������
        ctx_t *get_ctx ();

		io_thread_t*	get_thread();

		//�򱾶�����ֹͣ��Ϣ��
		void send_stop ();

		//�򱾶����ͳ�ʼ����Ϣ
		void send_plug ();

		//��ָ����������Ϣ
		void send_command (command_t &cmd_);

		//ִ������
        void process_command (aqueue::command_t &cmd_);

	

	public:
		//�����������������¼�
		virtual void in_event (){}

		//����¼���ִ��
		virtual void out_event (){}

		//��ʱ�¼���
		virtual void timer_event (int id_);

    public:
        void log (const char *format_, ...);

		//���������ִ��
        virtual void process_stop ();
        virtual void process_plug ();
		virtual void process_timer_add( int timeout, int id);
		virtual void process_timer_cancel(int id);
        virtual void process_msg(aqueue_msg_t* msg);

    private:
		i_asynobj_events* eventsink;
        aqueue::ctx_t *ctx;

        uint32_t tid;

		poller_t *poller;
		io_thread_t*	iothread;

        a_object_t (const a_object_t&);
        const a_object_t &operator = (const a_object_t&);

    };

}

#endif
