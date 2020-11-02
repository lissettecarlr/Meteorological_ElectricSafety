#include "jdrk_temp_humi.h"

#include "stm32f10x.h"
#include "TaskManager.h"
#include "UserInterrupt.h"
#include <string.h>
//modbus crc效验算法
static uint16_t  higa_modbus_crc(uint8_t *higa_modbus_buff,uint8_t size)
{
	uint16_t crc_register;
	uint8_t index,pos;
	//第一步预制一个0xffff的crc寄存器
	crc_register = 0xffff;
	//循环计算每个数据
	for ( index = 0; index < size; index++ ){
		//将数据帆中的第一个字节的八位与CRC寄存器中的低字节进行异或运算，结果存放CRC寄存器
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


 jdrk_TempHumi::jdrk_TempHumi(FreeUsart &Com,USART &Usart):mCom(Com),mUsart(Usart)
{
	 mData = TEMP_HUMI_TIMEOUT;
	 mState = SENSOR_STATE_READY;
   mTimeOut=TEMP_HUMI_TIMEOUT_DEFAULT;
}

SensorState  jdrk_TempHumi::Update(u8 order)
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
									mUsart.SendData(Sensor_TempHumi_Order,8);  
								}break;
			 }
		 
		 }break;
		 case SENSOR_STATE_BUSY: //正在进行传感器采集
		 {
		     if(mCom.ReciveIsOk())//如果获取到数据
			  {	
				 int16_t tmp;
				 u8  Temp[50];//暂存发送回来的数据
				 u16 DataLenth=0;//暂存发送回来的长度
				 DataLenth = mUsart.ReceiveBufferSize();
				 mUsart.GetReceivedData(Temp,DataLenth);
			  //数据处理,先进行CRC校验，在进行数据范围判断，最后存入mDta，如果出错则data = 111. 如果数据超出正常范围 data = 112
				
				  u16 crc = higa_modbus_crc(Temp,DataLenth-2);
				  u16 check = ((u16)Temp[DataLenth-1]<<8) + Temp[DataLenth-2];
					if(crc != check)
					{
						mData = TEMP_HUMI_CRC_ERROR;  
						mState = SENSOR_STATE_OK;
						break;
					}
				 //根据发送命令不同接收处理不同
				 switch(order)
			   {
					case 1:
					{ 
						
							//01 03 00 00 00 02 C4 0B
							//01 03 04 00 B8 03 3F 3A F6
					//	mData = ((u32)Temp[3]<<24) + ((u32)Temp[4]<<16) + ((u32)Temp[5]<<8) + (u32)Temp[6];	
							tmp = ((int16_t)Temp[3]<<8 )+ Temp[4];
							mData = (tmp / 10) << 8;
							mData += tmp % 10;
							tmp = ((int16_t)Temp[5]<<8 )+ Temp[6];
							mData  = (mData << 16);
							mData+=(tmp /10) << 8;
							mData += tmp % 10;
							
					}break;
			   }
				//end
				 mState = SENSOR_STATE_OK;
				 break;
			  }
	      if(tskmgr.TimeSlice(mRecord,mTimeOut))//如果超时
		  	{
				  mData   = TEMP_HUMI_TIMEOUT;
			    mState  = SENSOR_STATE_OK;
		  	}
		 }break;
	 
		 case SENSOR_STATE_OK:
		 {
		    
		 }break;
	 }
	 return mState;
}

int32_t jdrk_TempHumi::Read()
{
	if(mData == TEMP_HUMI_TIMEOUT)
		return mData;
	if(mData == TEMP_HUMI_CRC_ERROR)
		return mData;
	
	int32_t tmp;
	tmp = (mData&0xffff0000) >> 24;
	//如果数据不是报警值且数据范围超出了额定
	
	if( ((tmp <(-40)) || (tmp > 80)))
	{
  	mData = (TEMP_HUMI_DATA_UNUSUAL);		
	}
  tmp =	(mData % 0x10000)>>8;
	
	if( ((tmp < 0 ) || (tmp > 100)) )
	{
		mData = TEMP_HUMI_DATA_UNUSUAL;	
	}
  return mData;
}


//重新初始化状态和数据
void jdrk_TempHumi::init()
{
  mState = SENSOR_STATE_READY;//将状态重新置位准备状态
	mData = TEMP_HUMI_TIMEOUT; 
}



