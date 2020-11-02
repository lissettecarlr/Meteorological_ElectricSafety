#include "jdrk_windspeed.h"



//modbus crcЧ���㷨
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
	   case SENSOR_STATE_READY://��ʼ�������ɼ�
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
		 case SENSOR_STATE_BUSY: //���ڽ��д������ɼ�
		 {
		     if(mCom.ReciveIsOk())//�����ȡ������
			  {
				 u8  Temp[50];//�ݴ淢�ͻ���������
				 u16 DataLenth=0;//�ݴ淢�ͻ����ĳ���
				 DataLenth = mUsart.ReceiveBufferSize();
				 mUsart.GetReceivedData(Temp,DataLenth);
			  //���ݴ���,�Ƚ���CRCУ�飬�ڽ������ݷ�Χ�жϣ�������mDta�����������data = 111. ������ݳ���������Χ data = 112
				
				  u16 crc = higa_modbus_crc(Temp,DataLenth-2);
				  u16 check = ((u16)Temp[DataLenth-1]<<8) + Temp[DataLenth-2];
					if(crc != check)
					{
						mData = WINDSPEED_CRC_ERROR;  
						mState = SENSOR_STATE_OK;
						break;
					}
				 //���ݷ������ͬ���մ���ͬ
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
	      if(tskmgr.TimeSlice(mRecord,mTimeOut))//�����ʱ
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
	//������ݲ��Ǳ���ֵ�����ݷ�Χ�����˶
	if( mData > 0x12c && mData!= WINDSPEED_TIMEOUT && mData !=WINDSPEED_CRC_ERROR)
  		mData = WINDSPEED_DATA_UNUSUAL;
  return mData;
}


//���³�ʼ��״̬������
void jdrk_WindSpeed::init()
{
  mState = SENSOR_STATE_READY;//��״̬������λ׼��״̬
	mData = WINDSPEED_TIMEOUT; 
}
