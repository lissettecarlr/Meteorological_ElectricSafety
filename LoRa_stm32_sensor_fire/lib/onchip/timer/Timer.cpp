#include "Timer.h"

Timer::Timer(TIM_TypeDef *timer,u16 second,u16 millisecond,u16 microsecond,u8 Prioritygroup,u8 preemprionPriority,u8 subPriority)
{
	#ifdef USE_TIMER
	
	//通过计算的出了ARR PSC
	uint8_t timerIrqChannel;
	TIM_TimeBaseInitTypeDef TIM_BaseInitStructure;
	mTempTimer=timer;
	NVIC_InitTypeDef NVIC_InitStructure;
	
  if(!Conversion(second,millisecond,microsecond))
	{
		ErrorMassage = 1;//输入超出最大值
		return ;
	}
	#ifndef USE_TIMER1
	#ifndef USE_TIMER2
	#ifndef USE_TIMER3
	#ifndef USE_TIMER4
		return ;
	#endif
	#endif
	#endif
	#endif
	if(timer==TIM1)
	{
		#ifdef USE_TIMER1
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
		timerIrqChannel=TIM1_UP_IRQn;
		#endif
		
	}
	else if(timer==TIM2)
	{
		#ifdef USE_TIMER2
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		timerIrqChannel=TIM2_IRQn;
		#endif
	}
	else if(timer==TIM3)
	{
		#ifdef USE_TIMER3
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		timerIrqChannel=TIM3_IRQn;
		#endif
	}
	else if(timer==TIM4)
	{
		#ifdef USE_TIMER4
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
		timerIrqChannel=TIM4_IRQn;
		#endif
	}
	else
	{
	
	}
		//TIM_InternalClockConfig(timer);
//	TIM_DeInit(timer);//将寄存器重设为缺省值
		TIM_BaseInitStructure.TIM_Period = mArr-1; //设置初值
		TIM_BaseInitStructure.TIM_Prescaler =mPsc-1;//设置预分频
		TIM_BaseInitStructure.TIM_ClockDivision = 0;//设置时钟分割
	  TIM_BaseInitStructure.TIM_RepetitionCounter=0;//重复溢出多少次产生一个中断
		TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//设置计数方式
		TIM_TimeBaseInit(timer,&TIM_BaseInitStructure);
		TIM_ClearFlag(timer, TIM_FLAG_Update);//清空中断标识
		TIM_ITConfig(timer, TIM_IT_Update, ENABLE); //使能中断
	
	switch(Prioritygroup)//中断分组
	{
		case 0:
			NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
			break;
		case 1:
			NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
			break;
		case 2:
			NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
			break;
		default:
			NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);
			break;
		case 4:
			NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
			break;
	}
		NVIC_InitStructure.NVIC_IRQChannel =timerIrqChannel;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = preemprionPriority;  //先占优先
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = subPriority;  //从优先
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
		NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
	
	//TIM_Cmd(mTempTimer, ENABLE);//开启定时器
	/**/
	
	ErrorMassage = 0;//没有出错
	
	#endif
}

void Timer::Start()
{
	TIM_Cmd(mTempTimer, ENABLE);//开启定时器
}

void Timer::Stop()
{
	TIM_Cmd(mTempTimer,DISABLE);//关闭定时器
}



/*
算法思路：
	COEFFICIENT表示1分屏时最大的时间
首先判断是否可以用一分频解决，如果不能，则计算出相近的预分频
然后判断是是否能用此预分频得到一个整数初值，如果不能则预分频加一在继续查找
如果始终未查找到能够整除的预分频和初值，则使用一开始的初始预分频，然后计算出
一个相近的初值来代替。
*/	
bool Timer::Conversion(u16 s,u16 ms,u16 us) //将时分秒转化为预分频和初值
{
	u32 time;
	u16 tempPsc;//用于暂存计算值
	u32 tempArr=0xfffff;
	
	time=s*1000000+ms*1000+us; //计算总时间 单位us
	
	if(time >INPUTMAX)
		return false; //超出最大计数范围
	
	if(time<COEFFICIENT) //如果一分频可以满足
		mPsc=1;
	else
		mPsc=time/COEFFICIENT; //计算出最相近的预分频
	
	tempPsc=mPsc;	//保存初次计算结果
	
//	当计算出的ARR没有小数且ARR小于0xffff时退出循环，如果mps大于了65535 也退出
	while( ((time*72)% mPsc!=0  ||  tempArr>0xffff ) && mPsc<=65535)//如果计算的初值是个整数，或者没有找到可以计算出整数的分频数
	{
		mPsc++;	
		tempArr=(time*72)/mPsc; //计算出初值
	}
	
	if(mPsc>=65535) //如果找到能够整除的分频值，则选用精度最大的分频值
	{
		mPsc=tempPsc;
		tempArr=(time*72)/mPsc; //计算出初值
	}
	else
		mArr=tempArr;
	
	return true;
	
}

void Timer::OnOrOffIrq(bool Switch)
{
   if (Switch==true)
	   mTempTimer->DIER|=1<<0; //使能更新中断
   else 
	   mTempTimer->DIER&=0<<0;  //关闭更新中断
}

void Timer::ClearCNT(void)
{
   mTempTimer->CNT &=0;
}

u16 Timer::GetArr()
{
   return mArr;
}

u16 Timer::GetPsc()
{
  return mPsc;
}

