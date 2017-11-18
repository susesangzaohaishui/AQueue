

#ifndef __AQUEUE_SIGNALER_HPP_INCLUDED__
#define __AQUEUE_SIGNALER_HPP_INCLUDED__

#ifdef HAVE_FORK
#include <unistd.h>
#endif

#include "fd.hpp"

namespace aqueue
{
	//这是一个跨平台的消息通知机制（改造自ZeroMq）
    class signaler_t
    {
    public:

		signaler_t ();
		~signaler_t ();

		fd_t get_fd ();
		void send ();
		int wait (int timeout_);
		void recv ();

#ifdef HAVE_FORK
		void forked();
#endif

    private:

        //创建类似管道的socketpair，r用于监听线程的读，w用于数据写入线程的写
        static int make_fdpair (fd_t *r_, fd_t *w_);

        fd_t w;
        fd_t r;

        //  Disable copying of signaler_t object.
        signaler_t (const signaler_t&);
        const signaler_t &operator = (const signaler_t&);

#ifdef HAVE_FORK
		//创建子进程的状态，本队列没有此处理
        pid_t pid;
        void close_internal();
#endif
    };
}

#endif
