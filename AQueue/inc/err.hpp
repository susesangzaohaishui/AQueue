

#ifndef __AQUEUE_ERR_HPP_INCLUDED__
#define __AQUEUE_ERR_HPP_INCLUDED__

//  0MQ-specific error codes are defined in zmq.h
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "platform.hpp"
#include "likely.hpp"

#ifdef AQUEUE_HAVE_WINDOWS
#include "windows.hpp"
#else
#include <netdb.h>
#endif

namespace aqueue
{
    const char *errno_to_string (int errno_);
    void aqueue_abort (const char *errmsg_);
}

#ifdef AQUEUE_HAVE_WINDOWS

namespace aqueue
{
    const char *wsa_error ();
    const char *wsa_error_no (int no_);
    void win_error (char *buffer_, size_t buffer_size_);
    int wsa_error_to_errno (int errcode);
}

//  Provides convenient way to check WSA-style errors on Windows.
#define wsa_assert(x) \
    do {\
        if (unlikely (!(x))) {\
            const char *errstr = aqueue::wsa_error ();\
            if (errstr != NULL) {\
                fprintf (stderr, "Assertion failed: %s (%s:%d)\n", errstr, \
                    __FILE__, __LINE__);\
                aqueue::aqueue_abort (errstr);\
            }\
        }\
    } while (false)

//  Provides convenient way to assert on WSA-style errors on Windows.
#define wsa_assert_no(no) \
    do {\
        const char *errstr = aqueue::wsa_error_no (no);\
        if (errstr != NULL) {\
            fprintf (stderr, "Assertion failed: %s (%s:%d)\n", errstr, \
                __FILE__, __LINE__);\
            aqueue::aqueue_abort (errstr);\
        }\
    } while (false)

// Provides convenient way to check GetLastError-style errors on Windows.
#define win_assert(x) \
    do {\
        if (unlikely (!(x))) {\
            char errstr [256];\
            aqueue::win_error (errstr, 256);\
            fprintf (stderr, "Assertion failed: %s (%s:%d)\n", errstr, \
                __FILE__, __LINE__);\
            aqueue::aqueue_abort (errstr);\
        }\
    } while (false)

#endif

//  This macro works in exactly the same way as the normal assert. It is used
//  in its stead because standard assert on Win32 in broken - it prints nothing
//  when used within the scope of JNI library.
#define aqueue_assert(x) \
    do {\
        if (unlikely (!(x))) {\
            fprintf (stderr, "Assertion failed: %s (%s:%d)\n", #x, \
                __FILE__, __LINE__);\
            aqueue::aqueue_abort (#x);\
        }\
    } while (false) 

//  Provides convenient way to check for errno-style errors.
#define errno_assert(x) \
    do {\
        if (unlikely (!(x))) {\
            const char *errstr = strerror (errno);\
            fprintf (stderr, "%s (%s:%d)\n", errstr, __FILE__, __LINE__);\
            aqueue::aqueue_abort (errstr);\
        }\
    } while (false)

//  Provides convenient way to check for POSIX errors.
#define posix_assert(x) \
    do {\
        if (unlikely (x)) {\
            const char *errstr = strerror (x);\
            fprintf (stderr, "%s (%s:%d)\n", errstr, __FILE__, __LINE__);\
            aqueue::aqueue_abort (errstr);\
        }\
    } while (false)

//  Provides convenient way to check for errors from getaddrinfo.
#define gai_assert(x) \
    do {\
        if (unlikely (x)) {\
            const char *errstr = gai_strerror (x);\
            fprintf (stderr, "%s (%s:%d)\n", errstr, __FILE__, __LINE__);\
            aqueue::aqueue_abort (errstr);\
        }\
    } while (false)

//  Provides convenient way to check whether memory allocation have succeeded.
#define alloc_assert(x) \
    do {\
        if (unlikely (!x)) {\
            fprintf (stderr, "FATAL ERROR: OUT OF MEMORY (%s:%d)\n",\
                __FILE__, __LINE__);\
            aqueue::aqueue_abort ("FATAL ERROR: OUT OF MEMORY");\
        }\
    } while (false)

#endif


