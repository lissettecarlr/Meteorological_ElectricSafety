
#include "yishan_PM2_5.h"


yishan_PM2_5::yishan_PM2_5(FreeUsart &com,USART &ser,uint8_t mode):mCommunications(com),mSerial(ser)
{
	 mData = PM2_5_TIMEOUT;
	 mMode = mode;
	 if(mMode == 0)
	  OrderSensorClose();
	 else
		 OrderSensorStart();
	 OrderTimingOutputClose();
	 mTimeOut = PM2_5_TIMEOUT_DEFAULT;
	 
}

bool yishan_PM2_5::mCheck(u8 *Data,int Lenth,int CheckSum)
{
	int temp=0;
	for(int i=0;i<Lenth;i++)
	{
		temp+=Data[i];
	}
	
	if(temp == CheckSum)
		return true;
	else
		return false;
}

int16_t yishan_PM2_5::Read()
{
   
	return mData;
}

SensorState yishan_PM2_5::Update() //参数为命令类型，以后的接口，这里只是用1代表采集数据读取
{
	 switch (mState)
	 {
	   case SENSOR_STATE_READY://开始传感器采集
		 {
		   mState = SENSOR_STATE_BUSY;
		   mRecord = tskmgr.Time();
		   mCommunications.StartRecive();
       OrderGetData();
			 
		 }break;
		 case SENSOR_STATE_BUSY: //正在进行传感器采集
		 {
		     if(mCommunications.ReciveIsOk())//如果获取到数据
			  {
					
				 u8  Temp[50];//暂存发送回来的数据
				 u16 DataLenth=mSerial.ReceiveBufferSize();;//暂存发送回来的长度
		
				 //长度大于额定值
				 if(DataLenth>50)
				 {
					 mState = SENSOR_STATE_OK;
					 mData = PM2_5_DATA_UNUSUAL;
				   break;
				 }
				 mSerial.GetReceivedData(Temp,DataLenth);
				 
			  //数据处理,先进行CRC校验，在进行数据范围判断，最后存入mDta，如果出错则data = 111. 如果数据超出正常范围 data = 112
				 u16 check =(((u16)Temp[DataLenth-2])<<8) + Temp[DataLenth-1];
				 if(!mCheck(Temp,DataLenth-2,check))
				 {
				    mData = PM2_5_CRC_ERROR; 
					  mState = SENSOR_STATE_OK;
					  if(mMode ==0)
					    OrderSensorClose();
					  break;
				 }
				 //根据发送命令不同接收处理不同
	       mData =((int16_t)Temp[6])*256+Temp[7];
				 mState = SENSOR_STATE_OK;
				 if(mMode ==0)
				  OrderSensorClose();
				 break;
			  }
	      if(tskmgr.TimeSlice(mRecord,mTimeOut))//如果超时
		  	{
				  mData= PM2_5_TIMEOUT;
			    mState = SENSOR_STATE_OK;
					if(mMode ==0)
					 OrderSensorClose();
		  	}
		 }break;
	 
		 case SENSOR_STATE_OK:
		 {
		    
		 }break;
	 }
	 return mState;
}


void yishan_PM2_5::init()//获取前的数据初始化
{
  mData = PM2_5_TIMEOUT;
	mState = SENSOR_STATE_READY;
	if(mMode == 0)
	 OrderSensorStart();
}


void yishan_PM2_5::OrderSensorStart()
{
	 mSerial.SendData(OrderON,16);
}

void yishan_PM2_5::OrderSensorClose()
{
  mSerial.SendData(OrderOFF,16);
}

void yishan_PM2_5::OrderTimingOutputClose()
{
  mSerial.SendData(OrderTimeClose,16);
}

void yishan_PM2_5::OrderGetData()
{
  mSerial.SendData(OrderGet,16);
}
