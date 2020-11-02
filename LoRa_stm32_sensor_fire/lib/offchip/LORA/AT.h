#ifndef  _AT_H_
#define  _AT_H_


#include "USART.h"
#include "FreeUSART.h"

#define AT_BUFFER_LEN    250

#define AT_SEND_OK_WAIT  1
#define AT_RECIVE_WAIT   5

enum{
	 AT_SEND_FAIL = 0,
   AT_SEND_OK =1,
	 AT_SEND_RECIVE_OK = 2,
};

class AT
{
  private:
		USART &mSerial;
	  FreeUsart &mCommunications;
//	 u8 buffer[256];
//   char heard[20];
//   char tail[20];
//   u8 mHeardLenth;
//   u8 mTailLenth ;
	  bool ReceiveAndWait(const char* targetString,unsigned char timeOut);
	  bool ReceiveAndSave(const char* targetString,unsigned char timeOut);
	  bool ReceiveAndWait(const char* targetString,const char* targetString2,unsigned char timeOut);
    bool ClearBuffer();
	public:
		char mBuffer[AT_BUFFER_LEN];
	  uint8_t mBufferLenth;
	  uint8_t HaveNewRevice;
    AT(USART &Ser,FreeUsart &Com);
	  bool Join();
    u8 SendPck(char *data,u8 port);
	  u8 SendPckHex(u8 *data,u16 lenth,u8 port);
};


#endif
