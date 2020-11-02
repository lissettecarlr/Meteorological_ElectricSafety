extern "C"{
	
#include "stm32f10x.h"

#define SLIP_END      0xC0
#define SLIP_ESC      0xDB

#define SLIP_ESC_END  0xDC
#define SLIP_ESC_ESC  0xDD

#define DATA_BUFFER_LENTH 255


typedef struct slip_pck{
  uint8_t data[DATA_BUFFER_LENTH];
	uint8_t len;
}slip_pck,data_pck;


data_pck slip_read(const uint8_t *_ptr, int len);
slip_pck slip_write(const uint8_t *_ptr, int len);

}