void Timer::SetChIrqCmd(u8 ch,bool cmd)
{
   if(ch ==1)
	 {
	    if(cmd)
			  TIM_ITConfig(mTempTimer,TIM_IT_CC1,ENABLE);
			else
				TIM_ITConfig(mTempTimer,TIM_IT_CC1,DISABLE);
	 }
	 else if(ch ==2 )
	 {
	   	if(cmd)
			  TIM_ITConfig(mTempTimer,TIM_IT_CC2,ENABLE);
			else
				TIM_ITConfig(mTempTimer,TIM_IT_CC2,DISABLE);
	 }
	 else if(ch ==3 )
	 {
	   	if(cmd)
			  TIM_ITConfig(mTempTimer,TIM_IT_CC2,ENABLE);
			else
				TIM_ITConfig(mTempTimer,TIM_IT_CC2,DISABLE);
	 }
	 else if(ch ==4 )
	 {
	   	if(cmd)
			  TIM_ITConfig(mTempTimer,TIM_IT_CC2,ENABLE);
			else
				TIM_ITConfig(mTempTimer,TIM_IT_CC2,DISABLE);
	 }
}

//设置通道比较值
void Timer::SetChCompare(u8 ch,u16 value)
{
  if(ch ==1 )
	   TIM_SetCompare1(mTempTimer, value);
	else if(ch ==2)
		 TIM_SetCompare2(mTempTimer, value);
	else if(ch ==3)
		 TIM_SetCompare3(mTempTimer, value);
	else if (ch ==4)
		 TIM_SetCompare4(mTempTimer, value);
	else {}
	
}

u16 Timer::GetCNT()
{
  return TIM_GetCounter(mTempTimer);
}

//获取通道比较值
u16 Timer::GetChCompare(u8 ch)
{
	return 0 ;
}





//比较模式——定时器2的通道1
void OutputCompare_test()
{
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
//  RCC_PCLK1Config(RCC_HCLK_Div1); 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//使能TIM5时钟
// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //使能GPIOA时钟

	//初始化定时器5 TIM5
	TIM_TimeBaseStructure.TIM_Period = 0xffff; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =0; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
//  TIM_PrescalerConfig(TIM1, PrescalValue,TIM_PSCReloadMode_Immediate);
	//输出比较时间模式配置：通道1
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 10;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM2, &TIM_OCInitStructure);
	
	TIM_OC1PreloadConfig(TIM2,TIM_OCPreload_Disable);//禁止预装载使能
	TIM_ITConfig(TIM2,TIM_IT_CC1,ENABLE);//使能中断
//TIM_Cmd(TIM2,ENABLE ); 	//使能定时器5
	 
	 NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级2
	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	 NVIC_Init(&NVIC_InitStructure);
}

Timer::Timer(u8 timer)
{

	  if(timer == 2)
		{
			mTempTimer = TIM2;
			TIM_OCInitTypeDef  TIM_OCInitStructure;
			TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
			NVIC_InitTypeDef NVIC_InitStructure;
			
		//  RCC_PCLK1Config(RCC_HCLK_Div1); 
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

			//初始化定时器5 TIM5
			TIM_TimeBaseStructure.TIM_Period = 0xfffe; //设定计数器自动重装值 
			TIM_TimeBaseStructure.TIM_Prescaler =999; 	//预分频器   
			TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
			TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
			TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
		//  TIM_PrescalerConfig(TIM1, PrescalValue,TIM_PSCReloadMode_Immediate);
			//输出比较时间模式配置：通道1
			TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
			TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
			TIM_OCInitStructure.TIM_Pulse = 100;
			TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
			TIM_OC1Init(TIM2, &TIM_OCInitStructure);
			
			TIM_OC2Init(TIM2, &TIM_OCInitStructure);
			
			
			//TIM_OC1PreloadConfig(TIM2,TIM_OCPreload_Disable);//禁止预装载使能
			//TIM_ITConfig(TIM2,TIM_IT_CC1,ENABLE);//使能中断
			//TIM_ITConfig(TIM2,TIM_IT_CC2,ENABLE);//使能中断
			//TIM_Cmd(TIM2,ENABLE ); 	//使能定时器 
			
			 TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);//清楚中断标识位
			 TIM_ClearITPendingBit(TIM2, TIM_IT_CC2);
			 
			 NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
			 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级
			 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级
			 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
			 NVIC_Init(&NVIC_InitStructure);
		 }
	  if(timer == 3)
		{
			mTempTimer = TIM3;
			TIM_OCInitTypeDef  TIM_OCInitStructure;
			TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
			NVIC_InitTypeDef NVIC_InitStructure;
			
		//  RCC_PCLK1Config(RCC_HCLK_Div1); 
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

			TIM_TimeBaseStructure.TIM_Period = 0xfffe; //设定计数器自动重装值 
			TIM_TimeBaseStructure.TIM_Prescaler =999; 	//预分频器   
			TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
			TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
			TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
		//  TIM_PrescalerConfig(TIM1, PrescalValue,TIM_PSCReloadMode_Immediate);
			//输出比较时间模式配置：通道1
			TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
			TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
			TIM_OCInitStructure.TIM_Pulse = 100;
			TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
			TIM_OC1Init(TIM3, &TIM_OCInitStructure);
			
			//TIM_OC1PreloadConfig(TIM2,TIM_OCPreload_Disable);//禁止预装载使能
			//TIM_ITConfig(TIM2,TIM_IT_CC1,ENABLE);//使能中断
			//TIM_ITConfig(TIM2,TIM_IT_CC2,ENABLE);//使能中断
			//TIM_Cmd(TIM2,ENABLE ); 	//使能定时器 
			
			 TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);//清楚中断标识位
			 TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
			 
			 NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
			 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级
			 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级
			 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
			 NVIC_Init(&NVIC_InitStructure);
		}
}

