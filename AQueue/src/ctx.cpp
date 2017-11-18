

#include "platform.hpp"
#ifdef AQUEUE_HAVE_WINDOWS
#include "windows.hpp"
#else
#include <unistd.h>
#endif

#include <new>
#include <string.h>
#include "rwlock.hpp"
#include "ctx.hpp"
#include "aqueue.h"
#include "io_thread.hpp"
#include "err.hpp"
#include "a_object.hpp"

aqueue::ctx_t::ctx_t () :
    tag (0xabadcafe),
    starting (true),
    slot_count (0),
    slots (NULL),
    io_thread_count_msg (AQUEUE_IO_THREADS_DFLT),
	io_thread_count_timer(AQUEUE_IO_THREADS_DFLT)
{

 //   object_slots = (a_object_t**) malloc (sizeof (a_object_t*) * (AQUEUE_ASYNOBJ_ID_MAX + 1));
	//alloc_assert (object_slots);
	//memset(object_slots, 0, sizeof (a_object_t*) * (AQUEUE_ASYNOBJ_ID_MAX + 1));
}

bool aqueue::ctx_t::check_tag ()
{
    return tag == 0xabadcafe;
}

int aqueue::ctx_t::terminate ()
{
	delete this;
	return 0;
}

int aqueue::ctx_t::set (int option_, int io_msg_threads, int io_timer_threads)
{
	int rc = 0;

	if (option_ == AQUEUE_IO_THREADS && (io_msg_threads + io_timer_threads) >= 0){
		opt_sync.lock ();
		io_thread_count_msg = io_msg_threads;
		io_thread_count_timer = io_timer_threads;
		opt_sync.unlock ();
	}
	else {
		errno = EINVAL;
		rc = -1;
	}
	return rc;
}

int aqueue::ctx_t::get (int option_)
{
	/*int rc = 0;

    if (option_ == AQUEUE_IO_THREADS)
		rc = io_thread_count;
	else {
		errno = EINVAL;
		rc = -1;
	}
	return rc;*/
	return -1;
}

aqueue::ctx_t::~ctx_t ()
{
    //  Ask I/O threads to terminate. If stop signal wasn't sent to I/O
    //  thread subsequent invocation of destructor would hang-up.
    for (io_threads_t::size_type i = 0; i != io_threads.size (); i++)
        io_threads [i]->stop ();

    //  Wait till I/O threads actually terminate.
    for (io_threads_t::size_type i = 0; i != io_threads.size (); i++)
        delete io_threads [i];

    //  Deallocate the array of mailboxes. No special work is
    //  needed as mailboxes themselves were deallocated with their
    //  corresponding io_thread/socket objects.
    if (slots)
        free (slots);

	//if (object_slots)
	//	free (object_slots);


    //  Remove the tag, so that the object is considered dead.
    tag = 0xdeadbeef;
}

void aqueue::ctx_t::send_command (uint32_t tid_, const command_t &command_, bool exigence)
{
	slots [tid_]->send (command_, exigence);
}


int  aqueue::ctx_t::attach_asynobject(const string&  oid, void* sink_, thread_type t_type)
{
	//查找该对象ID是否被占用
	aobj_rwlock.rlock();
	map<string, a_object_t*>::iterator it;
	if((it = m_aobj.find(oid)) != m_aobj.end())
	{
		return -1;
	}
	aobj_rwlock.r_unlock();

	//选择一个线程
	io_thread_t *io_thread = choose_io_thread (0, t_type);
	aqueue_assert (io_thread);

	//创建一个异步对象(将注册对象传入)
	a_object_t* pObj = new a_object_t(io_thread, static_cast<i_asynobj_events*>(sink_));

	//管理关系
	aobj_rwlock.wlock();
	m_aobj[oid] = pObj;
	aobj_rwlock.w_unlock();

	//增加一个负载
	io_thread->adjust_load(1);

	//初始化
	pObj->send_plug();

	return io_thread->get_tid();
}

int  aqueue::ctx_t::dettach_asynobject(const string& oid)
{
	//查找对应ID的异步对象
	aobj_rwlock.wlock();
	map<string, a_object_t*>::iterator it;
	if((it = m_aobj.find(oid)) != m_aobj.end())
	{
		command_t cmd;
		cmd.type = command_t::stop;
		cmd.destination = it->second;
		
		//向目标对象发送消息
		send_command (cmd.destination->get_tid (), cmd);


        //减少负载
        //io_thread_t* pthread = it->second->get_thread();
        //if (pthread)
        //{
        //	pthread->adjust_load(-1);
        //}
        

		//删除管理对象
		m_aobj.erase(it);


		aobj_rwlock.w_unlock();
		return 0;
	}
	aobj_rwlock.w_unlock();
	return -1;
}

