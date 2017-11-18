

#ifndef __AQUEUE_MUTEX_HPP_INCLUDED__
#define __AQUEUE_MUTEX_HPP_INCLUDED__

#include "platform.hpp"
#include "err.hpp"

//  锁的跨平台（改造自ZeroMq）

#ifdef AQUEUE_HAVE_WINDOWS

#include "windows.hpp"

namespace aqueue
{

    class mutex_t
    {
    public:
        inline mutex_t ()
        {
            InitializeCriticalSection (&cs);
        }

        inline ~mutex_t ()
        {
            DeleteCriticalSection (&cs);
        }

        inline void lock ()
        {
            EnterCriticalSection (&cs);
        }

        inline void unlock ()
        {
            LeaveCriticalSection (&cs);
        }

    private:

        CRITICAL_SECTION cs;

        //  Disable copy construction and assignment.
        mutex_t (const mutex_t&);
        void operator = (const mutex_t&);
    };

}

#else

#include <pthread.h>

namespace aqueue
{

    class mutex_t
    {
    public:
        inline mutex_t ()
        {
            int rc = pthread_mutex_init (&mutex, NULL);
            posix_assert (rc);
        }

        inline ~mutex_t ()
        {
            int rc = pthread_mutex_destroy (&mutex);
            posix_assert (rc);
        }

        inline void lock ()
        {
            int rc = pthread_mutex_lock (&mutex);
            posix_assert (rc);
        }

        inline void unlock ()
        {
            int rc = pthread_mutex_unlock (&mutex);
            posix_assert (rc);
        }

    private:

        pthread_mutex_t mutex;

        // Disable copy construction and assignment.
        mutex_t (const mutex_t&);
        const mutex_t &operator = (const mutex_t&);
    };

}

#endif

#endif
