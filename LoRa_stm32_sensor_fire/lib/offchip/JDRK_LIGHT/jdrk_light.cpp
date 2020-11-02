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
			//ÏÈÈ¡³öcrc¼Ä´æÆ÷µÄ×îµÍÎ»¼ì²â
			//Èç¹û×îµÍÎ»ÊÐ1 ½«CRC¼Ä´æÆ÷ÓëÒ»¸öÔ¤ÉèµÄ¹Ì¶¨Öµ£¨0a001h£©½øÐÐÒì»òÔËËã
			if ( crc_register & 0x0001){
				crc_register = crc_register >> 1;//ÏÈ½«Êý¾ÝÓÒÒÆÒ»Î»
				crc_register ^= 0xa001;//Óë0x001½øÐÐÒì»ò
			}else{
			    //Èç¹û²»ÊÇ1 ÔòÖ±½ÓÒÆ³ö
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
	   case SENSOR_STATE_READY://¿
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
		     if(mCom.ReciveIsOk())//Èç¹û»ñÈ¡µ½Êý¾Ý
			  {
				 u8  Temp[50];//ÔÝ´æ·¢ËÍ»ØÀ´µÄÊý¾Ý
				 u16 DataLenth=0;//ÔÝ´æ·¢ËÍ»ØÀ´µÄ³¤¶È
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
				 //¸ù¾Ý·¢ËÍÃüÁî²»Í¬½ÓÊÕ´¦Àí²»Í¬
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
	      if(tskmgr.TimeSlice(mRecord,mTimeOut))//Èç¹û³¬Ê±
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
	//Èç¹ûÊý¾Ý²»ÊÇ±¨¾¯ÖµÇÒÊý¾Ý·¶Î§³¬³öÁË¶î¶¨
	if( mData >0xFFFF && mData!= LIGHT_TIMEOUT && mData !=LIGHT_CRC_ERROR)
  		mData = LIGHT_DATA_UNUSUAL;
  return mData;
}


//ÖØÐÂ³õÊ¼»¯×´Ì¬ºÍÊý¾Ý
void jdrk_Light::init()
{
  mState = SENSOR_STATE_READY;//½«×´Ì¬ÖØÐÂÖÃÎ»×¼±¸×´Ì¬
	mData = LIGHT_TIMEOUT; 
}