int  aqueue::ctx_t::send_msg(const string& oid, aqueue::aqueue_msg_t* msg, bool exigence)
{
	//std::hashmap
	//查找对应ID的异步对象
	aobj_rwlock.rlock();
	map<string, a_object_t*>::iterator it;
	if((it = m_aobj.find(oid)) != m_aobj.end())
	{
		command_t cmd;
		cmd.type = command_t::msg;
		cmd.destination = it->second;
		memcpy(&cmd.args.msg._msg, msg, sizeof(aqueue::aqueue_msg_t));

		//找到对象对应的线程并将命令抛送进去
		send_command (cmd.destination->get_tid (), cmd, exigence);
		aobj_rwlock.r_unlock();
		return 0;
	}
	aobj_rwlock.r_unlock();
	return -1;
}

int  aqueue::ctx_t::register_timer(const string& oid, int timeout_, int id_)
{
	//查找对应ID的异步对象
	map<string, a_object_t*>::iterator it;
	aobj_rwlock.rlock();
	if((it = m_aobj.find(oid)) != m_aobj.end())
	{
		//在异步对象自己的Poller的定时队列里面增加节点
		command_t cmd;
		cmd.type = command_t::timer_add;
		cmd.destination = it->second;
		cmd.args.timer_add.id = id_;
		cmd.args.timer_add.timeout = timeout_;

		send_command (cmd.destination->get_tid (), cmd);
		aobj_rwlock.r_unlock();
		return 0;
	}
	aobj_rwlock.r_unlock();
	return -1;
}

int  aqueue::ctx_t::unregister_timer(const string& oid, int id_)
{
	//查找对应ID的异步对象
	map<string, a_object_t*>::iterator it;
	aobj_rwlock.rlock();
	if((it = m_aobj.find(oid)) != m_aobj.end())
	{
		//删除异步队列中的定时节点
		command_t cmd;
		cmd.type = command_t::timer_cancel;
		cmd.destination = it->second;
		cmd.args.timer_cancel.id = id_;

		send_command (cmd.destination->get_tid (), cmd);
		aobj_rwlock.r_unlock();
		return 0;
	}
	aobj_rwlock.r_unlock();
	return -1;
}

aqueue::io_thread_t *aqueue::ctx_t::choose_io_thread (uint64_t affinity_, thread_type t_type)
{
    if (io_threads.empty ())//固定生成的应该不用加锁????
        return NULL;

    //  Find the I/O thread with minimum load.
    int min_load = -1;
    io_thread_t *selected_io_thread = NULL;
	slot_sync.rlock();
    for (io_threads_t::size_type i = 0; i != io_threads.size (); i++) {
		//指定为其他线程类，或者自己指定的线程类型则继续
		if((e_other == t_type) || (t_type == io_threads [i]->get_type()))
		{
			//affinity_为指定可选的线程实例
			if ((!affinity_ || (affinity_ & (uint64_t (1) << i)))) {
				int load = io_threads [i]->get_load ();
				if (selected_io_thread == NULL || load < min_load) {
					min_load = load;
					selected_io_thread = io_threads [i];
				}
			}
		}
    }
	slot_sync.r_unlock();
    return selected_io_thread;
}


int aqueue::ctx_t::start ()
{
	slot_sync.wlock();
	if (unlikely (starting)) {
#if 0
		starting = false;
		//  Initialise the array of mailboxes. Additional three slots are for
		//  zmq_term thread and reaper thread.
		opt_sync.lock ();
		int ios = io_thread_count;
		opt_sync.unlock ();
		slot_count = ios + 2;
		slots = (mailbox_t**) malloc (sizeof (mailbox_t*) * slot_count);
		alloc_assert (slots);

		//  Create I/O thread objects and launch them.
		for (int i = 2; i != ios + 2; i++) {
			io_thread_t *io_thread = new (std::nothrow) io_thread_t (this, i);
			alloc_assert (io_thread);
			io_threads.push_back (io_thread);
			slots [i] = io_thread->get_mailbox ();
			io_thread->start ();
		}
#else
		starting = false;
		//  Initialise the array of mailboxes. Additional three slots are for
		//  zmq_term thread and reaper thread.
		opt_sync.lock ();
		int ios_msg =  io_thread_count_msg;
		int ios_timer =  io_thread_count_timer;
		opt_sync.unlock ();

		//先生成mailbox的存储位置
		slot_count = ios_msg + ios_timer;
		slots = (mailbox_t**) malloc (sizeof (mailbox_t*) * slot_count);
		alloc_assert (slots);

		//  创建以消息为主类型的线程
		int i = 0;
		for (;i < ios_msg; ++i)
		{
			io_thread_t *io_thread = new (std::nothrow) io_thread_t (this, i);
			alloc_assert (io_thread);
			io_thread->set_type(e_msg);
			io_threads.push_back (io_thread);
			slots [i] = io_thread->get_mailbox ();
			io_thread->start ();
		}
		
		//  创建以定时器为主类型的线程
		for (;i < ios_msg + ios_timer; ++i)
		{
			io_thread_t *io_thread = new (std::nothrow) io_thread_t (this, i);
			alloc_assert (io_thread);
			io_thread->set_type(e_timer);
			io_threads.push_back (io_thread);
			slots [i] = io_thread->get_mailbox ();
			io_thread->start ();
		}
#endif
	}
	slot_sync.w_unlock();

	return 0;
}
