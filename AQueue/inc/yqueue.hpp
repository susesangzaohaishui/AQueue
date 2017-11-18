
#ifndef __AQUEUE_YQUEUE_HPP_INCLUDED__
#define __AQUEUE_YQUEUE_HPP_INCLUDED__

#include <stdlib.h>
#include <stddef.h>

#include "err.hpp"
#include "atomic_ptr.hpp"

namespace aqueue
{
	//��������ZeroMq��
	//yqueue_t��һ����Ч�����ݶ��У���Ҫ��Ŀ��������С�ķ�����ͷŴ���
	//���yqueue_t��һ���Է���N�����ݣ�Ҳһ�����ͷ�N�����ݣ����������ݷ������ͷŵĿ���;
	//yqueue_t����һ���߳̽���д������� push/back ������һ���߳̽��ж�����pop/front��
	//�û����뱣֤������Ϊ�յ�ʱ����pop������̲߳�����û��ͬ��������¶�ͬһ��������в���;
	//�ö�����һ��ģ�壬T������Ƕ��е����ͣ�N�������һ�����ĵ����ݶ������
    template <typename T, int N> class yqueue_t
    {
    public:

        inline yqueue_t ()
        {
			//��ʼ����ʱ�����һ��ͷ���
             begin_chunk = (chunk_t*) malloc (sizeof (chunk_t));
             alloc_assert (begin_chunk);
             begin_pos = 0;//��ʼλ��
             back_chunk = NULL;//
             back_pos = 0;
             end_chunk = begin_chunk;//��ֹ������ʼ���ָ���ַһ��
             end_pos = 0;//��ֹλ������Ϊ0
        }

        //  Destroy the queue.
        inline ~yqueue_t ()
        {
            while (true) {
                if (begin_chunk == end_chunk) {
                    free (begin_chunk);
                    break;
                } 
                chunk_t *o = begin_chunk;
                begin_chunk = begin_chunk->next;
                free (o);
            }

            chunk_t *sc = spare_chunk.xchg (NULL);
            if (sc)
                free (sc);
        }

		//���ص�ǰ����ͷλ��
        inline T &front ()
        {
             return begin_chunk->values [begin_pos];
        }

		//���ص�ǰ����β��ַ���ã���ַ��������Ϊ�յ�ʱ�������˲���
        inline T &back ()
        {
            return back_chunk->values [back_pos];
        }

		//��β���������һ�����
        inline void push ()
        {
            back_chunk	= end_chunk;
            back_pos	= end_pos;

            if (++end_pos != N)
                return;

			//�鿴֮ǰ�Ƿ��п��н��
            chunk_t *sc = spare_chunk.xchg (NULL);
            if (sc) {
                end_chunk->next = sc;
                sc->prev = end_chunk;
            } else {
                end_chunk->next = (chunk_t*) malloc (sizeof (chunk_t));
                alloc_assert (end_chunk->next);
                end_chunk->next->prev = end_chunk;
            }
            end_chunk = end_chunk->next;
            end_pos = 0;
        }

		//�Ӷ��е�β��ɾ�����󣬻��任˵���ǻع����push�������е����ݣ���Ҫע�����:
		//������������ȥ���ٻع������ݣ������߱���ͬʱ��֤�����˲���֮ǰ���в����ǿյģ�
        inline void unpush ()
        {
            //  First, move 'back' one position backwards.
            if (back_pos)
                --back_pos;
            else {
                back_pos = N - 1;
                back_chunk = back_chunk->prev;
            }

            //  Now, move 'end' position backwards. Note that obsolete end chunk
            //  is not used as a spare chunk. The analysis shows that doing so
            //  would require free and atomic operation per chunk deallocated
            //  instead of a simple free.
            if (end_pos)
                --end_pos;
            else {
                end_pos = N - 1;
                end_chunk = end_chunk->prev;
                free (end_chunk->next);
                end_chunk->next = NULL;
            }
        }

       //�����ǰ��ͷ��������Ѿ���ȡ������ͷŸý��
        inline void pop ()
        {
			//�����ǰλ�ó�����N����ý�����ɾ��
            if (++ begin_pos == N) {
                chunk_t *o = begin_chunk;
                begin_chunk = begin_chunk->next;
                begin_chunk->prev = NULL;
                begin_pos = 0;


				//��֮ǰ�ĵ�ͷ��㽻����spare_chunk���Ա�����ظ�ʹ��
                chunk_t *cs = spare_chunk.xchg (o);
                if (cs)
                    free (cs);
            }
        }

    private:

        //  һ���������N�����ݶ���
        struct chunk_t
        {
             T values [N];
             chunk_t *prev;
             chunk_t *next;
        };

		//�����ǿյ�ʱ��β���λ�ñ�ʶ(back_pos)ָ��һ����Ч���ڴ�;
		//��ʼ�ͽ���λ�ñ�ʶ(begin_pos,end_pos)ʱ��ָ�������Ч�ڴ�;
		//begin_pos�Ƕ�����ר������������������front/pop;
		//end_pos��back_pos�Ƕ���������д��������back/push
        chunk_t *begin_chunk;
        int begin_pos;
        chunk_t *back_chunk;
        int back_pos;
        chunk_t *end_chunk;
        int end_pos;

        //  People are likely to produce and consume at similar rates.  In
        //  this scenario holding onto the most recently freed chunk saves
        //  us from having to call malloc/free.
        atomic_ptr_t<chunk_t> spare_chunk;

        //  Disable copying of yqueue.
        yqueue_t (const yqueue_t&);
        const yqueue_t &operator = (const yqueue_t&);
    };

}

#endif
