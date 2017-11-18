

#include "select.hpp"
#if defined AQUEUE_USE_SELECT

#include "platform.hpp"
#if defined AQUEUE_HAVE_WINDOWS
#include "windows.hpp"
#elif defined AQUEUE_HAVE_HPUX
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#elif defined AQUEUE_HAVE_OPENVMS
#include <sys/types.h>
#include <sys/time.h>
#else
#include <sys/select.h>
#endif

#include <string.h>
#include <algorithm>
#include <iostream>

using namespace std;


#include "err.hpp"
#include "config.hpp"
#include "i_poll_events.hpp"

aqueue::select_t::select_t () :
    maxfd (retired_fd),
    retired (false),
    stopping (false)
{
    //  Clear file descriptor sets.
    FD_ZERO (&source_set_in);
    FD_ZERO (&source_set_out);
    FD_ZERO (&source_set_err);
}

aqueue::select_t::~select_t ()
{
    worker.stop ();
}

aqueue::select_t::handle_t aqueue::select_t::add_fd (fd_t fd_, i_poll_events *events_)
{
    //  Store the file descriptor.
    fd_entry_t entry = {fd_, events_};
    fds.push_back (entry);

    //  Ensure we do not attempt to select () on more than FD_SETSIZE
    //  file descriptors.
    aqueue_assert (fds.size () <= FD_SETSIZE);

    //  Start polling on errors.
    FD_SET (fd_, &source_set_err);

    //  Adjust maxfd if necessary.
    if (fd_ > maxfd)
        maxfd = fd_;

    //  Increase the load metric of the thread.
    adjust_load (1);

    return fd_;
}

void aqueue::select_t::rm_fd (handle_t handle_)
{
    //  Mark the descriptor as retired.
    fd_set_t::iterator it;
    for (it = fds.begin (); it != fds.end (); ++it)
        if (it->fd == handle_)
            break;
    aqueue_assert (it != fds.end ());
    it->fd = retired_fd;
    retired = true;

    //  Stop polling on the descriptor.
    FD_CLR (handle_, &source_set_in);
    FD_CLR (handle_, &source_set_out);
    FD_CLR (handle_, &source_set_err);

    //  Discard all events generated on this file descriptor.
    FD_CLR (handle_, &readfds);
    FD_CLR (handle_, &writefds);
    FD_CLR (handle_, &exceptfds);

    //  Adjust the maxfd attribute if we have removed the
    //  highest-numbered file descriptor.
    if (handle_ == maxfd) {
        maxfd = retired_fd;
        for (fd_set_t::iterator it = fds.begin (); it != fds.end (); ++it)
            if (it->fd > maxfd)
                maxfd = it->fd;
    }

    //  Decrease the load metric of the thread.
    adjust_load (-1);
}

void aqueue::select_t::set_pollin (handle_t handle_)
{
    FD_SET (handle_, &source_set_in);
}

void aqueue::select_t::reset_pollin (handle_t handle_)
{
    FD_CLR (handle_, &source_set_in);
}

void aqueue::select_t::set_pollout (handle_t handle_)
{
    FD_SET (handle_, &source_set_out);
}

void aqueue::select_t::reset_pollout (handle_t handle_)
{
    FD_CLR (handle_, &source_set_out);
}

void aqueue::select_t::start ()
{
    worker.start (worker_routine, this);
}

void aqueue::select_t::stop ()
{
    stopping = true;
}

void aqueue::select_t::loop ()
{
    while (!stopping) {

        //  Execute any due timers.
        int timeout = (int) execute_timers ();
        //  Intialise the pollsets.
        memcpy (&readfds, &source_set_in, sizeof source_set_in);
        memcpy (&writefds, &source_set_out, sizeof source_set_out);
        memcpy (&exceptfds, &source_set_err, sizeof source_set_err);

        //  Wait for events.
        struct timeval tv = {(long) (timeout / 1000),
            (long) (timeout % 1000 * 1000)};
#ifdef AQUEUE_HAVE_WINDOWS
		int rc = select (0, &readfds, &writefds, &exceptfds,
			timeout ? &tv : NULL);
		wsa_assert (rc != SOCKET_ERROR);
#else
        int rc = select (maxfd + 1, &readfds, &writefds, &exceptfds,
            timeout ? &tv : NULL);
        if (rc == -1) {
            errno_assert (errno == EINTR);
            continue;
        }
#endif

        //  If there are no events (i.e. it's a timeout) there's no point
        //  in checking the pollset.
        if (rc == 0)
            continue;

        for (fd_set_t::size_type i = 0; i < fds.size (); i ++) {
            if (fds [i].fd == retired_fd)
                continue;
            if (FD_ISSET (fds [i].fd, &exceptfds))
				fds [i].events->in_event ();
			if (fds [i].fd == retired_fd)
                continue;
            if (FD_ISSET (fds [i].fd, &writefds))
                fds [i].events->out_event ();
            if (fds [i].fd == retired_fd)
                continue;
			if (FD_ISSET (fds [i].fd, &readfds))
				fds [i].events->in_event ();
        }

        //  Destroy retired event sources.
        if (retired) {
            fds.erase (std::remove_if (fds.begin (), fds.end (),
                aqueue::select_t::is_retired_fd), fds.end ());
            retired = false;
        }
    }
}

void aqueue::select_t::worker_routine (void *arg_)
{
    ((select_t*) arg_)->loop ();
}

bool aqueue::select_t::is_retired_fd (const fd_entry_t &entry)
{
    return (entry.fd == retired_fd);
}

#endif
