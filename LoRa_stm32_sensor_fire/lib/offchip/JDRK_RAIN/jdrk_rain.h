#ifndef _JDRK_RAIN_H_
#define _JDRK_RAIN_H_

#include "TaskManager.h"
#include "USART.h"
#include "FreeUSART.h"
#include "IOT_sensor.h"

static u8 Sensor_Rain_Order[] = {0x03,0x03,0x00,0x00,0x00,0x01,0x85,0xE8};

class jdrk_Rain{
	private:
		FreeUsart &mCom;
	  USART &mUsart;
	  int8_t mData;//保存传感器数据
	  SensorState mState; //记录传感器状态
	  double mRecord; //超时记录时间搓
	  double mTimeOut;//超时时间
	public:
    jdrk_Rain(FreeUsart &Com,USART &Usart);//构造函数
	  SensorState Update(u8 order); //参数为命令类型，以后的接口，这里只是用1代表采集数据读取
    u16 Read();
	  void init();//获取前的数据初始化
};

#endif

