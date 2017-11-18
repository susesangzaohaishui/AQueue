
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

//��Ϣ���еĹ��������ģ�������ZeroMq��
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

		//��ָ���ĵ��̷߳�����Ϣ����ʵ�Ƿ��͸���Ӧ��MailBox��
        void send_command (uint32_t tid_, const command_t &command_, bool exigence = false);

		//ѡ������С���̣߳�������ʷͳ�ƣ�
        aqueue::io_thread_t *choose_io_thread (uint64_t affinity_, thread_type t_type = e_msg);


		//�������ID�Ͷ���ָ��
		 int  attach_asynobject(const string& oid, void* sink_, thread_type t_type = e_msg);

		 //�������ID
		 int  dettach_asynobject(const string& oid);

		 //������Ϣ
         int  send_msg(const string& oid, aqueue_msg_t* msg, bool exigence);

		 //ע�ᶨʱ��
		 int  register_timer(const string& oid, int timeout_, int id_);

		 //������ʱ��
		 int  unregister_timer(const string& oid, int id_);
	

        ~ctx_t ();

    private:


        //  Used to check whether the object is a context.
        uint32_t tag;

		//��ʶ�Ƿ�ʼ
        bool starting;

        rwlock_t slot_sync;

        // �߳�
        typedef std::vector <aqueue::io_thread_t*> io_threads_t;
        io_threads_t io_threads;

        //  ÿ��iothread�е�mailbox�ĵ�ַ
        uint32_t slot_count;
        mailbox_t **slots;

        //  ��ͬ�������͵��̸߳���
        int io_thread_count_msg;
		int io_thread_count_timer;

		//�첽����
		rwlock_t					aobj_rwlock;//�첽�����д��
		map<string, a_object_t*>	m_aobj;

        //  Synchronisation of access to context options.
        mutex_t opt_sync;

        ctx_t (const ctx_t&);
        const ctx_t &operator = (const ctx_t&);
    };

}

#endif
