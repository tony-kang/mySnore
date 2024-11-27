#define TUWAN_PILLOW_3    1    //三代枕头
//#define MP1_PLUS             该配置已转移至config.h中  //编译MP1 PLUS版本时打开
//#define TUWAN_MAT     1       //小垫子///MAT
//#define TUWAN_BLANKET     1   //止鼾毯
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include "I2s.h"
#include "stdbool.h"
#include "flash.h"
#include "cmsis_os.h"
#include "snore.h"
#include "pillow.h"
#include "InputOutput.h"
#include "Qstruct.h"
#include "SpiFlashFileSys.h"
#include "W25Q16.h"
#include "config.h"


#ifndef TUWAN_PARA
#define TUWAN_PARA
#ifdef TUWAN_BLANKET
	#define BOARD_VERSION     1    //止鼾毯
	#define MASTER_VERSION    0
	#define SOFTWARE_VERSION  2

	#define PRESSURE_CONTROL 	1  //气压控制开关
#endif
#ifdef TUWAN_PILLOW_3
 #ifdef MP1_PLUS
  #define BOARD_VERSION     5 //5   //四代简易枕/垫子
	#define MASTER_VERSION    1 //1    //新的电路板
	#define SOFTWARE_VERSION  2 //2   //23  MP2:1 
 #else
  #define BOARD_VERSION     4   //四代简易枕/垫子
	#define MASTER_VERSION    1    //新的电路板
	#define SOFTWARE_VERSION  20   //23  MP2:1
	//#define  ACTION_CHECK  1
 #endif
#endif

#ifdef TUWAN_MAT
	#define BOARD_VERSION     2    //小垫子
	#define MASTER_VERSION    0
	#define SOFTWARE_VERSION  11
	//#define  ACTION_CHECK  1
#endif

//#define UPGRADE_TEST 1
#define USE_RTOS_Q 1
//#define SEND_PCM   1
//#define PRINT_TASK_STACK 1
//#define SKIP_HEAD_BODY_KEEP_AIR 1
//#define AUTO_RUN      1
//#define SHOW_MODE       1
//#define SNORE_TESTMODE       1            //打开设高时9:00-21:00，任何声音都会识别鼾声


#define UART5_BAUDRATE 115200

#define SOFTIIC     1       //软件I2C

//************************************************************************

#define PILLOW_AIRTIME_HIGH  60000 //枕头止鼾充气时间，单位ms
#define PILLOW_AIRTIME_HIGHER 53000
#define PILLOW_AIRTIME_MID   45000 //枕头止鼾充气时间，单位ms
#define PILLOW_AIRTIME_LOWER 38000
#define PILLOW_AIRTIME_LOW   30000 //枕头止鼾充气时间，单位ms
#define PILLOW_AIROFFTIME    50000 //枕头止鼾放气时间，单位ms  45000

#define PILLOW_AIRTIME_TESTMODE 60000 //枕头测试模式充气时间 单位ms  //60000
#define PILLOW_AIROFFTIME_TESTMODE 50000 //枕头测试模式放气时间 单位ms //40000


#define PILLOW_AIRTIME_SHOWMODE 60000 //枕头演示模式充气时间 单位ms
#define PILLOW_AIROFFTIME_SHOWMODE 50000 //枕头演示模式放气时间 单位ms 40000


//************************************************************************

#define PROCESS_OVER_TIME       12*60*100
/////////presure control////////////////
#define INHALE_PRESURE_VALUE     0xf50 //0x0f50  //0x0750  //0x310        //充气阀值
#define EXHALE_PRESURE_VALUE     0x40     // 0x0400   //0x0400  //0x9          //泄气阀值


///////////////////////BLE command/////////////////
#define OK_CMD                   0x10
#define ERROR_CMD                0x11
#define SNORE_SENSITIVITY        0x20
#define ACTION_LEVEL             0x21
#define WORK_DELAY               0x22
#define GET_SLEEP_DATA_CNT       0x23
#define GET_SLEEP_DATA           0x24

