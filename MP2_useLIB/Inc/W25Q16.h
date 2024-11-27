/****************************************************************    
    ģ������(Filename):       w25q16.h 
    ��Ŀ����(Projectname):       
    �汾��(Version):	      
    ��������(Date):                  
    ����(Author):             
    ��������(Description):    
    ����˵��(Others):          
    �޸ļ�¼(History):
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
#define 	  SPIFLASH_DATA_LEN 	8//sizeof(stSentToPADPillowData)8�ֽ�256������ÿ�����ݴ����һ���ֽ�
#define 	  SPIFLASH_SIZE 		0x200000
#define FLASH_SPI_CS_ENABLE() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4, GPIO_PIN_RESET)
#define FLASH_SPI_CS_DISABLE() HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4, GPIO_PIN_SET)

#endif
