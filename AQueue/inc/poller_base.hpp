

#ifndef __AQUEUE_POLLER_BASE_HPP_INCLUDED__
#define __AQUEUE_POLLER_BASE_HPP_INCLUDED__

#include <map>

#include "clock.hpp"
#include "atomic_counter.hpp"

//在不同的平台的轮训器实现不一样:（改造自ZeroMq）
//在Linux上使用的Epoll，在Win上使用的select;
//在此事项的是一些通用的操作：定时操作，负载操作
namespace aqueue
{

    struct i_poll_events;

    class poller_base_t
    {
    public:

        poller_base_t ();
        virtual ~poller_base_t ();

		//获取当前轮训器的负载
        int get_load ();

        //增加定时器
        void add_timer (int timeout_, aqueue::i_poll_events *sink_, int id_);

        //  撤销定时器
        void cancel_timer (aqueue::i_poll_events *sink_, int id_);

		//清理对应对象的所有定时器(必须在线程内部执行)
		void clear_timer(aqueue::i_poll_events *sink_);

		//调用执行定时器
		uint64_t execute_timers ();

    protected:

		//增加或减小负载
		void adjust_load (int amount_);

    private:

        //时间操作（在支持CPU时间支持的平台上优先使用定时器时间）
        clock_t clock;

        //  描述定时信息
        struct timer_info_t
        {
            aqueue::i_poll_events *sink;
            int id;
            int timeout;
        };
		//时间点为Key,定时对象描述
        typedef std::multimap <uint64_t, timer_info_t> timers_t;
        timers_t timers;

        //原子计数器，用于对负载的计数
        atomic_counter_t load;

        poller_base_t (const poller_base_t&);
        const poller_base_t &operator = (const poller_base_t&);
    };

}

#endif
