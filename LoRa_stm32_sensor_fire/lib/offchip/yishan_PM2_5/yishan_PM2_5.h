/**
  ******************************************************************************
  * @file    yishan_PM2_5.h
  * @author  LPV6-lissettecarlr
  * @version V1.0
  * @date    08/04/2017(create)  
  * @brief   
						每秒发送一次，波特率9600 一个包长度为32个字节
						请求数据：
						33 3E 00 0C A4 00 00 00 00 00 00 00 00 00 01 21
						设定输出间隔（5s）
						33 3E 00 0C A3 00 00 05 00 00 00 00 00 00 01 25 
						定时输出  开/ / 关
						33 3E 00 0C A2 00 00 01 00 00 00 00 00 00 01 20 
						33 3E 00 0C A2 00 00 00 00 00 00 00 00 00 01 1F 
						传感器开关功能
						33 3E 00 0C A1 00 00 00 00 00 00 00 00 00 01 1E  关
						33 3E 00 0C A1 00 00 01 00 00 00 00 00 00 01 1F
    该传感器有个BUG，在发送MODBUS协议时当地址是1功能是3，将会收到回复。
  ******************************************************************************
*/

#ifndef __YISHAN_PM2_5_H__
#define __YISHAN_PM2_5_H__

#include "stm32f10x.h"
#include "FreeUSART.h"
#include "TaskManager.h"
#include "USART.h"
#include "IOT_sensor.h"

//static uint8_t order
static u8 OrderOFF[16]={0x33,0x3E,0x00,0x0C,0xA1,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x1E};
static u8 OrderON[16]={0x33,0x3E,0x00,0x0C,0xA1,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x1F};
static u8 OrderGet[16]={0x33,0x3E,0x00,0x0C,0xA4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x21};
static u8 OrderTimeClose[16]={0x33,0x3E,0x00,0x0C,0xA2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x1F};	

class yishan_PM2_5{

	private:
		FreeUsart &mCommunications;
	  USART &mSerial;
		int16_t mData;  //PM2.5浓度 单位 ug/m^3
	  SensorState mState; //记录传感器状态
	  double mRecord; //超时记录时间搓
	  double mTimeOut;//设定超时时间
    bool mCheck(u8 *Data,int Lenth,int CheckSum);
	  uint8_t mMode; //0是低功耗 ， 1是正常模式

	public:
		yishan_PM2_5(FreeUsart &com,USART &ser,uint8_t mode); 
		SensorState Update(); 
    int16_t Read();
	  void init();//获取前的数据初始化
		void OrderSensorStart();
	  void OrderSensorClose();
	  void OrderTimingOutputStart();
	  void OrderTimingOutputClose();
	  void OrderSetTimingOutput();
	  void OrderGetData();
};

#endif

