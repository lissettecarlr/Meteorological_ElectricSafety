#ifndef _MODBUS_RTU_H_
#define _MODBUS_RTU_H_


extern "C"{
#include "stm32f10x.h"

#define MODULE_ADDR_DEFAULT             0x01
#define MOUDLE_FUCTION_READ_DEFAULT     0x03
#define MOUDLE_FUCTION_WRITE_DEFAULT    0x10

	

static u8 module_addr = MODULE_ADDR_DEFAULT;
static u8 module_function_read = MOUDLE_FUCTION_READ_DEFAULT;
static u8 module_function_write = MOUDLE_FUCTION_WRITE_DEFAULT;

typedef struct modbus_pck{
	u8 lenth;
  u8 data[21];
}modbus_pck;

//设置地址码，功能码
void modbus_init(u8 addr,u8 fun_read,u8 fun_write);
//CRC校验
uint16_t  modbus_RTU_crc(uint8_t *modbus_buff,uint8_t size);
//Modbus读，起始地址，寄存器数量
modbus_pck modbus_rtu_read(uint16_t register_start_addr,uint16_t register_number);
//Modbus写，起始地址，寄存器数量，字节数，待写入的数据
modbus_pck modbus_rtu_write(u16 register_start_addr,u16 register_number,u8 data_number,u8 *data);


}

#endif
