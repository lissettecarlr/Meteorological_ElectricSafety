#ifndef _JDRK_WINDDIRECTION_H_
#define _JDRK_WINDDIRECTION_H_

#include "TaskManager.h"
#include "USART.h"
#include "FreeUSART.h"
#include "IOT_sensor.h"
static u8 Sensor_WindDirection_Order[] = {0x05,0x03,0x00,0x00,0x00,0x02,0xC5,0x8F};

class jdrk_WindDirection{
	private:
		FreeUsart &mCom;
	  USART &mUsart;
	  int16_t mData;//���洫��������
		int WindDirection_H;
		int WindDirection_L;
	  SensorState mState; //��¼������״̬
	  double mRecord; //��ʱ��¼ʱ���
	  double mTimeOut;//��ʱʱ��
	public:
    jdrk_WindDirection(FreeUsart &Com,USART &Usart);//���캯��
	  SensorState Update(u8 order); //����Ϊ�������ͣ��Ժ�Ľӿڣ�����ֻ����1����ɼ����ݶ�ȡ
    u16 Read();
	  void init();//��ȡǰ�����ݳ�ʼ��
};

#endif

