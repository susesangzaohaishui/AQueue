

#include "epoll.hpp"
#if defined AQUEUE_USE_EPOLL

#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <algorithm>
#include <new>

#include "epoll.hpp"
#include "err.hpp"
#include "config.hpp"
#include "i_poll_events.hpp"

aqueue::epoll_t::epoll_t () :
    stopping (false)
{
    epoll_fd = epoll_create (1);
    errno_assert (epoll_fd != -1);
}

aqueue::epoll_t::~epoll_t ()
{
    //�����߳�ѭ��
    worker.stop ();

    close (epoll_fd);
    for (retired_t::iterator it = retired.begin (); it != retired.end (); ++it)
        delete *it;
}

aqueue::epoll_t::handle_t aqueue::epoll_t::add_fd (fd_t fd_, i_poll_events *events_)
{
    poll_entry_t *pe = new (std::nothrow) poll_entry_t;
    alloc_assert (pe);


	//���ø�����Ϣ
    memset (pe, 0, sizeof (poll_entry_t));

    pe->fd = fd_;
    pe->ev.events = 0;
    pe->ev.data.ptr = pe;
    pe->events = events_;

    int rc = epoll_ctl (epoll_fd, EPOLL_CTL_ADD, fd_, &pe->ev);
    errno_assert (rc != -1);

    // ���Ӹ���
    adjust_load (1);

    return pe;
}

void aqueue::epoll_t::rm_fd (handle_t handle_)
{
    poll_entry_t *pe = (poll_entry_t*) handle_;
    int rc = epoll_ctl (epoll_fd, EPOLL_CTL_DEL, pe->fd, &pe->ev);
    errno_assert (rc != -1);
    pe->fd = retired_fd;
    retired.push_back (pe);

    // �����̸߳���
    adjust_load (-1);
}

//���ö����������¼�����Ȥ
void aqueue::epoll_t::set_pollin (handle_t handle_)
{
    poll_entry_t *pe = (poll_entry_t*) handle_;
    pe->ev.events |= EPOLLIN;
    int rc = epoll_ctl (epoll_fd, EPOLL_CTL_MOD, pe->fd, &pe->ev);
    errno_assert (rc != -1);
}

//�رն������¼�����Ȥ
void aqueue::epoll_t::reset_pollin (handle_t handle_)
{
    poll_entry_t *pe = (poll_entry_t*) handle_;
    pe->ev.events &= ~((short) EPOLLIN);
    int rc = epoll_ctl (epoll_fd, EPOLL_CTL_MOD, pe->fd, &pe->ev);
    errno_assert (rc != -1);
}

//���ÿ�����¼�����Ȥ�����IO��������жϸ�֪
void aqueue::epoll_t::set_pollout (handle_t handle_)
{
    poll_entry_t *pe = (poll_entry_t*) handle_;
    pe->ev.events |= EPOLLOUT;
    int rc = epoll_ctl (epoll_fd, EPOLL_CTL_MOD, pe->fd, &pe->ev);
    errno_assert (rc != -1);
}

//�رն�����¼�����Ȥ
void aqueue::epoll_t::reset_pollout (handle_t handle_)
{
    poll_entry_t *pe = (poll_entry_t*) handle_;
    pe->ev.events &= ~((short) EPOLLOUT);
    int rc = epoll_ctl (epoll_fd, EPOLL_CTL_MOD, pe->fd, &pe->ev);
    errno_assert (rc != -1);
}

//�����߳�
void aqueue::epoll_t::start ()
{
    worker.start (worker_routine, this);
}

void aqueue::epoll_t::stop ()
{
    stopping = true;
}

int aqueue::epoll_t::max_fds ()
{
    return -1;
}

void aqueue::epoll_t::loop ()
{
    epoll_event ev_buf [max_io_events];

    while (!stopping) {

		//��ʱ����ִ�У������������һ����ʱ���ڶ���Ժ�
        int timeout = (int) execute_timers ();

		//�ȴ��¼�����Ӧ
        int n = epoll_wait (epoll_fd, &ev_buf [0], max_io_events,
            timeout ? timeout : -1);
        if (n == -1) {
            errno_assert (errno == EINTR);
            continue;
        }

        for (int i = 0; i < n; i ++) {
            poll_entry_t *pe = ((poll_entry_t*) ev_buf [i].data.ptr);

            if (pe->fd == retired_fd)
                continue;
            if (ev_buf [i].events & (EPOLLERR | EPOLLHUP))
                pe->events->in_event ();
            if (pe->fd == retired_fd)
               continue;
            if (ev_buf [i].events & EPOLLOUT)
                pe->events->out_event ();
            if (pe->fd == retired_fd)
                continue;
            if (ev_buf [i].events & EPOLLIN)
                pe->events->in_event ();
        }

        // ɾ����Ч�ĵ�ע���¼�
        for (retired_t::iterator it = retired.begin (); it != retired.end ();
              ++it)
            delete *it;
        retired.clear ();
    }
}

void aqueue::epoll_t::worker_routine (void *arg_)
{
    ((epoll_t*) arg_)->loop ();
}

#endif
