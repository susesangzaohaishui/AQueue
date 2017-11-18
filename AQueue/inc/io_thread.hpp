

#ifndef __AQUEUE_IO_THREAD_HPP_INCLUDED__
#define __AQUEUE_IO_THREAD_HPP_INCLUDED__

#include <vector>

#include "stdint.hpp"
#include "a_object.hpp"
#include "mailbox.hpp"

//用于描述一个线程（改造自ZeroMq）

namespace aqueue
{

    class ctx_t;

    //  Generic part of the I/O thread. Polling-mechanism-specific features
    //  are implemented in separate "polling objects".

    class io_thread_t : public a_object_t
    {
    public:

		

        io_thread_t (aqueue::ctx_t *ctx_, uint32_t tid_);

        //  Clean-up. If the thread was started, it's neccessary to call 'stop'
        //  before invoking destructor. Otherwise the destructor would hang up.
        ~io_thread_t ();

        //  Launch the physical thread.
        void start ();

        //  Ask underlying thread to stop.
        void stop ();

        //  Returns mailbox associated with this I/O thread.
        mailbox_t *get_mailbox ();

        //  i_poll_events implementation.
        void in_event ();
        void out_event ();
        void timer_event (int id_);

        //  Used by io_objects to retrieve the assciated poller object.
        poller_t *get_poller ();

        //  Command handlers.
        void process_stop ();

        //  Returns load experienced by the I/O thread.
        int get_load ();

		void adjust_load (int amount_);

		void set_type(thread_type _type)
		{
			a_type = _type;
		}

		thread_type get_type()
		{
			return a_type;
		}

    private:

        //  I/O thread accesses incoming commands via this mailbox.
        mailbox_t mailbox;

        //  Handle associated with mailbox' file descriptor.
        poller_t::handle_t mailbox_handle;

        //  I/O multiplexing is performed using a poller object.
        poller_t *poller;

		thread_type a_type;//建议线程类型

		atomic_counter_t asynobjload;

        io_thread_t (const io_thread_t&);
        const io_thread_t &operator = (const io_thread_t&);
    };

}

#endif
