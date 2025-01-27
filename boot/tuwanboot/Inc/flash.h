#include "stdbool.h"
#ifndef TUWAN_3G_FLASH
#define  TUWAN_3G_FLASH

//add by zjp 20211020
#define SYSTEM_PARAMETER1_ADDRESS            0x0800C000  //作为SYSTEM_PARAMETER的备份地址，该地址实为bootloader地址，但bootloader代码没这么大
#define SYSTEM_PARAMETER1_SECTOR             FLASH_SECTOR_3
#define SYSTEM_PARAMETER1_SECTOR_NUM         1
//

#define SYSTEM_PARAMETER_ADDRESS            0x08010000
#define IMAGE_A_ADDRESS                     0x08020000
#define IMAGE_B_ADDRESS                     0x08060000
#define FIRMWARE_PARAMETER_ADDRESS          0x080E0000

#define SYSTEM_PARAMETER_SECTOR             FLASH_SECTOR_4
#define SYSTEM_IMAGE_A_SECTOR               FLASH_SECTOR_5
#define SYSTEM_IMAGE_B_SECTOR               FLASH_SECTOR_7             //FLASH_SECTOR_8
#define FIRMWARE_PARAMETER_SECTOR           FLASH_SECTOR_11

#define SYSTEM_PARAMETER_SECTOR_NUM             1
#define SYSTEM_IMAGE_A_SECTOR_NUM               2 //3
#define SYSTEM_IMAGE_B_SECTOR_NUM               2 //3
#define FIRMWARE_PARAMETER_SECTOR_NUM           1


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
bool ProgramFlashBlock(unsigned int address,unsigned int size,unsigned int *data);
void ResetToDefaultSystemParameter(void);
void SetFlashRDProtection(void);
void ResetToDefaultFirmwareParameter(void);
void BackupSystemParameter(void);
void BackupSystemParameter1(void);
unsigned char  GetSystemParameter(void);
unsigned char  GetSystemParameter1(void);
unsigned char  GetSystemPara(void);
unsigned char JumpToImage(unsigned char type);
void BootProcess(void);

extern system_parameter_bak_def g_systemParameter,g_systemParameter1;
extern system_parameter_bak_def  *SystemPara;
