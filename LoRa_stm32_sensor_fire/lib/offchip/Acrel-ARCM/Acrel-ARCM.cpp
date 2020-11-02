#include "Acrel-ARCM.h"
#include "Main.h"
//#include "USART.h"

//USART xx(1,9600,false);

ARCM::ARCM(FreeUsart &Com,uint16_t GetStatePeriod,uint16_t GetMeasurePeriod):mCom(Com)
{
	 for(int i=0;i<4;i++)
	  mData[i] = ARCM_TIMEOUT;
	
	 mState   = ARCM_STATE_READY;
   mTimeOut = TIMEOUT_DEFAULT;
	 mUseNmuber = USE_CHANNE_DEFAULE;
	 mWarningState = 0;
	
	 mGetStatePeriod = GetStatePeriod;
	 mGetMeasurePeriod = GetMeasurePeriod;
}

//order: 发送指令分类
//BUG 如果直接使用mCom.SendData来输出只能输出一个字符，暂时解决办法是先将一个串口传入来进行输出。
uint8_t ARCM::Update(u16 order,USART &com)
{
	 switch (mState)
	 {
	   case ARCM_STATE_READY://开始传感器采集
		 {
		   mState = ARCM_STATE_BUSY;
		   mRecord = tskmgr.Time();
		   mCom.StartRecive();
		   switch(order)
			 {
			   case ARCM_REGISTER_WARNING_STATE :
         {  
					 modbus_pck pck = modbus_rtu_read(order,1);
					 com.SendData(pck.data,pck.lenth);
				 }break;
				 
				 case ARCM_REGISTER_MEASURE_VALUE:
         { 
					   modbus_pck pck = modbus_rtu_read(order,mUseNmuber);
					   com.SendData(pck.data,pck.lenth);
				 }break;
			 }
		 }break;
		 case ARCM_STATE_BUSY: //正在进行传感器采集
		 {
		    if(mCom.ReciveIsOk())//如果获取到数据
			  {
				  u8  Temp[50];//暂存发送回来的数据
				  u16 DataLenth=0;//暂存发送回来的长度
				  //DataLenth = mCom.GetUsart().ReceiveBufferSize();
				  //mCom.GetUsart().GetReceivedData(Temp,DataLenth);
					DataLenth = com.ReceiveBufferSize();
					com.GetReceivedData(Temp,DataLenth);
			    //数据处理,先进行CRC校验，在进行数据范围判断，最后存入mDta
				  u16 crc = modbus_RTU_crc(Temp,DataLenth-2);
				  u16 check = ((u16)Temp[DataLenth-1]<<8) + Temp[DataLenth-2];
				
				 //根据发送命令不同接收处理不同
				 switch(order)
			   {
					case ARCM_REGISTER_WARNING_STATE://读取报警值
					{  
						if(crc != check)
						{
						  mWarningState = ARCM_CRC_ERROR;
							break;
						}
						//01 03 02 00 00 B8 44
						mWarningState = ((u16)Temp[3]<<8)+Temp[4];							
					}break;
					case ARCM_REGISTER_MEASURE_VALUE://读取测量值
					{
						if(crc != check)
						{
						  for(int i=0;i<4;i++)
	             mData[i] = ARCM_CRC_ERROR;
							break;
						}
						//01 03 08 00 2D F8 30 F8 30 F8 30 4F D2
						 u8 flag=3;
					   for(u8 i=0;i<mUseNmuber;i++)
						 {
               mData[i] = ((u16)Temp[flag]<<8)+Temp[flag+1];
							 if(mData[i] == 0xf830)
								 mData[i] = ARCM_SENSOR_LOSE;
               flag+=2;							 
						 }
						 
					}break;
			   }
				//end
				 mState = ARCM_STATE_OK;
				 break;
			  }//if get data end
	      if(tskmgr.TimeSlice(mRecord,3))//如果超时,使用宏定义时发送数据异常
		  	{
				 for(int i=0;i<4;i++)
	        mData[i] = ARCM_TIMEOUT;
				 mWarningState = ARCM_TIMEOUT;
			   mState = ARCM_STATE_OK;
		  	}
		 }break;
	 
		 case ARCM_STATE_OK:
		 {
		    
		 }break;
	 }
	 return mState;
}

u16 ARCM::ReadWarningState()
{
  return mWarningState;
}

u16 ARCM::Read(u8 number)
{
	if(number <= 8 && number >0)
    return mData[number-1];
	else
		return 0;
}

void ARCM::SetWarningTempThreshold(uint16_t current,uint16_t temp1,uint16_t temp2,uint16_t temp3)
{
	  uint8_t data[] = { (uint8_t)(current>>8),(uint8_t)current, \
		                   (uint8_t)(temp1>>8),(uint8_t)temp1,
			                 (uint8_t)(temp2>>8),(uint8_t)temp2,
			                 (uint8_t)(temp3>>8),(uint8_t)temp3};
    modbus_pck pck = modbus_rtu_write(ARCM_REGISTER_SET_WARNING_THREASHOLD,4,8,data);
		mCom.Enable();
		mCom.Send(pck.data,pck.lenth);
}

//01 10 10 63 00 01 02 12 34 B3 75
void ARCM::SendClearwarning()
{
	 u8 data[2]={0x12,0x34};
   modbus_pck pck = modbus_rtu_write(ARCM_REGISTER_CLEAR_WARNING,0x001,2,data);
	 //mCom.GetUsart().SendData(pck.data,pck.lenth);
	 mCom.Enable();
	 mCom.Send(pck.data,pck.lenth);	
}




//重新初始化状态和数据，新一轮数据请求前调用
void ARCM::init()
{
  mState = ARCM_STATE_READY;//将状态重新置位准备状态
	for(int i=0;i<4;i++)
	  mData[i] = ARCM_TIMEOUT;
	mUseNmuber = USE_CHANNE_DEFAULE;
}

bool ARCM::DeelRcvOrder(uint8_t *data,uint8_t lenth)
{
   if(lenth <=3)
		 return false;
	 uint16_t crc = modbus_RTU_crc(data,lenth-2);
	 //u16 check = ((u16)Temp[DataLenth-1]<<8) + Temp[DataLenth-2];
	 uint16_t check =((uint16_t) (data[lenth-1]<<8) + data[lenth-2]);
	 if(check == crc)
	 {
	     switch (data[0])
			 {
			  case 1:{//清除报警6
				  SendClearwarning();
				}break;
			  case 2:{//设置报警值
					uint16_t current = (((u16)data[1])<<8)+ data[2];
					uint16_t temp1   = (((u16)data[3])<<8) + data[4];
					uint16_t temp2   = (((u16)data[5])<<8) + data[6];
					uint16_t temp3   = (((u16)data[7])<<8) + data[8];
					SetWarningTempThreshold(current,temp1,temp2,temp3);
				}break; 
			  case 3:{  //设置报警采集周期
				  uint16_t period1 = data[1] * 60 * 60 + data[2] * 60 +data[3];
          mGetStatePeriod = period1;			

				}break;
			  case 4:{//设置测量值采集周期
				  uint16_t period2 = data[1] * 60 * 60 + data[2] * 60 +data[3];
          mGetMeasurePeriod = period2;
				}break;
				case 5:{
						m100d_rst();
						*((u32 *)0xE000ED0C) = 0x05fa0004;

				//复位唯传模组和主控
				}break;
				case 6:{
					m100d_rst();
				}break;
			 }
			 return true;
	 }
   else
	 {
	   return false;
	 }		 
}
