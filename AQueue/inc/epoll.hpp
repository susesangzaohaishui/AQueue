

#ifndef __AQUEUE_EPOLL_HPP_INCLUDED__
#define __AQUEUE_EPOLL_HPP_INCLUDED__

//  poller.hpp decides which polling mechanism to use.
#include "poller.hpp"
#if defined AQUEUE_USE_EPOLL

#include <vector>
#include <sys/epoll.h>

#include "fd.hpp"
#include "thread.hpp"
#include "poller_base.hpp"
//��Linux��ʵ�ֵ���ѵ����������ZeroMq��
namespace aqueue
{

    struct i_poll_events;

    class epoll_t : public poller_base_t
    {
    public:

        typedef void* handle_t;

        epoll_t ();
        ~epoll_t ();

        handle_t add_fd (fd_t fd_, aqueue::i_poll_events *events_);
        void rm_fd (handle_t handle_);
        void set_pollin (handle_t handle_);
        void reset_pollin (handle_t handle_);
        void set_pollout (handle_t handle_);
        void reset_pollout (handle_t handle_);
        void start ();
        void stop ();

        static int max_fds ();

    private:

        //  �߳��еĲ�������
        static void worker_routine (void *arg_);

        //  ���¼�ѭ��
        void loop ();

        //  Epoll�е��ļ�������
        fd_t epoll_fd;

        struct poll_entry_t
        {
            fd_t fd;
            epoll_event ev;
            aqueue::i_poll_events *events;
        };

        //ע�ᵽ��ѵ���еĶ���
        typedef std::vector <poll_entry_t*> retired_t;
        retired_t retired;

		//��ʶ��ѵ���̵߳Ĺ���״̬
        bool stopping;

        //��ѵ�����߳�
        thread_t worker;

        epoll_t (const epoll_t&);
        const epoll_t &operator = (const epoll_t&);
    };

    typedef epoll_t poller_t;

}

#endif

#endif
