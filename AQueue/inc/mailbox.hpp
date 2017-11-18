
#ifndef __AQUEUE_MAILBOX_HPP_INCLUDED__
#define __AQUEUE_MAILBOX_HPP_INCLUDED__

#include <stddef.h>

#include "platform.hpp"
#include "signaler.hpp"
#include "fd.hpp"
#include "config.hpp"
#include "command.hpp"
#include "ypipe.hpp"
#include "mutex.hpp"

namespace aqueue
{
	//������Ϣ����������ZeroMq��
	class mailbox_t
	{
	public:

		mailbox_t ();
		~mailbox_t ();

		fd_t get_fd ();
		void send (const command_t &cmd_, bool exigence = false);
		int recv (command_t *cmd_, int timeout_);

	private:

        //�����������ڴ洢����
        typedef ypipe_t <command_t, command_pipe_granularity> cpipe_t;
        cpipe_t cpipe;

        //д���߳�֪ͨ���߳�ȥ������
        signaler_t signaler;

		//�п����ж���߳�����mailbox������Ϣ������ֻ��һ���߳��ڷ�����Ϣ����ҲҪ�Է��Ͷ˽���ͬ��
        mutex_t sync;

		//��ʶ���������Ƿ�����Ϣ����
        bool active;

        //  ��ֹ����
        mailbox_t (const mailbox_t&);
        const mailbox_t &operator = (const mailbox_t&);
    };

}

#endif
