

#ifndef __AQUEUE_H_INCLUDED__
#define __AQUEUE_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined WINCE
#include <errno.h>
#endif
#include <stddef.h>
#include <stdio.h>
#include "aqueue_msg.h"
#include "i_aqueue_callback.h"

/*
	@AUTHOR:AdamZhao
*/
//�����첽��Ϣ���е�ʵ��(������Zeromq)
//��ʱ���кܶ�ط����ȶ������������£�ע�������ϢԽ��,δ�����ౣ��
//ʹ��ʱ��������������߳�֮������Դ��������ʹ��Ч��

//������������
//1�����ĸ��̵߳Ķ��������ĸ��߳��д�����֤�߳�֮��û�����ݾ������Ӷ��������Ч��; 
//2����֤�߳��ڲ���˳��ִ�У�û��ϵͳͬ�����ж�;
//3��ͨ��IO�������������̼߳����Ϣ���ݣ��Ӷ��γɶ���̵߳���Ϣ��������������
//4��ʹ���������У�������ݶ��еĴ����ٶȣ��ڴ��Ż����������ٷ������ͷŵ�Ƶ�ʣ���ԭ�Ӳ����Ż�(����ԭ����������)����
//

#if defined _WIN32
#   if defined DLL_EXPORT
#       define AQUEUE_EXPORT __declspec(dllexport)
#   else
#       define AQUEUE_EXPORT __declspec(dllimport)
#   endif
#else
#   if defined __SUNPRO_C  || defined __SUNPRO_CC
#       define AQUEUE_EXPORT __global
#   elif (defined __GNUC__ && __GNUC__ >= 4) || defined __INTEL_COMPILER
#       define AQUEUE_EXPORT __attribute__ ((visibility("default")))
#   else
#       define AQUEUE_EXPORT
#   endif
#endif

#define AQUEUE_IO_THREADS       1
#define AQUEUE_IO_THREADS_DFLT  1
#define AQUEUE_ASYNOBJ_ID_MIN   1
#define AQUEUE_ASYNOBJ_ID_MAX   100

//�����߳����ͣ�ʵ�ʶ���ȫ���ܵ�
//����ǿ�ƶ���
enum thread_type
{
		e_msg,//������ϢΪ��������߳�
		e_timer,//���鶨ʱ��Ϊ��������߳�
		e_other = -1//����ȫ����ѡȡ
};

//����������
AQUEUE_EXPORT void *aqueue_ctx_new (void);

//�����û������߳�������������,������ϢΪ���ߵ��̣߳����Զ�ʱ��Ϊ���ߵ��߳�(����)��
AQUEUE_EXPORT void *aqueue_init (int io_msg_threads, int io_timer_threads);

//�����߳�
AQUEUE_EXPORT int  aqueue_start(void *context);

//����������
AQUEUE_EXPORT int  aqueue_ctx_destroy (void *context);

//�����󸽼ӵ�ĳһ�߳���ȥ
AQUEUE_EXPORT int  aqueue_attach_asynobject(const char* oid, i_asynobj_events* sink_, void *context, thread_type t_type = e_msg);

//�������
AQUEUE_EXPORT int  aqueue_dettach_asynobject(const char* oid, void *context);

//��ĳ��������Ϣ
AQUEUE_EXPORT int  aqueue_send_msg(const char* oid, aqueue::aqueue_msg_t* msg, void *context,bool exigence = false);

//Ϊĳ����ע�ᶨʱ��
AQUEUE_EXPORT int  aqueue_register_timer(const char* oid, int timeout_, int id_, void *context);

//ɾ����ʱ��
AQUEUE_EXPORT int  aqueue_unregister_timer(const char* oid, int id_, void *context);

//����ͬ����Ϣ
AQUEUE_EXPORT int syn_send_msg( const char* oid, aqueue::aqueue_msg_t* msg, void *context );

#undef AQUEUE_EXPORT

#ifdef __cplusplus
}
#endif

#endif

