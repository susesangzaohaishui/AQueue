#ifndef __AQUEUE_I_CALLBACK_HPP_INCLUDED__
#define __AQUEUE_I_CALLBACK_HPP_INCLUDED__
//�첽����ص��ӿ�
class i_asynobj_events
{
public:
	//���󸽼ӳɹ��¼����ڸ��ӳɹ����߳�ִ�У�
	virtual void attach_event () = 0;

	//�����̷߳���ɹ��¼����ڸ��ӵ��߳���ִ�У�������Դ�ͷſ��̣߳�
	virtual void dettach_event () = 0;

	//��ʱ�¼�
	virtual void timer_event (int id_) = 0;

	//��Ϣ�¼�
    virtual void msg_event(aqueue::aqueue_msg_t* msg) = 0;
};
#endif
