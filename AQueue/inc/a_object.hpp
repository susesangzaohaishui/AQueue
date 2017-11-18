

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
	//后台管理的异步对象
    class a_object_t: public i_poll_events
    {
    public:
        a_object_t (aqueue::ctx_t *ctx_, uint32_t tid_, i_asynobj_events* evt_callback = NULL);
        a_object_t (io_thread_t *io_thread_, i_asynobj_events* evt_callback = NULL);
        virtual ~a_object_t ();

		//获取
        uint32_t get_tid ();

		//获取上下文
        ctx_t *get_ctx ();

		io_thread_t*	get_thread();

		//向本对象发送停止消息。
		void send_stop ();

		//向本对象发送初始化消息
		void send_plug ();

		//向指定对象发送消息
		void send_command (command_t &cmd_);

		//执行命令
        void process_command (aqueue::command_t &cmd_);

	

	public:
		//监听描述符有输入事件
		virtual void in_event (){}

		//输出事件可执行
		virtual void out_event (){}

		//定时事件到
		virtual void timer_event (int id_);

    public:
        void log (const char *format_, ...);

		//几种命令的执行
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
