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
	  int16_t mData;//保存传感器数据
		int WindDirection_H;
		int WindDirection_L;
	  SensorState mState; //记录传感器状态
	  double mRecord; //超时记录时间搓
	  double mTimeOut;//超时时间
	public:
    jdrk_WindDirection(FreeUsart &Com,USART &Usart);//构造函数
	  SensorState Update(u8 order); //参数为命令类型，以后的接口，这里只是用1代表采集数据读取
    u16 Read();
	  void init();//获取前的数据初始化
};

#endif

