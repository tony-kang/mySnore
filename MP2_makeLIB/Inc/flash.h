#include "stdbool.h"
#ifndef TUWAN_3G_FLASH
#define  TUWAN_3G_FLASH
#define SYSTEM_PARAMETER_ADDRESS            0x08010000
#define IMAGE_A_ADDRESS                     0x08020000
#define IMAGE_B_ADDRESS                     0x08080000
#define PUBLIC_SNORE_LIB_ADDRESS            0x080E0000

#define SYSTEM_PARAMETER_SECTOR             FLASH_SECTOR_4
#define SYSTEM_IMAGE_A_SECTOR               FLASH_SECTOR_5
#define SYSTEM_IMAGE_B_SECTOR               FLASH_SECTOR_8
#define PUBLIC_SNORE_LIB_SECTOR             FLASH_SECTOR_11

#define SYSTEM_PARAMETER_SECTOR_NUM             1
#define SYSTEM_IMAGE_A_SECTOR_NUM               3
#define SYSTEM_IMAGE_B_SECTOR_NUM               3
#define PUBLIC_LIB_SECTOR_NUM                   1


#define STORE_DEBUG_LOG                         20
#define DEBUG_LOG_LEN                           20

#define SYSTEM_FLAG 0X55AA55AA
#define IMAGEA    0x0a
#define IMAGEB    0x0b
typedef struct {
unsigned int sign;    //0x55aa55aa
unsigned int imgAver;
unsigned int imgBver;
unsigned int upgradeType;  //0x0a for image A ; 0x0b for image B; 0x00 no upgrade
}system_parameter_bak_def;


typedef struct{
char DebugStack[STORE_DEBUG_LOG][DEBUG_LOG_LEN];
int current;
}DebugLog_def;



typedef struct {
unsigned int sign;             //0x55aa55aa
unsigned int workDelay;       //延时启动时间
unsigned int action_pfm;      //动作强度
unsigned int snoreSensitivity;  //鼾声灵敏度
unsigned int workTimeOn;    //启用工作时间
unsigned int workingTime;    //工作时间段
unsigned int 	 SPIFlashAddress;     //鼾声
unsigned int   SPIFlashOutAddress;
unsigned int 	 SPIFlashLocalAddress;    //头部
unsigned int   SPIFlashLocalOutAddress;
DebugLog_def   DebugLog;
}firmware_parameter_bak_def;
#endif 
extern system_parameter_bak_def g_systemParameter;
extern firmware_parameter_bak_def g_firmwareParameter;
bool ProgramFlashBlock(unsigned int address,unsigned int size,unsigned int *data);
void ResetToDefaultSystemParameter(void);
void SetFlashRDProtection(void);
void ResetToDefaultFirmwareParameter(void);
void BackupSystemParameter(void);
unsigned char GetSystemParameter(void);
void EraseImageAFlash(void);
void EraseImageBFlash(void);

