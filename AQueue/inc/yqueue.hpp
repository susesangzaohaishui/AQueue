
#ifndef __AQUEUE_YQUEUE_HPP_INCLUDED__
#define __AQUEUE_YQUEUE_HPP_INCLUDED__

#include <stdlib.h>
#include <stddef.h>

#include "err.hpp"
#include "atomic_ptr.hpp"

namespace aqueue
{
	//（改造自ZeroMq）
	//yqueue_t是一个高效的数据队列，主要的目标是在最小的分配和释放次数
	//因此yqueue_t会一次性分配N个数据，也一次性释放N个数据，减少了数据分配与释放的开销;
	//yqueue_t允许一个线程进行写入操作如 push/back ，另外一个线程进行读操作pop/front；
	//用户必须保证当队列为空的时候不能pop，多个线程不能在没有同步的情况下对同一个对象进行操作;
	//该队列是一个模板，T代表的是队列的类型，N代表的是一个结点的的数据对象个数
    template <typename T, int N> class yqueue_t
    {
    public:

        inline yqueue_t ()
        {
			//初始化的时候分配一个头结点
             begin_chunk = (chunk_t*) malloc (sizeof (chunk_t));
             alloc_assert (begin_chunk);
             begin_pos = 0;//起始位置
             back_chunk = NULL;//
             back_pos = 0;
             end_chunk = begin_chunk;//终止结点和起始结点指向地址一致
             end_pos = 0;//终止位置设置为0
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

		//返回当前结点的头位置
        inline T &front ()
        {
             return begin_chunk->values [begin_pos];
        }

		//返回当前结点的尾地址引用（地址），队列为空的时候不能做此操作
        inline T &back ()
        {
            return back_chunk->values [back_pos];
        }

		//在尾结点上增加一个结点
        inline void push ()
        {
            back_chunk	= end_chunk;
            back_pos	= end_pos;

            if (++end_pos != N)
                return;

			//查看之前是否有空闲结点
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

		//从队列的尾部删除对象，换句换说就是回滚最近push到队列中的数据，需要注意的是:
		//调用者有责任去销毁回滚的数据，调用者必须同时保证在做此操作之前队列不能是空的；
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

       //如果当前的头结点数据已经读取完成则释放该结点
        inline void pop ()
        {
			//如果当前位置超过了N，则该结点可以删除
            if (++ begin_pos == N) {
                chunk_t *o = begin_chunk;
                begin_chunk = begin_chunk->next;
                begin_chunk->prev = NULL;
                begin_pos = 0;


				//将之前的的头结点交换入spare_chunk中以便后面重复使用
                chunk_t *cs = spare_chunk.xchg (o);
                if (cs)
                    free (cs);
            }
        }

    private:

        //  一个结点中有N个数据对象
        struct chunk_t
        {
             T values [N];
             chunk_t *prev;
             chunk_t *next;
        };

		//队列是空的时候，尾结点位置标识(back_pos)指向一个无效的内存;
		//起始和结束位置标识(begin_pos,end_pos)时常指向的是有效内存;
		//begin_pos是队列中专门用来做读操作的如front/pop;
		//end_pos和back_pos是队列用来做写操作的如back/push
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
