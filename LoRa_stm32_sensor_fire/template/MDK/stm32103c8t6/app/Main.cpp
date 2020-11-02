/*
   LORA STM32 SENSOR 
	 2017年8月25日10:32:57  V0.1  lissettecarlr create
   2017年9月11日14:41:33  V0.2  lissettecarlr 去除多余程序	
   2017年9月21日14:28:12  V0.3  lissettecarlr 电气火灾程序修改	 
	 2017年9月22日19:38:08  V0.4  lissettecarlr 解决变量意外被修改问题
	 2018年1月24日10:26:05  V1.0  ygh 增加重启命令，设备忙时自动复位
*/
#include <stdio.h>
#include "stm32f10x.h"
#include "TaskManager.h"
#include "UserInterrupt.h"
#include <string.h>

// 2:  电气火灾
// 4:  DTU CLASS_C

#define DOING 2

/*system*/
Timer T1(2);
Timer T2(3);
USART serial(1,9600,false);//与传感器交互
USART serial2(2,9600,false);//与模组交互
USART serial3(3,115200,false);//打印信息

FreeUsart Communications(serial,T1);
FreeUsart Communications2(serial2,T2);

#if DOING==2

typedef struct pck{
   uint8_t len;
	 uint8_t data[255];  // 数据位最大负荷255+6
}pck;

//串口1 对传感器连接
//串口2 对模组连接

#include "weichuan-m100d.h"
#include "Acrel-ARCM.h"
#include "wdg.h"
#include "Main.h"
//定期查询DTU的状态，单位秒
#define HEARTBEAT 60

//低亮
GPIO ledGPIO(GPIOB,5,GPIO_Mode_Out_PP,GPIO_Speed_50MHz);//LED GPIO

GPIO RESGPIO(GPIOA,5,GPIO_Mode_Out_PP,GPIO_Speed_50MHz);//唯传GPIO
//报警状态采集周期:30 s
//测量值采集周期:60 s
//超时时间: 3 s
ARCM fire(Communications,30,60);

void m100d_rst()
{
	RESGPIO.SetLevel(0);	
	tskmgr.DelayMs(110);
	RESGPIO.SetLevel(1);
	ledGPIO.SetLevel(1);//复位熄灯
	
}


