#ifndef _TN_H_
#define _TN_H_

#include "USART.h"
#include "FreeUSART.h"
#include "IOT_sensor.h"
#include "TaskManager.h"



static u8 Sensor_TH_Order[] = {0x01,0x03,0x00,0x00,0x00,0x06,0xc5,0xc8};

class TN_water{
	private:
		FreeUsart &mCommunications;
		USART &mSerial;
		
	  SensorState mState; //记录传感器状态
	  double  mRecord;   //超时记录时间搓
	  double  mTimeOut;  //设定超时时间
	  float   mData;       //   单位:mg/L ，原始数据 * 0.01
	  int16_t mOriginalData;
	  //年月日时分秒
	public:
		char time[22];
	  TN_water(FreeUsart &com,USART &ser); 
		SensorState Update(); 
    int16_t ReadOriginal(); //获取原始数据
	  float Read();  //获取总氮含量
	  void init();   //获取前的数据初始化
};

#endif
