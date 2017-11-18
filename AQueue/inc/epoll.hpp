

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
//在Linux中实现的轮训器（改造自ZeroMq）
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

        //  线程中的操作函数
        static void worker_routine (void *arg_);

        //  主事件循环
        void loop ();

        //  Epoll中的文件描述符
        fd_t epoll_fd;

        struct poll_entry_t
        {
            fd_t fd;
            epoll_event ev;
            aqueue::i_poll_events *events;
        };

        //注册到轮训器中的对象
        typedef std::vector <poll_entry_t*> retired_t;
        retired_t retired;

		//标识轮训器线程的工作状态
        bool stopping;

        //轮训器的线程
        thread_t worker;

        epoll_t (const epoll_t&);
        const epoll_t &operator = (const epoll_t&);
    };

    typedef epoll_t poller_t;

}

#endif

#endif
