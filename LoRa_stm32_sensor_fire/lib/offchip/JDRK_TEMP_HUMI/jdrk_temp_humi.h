#ifndef _JDRK_TEMP_HUMI_H_
#define _JDRK_TEMP_HUMI_H_

#include "TaskManager.h"
#include "USART.h"
#include "FreeUSART.h"
#include "IOT_sensor.h"
static u8 Sensor_TempHumi_Order[] = {0x06,0x03,0x00,0x00,0x00,0x02,0xC5,0xBC};

class jdrk_TempHumi{
	private:
		FreeUsart &mCom;
	  USART &mUsart;
	  int32_t mData;//���洫��������
	  SensorState mState; //��¼������״̬
	  double mRecord; //��ʱ��¼ʱ���
	  double mTimeOut;//��ʱʱ��
	public:
    jdrk_TempHumi(FreeUsart &Com,USART &Usart);//���캯��
	  SensorState Update(u8 order); //����Ϊ�������ͣ��Ժ�Ľӿڣ�����ֻ����1����ɼ����ݶ�ȡ
    int32_t Read();
	  void init();//��ȡǰ�����ݳ�ʼ��
};

#endif

