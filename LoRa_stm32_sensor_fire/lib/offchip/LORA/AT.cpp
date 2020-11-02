/*
AT+MODE=OTAA
AT+BAND=CN470
AT+CHMASK=3,65280
其他信道全部改为0
AT+KEY=APPKEY,01020304050607080910111213141516
*/


#include "AT.h"
#include <string.h>
#include "TaskManager.h"
#include "string.h"

AT::AT(USART &Ser,FreeUsart &Com):mSerial(Ser),mCommunications(Com)
{
    
}


bool AT::Join()
{
	 //至少需要10s加入等待
   mSerial<<"AT+JOIN=\r\n";
	 if(ReceiveAndWait("Success","OK",10))
	   return true;
	 else
		 return false;
}


u8 AT::SendPckHex(u8 *data,u16 lenth,u8 port)
{
	u8 buffer[256];
	char heard[11] = "AT+MSGHEX=";
	char tail[2] = {0x0d,0x0a};
  uint16_t cnt=10;
  
  memcpy(buffer,heard,10);
  buffer[cnt++] = port+'0';
	buffer[cnt++] = ',';
	memcpy(buffer+cnt,data,lenth);
	cnt=cnt+lenth;
	memcpy(buffer+cnt,tail,2);
  cnt=cnt+2;
	mSerial.SendData(buffer,cnt); 
	
	if(ReceiveAndWait("OK",AT_SEND_OK_WAIT))
	{
		if(ReceiveAndSave("P:8,",AT_RECIVE_WAIT))//接收下行数据
		{
		   return AT_SEND_RECIVE_OK;
		}
	  return AT_SEND_OK;  
	}
	else 
		return AT_SEND_FAIL;//发送失败
	
}

u8 AT::SendPck(char *data,u8 port)
{
	char buffer[256];
	char heard[8] = "AT+MSG=";
	char tail[2] = {0x0d,0x0a};
  u8 lenth = strlen(data);
	
  memcpy(buffer,heard,7);
  buffer[7] = port+'0';
	buffer[8] = ',';
	memcpy(buffer+8+1,data,lenth);
	memcpy(buffer+8+1+lenth,tail,2); 
	mSerial.SendData((u8 *)buffer,8+1+lenth+2); 
	
	if(ReceiveAndWait("OK",AT_SEND_OK_WAIT))
	{
		if(ReceiveAndSave("P:8,",AT_RECIVE_WAIT))//接收下行数据
		{
		   return AT_SEND_RECIVE_OK;
		}
	  return AT_SEND_OK;  
	}
	else 
		return AT_SEND_FAIL;//发送失败
	
}


bool AT::ReceiveAndWait(const char* targetString,unsigned char timeOut)
{
		mCommunications.StartRecive();
		u8 len;
		double tartTime=TaskManager::Time();
	  u8 data[100];	
		while((TaskManager::Time()-tartTime)<timeOut)
		{
			if(mCommunications.ReciveIsOk())
			{
				 len = mSerial.ReceiveBufferSize();
				 mSerial.GetReceivedData(data,len);
				 data[len] = '\0';
				if(strstr((char *)data,targetString))
				   return true;
				else
					mCommunications.StartRecive();
			}		
		}
		return false;
}

bool AT::ReceiveAndWait(const char* targetString,const char* targetString2,unsigned char timeOut)
{
		mCommunications.StartRecive();
		u8 len;
		double tartTime=TaskManager::Time();
	  u8 data[100];	
		while((TaskManager::Time()-tartTime)<timeOut)
		{
			if(mCommunications.ReciveIsOk())
			{
				 len = mSerial.ReceiveBufferSize();
				 mSerial.GetReceivedData(data,len);
				 data[len] = '\0';
				if(strstr((char *)data,targetString))
				   return true;
				else if(strstr((char *)data,targetString2))
					 return true;
				else
					mCommunications.StartRecive();
			}		
		}
		return false;
}



bool AT::ReceiveAndSave(const char* targetString,unsigned char timeOut)
{
	  mCommunications.StartRecive();
  	u8 len;
	  char* str;
		double tartTime=TaskManager::Time();
	  u8 data[100];	
		while((TaskManager::Time()-tartTime)<timeOut)
		{
			if(mCommunications.ReciveIsOk())
			{
				 ClearBuffer();
				 len = mSerial.ReceiveBufferSize();
				 mSerial.GetReceivedData(data,len);
				 data[len] = '\0';
				 str=strstr((char *)data,targetString);
				 if(str != NULL)
				 {
					 	str+=strlen(targetString);
					  mBufferLenth = strlen(str)-2;//这里的-2是为了去掉末尾的\r\n
					  memcpy(mBuffer,str,mBufferLenth);
					 //int a=strlen(str);
					 //mSerial<<str<<";"<<a;
					 HaveNewRevice=1;
					 return true;
				 }		  
				 else
					 mCommunications.StartRecive();
			}		
		}
		return false;
}

bool AT::ClearBuffer()
{
   for(u8 i=0;i<AT_BUFFER_LEN;i++)
	   mBuffer[i] = 0;
	 return true;
}
