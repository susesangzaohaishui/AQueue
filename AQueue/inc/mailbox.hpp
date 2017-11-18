
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
	//用于消息处理（改造自ZeroMq）
	class mailbox_t
	{
	public:

		mailbox_t ();
		~mailbox_t ();

		fd_t get_fd ();
		void send (const command_t &cmd_, bool exigence = false);
		int recv (command_t *cmd_, int timeout_);

	private:

        //无锁队列用于存储命令
        typedef ypipe_t <command_t, command_pipe_granularity> cpipe_t;
        cpipe_t cpipe;

        //写入线程通知本线程去读数据
        signaler_t signaler;

		//有可能有多个线程在往mailbox发送消息，尽管只有一个线程在发送消息，但也要对发送端进行同步
        mutex_t sync;

		//标识无锁队列是否有消息过来
        bool active;

        //  禁止拷贝
        mailbox_t (const mailbox_t&);
        const mailbox_t &operator = (const mailbox_t&);
    };

}

#endif
