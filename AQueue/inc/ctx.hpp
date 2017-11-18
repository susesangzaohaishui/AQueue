
#ifndef __AQUEUE_CTX_HPP_INCLUDED__
#define __AQUEUE_CTX_HPP_INCLUDED__

#include <map>
#include <vector>
#include <string>
#include <stdarg.h>

using namespace std;


#include "mailbox.hpp"
#include "config.hpp"
#include "mutex.hpp"
#include "stdint.hpp"
#include "rwlock.hpp"
#include "atomic_counter.hpp"
#include "aqueue.h"

//消息队列的管理上下文（改造自ZeroMq）
namespace aqueue
{
    class a_object_t;
    class io_thread_t;
	//enum thread_type;
    class ctx_t
    {
    public:

        //  Create the context object.
        ctx_t ();

        //  Returns false if object is not a context.
        bool check_tag ();

	
		int terminate ();

		int start();

		int set (int option_, int io_msg_threads, int io_timer_threads);
		int get (int option_);

		//向指定的的线程发送消息（其实是发送给对应的MailBox）
        void send_command (uint32_t tid_, const command_t &command_, bool exigence = false);

		//选择负载最小的线程（根据历史统计）
        aqueue::io_thread_t *choose_io_thread (uint64_t affinity_, thread_type t_type = e_msg);


		//输入对象ID和对象指针
		 int  attach_asynobject(const string& oid, void* sink_, thread_type t_type = e_msg);

		 //输入对象ID
		 int  dettach_asynobject(const string& oid);

		 //发送消息
         int  send_msg(const string& oid, aqueue_msg_t* msg, bool exigence);

		 //注册定时器
		 int  register_timer(const string& oid, int timeout_, int id_);

		 //撤销定时器
		 int  unregister_timer(const string& oid, int id_);
	

        ~ctx_t ();

    private:


        //  Used to check whether the object is a context.
        uint32_t tag;

		//标识是否开始
        bool starting;

        rwlock_t slot_sync;

        // 线程
        typedef std::vector <aqueue::io_thread_t*> io_threads_t;
        io_threads_t io_threads;

        //  每个iothread中的mailbox的地址
        uint32_t slot_count;
        mailbox_t **slots;

        //  不同建议类型的线程个数
        int io_thread_count_msg;
		int io_thread_count_timer;

		//异步对象
		rwlock_t					aobj_rwlock;//异步对象读写锁
		map<string, a_object_t*>	m_aobj;

        //  Synchronisation of access to context options.
        mutex_t opt_sync;

        ctx_t (const ctx_t&);
        const ctx_t &operator = (const ctx_t&);
    };

}

#endif
