#include "TN.h"
#include "stdio.h"

//modbus crc效验算法
static uint16_t  TN_modbus_crc(uint8_t *higa_modbus_buff,uint8_t size)
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

TN_water::TN_water(FreeUsart &com,USART &ser):mCommunications(com),mSerial(ser)
{
	 mOriginalData = RAIN_TIMEOUT;
	 mState = SENSOR_STATE_READY;
   mTimeOut=SENSOR_DEFAULT_TIMEOUT;
}

int16_t TN_water::ReadOriginal()
{
   return mOriginalData;
}

float TN_water::Read()
{
  return mData;
}

//重新初始化状态和数据
void TN_water::init()
{
  mState = SENSOR_STATE_READY;//将状态重新置位准备状态
	mData = RAIN_TIMEOUT; //将数据初始化为超时
}

SensorState TN_water::Update()
{
	 switch (mState)
	 {
	   case SENSOR_STATE_READY://开始传感器采集
		 {
		   mState = SENSOR_STATE_BUSY;
		   mRecord = tskmgr.Time();
		   mCommunications.StartRecive();
		   switch(1)
			 {
			   case 1:{  mSerial.SendData(Sensor_TH_Order,8);  }break;
			 }
		 
		 }break;
		 case SENSOR_STATE_BUSY: //正在进行传感器采集
		 {
		     if(mCommunications.ReciveIsOk())//如果获取到数据
			  {
				 u8  Temp[50];//暂存发送回来的数据
				 u16 DataLenth=0;//暂存发送回来的长度
				 DataLenth = mSerial.ReceiveBufferSize();
				 mSerial.GetReceivedData(Temp,DataLenth);
			  //数据处理,先进行CRC校验，在进行数据范围判断，最后存入mDta，如果出错则data = 111. 如果数据超出正常范围 data = 112
				
				 u16 crc = TN_modbus_crc(Temp,DataLenth-2);
				 u16 check = ((u16)Temp[DataLenth-1]<<8) + Temp[DataLenth-2];
				 if(crc != check)
				 {
					mData = RAIN_CRC_ERROR;  
					mState = SENSOR_STATE_OK;
					sprintf(time,"ERROR : %d",(int)mData);
					break;
				 }
				 //根据发送命令不同接收处理不同
				 switch(1)
			   {
					case 1:
					{ 
						//01 03 0c     07 e1    00 06    00 09   00 10    00 11    00 32    97 37
						//mData = ((u16)Temp[3]<<8)+Temp[4];	
            int year   = Temp[3]*256+Temp[4];
						int month  = Temp[5]*256+Temp[6];
						int day    = Temp[7]*256+Temp[8];
						int hours  = Temp[9]*256+Temp[10];
						int minutes= Temp[11]*256+Temp[12];	
						//int len=0; 
						sprintf(time,"%4dY %02dM %02dD %02dH %02dm",year,month,day,hours,minutes);
 						mOriginalData = Temp[13]*256+Temp[14];
						mData = mOriginalData * 0.01; // mg/L
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
					sprintf(time,"ERROR : %d",(int)mData);
		  	}
		 }break;
	   
		 
		 case SENSOR_STATE_OK:
		 {
		    //获取成功，暂时无处理
		 }break;
	 }
	 return mState;
}
