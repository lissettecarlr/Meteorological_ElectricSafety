/**
  ******************************************************************************
  * @file    Timer.h 
  * @author  lissettecarlr
  * @version V1.0
             V1.1 增加开关中断功能
			       V1.2 增加清除CNT计数器的值的功能
						 V1.3 增加了最大计数限制
									增加错误标识ErrorMassage
									修复了使用了宏定义却没有引入配置文的BUG
			 
  * @date    10/25/2015(create)  
			       10/28/2015(Debug)
			       11/02/2015(add)
						 08/25/2016(add Dubug) 
  * @brief   必须配套Interrupt文件使用，用户如需需要编写中断函数，直接编写
TIMX	CH1		CH2		CH3		CH4
1		PA8		PA9		PA10 	PA11
2		PA0		PA1		PA2		PA3
3		PA6		PA7		PB0		PB1
4		PB6		PB7		PB8		PB9
  ******************************************************************************
*/

#ifndef __TIMER_H
#define __TIMER_H

/* Includes ------------------------------------------------------------------*/
extern "C"{
#include "stm32f10x.h"
}

//#include "Interrupt.h"


/* define -------------------------------------------------------------------*/
#define INPUTMAX     59702385 //911*65535 最大计数
#define COEFFICIENT  911    //72M下不分频最大计时us      
//  65535/72
/* class----------------------------------------------------------------------*/

class Timer{
	private:
		u16 mArr;//计数器初值
		u16 mPsc;//计数器预分频
		TIM_TypeDef *mTempTimer;//时钟选择
		u8 ErrorMassage;//错误信息标识 0标识没有出错 1：输出时间超出最大值
		bool Conversion(u16 s,u16 ms,u16 us);//将时分秒转换为初值和预分频
		
	public:
		
	////////////////////////////////////////
	///定时器初始化，默认定时器1，定时1ms
	///@param timer 选择使用的定时器
	///@param Prioritygroup 中断分组
	///@param preemprionPriority 抢占优先级
	///@param subPriority 响应优先级
	///@param second 秒
	///@param millisecond 毫秒
	///@param microsecond 微秒
	////////////////////////////////////////
		Timer(TIM_TypeDef *timer=TIM1,u16 second=0,u16 millisecond=1,u16 microsecond=0,u8 Prioritygroup=2,u8 preemprionPriority=2,u8 subPriority=2);
	
	//test
	 Timer(u8 timer);
	
	////////////////////
	///开启定时器
	///////////////////
		void Start();
	
	///////////////////
	///关闭定时器
	//////////////////
		void Stop();
		
	/////////////////////////////////////////
	///中断开关
	///@param bool true 开启中断  false 关闭中断 
	//////////////////////////////////////////
		void OnOrOffIrq(bool Switch);

	///////////////////
	///清空计数器的值
	//////////////////
	  void ClearCNT(void);
		
		//获取计时器值
		u16 GetCNT();
		
	//设置通道使能
	void SetChIrqCmd(u8 ch,bool cmd);
		
  //设置通道比较值
	void SetChCompare(u8 ch,u16 value);
	
	//获取通道比较值
	u16 GetChCompare(u8 ch);
		
		u16 GetArr();
		u16 GetPsc();
	
		 		
};

#endif
