

#include <new>
#include "io_thread.hpp"
#include "platform.hpp"
#include "err.hpp"
#include "ctx.hpp"

aqueue::io_thread_t::io_thread_t (ctx_t *ctx_, uint32_t tid_) :
    a_object_t (ctx_, tid_)
{
    poller = new (std::nothrow) poller_t;
    alloc_assert (poller);

	mailbox_handle = poller->add_fd (mailbox.get_fd (), this);
	poller->set_pollin (mailbox_handle);
}

aqueue::io_thread_t::~io_thread_t ()
{ 
    delete poller;
}

void aqueue::io_thread_t::start ()
{
    //  Start the underlying I/O thread.
    poller->start ();
}

void aqueue::io_thread_t::stop ()
{
    send_stop ();
}

aqueue::mailbox_t *aqueue::io_thread_t::get_mailbox ()
{
    return &mailbox;
}

int aqueue::io_thread_t::get_load ()
{
    //return poller->get_load ();
	return asynobjload.get();
}

void aqueue::io_thread_t::adjust_load (int amount_)
{
	if (amount_ > 0)
		asynobjload.add (amount_);
	else if (amount_ < 0)
		asynobjload.sub (-amount_);
}

void aqueue::io_thread_t::in_event ()
{
    //  TODO: Do we want to limit number of commands I/O thread can
    //  process in a single go?

    command_t cmd;
    int rc = mailbox.recv (&cmd, 0);

    while (rc == 0 || errno == EINTR) {
		if (rc == 0){
			if (cmd.type == command_t::plug){
				//这个需要在主线程统一增加负载标识
				//adjust_load(1);
				//poller->adjust_load(1);
			}
            cmd.destination->process_command (cmd);
		}
		
        rc = mailbox.recv (&cmd, 0);
        
        //如果有数据则继续执行定时器
		if (0 == rc || errno == EINTR)
		{
			//在这里执行定时器可否???
			poller->execute_timers();
		}
		
		
    }

    errno_assert (rc != 0 && errno == EAGAIN);
}

void aqueue::io_thread_t::out_event ()
{
    //  We are never polling for POLLOUT here. This function is never called.
    aqueue_assert (false);
}

void aqueue::io_thread_t::timer_event (int)
{
    //  No timers here. This function is never called.
    aqueue_assert (false);
}

aqueue::poller_t *aqueue::io_thread_t::get_poller ()
{
    aqueue_assert (poller);
    return poller;
}

void aqueue::io_thread_t::process_stop ()
{
    poller->rm_fd (mailbox_handle);
    poller->stop ();
}
