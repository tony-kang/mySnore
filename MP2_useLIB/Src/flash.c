#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"
#include "tuwan.h"
#include "flash.h"
system_parameter_bak_def g_systemParameter,g_systemParameter1;
firmware_parameter_bak_def g_firmwareParameter;
unsigned char CurrentRom=0;

unsigned char GetSystemParameter(void)
{
   memcpy(&g_systemParameter,(unsigned char *)SYSTEM_PARAMETER_ADDRESS,sizeof(system_parameter_bak_def));
   if(g_systemParameter.sign!=SYSTEM_FLAG)
		 return 0;
	 if(((g_systemParameter.imgAver==0)&&(g_systemParameter.imgBver==0)) || ((g_systemParameter.imgAver==0xffff)&&(g_systemParameter.imgBver==0xffff)))
		 return 0;
	 return 1;
}
unsigned char GetSystemParameter1(void)
{
   memcpy(&g_systemParameter1,(unsigned char *)SYSTEM_PARAMETER1_ADDRESS,sizeof(system_parameter_bak_def));
   if(g_systemParameter1.sign!=SYSTEM_FLAG)
		 return 0;
	 if(((g_systemParameter1.imgAver==0)&&(g_systemParameter1.imgBver==0)) || ((g_systemParameter1.imgAver==0xffff)&&(g_systemParameter1.imgBver==0xffff)))
		 return 0;
	 return 1;
}


/*需要先调用EraseSector*/
bool ProgramFlashBlock(unsigned int address,unsigned int size,unsigned int *data)
{    int index;
	   HAL_StatusTypeDef  status;
	  HAL_FLASH_Unlock();
		for (index = 0; index < size; index++)
    {
       status=HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address+(index*4), *data++);
			 if(status==HAL_ERROR)
			 { HAL_FLASH_Lock();
				 return false;
			 }
    }
		HAL_FLASH_Lock();
    return true;
}


void EraseSector(unsigned char sector,unsigned char nSector)
{ 
	uint32_t  page_error = 0;
	FLASH_EraseInitTypeDef  FlashEraseInit;
	FlashEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;
	FlashEraseInit.Sector=sector;
	FlashEraseInit.NbSectors=nSector;
	FlashEraseInit.VoltageRange=FLASH_VOLTAGE_RANGE_3;
	FlashEraseInit.Banks=0;
  HAL_FLASH_Unlock();
	HAL_FLASHEx_Erase(&FlashEraseInit, &page_error);
	HAL_FLASH_Lock();
}

void ResetToDefaultSystemParameter(void)
{ 
	g_systemParameter.sign=0x55aa55aa;
	g_systemParameter.imgAver=1;
	g_systemParameter.imgBver=0;
	g_systemParameter.upgradeType=0;
  EraseSector(SYSTEM_PARAMETER_SECTOR,SYSTEM_PARAMETER_SECTOR_NUM);	
	ProgramFlashBlock(SYSTEM_PARAMETER_ADDRESS,(sizeof(system_parameter_bak_def))/4,(unsigned int *)&g_systemParameter);
}

void EraseImageAFlash(void)
{
	EraseSector(SYSTEM_IMAGE_A_SECTOR,SYSTEM_IMAGE_A_SECTOR_NUM);
}
void EraseImageBFlash(void)
{
	EraseSector(SYSTEM_IMAGE_B_SECTOR,SYSTEM_IMAGE_B_SECTOR_NUM);
}


void BackupSystemParameter(void)
{
  EraseSector(SYSTEM_PARAMETER_SECTOR,SYSTEM_PARAMETER_SECTOR_NUM);	
	ProgramFlashBlock(SYSTEM_PARAMETER_ADDRESS,(sizeof(system_parameter_bak_def))/4,(unsigned int *)&g_systemParameter);
}

void BackupSystemParameter1(void)
{
  EraseSector(SYSTEM_PARAMETER1_SECTOR,SYSTEM_PARAMETER1_SECTOR_NUM);	
	ProgramFlashBlock(SYSTEM_PARAMETER1_ADDRESS,(sizeof(system_parameter_bak_def))/4,(unsigned int *)&g_systemParameter1);
}


void BackupPublicSnoreLib(float *pt,unsigned int cnt)
{
	EraseSector(PUBLIC_SNORE_LIB_SECTOR,PUBLIC_LIB_SECTOR_NUM);	
	ProgramFlashBlock(PUBLIC_SNORE_LIB_ADDRESS,cnt,(unsigned int *)pt);
}


void SetFlashRDProtection(void)
{    FLASH_OBProgramInitTypeDef   Flash_OB;
	   HAL_FLASH_Unlock();
     HAL_FLASH_OB_Unlock();
	   HAL_FLASHEx_OBGetConfig(&Flash_OB);
    if(OB_RDP_LEVEL_1 != Flash_OB.RDPLevel)
    {   
			  Flash_OB.OptionType=OPTIONBYTE_RDP;
			  Flash_OB.RDPLevel=OB_RDP_LEVEL_1;
			  HAL_FLASHEx_OBProgram(&Flash_OB);
       	HAL_FLASH_OB_Launch();
    }
	  HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
	//	HAL_FLASHEx_OBGetConfig(&Flash_OB);
}




