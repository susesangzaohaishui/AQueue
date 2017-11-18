
#ifndef __AQUEUE_COMMAND_HPP_INCLUDED__
#define __AQUEUE_COMMAND_HPP_INCLUDED__

#include "stdint.hpp"
#include "aqueue_msg.h"

namespace aqueue
{

    class a_object_t;
    //  This structure defines the commands that can be sent between threads.

    struct command_t
    {
        //  Object to process the command.
        aqueue::a_object_t *destination;

        enum type_t
        {
            stop,
            plug,
			msg,
			timer_add,
			timer_cancel
        } type;

        union {

            //  Sent to I/O thread to let it know that it should
            //  terminate itself.
            struct {
            } stop;

            //  Sent to I/O object to make it register with its I/O thread.
            struct {
            } plug;

			struct {
                aqueue_msg_t _msg;
			}msg;

			struct {
				 int timeout;
				 int id;
			}timer_add;

			struct {
				int id;
			}timer_cancel;
        } args;
    };

}    

#endif
