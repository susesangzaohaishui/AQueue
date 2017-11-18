

#include "thread.hpp"
#include "err.hpp"
#include "platform.hpp"

#ifdef AQUEUE_HAVE_WINDOWS

extern "C"
{
#if defined _WIN32_WCE
	static DWORD thread_routine (LPVOID arg_)
#else
    static unsigned int __stdcall thread_routine (void *arg_)
#endif
    {
        aqueue::thread_t *self = (aqueue::thread_t*) arg_;
        self->tfn (self->arg);
        return 0;
    }
}

void aqueue::thread_t::start (thread_fn *tfn_, void *arg_)
{
    tfn = tfn_;
    arg =arg_;
#if defined WINCE
    descriptor = (HANDLE) CreateThread (NULL, 0,
        &::thread_routine, this, 0 , NULL);
#else
    descriptor = (HANDLE) _beginthreadex (NULL, 0,
        &::thread_routine, this, 0 , NULL);
#endif
    win_assert (descriptor != NULL);    
}

void aqueue::thread_t::stop ()
{
    DWORD rc = WaitForSingleObject (descriptor, INFINITE);
    win_assert (rc != WAIT_FAILED);
    BOOL rc2 = CloseHandle (descriptor);
    win_assert (rc2 != 0);
}

#else

#include <signal.h>

extern "C"
{
    static void *thread_routine (void *arg_)
    {
#if !defined AQUEUE_HAVE_OPENVMS && !defined AQUEUE_HAVE_ANDROID
        //  Following code will guarantee more predictable latencies as it'll
        //  disallow any signal handling in the I/O thread.
        sigset_t signal_set;
        int rc = sigfillset (&signal_set);
        errno_assert (rc == 0);
        rc = pthread_sigmask (SIG_BLOCK, &signal_set, NULL);
        posix_assert (rc);
#endif

        aqueue::thread_t *self = (aqueue::thread_t*) arg_;
        self->tfn (self->arg);
        return NULL;
    }
}

void aqueue::thread_t::start (thread_fn *tfn_, void *arg_)
{
    tfn = tfn_;
    arg =arg_;
    int rc = pthread_create (&descriptor, NULL, thread_routine, this);
    posix_assert (rc);
}

void aqueue::thread_t::stop ()
{
    int rc = pthread_join (descriptor, NULL);
    posix_assert (rc);
}

#endif





