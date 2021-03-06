#include "jdrk_light.h"



static uint16_t  light_modbus_crc(uint8_t *higa_modbus_buff,uint8_t size)
{
	uint16_t crc_register;
	uint8_t index,pos;
	crc_register = 0xffff;
	for ( index = 0; index < size; index++ ){
		crc_register = crc_register ^ (higa_modbus_buff[0] & 0x00ff);
		higa_modbus_buff++;
		for ( pos = 0; pos < 8; pos++ ){
			//先取出crc寄存器的最低位检测
			//如果最低位市1 将CRC寄存器与一个预设的固定值（0a001h）进行异或运算
			if ( crc_register & 0x0001){
				crc_register = crc_register >> 1;//先将数据右移一位
				crc_register ^= 0xa001;//与0x001进行异或
			}else{
			    //如果不是1 则直接移出
				crc_register = crc_register >> 1;
			}
		}
	}
	return crc_register;
}


 jdrk_Light::jdrk_Light(FreeUsart &Com,USART &Usart):mCom(Com),mUsart(Usart)
{
	 mData = LIGHT_TIMEOUT;
	 mState = SENSOR_STATE_READY;
   mTimeOut=LIGHT_TIMEOUT_DEFAULT;
}

SensorState jdrk_Light::Update(u8 order)
{
	 switch (mState)
	 {
	   case SENSOR_STATE_READY://�
		 {
		   mState = SENSOR_STATE_BUSY;
		   mRecord = tskmgr.Time();
		   mCom.StartRecive();
		   switch(order)
			 {
			   case 1:{  
									mUsart.SendData(Sensor_Light_Order,8);  
								}break;
			 }
		 
		 }break;
		 case SENSOR_STATE_BUSY:
		 {
		     if(mCom.ReciveIsOk())//如果获取到数据
			  {
				 u8  Temp[50];//暂存发送回来的数据
				 u16 DataLenth=0;//暂存发送回来的长度
				 DataLenth = mUsart.ReceiveBufferSize();
				 mUsart.GetReceivedData(Temp,DataLenth);	
				  u16 crc = light_modbus_crc(Temp,DataLenth-2);
				  u16 check = ((u16)Temp[DataLenth-1]<<8) + Temp[DataLenth-2];
					if(crc != check)
					{
						mData = LIGHT_CRC_ERROR;  
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
				  mData= LIGHT_TIMEOUT;
			    mState = SENSOR_STATE_OK;
		  	}
		 }break;
	 
		 case SENSOR_STATE_OK:
		 {
		    
		 }break;
	 }
	 return mState;
}

u16 jdrk_Light::Read()
{
	//如果数据不是报警值且数据范围超出了额定
	if( mData >0xFFFF && mData!= LIGHT_TIMEOUT && mData !=LIGHT_CRC_ERROR)
  		mData = LIGHT_DATA_UNUSUAL;
  return mData;
}


//重新初始化状态和数据
void jdrk_Light::init()
{
  mState = SENSOR_STATE_READY;//将状态重新置位准备状态
	mData = LIGHT_TIMEOUT; 
}
