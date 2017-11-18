

#ifndef __AQUEUE_RWLOCK_HPP_INCLUDED__
#define __AQUEUE_RWLOCK_HPP_INCLUDED__

//  poller.hpp decides which polling mechanism to use.
#ifdef AQUEUE_HAVE_WINDOWS

#include "windows.hpp"

//读写锁定义(对读线程的独占资源还是要加锁，如硬件)
namespace aqueue
{
	class rwlock_t
	{
	public:
		rwlock_t()
		{
			//初始化读写锁
			InitializeSRWLock(&rwlock);
		}

		void rlock()
		{
			//读锁
			AcquireSRWLockShared(&rwlock);
		}

		void wlock()
		{
			//写锁
			AcquireSRWLockExclusive(&rwlock);
		}

		void r_unlock()
		{
			//读解锁
			ReleaseSRWLockShared(&rwlock);
		}

		void w_unlock()
		{
			//写解锁
			ReleaseSRWLockExclusive(&rwlock);
		}
	private:
		SRWLOCK rwlock;
		rwlock_t (const rwlock_t&);
		void operator = (const rwlock_t&);
	};
}


#else 
//	//在Linux的情况下
//class rwlock_t
//{
//public:
//	rwlock_t()
//	{
//		//初始化读写锁
//		pthread_rwlock_init(&rwlock);
//	}
//
//	void rlock()
//	{
//		//读锁
//		AcquireSRWLockShared(&rwlock);
//	}
//
//	void wlock()
//	{
//		//写锁
//		AcquireSRWLockExclusive(&rwlock);
//	}
//
//	void r_unlock()
//	{
//		//读解锁
//		ReleaseSRWLockShared(&rwlock);
//	}
//
//	void w_unlock()
//	{
//		//写解锁
//		ReleaseSRWLockExclusive(&rwlock);
//	}
//private:
//	pthread_rwlock_t rwlock;
//	rwlock_t (const rwlock_t&);
//	void operator = (const rwlock_t&);
//};

#endif

#endif
