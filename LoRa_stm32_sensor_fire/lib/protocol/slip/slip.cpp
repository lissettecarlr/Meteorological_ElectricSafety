extern "C"{

#include "slip.h"

/*输入数据包，输出slip包*/
slip_pck slip_write(const uint8_t *_ptr, int len)
{
  const uint8_t *ptr = _ptr;
  uint8_t p_slip_pck=0;//slip包指针偏移
	uint8_t p_pck=0;//传入数据包指针偏移
  uint8_t c;
  slip_pck return_pck;
	
	//防止数值越界
	if(len >DATA_BUFFER_LENTH)
	{
	  return_pck.len=0;
		return return_pck;
	}	
	return_pck.data[p_slip_pck++] = SLIP_END;
  for(p_pck = 0; p_pck < len; ++p_pck) {
    c  = *ptr++;
    if(c == SLIP_END) 
		{
			return_pck.data[p_slip_pck++] = SLIP_ESC;
      c = SLIP_ESC_END;
    }
		else if(c == SLIP_ESC)
		{
			return_pck.data[p_slip_pck++] = SLIP_ESC;
      c = SLIP_ESC_ESC;
    }
		return_pck.data[p_slip_pck++] = c;
  }
	return_pck.data[p_slip_pck++] = SLIP_END;
	return_pck.len = p_slip_pck;
	
  return return_pck;
}

//输入slip包，输出数据包
data_pck slip_read(const uint8_t *_ptr, int len)
{
  const uint8_t *ptr = _ptr;
  int p_slip_pck=0;//slip包指针偏移
	int p_pck=0;//传入数据包指针偏移
  uint8_t head;
	uint8_t c;
  slip_pck return_pck;
	
	head  = *ptr;
	c= *(ptr + len-1);
	ptr++;
	if( (len <= 2) || (head !=SLIP_END) || (c != SLIP_END) ) 
	{
	  return_pck.len=0;
		return return_pck;
	}
	else
	{
		 for(p_slip_pck = 1; p_slip_pck < (len-1); ++p_slip_pck)
		{
			 static uint8_t flag=0;
	     c  = *ptr++;
			 switch(c)
			 {
			   case SLIP_ESC:
					 flag=1;
				   break;
				 
				 case SLIP_ESC_END:
					 if(flag == 0)
						 return_pck.data[p_pck++] = c;
					 else
						 return_pck.data[p_pck++] = SLIP_END;
					 flag=0;
					 break;
					 
				 case SLIP_ESC_ESC:
					 if(flag == 0)
						 return_pck.data[p_pck++] = c;
					 else
						 return_pck.data[p_pck++] = SLIP_ESC;
					 flag=0;
				   break;
					 
				 default:
					  flag=0;
					  return_pck.data[p_pck++] = c;
			 }
		}
		return_pck.len = p_pck;
		return return_pck;
	}
	
}

}
