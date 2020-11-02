extern "C"{



#include "Modbus-RTU.h"

void modbus_init(u8 addr,u8 fun_read,u8 fun_write)
{
   module_addr = addr;
   module_function_read = fun_read;
	 module_function_write = fun_write;
}

//modbus crc效验算法
//example:
// u16 crc = Modubus_RTU_crc(Data,DataLenth-2);
// u16 check = ((u16)Data[DataLenth-1]<<8) + Temp[DataLenth-2];
uint16_t  modbus_RTU_crc(uint8_t *modbus_buff,uint8_t size)
{
	uint16_t crc_register;
	uint8_t index,pos;
	//第一步预制一个0xffff的crc寄存器
	crc_register = 0xffff;
	//循环计算每个数据
	for ( index = 0; index < size; index++ ){
		//将数据帆中的第一个字节的八位与CRC寄存器中的低字节进行异或运算，结果存放CRC寄存器
		crc_register = crc_register ^ (modbus_buff[0] & 0x00ff);
		modbus_buff++;
		for ( pos = 0; pos < 8; pos++ ){
			//先取出crc寄存器的最低位检测
			//如果最低位市1 将CRC寄存器与一个预设的固定值（0a001h）进行异或运算
			if ( crc_register & 0x0001){
				crc_register = crc_register >> 1;//先将数据右移一位
				crc_register ^= 0xa001;//与0x001进行异或
			}else{
			    //如果不是1 则直接移出
				crc_register = crc_register >> 1;
			}
		}
	}
	return crc_register;
}


modbus_pck modbus_rtu_read(uint16_t register_start_addr,uint16_t register_number)
{
   modbus_pck pck;
	 uint16_t crc;
	 pck.lenth = 8;
	 pck.data[0] = module_addr;
   pck.data[1] = module_function_read;
	 pck.data[2] = register_start_addr>>8;
	 pck.data[3] = (uint8_t)register_start_addr;
	 pck.data[4] = register_number>>8;
	 pck.data[5] = (uint8_t)register_number;
	 //CRC
	 crc = modbus_RTU_crc(pck.data,pck.lenth-2);
	 pck.data[6] = (u8)crc;
	 pck.data[7] = crc>>8;
	 return pck;
}

//modbus_pck* modbus_rtu_read(uint16_t register_start_addr,uint16_t register_number)
//{
//   modbus_pck *pck;
//	 uint16_t crc;
//	 pck->lenth = 8;
//	 pck->data[0] = module_addr;
//   pck->data[1] = module_function_read;
//	 pck->data[2] = register_start_addr>>8;
//	 pck->data[3] = (uint8_t)register_start_addr;
//	 pck->data[4] = register_number>>8;
//	 pck->data[5] = (uint8_t)register_number;
//	 //CRC
//	 crc = modbus_RTU_crc(pck->data,pck->lenth-2);
//	 pck->data[6] = (u8)crc;
//	 pck->data[7] = crc>>8;
//	 return pck;
//}


//modbus_pck pck = modbus_rtu_write(ARCM_REGISTER_CLEAR_WARING,0x001,2,data);
modbus_pck modbus_rtu_write(u16 register_start_addr,u16 register_number,u8 data_number,u8 *data)
{
  modbus_pck pck;
	uint16_t crc;
	pck.lenth = data_number + 9;
	pck.data[0] = module_addr;
  pck.data[1] = module_function_write;
	pck.data[2] = register_start_addr>>8;
	pck.data[3] = (uint8_t)register_start_addr;
	pck.data[4] = register_number>>8;
	pck.data[5] = (uint8_t)register_number;
	pck.data[6] = data_number;
	for(u8 i=0;i<data_number;i++)
	 pck.data[i+7] = data[i];
	crc = modbus_RTU_crc(pck.data,pck.lenth-2);
	pck.data[pck.lenth-2] = (u8)crc;
	pck.data[pck.lenth-1] = crc>>8;
	return pck;
}


}//extern "C"