#define SET_TIME                 0x25
#define GET_FIRMWARE_VERSION   	 0x26
#define REQUEST_UPGRADE          0x27
#define SEND_FIRMWARE_PARA       0x28
#define SEND_FIRMWARE_DATA       0x29
#define SEND_FIRMWARE_END        0x30
#define SET_WORK_MODE            0x31
#define GET_DEVICE_READY         0x32
#define SET_WORK_STATE           0x33

#define START_DEMO_MODE           0x34
#define CLOSE_DEMO_MODE           0x35

#define SET_USER_KEYCODE          0x36
#define INPUT_USER_KEYCODE        0x37

#define READ_SNORE_SENSITIVITY    0x50
#define READ_ACTION_LEVEL         0x51
#define READ_WORK_DELAY           0x52
#define READ_WORK_MODE            0x53
#define READ_WORK_STATUS          0x54
#define READ_ERROR_STATUS         0x55
#define CONTROL_AIR_BAG           0x56
#define GET_BODY_POSITON          0x57
#define GET_SNORE_DB              0x58
#define SET_WORK_STATUS           0x59
#define RESET_FACTORY_DEFAULT     0x5a
#define INPUT_POSITION_STANDARD   0x5b   //位置定标
#define DO_POSITION_STANDARD_STATUS   0x5c   //位置定标状态查询
#define AIR_4_BAG   0x5d
#define SET_WORK_TIME     0x5e
#define GET_WORK_TIME     0x5f
#define SET_MAINSLEEP_TIME 0x60
#define GET_MAINSLEEP_TIME 0x61
#define GET_SNORE_VECTORS  0x62

#define SET_HEADPOSITION_SENSITIVITY 0x63
#define GET_HEADPOSITION_SENSITIVITY 0x64

#define SET_HEADPOSITION_SENSITIVITY_PARA 0x65
#define GET_HEADPOSITION_SENSITIVITY_PARA 0x66

#define SET_SNORE_MINDB 0x67
#define GET_SNORE_MINDB 0x68

#define SET_BODYON_THRESHOLD 0x69
#define GET_BODYON_THRESHOLD 0x6a

#define SET_BODYOFF_THRESHOLD 0x6b
#define GET_BODYOFF_THRESHOLD 0x6c

#define SET_ONEHOUR_AUTODETECT 0x6d
#define GET_ONEHOUR_AUTODETECT 0x6e

#define GET_TIME                 0x6f

#define SET_AUTODETECT_CYCLE 0x70
#define GET_AUTODETECT_CYCLE 0x71

#define GET_LATEST_SNORE_TIME 0x72

#define SET_VALID_TIME 0x73
#define GET_VALID_TIME 0x74




#define SEND_FIRMWARE_CHK        0x90
#define GET_NEXT_SLEEP_DATA      0x94
#define GET_NEXT_NEXT_SLEEP_DATA 0x95


#define GET_TEST_WAV_DATA0        0x96
#define GET_TEST_WAV_DATA1        0x97
#define GET_TEST_WAV_DATA2        0x98

///////////////////////////////////////////////



#define ERROR_CODE_INHALE_OVER    	0X00
#define ERROR_CODE_EXHALE_OVER    	0X01
#define ERROR_CODE_POSTION_ERROR    0X02
#define ERROR_CODE_SNORE_ERROR    	0X03
//////////////////////////////////////////////////
#define RED_LED      0
#define BLUE_LED_LOW      1
#define BLUE_LED_HIGH      2






typedef struct{
unsigned char PDMbuf[INTERNAL_BUFF_SIZE*2];
}PDMubf_def;
typedef struct{
 short SnoreBuffer[256];
}SnoreBuffer_def;


typedef struct{
unsigned char ver1;
unsigned char ver2;
unsigned char ver3;
unsigned char firmwareType;
unsigned int firmwareSize;
unsigned char upgradeOk;
unsigned short checksum;
unsigned char startupgrade;
unsigned int timer;
unsigned int address;
unsigned short countCheckSum;
}NeedUpgradePara_def;


