/**
  ******************************************************************************

  ******************************************************************************
*/

#ifndef __FREEUSART_H
#define __FREEUSART_H

/* Includes ------------------------------------------------------------------*/
extern "C"{
#include "stm32f10x.h"

}
#include "USART.h"
#include "Timer.h"
#include "FIFOBuffer.h"

class FreeUsart{
	private:

    FIFOBuffer<u8,10>  DataBuffer; //每帧数据存放区域  
	public:
		
	  USART &mCOM;
	  Timer &mTimer;
		u8 Data[50];
		u16 DataLenth;
		bool DataStartReciveFlag;//标志数据开始接收
	  bool DatareciveFlag;//标志数据正在接收
	  void StartRecive();
	  bool ReciveIsOk();
	
		FreeUsart(USART &USART,Timer &TIM);
		bool GetData(u8* buffer,u16 number);
	  u16 BufferSize();
	  void ClearBuffer();
	  void SetReciveStart();
	  void SetReciveIng();
	  void SetReciveEnd();
	  void UsartIrqDeal();
	  USART GetUsart();
	  void Send(uint8_t *pbuffer, uint32_t size);
	  void Enable();
	  
};

#endif