int main()
{
	ledGPIO.SetLevel(1);//clear
	RESGPIO.SetLevel(1);
	

	double ConnectState=0;
	double RecordState=0;
	double RecordMeasure=0;
	double RecordIWDG=0;
	
	u8 RunFlag=0;
	bool ResponseStateFlag=true; //标志是否响应了状态返回
	pck RecvSensorPck;
	lora_send_pck RecvModPck;
	lora_send_pck SendModPck;
	Communications.StartRecive();
	Communications2.StartRecive();
//	u8 NetworkFlag=0;//入网标识
		serial3.cmd(1);
	
	  m100d_rst();//开始复位唯传

		serial3<<"delay network\n";
//	while(1)//等待入网
//	{
//		tskmgr.DelayMs(1000);
//	   if(NetworkFlag)
//			 break;
//	}
	
	IWDG_Init(4,625); //设定1s看门狗

	while(1)
	{ 
		  //启动定时器，周期检测DTU连接状态
			if(tskmgr.TimeSlice(ConnectState,60) )
			{
			  if(!ResponseStateFlag)  //如果第二次进入任然为false，表示模组丢失，熄灭LED
				{
				    ledGPIO.SetLevel(1); //clear
				}
				ResponseStateFlag = false;
				serial2.cmd(1);
				SendModPck = lora_send_request_state();
				serial2.SendData(SendModPck.data,SendModPck.Lenth);
			}			
			//读取传感器状态周期到了
		  if(tskmgr.TimeSlice(RecordState,fire.mGetStatePeriod))
			{
         fire.init();
				 RunFlag = 1;
			}
			//读取传感器测量值周期到了
			if(tskmgr.TimeSlice(RecordMeasure,fire.mGetMeasurePeriod))
			{
				 fire.init();
			   RunFlag = 2;
			}
			//设定看门狗超时
			if(tskmgr.TimeSlice(RecordIWDG,0.01))
				{
					IWDG_Feed();	
			}
						
		  switch(RunFlag) 
			{
			  case 1:{
				    if(fire.Update(ARCM_REGISTER_WARNING_STATE,serial) == ARCM_STATE_OK) //接收状态数据
						{
						   if(fire.ReadWarningState() ==1){
								 RunFlag = 2;//如果值是报警，则立即进行数据采集
								 fire.init();
							 }else
								 RunFlag = 0;
						}
				  RecordState=tskmgr.Time();//刷新周期采集时间，确保定时时间只算在空闲时间中，而不在传感器更新时间中累计
				}break;
				case 2:{
					 if(fire.Update(ARCM_REGISTER_MEASURE_VALUE,serial) == ARCM_STATE_OK) //接收测量值数据
					 {
						RecvSensorPck.data[0] = (uint8_t)((fire.Read(1))>>8); 
						RecvSensorPck.data[1] = (uint8_t)(fire.Read(1));
						RecvSensorPck.data[2] = (uint8_t)((fire.Read(2))>>8);
						RecvSensorPck.data[3] = (uint8_t)(fire.Read(2));
						RecvSensorPck.data[4] = (uint8_t)((fire.Read(3))>>8);
						RecvSensorPck.data[5] = (uint8_t)(fire.Read(3));
						RecvSensorPck.data[6] = (uint8_t)((fire.Read(4))>>8);
						RecvSensorPck.data[7] = (uint8_t)(fire.Read(4));
						RecvSensorPck.len = 8; 
						RunFlag=0xff;	//上传数据
					 }
					RecordState=tskmgr.Time();
				  RecordMeasure=tskmgr.Time();
				}break;
        case 0xff:{	
				  //接收完成,发送数据给网关 
				  Communications.StartRecive();
					Communications2.StartRecive();
			    //封装上行包重传0次,通道128
			    SendModPck = M100dFrameSend(RecvSensorPck.data,RecvSensorPck.len,0,128);
 			    serial2.SendData(SendModPck.data,SendModPck.Lenth);
					
				  RunFlag = 0;
				 	RecordState=tskmgr.Time();
				  RecordMeasure=tskmgr.Time();
				}break;					
			}//SWITCH END
 
		if(Communications2.ReciveIsOk())//接收模组的信息
		{  
					Communications2.StartRecive();
			    RecvModPck.Lenth = serial2.ReceiveBufferSize();
				  serial2.GetReceivedData(RecvModPck.data,RecvModPck.Lenth);
			  
			    RecvModPck=M100FrameReciveData(RecvModPck.data,RecvModPck.Lenth);
			    SendModPck = M100dFrameACK(SEND_OK);//应答包
			
			if(RecvModPck.type == LORA_MOD_DATA) //如果接受到平台下发数据
        {
					 serial2.SendData(SendModPck.data,SendModPck.Lenth);
						
					 fire.DeelRcvOrder(RecvModPck.data+1,RecvModPck.Lenth-1);		
					 Communications2.StartRecive();
				}
				else if(RecvModPck.type == LORA_MOD_STATE)
				{
				   switch(RecvModPck.data[0])
					 {
						 case LORA_JOIN_ING:
						 {
						   serial2.SendData(SendModPck.data,SendModPck.Lenth);
						  serial3<<"join ing\n";
						 }break;
						 case	LORA_JOIN_OK:{
							 ledGPIO.SetLevel(0);
						//	 NetworkFlag=1;
						   serial2.SendData(SendModPck.data,SendModPck.Lenth);
										serial3<<"join OK\n";		

						 }break;
						 case LORA_JOIN_FAIL:{
							 ledGPIO.SetLevel(1);
							 //NetworkFlag=0;
							 serial2.SendData(SendModPck.data,SendModPck.Lenth);
				

						 }break;
					 }
				}
        else if(RecvModPck.type == LORA_MOD_RST_OK)
				{
				   serial2.SendData(SendModPck.data,SendModPck.Lenth);
				}
				else if(RecvModPck.type == LORA_MOD_RESPONSE)
				{	
					
					static int timeflag = 0;
					
					
						switch(RecvModPck.data[0])
						 {
							 case SEND_OK:
								 break;
							 case SEND_FINISH: //发送完成 
								 serial3<<"send over\n";
								 break;
							 case MSG_ERR_NOT_JOIN: //未入网
								 ledGPIO.SetLevel(1);
								 break;
							 case ERR_BUSY:{ //设备忙
								 timeflag++;
							if(timeflag == 2){
								m100d_rst();
								timeflag = 0;
							}
						}	break;
							
							
							 default:
									{
									}
						 }
					
				}else if(RecvModPck.type == LORA_MOD_RESPONSE_STATE) //这里处理的是返回的状态数据
				{
					 ResponseStateFlag = true;
				
				   switch(RecvModPck.data[0])
					 {
						 case 2: //已入网
						 {
						   ledGPIO.SetLevel(0);
					
						  // NetworkFlag=1;
						 }break;
					   case 3: //入网失败
						 {
						   ledGPIO.SetLevel(1);
							// NetworkFlag=0;
						 }break;
						 case 4://发送中
			       {

							
			       }break;
						 	case 5://发送成功
						 {
						   ledGPIO.SetLevel(0);
					
						 }break;
					 }
				}
		}//RCV
	}
 // return 1;
}


#endif

#if DOING==4

//串口1 对传感器连接
//串口2 对模组连接

#include "weichuan-m100d.h"

//定期查询DTU的状态，单位秒
#define HEARTBEAT 60

