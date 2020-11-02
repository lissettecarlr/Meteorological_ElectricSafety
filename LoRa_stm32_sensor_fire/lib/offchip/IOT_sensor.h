#ifndef _IOT_SENSOR_H_
#define _IOT_SENSOR_H_


typedef enum
{
	SENSOR_STATE_READY,	  //准备阶段
	SENSOR_STATE_BUSY,		//数据更新阶段
	SENSOR_STATE_OK,			//更新完成阶段
}SensorState;


#define TEMP_HUMI_TIMEOUT_DEFAULT     3  //超时默认值
#define LIGHT_TIMEOUT_DEFAULT         3  
#define RAIN_TIMEOUT_DEFAULT          3  
#define WINDDIR_TIMEOUT_DEFAULT       3  
#define WAINDSPEED_TIMEOUT_DEFAULT    3  
#define PM2_5_TIMEOUT_DEFAULT         5  

//非特异传感器超时时间
#define SENSOR_TIMEOUT_DEFAULT        5 


//错误代码
enum{
 TEMP_HUMI_TIMEOUT       =0xff92ff92,        //超时       -110
 TEMP_HUMI_CRC_ERROR     =0xff91ff91,       //CRC错误     -111
 TEMP_HUMI_DATA_UNUSUAL  =0xff00ff90,        //数据异常   -112
};

//默认错误代码
enum{
SENSOR_DEFAULT_TIMEOUT = -110,//超时
SENSOR_DEFAULT_CRC_ERROR = -111,//CRC错误
SENSOR_DEFAULT_DATA_UNUSUAL = -112,//数据异常
};


//错误代码
enum{
LIGHT_TIMEOUT = -110,//超时
LIGHT_CRC_ERROR = -111,//CRC错误
LIGHT_DATA_UNUSUAL = -112,//数据异常
};

//错误代码
enum{
RAIN_TIMEOUT      = -1,//超时
RAIN_CRC_ERROR    = -2,//CRC错误
RAIN_DATA_UNUSUAL = -3,//数据异常
};

//错误代码
enum{
WINDSPEED_TIMEOUT = -110,//超时
WINDSPEED_CRC_ERROR = -111,//CRC错误
WINDSPEED_DATA_UNUSUAL = -112,//数据异常
};

//错误代码
enum{
WINDDIRECTION_TIMEOUT = -110,//超时
WINDDIRECTION_CRC_ERROR = -111,//CRC错误
WINDDIRECTION_DATA_UNUSUAL = -112,//数据异常
};

enum{
  PM2_5_TIMEOUT      = -110,//超时
  PM2_5_CRC_ERROR    = -111,//CRC错误
  PM2_5_DATA_UNUSUAL = -112,//数据异常
};
#endif
