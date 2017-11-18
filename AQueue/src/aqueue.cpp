
#define AQUEUE_TYPE_UNSAFE

#include "platform.hpp"

#if defined AQUEUE_FORCE_SELECT
#define AQUEUE_POLL_BASED_ON_SELECT
#elif defined AQUEUE_FORCE_POLL
#define AQUEUE_POLL_BASED_ON_POLL
#elif defined AQUEUE_HAVE_LINUX || defined AQUEUE_HAVE_FREEBSD ||\
    defined AQUEUE_HAVE_OPENBSD || defined AQUEUE_HAVE_SOLARIS ||\
    defined AQUEUE_HAVE_OSX || defined AQUEUE_HAVE_QNXNTO ||\
    defined AQUEUE_HAVE_HPUX || defined AQUEUE_HAVE_AIX ||\
    defined AQUEUE_HAVE_NETBSD
#define AQUEUE_POLL_BASED_ON_POLL
#elif defined AQUEUE_HAVE_WINDOWS || defined AQUEUE_HAVE_OPENVMS ||\
     defined AQUEUE_HAVE_CYGWIN
#define AQUEUE_POLL_BASED_ON_SELECT
#endif

//  On AIX platform, poll.h has to be included first to get consistent
//  definition of pollfd structure (AIX uses 'reqevents' and 'retnevents'
//  instead of 'events' and 'revents' and defines macros to map from POSIX-y
//  names to AIX-specific names).
#if defined AQUEUE_POLL_BASED_ON_POLL
#include <poll.h>
#endif

// zmq.h must be included *after* poll.h for AIX to build properly
#include "aqueue.h"

#if defined AQUEUE_HAVE_WINDOWS
#include "windows.hpp"
#else
#include <unistd.h>
#endif


#include <string.h>
#include <stdlib.h>
#include <new>
#include <string>

#include "stdint.hpp"
#include "config.hpp"
#include "likely.hpp"
#include "clock.hpp"
#include "ctx.hpp"
#include "err.hpp"
#include "fd.hpp"

using namespace std;


#if !defined AQUEUE_HAVE_WINDOWS
#include <unistd.h>
#endif

#if defined AQUEUE_HAVE_OPENPGM
#define __PGM_WININT_H__
#include <pgm/pgm.h>
#endif


int aqueue_errno ()
{
    return errno;
}


//  New context API
#if 1

void *aqueue_ctx_new (void)
{
#if defined AQUEUE_HAVE_OPENPGM

    //  Init PGM transport. Ensure threading and timer are enabled. Find PGM
    //  protocol ID. Note that if you want to use gettimeofday and sleep for
    //  openPGM timing, set environment variables PGM_TIMER to "GTOD" and
    //  PGM_SLEEP to "USLEEP".
    pgm_error_t *pgm_error = NULL;
    const bool ok = pgm_init (&pgm_error);
    if (ok != TRUE) {

        //  Invalid parameters don't set pgm_error_t
        aqueue_assert (pgm_error != NULL);
        if (pgm_error->domain == PGM_ERROR_DOMAIN_TIME && (
              pgm_error->code == PGM_ERROR_FAILED)) {

            //  Failed to access RTC or HPET device.
            pgm_error_free (pgm_error);
            errno = EINVAL;
            return NULL;
        }

        //  PGM_ERROR_DOMAIN_ENGINE: WSAStartup errors or missing WSARecvMsg.
        aqueue_assert (false);
    }
#endif

#ifdef AQUEUE_HAVE_WINDOWS
    //  Intialise Windows sockets. Note that WSAStartup can be called multiple
    //  times given that WSACleanup will be called for each WSAStartup.
   //  We do this before the ctx constructor since its embedded mailbox_t
   //  object needs Winsock to be up and running.
    WORD version_requested = MAKEWORD (2, 2);
    WSADATA wsa_data;
    int rc = WSAStartup (version_requested, &wsa_data);
    aqueue_assert (rc == 0);
    aqueue_assert (LOBYTE (wsa_data.wVersion) == 2 &&
        HIBYTE (wsa_data.wVersion) == 2);
#endif

    //  Create 0MQ context.
    aqueue::ctx_t *ctx = new (std::nothrow) aqueue::ctx_t;
    alloc_assert (ctx);
    return ctx;
}

int aqueue_ctx_destroy (void *ctx_)
{
    if (!ctx_ || !((aqueue::ctx_t*) ctx_)->check_tag ()) {
        errno = EFAULT;
        return -1;
    }

    int rc = ((aqueue::ctx_t*) ctx_)->terminate ();
    int en = errno;

    //  Shut down only if termination was not interrupted by a signal.
    if (!rc || en != EINTR) {
#ifdef AQUEUE_HAVE_WINDOWS
        //  On Windows, uninitialise socket layer.
        rc = WSACleanup ();
        wsa_assert (rc != SOCKET_ERROR);
#endif

#if defined AQUEUE_HAVE_OPENPGM
        //  Shut down the OpenPGM library.
        if (pgm_shutdown () != TRUE)
            aqueue_assert (false);
#endif
    }

    errno = en;
    return rc;
}

//  Stable/legacy context API
//后两个参数表示线程的数量
int aqueue_ctx_set (void *ctx_, int option_,int io_msg_threads, int io_timer_threads)
{
    if (!ctx_ || !((aqueue::ctx_t*) ctx_)->check_tag ()) {
		errno = EFAULT;
		return -1;
	}
    return ((aqueue::ctx_t*) ctx_)->set (option_, io_msg_threads, io_timer_threads);
}

int aqueue_ctx_get (void *ctx_, int option_)
{
    if (!ctx_ || !((aqueue::ctx_t*) ctx_)->check_tag ()) {
		errno = EFAULT;
		return -1;
	}
    return ((aqueue::ctx_t*) ctx_)->get (option_);
}

void *aqueue_init (int io_msg_threads, int io_timer_threads)
{
	//初始化的线程数必须大于0
	if (( io_msg_threads + io_timer_threads) > 0) {
		void *ctx = aqueue_ctx_new ();
		aqueue_ctx_set (ctx, AQUEUE_IO_THREADS, io_msg_threads, io_timer_threads);
		return ctx;
	}
	errno = EINVAL;
	return NULL;   
}

int aqueue_start(void *context)
{
    if (!context || !((aqueue::ctx_t*)context)->check_tag ()) {
		errno = EFAULT;
		return -1;
	}
    return ((aqueue::ctx_t*)context)->start();
}

 int  aqueue_attach_asynobject(const char* oid, i_asynobj_events* sink_, void *context, thread_type t_type)
 {
    return ((aqueue::ctx_t*)context)->attach_asynobject(oid, sink_,  t_type);
 }

 int  aqueue_dettach_asynobject(const char* oid, void *context)
 {
    return ((aqueue::ctx_t*)context)->dettach_asynobject(oid);
 }

 int  aqueue_send_msg(const char* oid, aqueue::aqueue_msg_t* msg, void *context, bool exigence)
 {
    return ((aqueue::ctx_t*)context)->send_msg(oid, msg, exigence);
 }

 int  aqueue_register_timer(const char* oid, int timeout_, int id_, void *context)
 {
    return ((aqueue::ctx_t*)context)->register_timer(oid, timeout_, id_);
 }

 int  aqueue_unregister_timer(const char* oid, int id_, void *context)
 {
    return ((aqueue::ctx_t*)context)->unregister_timer(oid, id_);
 }

 int syn_send_msg( const char* oid, aqueue::aqueue_msg_t* msg, void *context )
 {
	 return -1;//简易在应用逻辑中使用信号量进行实现
 }

 #endif