//低亮
GPIO ledGPIO(GPIOB,5,GPIO_Mode_Out_PP,GPIO_Speed_50MHz);//LED GPIO

int main()
{
	//serial<<" lora-dtu \n Deui : a004000010ffffff\n Version: V0.2";
	ledGPIO.SetLevel(1);//clear
	double SendStateRecord=0;
	double a=0;
	bool ResponseStateFlag=false; //标志是否响应了状态返回
	lora_send_pck RecvSensorPck;
	lora_send_pck RecvModPck;
	lora_send_pck SendModPck;
	
	Communications.StartRecive();
	while(1)
	{ 
		if(tskmgr.TimeSlice(a,1) )
		{
		   serial<<"afd\n";
		}
		//启动定时器，周期检测DTU连接状态
		if(tskmgr.TimeSlice(SendStateRecord,HEARTBEAT) )
		{
			if(!ResponseStateFlag)  //如果第二次进入任然为false，表示模组丢失，熄灭LED
			{
					ledGPIO.SetLevel(1); //clear
			}
			ResponseStateFlag = false;
			serial2.cmd(1);
			SendModPck = lora_send_request_state();
			serial2.SendData(SendModPck.data,SendModPck.Lenth);
		}	
		
		
    //接收传感器数据
		if(Communications.ReciveIsOk())
		{
				  Communications.StartRecive();
			    RecvSensorPck.Lenth = serial.ReceiveBufferSize();
			    if(RecvSensorPck.Lenth >255) //数据接收过多
					{
					  serial.ClearReceiveBuffer();
						continue;
					}
				  serial.GetReceivedData(RecvSensorPck.data,RecvSensorPck.Lenth);
			    //封装上行包重传0次,通道128
			    RecvSensorPck=M100dFrameSend(RecvSensorPck.data,RecvSensorPck.Lenth,1,128);
			    serial2.SendData(RecvSensorPck.data,RecvSensorPck.Lenth);
		}	
		if(Communications2.ReciveIsOk()) //接收模组的信息
		{  
					Communications2.StartRecive();
			    RecvModPck.Lenth = serial2.ReceiveBufferSize();
				  serial2.GetReceivedData(RecvModPck.data,RecvModPck.Lenth);
			    RecvModPck=M100FrameReciveData(RecvModPck.data,RecvModPck.Lenth);
			    RecvSensorPck = M100dFrameACK(SEND_OK);//应答包
			
			if(RecvModPck.type == LORA_MOD_DATA) //如果接受到平台下发数据
        {
					 //拆包后发送给传感器
					 serial2.SendData(RecvSensorPck.data,RecvSensorPck.Lenth);
					 serial.SendData(RecvModPck.data+1,RecvModPck.Lenth-1); //获取数据的第一位是端口号 
				}
				else if(RecvModPck.type == LORA_MOD_STATE)
				{
				   switch(RecvModPck.data[0])
					 {
						 case LORA_JOIN_ING:
						 {
						   serial2.SendData(RecvSensorPck.data,RecvSensorPck.Lenth);
						 }break;
						 case	LORA_JOIN_OK:{
							 ledGPIO.SetLevel(0);
						   serial2.SendData(RecvSensorPck.data,RecvSensorPck.Lenth);
						 }break;
						 case LORA_JOIN_FAIL:{
							 ledGPIO.SetLevel(1);
							 serial2.SendData(RecvSensorPck.data,RecvSensorPck.Lenth);
						 }break;
					 }
				}
        else if(RecvModPck.type == LORA_MOD_RST_OK)
				{
				   serial2.SendData(RecvSensorPck.data,RecvSensorPck.Lenth);
				}
				else if(RecvModPck.type == LORA_MOD_RESPONSE)
				{
					
				}else if(RecvModPck.type == LORA_MOD_RESPONSE_STATE) //这里处理的是返回的状态数据
				{
					 ResponseStateFlag = true;
				   switch(RecvModPck.data[0])
					 {
						 case 2: //已入网
						 {
						   ledGPIO.SetLevel(0);
						 }break;
					   case 3: //入网失败
						 {
						   ledGPIO.SetLevel(1);
						 }break;
						 	case 5://发送成功
						 {
						   ledGPIO.SetLevel(0);
						 }break;
						 default:
							 ledGPIO.SetLevel(0);
					 }
				}
		}
	}
 // return 1;
}

#endif


/*******串口中断（公用）*******/
void UserUsartReciveIRQ(USART_TypeDef* usart)
{
	if(usart == USART1)
	{
    Communications.UsartIrqDeal();
  }
	
	if(usart == USART2)
	{
     Communications2.UsartIrqDeal();	
	}
}

//定时器中断 数据获取完毕
void Timer2_CC1_IRQ()
{
		Communications.SetReciveEnd(); 
}

void Timer3_CC1_IRQ()
{
    Communications2.SetReciveEnd();   
}
/*******串口中断 END *******/

