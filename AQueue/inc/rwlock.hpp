

#ifndef __AQUEUE_RWLOCK_HPP_INCLUDED__
#define __AQUEUE_RWLOCK_HPP_INCLUDED__

//  poller.hpp decides which polling mechanism to use.
#ifdef AQUEUE_HAVE_WINDOWS

#include "windows.hpp"

//��д������(�Զ��̵߳Ķ�ռ��Դ����Ҫ��������Ӳ��)
namespace aqueue
{
	class rwlock_t
	{
	public:
		rwlock_t()
		{
			//��ʼ����д��
			InitializeSRWLock(&rwlock);
		}

		void rlock()
		{
			//����
			AcquireSRWLockShared(&rwlock);
		}

		void wlock()
		{
			//д��
			AcquireSRWLockExclusive(&rwlock);
		}

		void r_unlock()
		{
			//������
			ReleaseSRWLockShared(&rwlock);
		}

		void w_unlock()
		{
			//д����
			ReleaseSRWLockExclusive(&rwlock);
		}
	private:
		SRWLOCK rwlock;
		rwlock_t (const rwlock_t&);
		void operator = (const rwlock_t&);
	};
}


#else 
//	//��Linux�������
//class rwlock_t
//{
//public:
//	rwlock_t()
//	{
//		//��ʼ����д��
//		pthread_rwlock_init(&rwlock);
//	}
//
//	void rlock()
//	{
//		//����
//		AcquireSRWLockShared(&rwlock);
//	}
//
//	void wlock()
//	{
//		//д��
//		AcquireSRWLockExclusive(&rwlock);
//	}
//
//	void r_unlock()
//	{
//		//������
//		ReleaseSRWLockShared(&rwlock);
//	}
//
//	void w_unlock()
//	{
//		//д����
//		ReleaseSRWLockExclusive(&rwlock);
//	}
//private:
//	pthread_rwlock_t rwlock;
//	rwlock_t (const rwlock_t&);
//	void operator = (const rwlock_t&);
//};

#endif

#endif
