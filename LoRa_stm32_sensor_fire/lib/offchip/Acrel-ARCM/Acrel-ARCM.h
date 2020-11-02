/*
安科瑞电气火灾驱动
lissettecarlr
V0.1
V0.2 2017年9月14日17:15:06 
V0.3 2017年9月21日14:23:08

使用说明: 
init()
Update() 循环该函数，传入需要更新的类型，当返回ARCM_STATE_OK时表示更新完成，更新最长时间是
TIMEOUT_DEFAULT 定义的超时时间。
之后可以通过Read()读出测量值，或者ReadWarningState()读取是否报警
*/

#ifndef _ACREL_H_
#define _ACREL_H_

#include "TaskManager.h"
#include "USART.h"
#include "FreeUSART.h"
#include "Modbus-RTU.h"

#define TIMEOUT_DEFAULT         3  //超时默认值
#define USE_CHANNE_DEFAULE      4  //使用几个测量通道，该值将决定读取几组测量值

enum
{
	ARCM_STATE_READY =0x01,	   //准备阶段
	ARCM_STATE_BUSY  =0x02,		//数据更新阶段
	ARCM_STATE_OK    =0x03,		  	//更新完成阶段
};



//错误代码
enum ErrorCode{
 ARCM_TIMEOUT       = 0xff92,//电气火灾设备未回应     -110
 ARCM_CRC_ERROR     = 0xff91,//CRC错误                -111
 ARCM_SENSOR_LOSE   = 0xff90,//电流或温度检测这边丢失 -112
};

//更新选择
enum ArcmRegister{
  ARCM_REGISTER_WARNING_STATE  = 0X1003,//报警状态寄存器地址
  ARCM_REGISTER_MEASURE_VALUE =  0X1005,
  ARCM_REGISTER_CLEAR_WARNING =  0X1063,
	ARCM_REGISTER_SET_WARNING_THREASHOLD = 0x1033, //报警阀值 
};

static uint8_t OrderReadState[] =   {0x01,0x03,0x10,0x03,0x00,0x01,0x70,0xCA};
static uint8_t OrderReadMeasure[] = {0x01,0x03,0x10,0x05,0x00,0x04,0x50,0xC8};
static uint8_t OrderCleanwarning[] = {0x01,0x10,0x10,0x63,0x00,0x01,0x02,0x12,0x34,0xB3,0x75};

class ARCM{
	private:
		FreeUsart &mCom;
	  uint16_t mData[4];//保存测量值
	  uint8_t mUseNmuber;//保存使用的通道个数,最大4个通道
	  uint16_t mWarningState;//保存是否报警
	  uint8_t mState; //记录传感器状态
	   
	  double mRecord; //超时记录时间搓
	  double mTimeOut;//超时时间
	
	public:
    ARCM(FreeUsart &Com,uint16_t GetStatePeriod,uint16_t GetMeasurePeriod);//构造函数
	  uint16_t mGetStatePeriod;
	  uint16_t mGetMeasurePeriod;
	  uint8_t Update(u16 order,USART &com); //参数为命令类型 order = ArcmRegister
	  u16 ReadWarningState();
    u16 Read(u8 number);//读取测量值，最大保存4组
	  void init();//获取前的数据初始化
	  void SendClearwarning();//发送清除报警信息
	  void SetwarningValue(uint16_t Current_mA,uint16_t T1,uint16_t T2,uint16_t T3);//设置电流和温度的报警值
	  bool DeelRcvOrder(uint8_t *data,uint8_t lenth);
	  void SetWarningTempThreshold(uint16_t warning,uint16_t temp1,uint16_t temp2,uint16_t temp3);

};

#endif

