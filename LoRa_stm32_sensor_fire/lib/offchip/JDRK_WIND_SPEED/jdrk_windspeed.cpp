#include "jdrk_windspeed.h"



//modbus crc效验算法
static uint16_t  higa_modbus_crc(uint8_t *higa_modbus_buff,uint8_t size)
{
	uint16_t crc_register;
	uint8_t index,pos;
	crc_register = 0xffff;
	for ( index = 0; index < size; index++ ){
		crc_register = crc_register ^ (higa_modbus_buff[0] & 0x00ff);
		higa_modbus_buff++;
		for ( pos = 0; pos < 8; pos++ ){
			if ( crc_register & 0x0001){
				crc_register = crc_register >> 1;
				crc_register ^= 0xa001;
			}else{
				crc_register = crc_register >> 1;
			}
		}
	}
	return crc_register;
}


 jdrk_WindSpeed::jdrk_WindSpeed(FreeUsart &Com,USART &Usart):mCom(Com),mUsart(Usart)
{
	 mData = WINDSPEED_TIMEOUT;
	 mState = SENSOR_STATE_READY;
   mTimeOut=WAINDSPEED_TIMEOUT_DEFAULT;
}

SensorState  jdrk_WindSpeed::Update(u8 order)
{
	 switch (mState)
	 {
	   case SENSOR_STATE_READY://开始传感器采集
		 {
		   mState = SENSOR_STATE_BUSY;
		   mRecord = tskmgr.Time();
		   mCom.StartRecive();
		   switch(order)
			 {
			   case 1:{  
									mUsart.SendData(Sensor_WindSpeed_Order,8);  
								}break;
			 }
		 
		 }break;
		 case SENSOR_STATE_BUSY: //正在进行传感器采集
		 {
		     if(mCom.ReciveIsOk())//如果获取到数据
			  {
				 u8  Temp[50];//暂存发送回来的数据
				 u16 DataLenth=0;//暂存发送回来的长度
				 DataLenth = mUsart.ReceiveBufferSize();
				 mUsart.GetReceivedData(Temp,DataLenth);
			  //数据处理,先进行CRC校验，在进行数据范围判断，最后存入mDta，如果出错则data = 111. 如果数据超出正常范围 data = 112
				
				  u16 crc = higa_modbus_crc(Temp,DataLenth-2);
				  u16 check = ((u16)Temp[DataLenth-1]<<8) + Temp[DataLenth-2];
					if(crc != check)
					{
						mData = WINDSPEED_CRC_ERROR;  
						mState = SENSOR_STATE_OK;
						break;
					}
				 //根据发送命令不同接收处理不同
				 switch(order)
			   {
					case 1:
					{

						mData = ((u16)Temp[3]<<8)+Temp[4];
					}break;
			   }
				//end
				 mState = SENSOR_STATE_OK;
				 break;
			  }
	      if(tskmgr.TimeSlice(mRecord,mTimeOut))//如果超时
		  	{
				  mData= WINDSPEED_TIMEOUT;
			    mState = SENSOR_STATE_OK;
		  	}
		 }break;
	 
		 case SENSOR_STATE_OK:
		 {
		    
		 }break;
	 }
	 return mState;
}

u16 jdrk_WindSpeed::Read()
{
	//如果数据不是报警值且数据范围超出了额定
	if( mData > 0x12c && mData!= WINDSPEED_TIMEOUT && mData !=WINDSPEED_CRC_ERROR)
  		mData = WINDSPEED_DATA_UNUSUAL;
  return mData;
}


//重新初始化状态和数据
void jdrk_WindSpeed::init()
{
  mState = SENSOR_STATE_READY;//将状态重新置位准备状态
	mData = WINDSPEED_TIMEOUT; 
}
