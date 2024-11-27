/****************************************************************    
    模块名称(Filename):       w25q16.h 
    项目名称(Projectname):       
    版本号(Version):	      
    创建日期(Date):                  
    作者(Author):             
    功能描述(Description):    
    其他说明(Others):          
    修改记录(History):
****************************************************************/

#ifndef  _W25Q16_H
#define  _W25Q16_H
#include "stm32f4xx_hal.h"
void ReadFlash(unsigned char *databuffer,unsigned int datasize,unsigned int address);
void FlashProgram(unsigned char *databuffer,unsigned int datasize,unsigned int address);
void ChipErase(void);
void SectorErase(unsigned int address);
void BlockErase64K(unsigned int address);
void BlockErase32K(unsigned int address);

int ReadUniqueID(unsigned char *databuffer);
#define 	  SPIFLASH_DATA_LEN 	8//sizeof(stSentToPADPillowData)8字节256整除，每个数据串后多一个字节
#define 	  SPIFLASH_SIZE 		0x200000
#define FLASH_SPI_CS_ENABLE() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4, GPIO_PIN_RESET)
#define FLASH_SPI_CS_DISABLE() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4, GPIO_PIN_SET)

#endif
