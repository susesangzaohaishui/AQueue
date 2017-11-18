

#ifndef __AQUEUE_ATOMIC_COUNTER_HPP_INCLUDED__
#define __AQUEUE_ATOMIC_COUNTER_HPP_INCLUDED__

#include "stdint.hpp"
#include "platform.hpp"

#if defined AQUEUE_FORCE_MUTEXES
#define AQUEUE_ATOMIC_COUNTER_MUTEX
#elif (defined __i386__ || defined __x86_64__) && defined __GNUC__
#define AQUEUE_ATOMIC_COUNTER_X86
#elif defined __ARM_ARCH_7A__ && defined __GNUC__
#define AQUEUE_ATOMIC_COUNTER_ARM
#elif defined AQUEUE_HAVE_WINDOWS
#define AQUEUE_ATOMIC_COUNTER_WINDOWS
#elif (defined AQUEUE_HAVE_SOLARIS || defined AQUEUE_HAVE_NETBSD)
#define AQUEUE_ATOMIC_COUNTER_ATOMIC_H
#else
#define AQUEUE_ATOMIC_COUNTER_MUTEX
#endif

#if defined AQUEUE_ATOMIC_COUNTER_MUTEX
#include "mutex.hpp"
#elif defined AQUEUE_ATOMIC_COUNTER_WINDOWS
#include "windows.hpp"
#elif defined AQUEUE_ATOMIC_COUNTER_ATOMIC_H
#include <atomic.h>
#endif

namespace aqueue
{

    //  This class represents an integer that can be incremented/decremented
    //  in atomic fashion.

    class atomic_counter_t
    {
    public:

        typedef uint32_t integer_t;

        inline atomic_counter_t (integer_t value_ = 0) :
            value (value_)
        {
        }

        inline ~atomic_counter_t ()
        {
        }

        //  Set counter value (not thread-safe).
        inline void set (integer_t value_)
        {
            value = value_;
        }

        //  Atomic addition. Returns the old value.
        inline integer_t add (integer_t increment_)
        {
            integer_t old_value;

#if defined AQUEUE_ATOMIC_COUNTER_WINDOWS
            old_value = InterlockedExchangeAdd ((LONG*) &value, increment_);
#elif defined AQUEUE_ATOMIC_COUNTER_ATOMIC_H
            integer_t new_value = atomic_add_32_nv (&value, increment_);
            old_value = new_value - increment_;
#elif defined AQUEUE_ATOMIC_COUNTER_X86
            __asm__ volatile (
                "lock; xadd %0, %1 \n\t"
                : "=r" (old_value), "=m" (value)
                : "0" (increment_), "m" (value)
                : "cc", "memory");
#elif defined AQUEUE_ATOMIC_COUNTER_ARM
            integer_t flag, tmp;
            __asm__ volatile (
                "       dmb     sy\n\t"
                "1:     ldrex   %0, [%5]\n\t"
                "       add     %2, %0, %4\n\t"
                "       strex   %1, %2, [%5]\n\t"
                "       teq     %1, #0\n\t"
                "       bne     1b\n\t"
                "       dmb     sy\n\t"
                : "=&r"(old_value), "=&r"(flag), "=&r"(tmp), "+Qo"(value)
                : "Ir"(increment_), "r"(&value)
                : "cc");
#elif defined AQUEUE_ATOMIC_COUNTER_MUTEX
            sync.lock ();
            old_value = value;
            value += increment_;
            sync.unlock ();
#else
#error atomic_counter is not implemented for this platform
#endif
            return old_value;
        }

        //  Atomic subtraction. Returns false if the counter drops to zero.
        inline bool sub (integer_t decrement)
        {
#if defined AQUEUE_ATOMIC_COUNTER_WINDOWS
            LONG delta = - ((LONG) decrement);
            integer_t old = InterlockedExchangeAdd ((LONG*) &value, delta);
            return old - decrement != 0;
#elif defined AQUEUE_ATOMIC_COUNTER_ATOMIC_H
            int32_t delta = - ((int32_t) decrement);
            integer_t nv = atomic_add_32_nv (&value, delta);
            return nv != 0;
#elif defined AQUEUE_ATOMIC_COUNTER_X86
            integer_t oldval = -decrement;
            volatile integer_t *val = &value;
            __asm__ volatile ("lock; xaddl %0,%1"
                : "=r" (oldval), "=m" (*val)
                : "0" (oldval), "m" (*val)
                : "cc", "memory");
            return oldval != decrement;
#elif defined AQUEUE_ATOMIC_COUNTER_ARM
            integer_t old_value, flag, tmp;
            __asm__ volatile (
                "       dmb     sy\n\t"
                "1:     ldrex   %0, [%5]\n\t"
                "       sub     %2, %0, %4\n\t"
                "       strex   %1, %2, [%5]\n\t"
                "       teq     %1, #0\n\t"
                "       bne     1b\n\t"
                "       dmb     sy\n\t"
                : "=&r"(old_value), "=&r"(flag), "=&r"(tmp), "+Qo"(value)
                : "Ir"(decrement), "r"(&value)
                : "cc");
            return old_value - decrement != 0;
#elif defined AQUEUE_ATOMIC_COUNTER_MUTEX
            sync.lock ();
            value -= decrement;
            bool result = value ? true : false;
            sync.unlock ();
            return result;
#else
#error atomic_counter is not implemented for this platform
#endif
        }

        inline integer_t get ()
        {
            return value;
        }

    private:

        volatile integer_t value;
#if defined AQUEUE_ATOMIC_COUNTER_MUTEX
        mutex_t sync;
#endif

        atomic_counter_t (const atomic_counter_t&);
        const atomic_counter_t& operator = (const atomic_counter_t&);
    };

}

//  Remove macros local to this file.
#if defined AQUEUE_ATOMIC_COUNTER_WINDOWS
#undef AQUEUE_ATOMIC_COUNTER_WINDOWS
#endif
#if defined AQUEUE_ATOMIC_COUNTER_ATOMIC_H
#undef AQUEUE_ATOMIC_COUNTER_ATOMIC_H
#endif
#if defined AQUEUE_ATOMIC_COUNTER_X86
#undef AQUEUE_ATOMIC_COUNTER_X86
#endif
#if defined AQUEUE_ATOMIC_COUNTER_ARM
#undef AQUEUE_ATOMIC_COUNTER_ARM
#endif
#if defined AQUEUE_ATOMIC_COUNTER_MUTEX
#undef AQUEUE_ATOMIC_COUNTER_MUTEX
#endif

#endif

