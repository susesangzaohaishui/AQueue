
#ifndef __AQUEUE_ATOMIC_PTR_HPP_INCLUDED__
#define __AQUEUE_ATOMIC_PTR_HPP_INCLUDED__

#include "platform.hpp"

#if defined AQUEUE_FORCE_MUTEXES
#define AQUEUE_ATOMIC_PTR_MUTEX
#elif (defined __i386__ || defined __x86_64__) && defined __GNUC__
#define AQUEUE_ATOMIC_PTR_X86
#elif defined __ARM_ARCH_7A__ && defined __GNUC__
#define AQUEUE_ATOMIC_PTR_ARM
#elif defined AQUEUE_HAVE_WINDOWS
#define AQUEUE_ATOMIC_PTR_WINDOWS
#elif (defined AQUEUE_HAVE_SOLARIS || defined AQUEUE_HAVE_NETBSD)
#define AQUEUE_ATOMIC_PTR_ATOMIC_H
#else
#define AQUEUE_ATOMIC_PTR_MUTEX
#endif

#if defined AQUEUE_ATOMIC_PTR_MUTEX
#include "mutex.hpp"
#elif defined AQUEUE_ATOMIC_PTR_WINDOWS
#include "windows.hpp"
#elif defined AQUEUE_ATOMIC_PTR_ATOMIC_H
#include <atomic.h>
#endif

namespace aqueue
{

    //  This class encapsulates several atomic operations on pointers.

    template <typename T> class atomic_ptr_t
    {
    public:

        //  Initialise atomic pointer
        inline atomic_ptr_t ()
        {
            ptr = NULL;
        }

        //  Destroy atomic pointer
        inline ~atomic_ptr_t ()
        {
        }

        //  Set value of atomic pointer in a non-threadsafe way
        //  Use this function only when you are sure that at most one
        //  thread is accessing the pointer at the moment.
        inline void set (T *ptr_)
        {
            this->ptr = ptr_;
        }

        //  Perform atomic 'exchange pointers' operation. Pointer is set
        //  to the 'val' value. Old value is returned.
        inline T *xchg (T *val_)
        {
#if defined AQUEUE_ATOMIC_PTR_WINDOWS
            return (T*) InterlockedExchangePointer ((PVOID*) &ptr, val_);
#elif defined AQUEUE_ATOMIC_PTR_ATOMIC_H
            return (T*) atomic_swap_ptr (&ptr, val_);
#elif defined AQUEUE_ATOMIC_PTR_X86
			T *old;
			__asm__ volatile (
				"lock; xchg %0, %2"
				: "=r" (old), "=m" (ptr)
				: "m" (ptr), "0" (val_));
			return old;
#elif defined AQUEUE_ATOMIC_PTR_ARM
            T* old;
            unsigned int flag;
            __asm__ volatile (
                "       dmb     sy\n\t"
                "1:     ldrex   %1, [%3]\n\t"
                "       strex   %0, %4, [%3]\n\t"
                "       teq     %0, #0\n\t"
                "       bne     1b\n\t"
                "       dmb     sy\n\t"
                : "=&r"(flag), "=&r"(old), "+Qo"(ptr)
                : "r"(&ptr), "r"(val_)
                : "cc");
            return old;
#elif defined AQUEUE_ATOMIC_PTR_MUTEX
            sync.lock ();
            T *old = (T*) ptr;
            ptr = val_;
            sync.unlock ();
            return old;
#else
#error atomic_ptr is not implemented for this platform
#endif
        }

        //  Perform atomic 'compare and swap' operation on the pointer.
        //  The pointer is compared to 'cmp' argument and if they are
        //  equal, its value is set to 'val'. Old value of the pointer
        //  is returned.
        inline T *cas (T *cmp_, T *val_)
        {
#if defined AQUEUE_ATOMIC_PTR_WINDOWS
            return (T*) InterlockedCompareExchangePointer (
                (volatile PVOID*) &ptr, val_, cmp_);
#elif defined AQUEUE_ATOMIC_PTR_ATOMIC_H
            return (T*) atomic_cas_ptr (&ptr, cmp_, val_);
#elif defined AQUEUE_ATOMIC_PTR_X86
            T *old;
            __asm__ volatile (
                "lock; cmpxchg %2, %3"
                : "=a" (old), "=m" (ptr)
                : "r" (val_), "m" (ptr), "0" (cmp_)
                : "cc");
            return old;
#elif defined AQUEUE_ATOMIC_PTR_ARM
            T *old;
            unsigned int flag;
            __asm__ volatile (
                "       dmb     sy\n\t"
                "1:     ldrex   %1, [%3]\n\t"
                "       mov     %0, #0\n\t"
                "       teq     %1, %4\n\t"
                "       it      eq\n\t"
                "       strexeq %0, %5, [%3]\n\t"
                "       teq     %0, #0\n\t"
                "       bne     1b\n\t"
                "       dmb     sy\n\t"
                : "=&r"(flag), "=&r"(old), "+Qo"(ptr)
                : "r"(&ptr), "r"(cmp_), "r"(val_)
                : "cc");
            return old;
#elif defined AQUEUE_ATOMIC_PTR_MUTEX
            sync.lock ();
            T *old = (T*) ptr;
            if (ptr == cmp_)
                ptr = val_;
            sync.unlock ();
            return old;
#else
#error atomic_ptr is not implemented for this platform
#endif
        }

    private:

        volatile T *ptr;
#if defined AQUEUE_ATOMIC_PTR_MUTEX
        mutex_t sync;
#endif

        atomic_ptr_t (const atomic_ptr_t&);
        const atomic_ptr_t &operator = (const atomic_ptr_t&);
    };

}

//  Remove macros local to this file.
#if defined AQUEUE_ATOMIC_PTR_WINDOWS
#undef AQUEUE_ATOMIC_PTR_WINDOWS
#endif
#if defined AQUEUE_ATOMIC_PTR_ATOMIC_H
#undef AQUEUE_ATOMIC_PTR_ATOMIC_H
#endif
#if defined AQUEUE_ATOMIC_PTR_X86
#undef AQUEUE_ATOMIC_PTR_X86
#endif
#if defined AQUEUE_ATOMIC_PTR_ARM
#undef AQUEUE_ATOMIC_PTR_ARM
#endif
#if defined AQUEUE_ATOMIC_PTR_MUTEX
#undef AQUEUE_ATOMIC_PTR_MUTEX
#endif

#endif

