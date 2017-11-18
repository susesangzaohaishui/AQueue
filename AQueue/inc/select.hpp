

#ifndef __AQUEUE_SELECT_HPP_INCLUDED__
#define __AQUEUE_SELECT_HPP_INCLUDED__

//  poller.hpp decides which polling mechanism to use.
#include "poller.hpp"
#if defined AQUEUE_USE_SELECT

#include "platform.hpp"

#include <stddef.h>
#include <vector>

#ifdef AQUEUE_HAVE_WINDOWS
#include "winsock2.h"
#elif defined AQUEUE_HAVE_OPENVMS
#include <sys/types.h>
#include <sys/time.h>
#else
#include <sys/select.h>
#endif

#include "fd.hpp"
#include "thread.hpp"
#include "poller_base.hpp"


//基于select的轮训器的实现参考Zeromq（改造自ZeroMq）
namespace aqueue
{

    struct i_poll_events;

    //  Implements socket polling mechanism using POSIX.1-2001 select()
    //  function.

    class select_t : public poller_base_t
    {
    public:

        typedef fd_t handle_t;

        select_t ();
        ~select_t ();

        //  "poller" concept.
        handle_t add_fd (fd_t fd_, aqueue::i_poll_events *events_);
        void rm_fd (handle_t handle_);
        void set_pollin (handle_t handle_);
        void reset_pollin (handle_t handle_);
        void set_pollout (handle_t handle_);
        void reset_pollout (handle_t handle_);
        void start ();
        void stop ();

    private:

        //  Main worker thread routine.
        static void worker_routine (void *arg_);

        //  Main event loop.
        void loop ();

        struct fd_entry_t
        {
            fd_t fd;
            aqueue::i_poll_events *events;
        };

        //  Checks if an fd_entry_t is retired.
        static bool is_retired_fd (const fd_entry_t &entry);

        //  Set of file descriptors that are used to retreive
        //  information for fd_set.
        typedef std::vector <fd_entry_t> fd_set_t;
        fd_set_t fds;

        fd_set source_set_in;
        fd_set source_set_out;
        fd_set source_set_err;

        fd_set readfds;
        fd_set writefds;
        fd_set exceptfds;

        //  Maximum file descriptor.
        fd_t maxfd;

        //  If true, at least one file descriptor has retired.
        bool retired;

        //  If true, thread is shutting down.
        bool stopping;

        //  Handle of the physical thread doing the I/O work.
        thread_t worker;

        select_t (const select_t&);
        const select_t &operator = (const select_t&);
    };

    typedef select_t poller_t;

}

#endif

#endif
