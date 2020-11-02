#ifndef _JDRK_LIGHT_H_
#define _JDRK_LIGHT_H_

#include "TaskManager.h"
#include "USART.h"
#include "FreeUSART.h"
#include "IOT_sensor.h"


static u8 Sensor_Light_Order[] = {0x02,0x03,0x00,0x06,0x00,0x01,0x64,0x38};

class jdrk_Light{
	private:
		FreeUsart &mCom;
	  USART &mUsart;
	  int16_t mData;
	  SensorState mState;
	  double mRecord; 
	  double mTimeOut;
	public:
    jdrk_Light(FreeUsart &Com,USART &Usart);
	  SensorState Update(u8 order); 
    u16 Read();
	  void init();
};

#endif

