

#ifndef __AQUEUE_SIGNALER_HPP_INCLUDED__
#define __AQUEUE_SIGNALER_HPP_INCLUDED__

#ifdef HAVE_FORK
#include <unistd.h>
#endif

#include "fd.hpp"

namespace aqueue
{
	//����һ����ƽ̨����Ϣ֪ͨ���ƣ�������ZeroMq��
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

        //�������ƹܵ���socketpair��r���ڼ����̵߳Ķ���w��������д���̵߳�д
        static int make_fdpair (fd_t *r_, fd_t *w_);

        fd_t w;
        fd_t r;

        //  Disable copying of signaler_t object.
        signaler_t (const signaler_t&);
        const signaler_t &operator = (const signaler_t&);

#ifdef HAVE_FORK
		//�����ӽ��̵�״̬��������û�д˴���
        pid_t pid;
        void close_internal();
#endif
    };
}

#endif
