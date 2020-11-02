#include "jdrk_temp_humi.h"

#include "stm32f10x.h"
#include "TaskManager.h"
#include "UserInterrupt.h"
#include <string.h>
//modbus crcЧ���㷨
static uint16_t  higa_modbus_crc(uint8_t *higa_modbus_buff,uint8_t size)
{
	uint16_t crc_register;
	uint8_t index,pos;
	//��һ��Ԥ��һ��0xffff��crc�Ĵ���
	crc_register = 0xffff;
	//ѭ������ÿ������
	for ( index = 0; index < size; index++ ){
		//�����ݷ��еĵ�һ���ֽڵİ�λ��CRC�Ĵ����еĵ��ֽڽ���������㣬������CRC�Ĵ���
		crc_register = crc_register ^ (higa_modbus_buff[0] & 0x00ff);
		higa_modbus_buff++;
		for ( pos = 0; pos < 8; pos++ ){
			//��ȡ��crc�Ĵ��������λ���
			//������λ��1 ��CRC�Ĵ�����һ��Ԥ��Ĺ̶�ֵ��0a001h�������������
			if ( crc_register & 0x0001){
				crc_register = crc_register >> 1;//�Ƚ���������һλ
				crc_register ^= 0xa001;//��0x001�������
			}else{
			    //�������1 ��ֱ���Ƴ�
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
	   case SENSOR_STATE_READY://��ʼ�������ɼ�
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
		 case SENSOR_STATE_BUSY: //���ڽ��д������ɼ�
		 {
		     if(mCom.ReciveIsOk())//�����ȡ������
			  {	
				 int16_t tmp;
				 u8  Temp[50];//�ݴ淢�ͻ���������
				 u16 DataLenth=0;//�ݴ淢�ͻ����ĳ���
				 DataLenth = mUsart.ReceiveBufferSize();
				 mUsart.GetReceivedData(Temp,DataLenth);
			  //���ݴ���,�Ƚ���CRCУ�飬�ڽ������ݷ�Χ�жϣ�������mDta�����������data = 111. ������ݳ���������Χ data = 112
				
				  u16 crc = higa_modbus_crc(Temp,DataLenth-2);
				  u16 check = ((u16)Temp[DataLenth-1]<<8) + Temp[DataLenth-2];
					if(crc != check)
					{
						mData = TEMP_HUMI_CRC_ERROR;  
						mState = SENSOR_STATE_OK;
						break;
					}
				 //���ݷ������ͬ���մ���ͬ
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
	      if(tskmgr.TimeSlice(mRecord,mTimeOut))//�����ʱ
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
	//������ݲ��Ǳ���ֵ�����ݷ�Χ�����˶
	
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


//���³�ʼ��״̬������
void jdrk_TempHumi::init()
{
  mState = SENSOR_STATE_READY;//��״̬������λ׼��״̬
	mData = TEMP_HUMI_TIMEOUT; 
}



