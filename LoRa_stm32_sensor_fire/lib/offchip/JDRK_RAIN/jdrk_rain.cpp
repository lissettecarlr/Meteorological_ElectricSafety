#include "jdrk_rain.h"


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


jdrk_Rain::jdrk_Rain(FreeUsart &Com,USART &Usart):mCom(Com),mUsart(Usart)
{
	 mData = RAIN_TIMEOUT;
	 mState = SENSOR_STATE_READY;
   mTimeOut=RAIN_TIMEOUT_DEFAULT;
}

//order: 发送指令分类
SensorState jdrk_Rain::Update(u8 order)
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
			   case 1:{  mUsart.SendData(Sensor_Rain_Order,8);  }break;
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
						mData = RAIN_CRC_ERROR;  
						mState = SENSOR_STATE_OK;
						break;
					}
				 //根据发送命令不同接收处理不同
				 switch(order)
			   {
					case 1:
					{ 
							//02 03 02 00 00 FC 44
					//	Rain_YN = Temp[4];
						mData = ((u16)Temp[3]<<8)+Temp[4];						
					}break;
			   }
				//end
				 mState = SENSOR_STATE_OK;
				 break;
			  }
	      if(tskmgr.TimeSlice(mRecord,mTimeOut))//如果超时
		  	{
				  mData= RAIN_TIMEOUT;
			    mState = SENSOR_STATE_OK;
		  	}
		 }break;
	 
		 case SENSOR_STATE_OK:
		 {
		    
		 }break;
	 }
	 return mState;
}


u16 jdrk_Rain::Read()
{
	//如果数据超出了额定值
	if( mData !=1 && mData !=0 && mData!= RAIN_TIMEOUT && mData !=RAIN_CRC_ERROR)
  		mData = RAIN_DATA_UNUSUAL;
  return mData;
}


//重新初始化状态和数据
void jdrk_Rain::init()
{
  mState = SENSOR_STATE_READY;//将状态重新置位准备状态
	mData = RAIN_TIMEOUT; //将数据初始化为超时
}
