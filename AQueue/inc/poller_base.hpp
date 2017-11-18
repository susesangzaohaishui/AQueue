

#ifndef __AQUEUE_POLLER_BASE_HPP_INCLUDED__
#define __AQUEUE_POLLER_BASE_HPP_INCLUDED__

#include <map>

#include "clock.hpp"
#include "atomic_counter.hpp"

//�ڲ�ͬ��ƽ̨����ѵ��ʵ�ֲ�һ��:��������ZeroMq��
//��Linux��ʹ�õ�Epoll����Win��ʹ�õ�select;
//�ڴ��������һЩͨ�õĲ�������ʱ���������ز���
namespace aqueue
{

    struct i_poll_events;

    class poller_base_t
    {
    public:

        poller_base_t ();
        virtual ~poller_base_t ();

		//��ȡ��ǰ��ѵ���ĸ���
        int get_load ();

        //���Ӷ�ʱ��
        void add_timer (int timeout_, aqueue::i_poll_events *sink_, int id_);

        //  ������ʱ��
        void cancel_timer (aqueue::i_poll_events *sink_, int id_);

		//�����Ӧ��������ж�ʱ��(�������߳��ڲ�ִ��)
		void clear_timer(aqueue::i_poll_events *sink_);

		//����ִ�ж�ʱ��
		uint64_t execute_timers ();

    protected:

		//���ӻ��С����
		void adjust_load (int amount_);

    private:

        //ʱ���������֧��CPUʱ��֧�ֵ�ƽ̨������ʹ�ö�ʱ��ʱ�䣩
        clock_t clock;

        //  ������ʱ��Ϣ
        struct timer_info_t
        {
            aqueue::i_poll_events *sink;
            int id;
            int timeout;
        };
		//ʱ���ΪKey,��ʱ��������
        typedef std::multimap <uint64_t, timer_info_t> timers_t;
        timers_t timers;

        //ԭ�Ӽ����������ڶԸ��صļ���
        atomic_counter_t load;

        poller_base_t (const poller_base_t&);
        const poller_base_t &operator = (const poller_base_t&);
    };

}

#endif
