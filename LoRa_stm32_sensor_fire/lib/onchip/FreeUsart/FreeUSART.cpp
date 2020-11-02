#include "FreeUSART.h"


bool FreeUsart::GetData(u8* buffer,u16 number)
{
	if(DataBuffer.Size()<number)//没有足够长的数据
		return false;
	else
	{
		DataBuffer.Gets(buffer,number);//数据出队
		return true;
	}
}

	
u16 FreeUsart::BufferSize()
{
	return DataBuffer.Size();
}

void FreeUsart::ClearBuffer()
{
	DataBuffer.Clear();
}


FreeUsart::FreeUsart(USART &USART,Timer &TIM):mCOM(USART),mTimer(TIM)
{
	
	DataStartReciveFlag=0;
	DatareciveFlag=0;
	
}

void FreeUsart::StartRecive()
{
  DataStartReciveFlag=0;
	DatareciveFlag=0;
	mCOM.cmd(1);
}

bool FreeUsart::ReciveIsOk()
{
	 if(DataStartReciveFlag == 1 && DatareciveFlag==0)
     return true;
	 else
		 return false;
}

void FreeUsart::SetReciveStart()
{
   	mCOM.ClearReceiveBuffer();
		DataStartReciveFlag = 0;
		DatareciveFlag = 1;
		   
	  mTimer.Start();
	  mTimer.SetChIrqCmd(1,true);
	  mTimer.SetChCompare(1,mTimer.GetCNT()+900);
		//TIM_Cmd(mTimer,ENABLE);
		//TIM_ITConfig(mTimer,TIM_IT_CC1,ENABLE);
		//TIM_SetCompare1(mTimer, TIM_GetCounter(TIM2)+ 200);
}

void FreeUsart::SetReciveIng()
{
  DataStartReciveFlag = 0;
	DatareciveFlag = 1;
	mTimer.SetChCompare(1,mTimer.GetCNT()+900);
}


void FreeUsart::SetReciveEnd()
{
	  mTimer.Stop();
		mTimer.SetChIrqCmd(1,false);
		mCOM.cmd(0);
		//更改为不需要获取数据
		DataStartReciveFlag = 1;
		DatareciveFlag = 0;	
}

void FreeUsart::UsartIrqDeal()
{
  	if(!DataStartReciveFlag && !DatareciveFlag) //需要获取数据且还未开始接收
	 {
     SetReciveStart();
		 //开启定时器
		 //TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);//该句用于第一次获取时，如果不写则第一次会立即超时
	 }
	 else if(!DataStartReciveFlag && DatareciveFlag) //需要获取数据且正在接收
	 {
		 SetReciveIng();
	 }
	 else 
	 {
	   //关闭定时器，关闭串口
		 //TIM_Cmd(TIM2,DISABLE);
		// TIM_ITConfig(TIM2,TIM_IT_CC1,DISABLE);
		 //com.cmd(0);
	 }
}

USART FreeUsart::GetUsart()
{
  return mCOM;
}

void FreeUsart::Send(uint8_t *pbuffer, uint32_t size)
{
   mCOM.SendData(pbuffer,size);
}

void FreeUsart::Enable()
{
  mCOM.cmd(1);
}
///9600    0.11ms
//115200  0.008ms
//0.0027

//0.014ms * 20 = 0.28ms
//9.175