typedef struct{
bool  on;
unsigned int tick;
unsigned char LastStep;
}ProcessControl_def;


typedef struct
{
      uint8_t    Airbag_no_person_counter;
	    uint8_t    Airbag_person_postion_chaged_counter;
	    uint8_t    Airbag_body_left_right;
	    uint8_t    Airbag_last_body_left_right; 
     	uint8_t    Airbag_last_maxvalue_position;
 	    uint8_t    Airbag_now_maxvalue_position;
	    uint8_t    Airbag_postion_changed;
	    uint8_t    Airbag_need_process_flag;
	    
}cSensor_airbag_monitor_struct;

typedef enum
{
    SHORT_PRESS = 0,
    LONG_PRESS = 1,
	  UPDATE_SHORT_PRESS = 2,
	  UPDATE_LONG_PRESS
}GPIO_PressState;
#endif

typedef struct{
	int tick;
	unsigned char enable;
}Read_Pos_def;

typedef struct{

 unsigned char sum;
 unsigned short pos;
 unsigned int UpgradeBuffer[128];
 uint8_t  block_package_number; //校验码里包的数量
 uint8_t  package_number_ever_recieved[32];//校验码里的包序是否都已收到过
 uint16_t sector_sequence;
}upgrade_control_def;


#if defined(TUWAN_PILLOW_3) || defined(TUWAN_MAT)

#define   SLOW_MODE  1
#define   FAST_MODE  2
#define   ONCE_TIME  3
//extern uint8_t AirSensor_Checkposition_Mode;  // 1:slow mode 2:fast mode 3:once time(fast)
#endif

////////////////////////
extern osMessageQId UART5QHandle,UART3QHandle;
//extern osMessageQId SnoreBufferQHandle;
extern UART_HandleTypeDef huart5,huart3;
extern osThreadId ForthTaskHandle;
extern unsigned char StartSend,Masaji;
extern unsigned long RecordTime;
//extern RTC_HandleTypeDef hrtc;
extern PillowControl_def PillowControl;
extern HeadPosition_def  HeadPosition;
extern SPI_HandleTypeDef hspi1;
//extern IWDG_HandleTypeDef hiwdg;
extern unsigned char ActionSteps;
extern unsigned char   PVEvol8, Avol8,SnoreType,SnoreNose,Snore_Time_Enable,SnoreNoseLow,SnoreNoseHigh,SnoreAver;
extern RTC_TimeTypeDef stimestructure;
extern RTC_DateTypeDef sdatestructure;
extern unsigned char StartAllAction;
extern unsigned char ErrorCode;
extern uint8_t AirSensor_factory_calibration_flag,AirSensor_cp_calibrationflag,RequestControlValve;
extern Read_Pos_def  ReadPos;
extern Read_Pos_def  AppCtrlBag;
extern short int   TestOkWaveData[24];
////////////////////////




HAL_StatusTypeDef uart_send(unsigned char *data,unsigned int len);
void userStart(void);
void FT_Kernel(void);
extern bool LOGO_ON;
void uart_printf(char *fmt, ...);
void DebugLog(char *fmt, ...);
void CalculatePCM(unsigned char *Buffer);
//int CheckSnore(short *snore);
//void SnoreInit(void);
void BCD2Int(unsigned char *time,int cnt);
void Int2BCD(unsigned char *time,int cnt);
void Get_F407ChipID(void);
void uart5Send(unsigned char *data,unsigned int len);
void vPrintfTaskStatus(void);
unsigned char JumpToImage(unsigned char type);
void firstSnoreInit(void);
void CheckSystemParameter(void);
 void EntryStandbyMode(void);
 void ERP_ON(void);
 void MasajiControlAirBag(unsigned char w);
 void WrokingLed(unsigned char type,unsigned char state);
 extern void gStorageDataResetDefault(void);
 extern void uart3Send(unsigned char *data,unsigned int len);

