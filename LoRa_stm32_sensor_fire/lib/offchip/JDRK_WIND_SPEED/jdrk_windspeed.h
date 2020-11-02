#ifndef _JDRK_WINDSPEED_H_
#define _JDRK_WINDSPEED_H_

#include "TaskManager.h"
#include "USART.h"
#include "FreeUSART.h"
#include "IOT_sensor.h"
static u8 Sensor_WindSpeed_Order[] = {0x04,0x03,0x00,0x00,0x00,0x01,0x84,0x5F};

class jdrk_WindSpeed{
	private:
		FreeUsart &mCom;
	  USART &mUsart;
	  int16_t mData;//���洫��������
		int WindSpeed_H;
		int WindSpeed_L;
	  SensorState mState; //��¼������״̬
	  double mRecord; //��ʱ��¼ʱ���
	  double mTimeOut;//��ʱʱ��
	public:
    jdrk_WindSpeed(FreeUsart &Com,USART &Usart);//���캯��
	  SensorState Update(u8 order); //����Ϊ�������ͣ��Ժ�Ľӿڣ�����ֻ����1����ɼ����ݶ�ȡ
    u16 Read();
	  void init();//��ȡǰ�����ݳ�ʼ��
};

#endif

