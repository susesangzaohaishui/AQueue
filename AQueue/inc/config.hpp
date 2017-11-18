

#ifndef __AQUEUE_CONFIG_HPP_INCLUDED__
#define __AQUEUE_CONFIG_HPP_INCLUDED__

namespace aqueue
{

    //  Compile-time settings.

    enum
    {
        //  Number of new messages in message pipe needed to trigger new memory
        //  allocation. Setting this parameter to 256 decreases the impact of
        //  memory allocation by approximately 99.6%
        message_pipe_granularity = 256,

        //���е�ÿ���ڵ�洢����������������ٷ��������ͷ��ڴ�Ĵ��������Ч��
        command_pipe_granularity = 512,

        //  Determines how often does socket poll for new commands when it
        //  still has unprocessed messages to handle. Thus, if it is set to 100,
        //  socket will process 100 inbound messages before doing the poll.
        //  If there are no unprocessed messages available, poll is done
        //  immediately. Decreasing the value trades overall latency for more
        //  real-time behaviour (less latency peaks).
        inbound_poll_rate = 100,

        //  Maximal batching size for engines with receiving functionality.
        //  So, if there are 10 messages that fit into the batch size, all of
        //  them may be read by a single 'recv' system call, thus avoiding
        //  unnecessary network stack traversals.
        in_batch_size = 8192,

        //  Maximal batching size for engines with sending functionality.
        //  So, if there are 10 messages that fit into the batch size, all of
        //  them may be written by a single 'send' system call, thus avoiding
        //  unnecessary network stack traversals.
        out_batch_size = 8192,

        //  Maximal delta between high and low watermark.
        max_wm_delta = 1024,

        //  Maximum number of events the I/O thread can process in one go.
        max_io_events = 256,

        //  Maximal delay to process command in API thread (in CPU ticks).
        //  3,000,000 ticks equals to 1 - 2 milliseconds on current CPUs.
        //  Note that delay is only applied when there is continuous stream of
        //  messages to process. If not so, commands are processed immediately.
        max_command_delay = 3000000,

        //  Low-precision clock precision in CPU ticks. 1ms. Value of 1000000
        //  should be OK for CPU frequencies above 1GHz. If should work
        //  reasonably well for CPU frequencies above 500MHz. For lower CPU
        //  frequencies you may consider lowering this value to get best
        //  possible latencies.
        clock_precision = 1000000,

        //  Maximum transport data unit size for PGM (TPDU).
        pgm_max_tpdu = 1500,

        //  On some OSes the signaler has to be emulated using a TCP
        //  connection. In such cases following port is used.
        signaler_port = 5905
    };

}

#endif
