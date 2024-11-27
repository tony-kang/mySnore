#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "tuwan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"
unsigned char temp_buf[256];
stStorageData_Def gStorageData;
SemaphoreHandle_t xSemaphore = NULL;

//SPI Flash擦除是以sector为最小单位进行，一个sector为4kB
//系统参数只有64个字节，由于经常更新，所以选择了前8K的空间来不断变化位置保存，避免总写在一个地方容易损坏：marked by zjp

//从系统参数备份区域中将参数读出
//1。成功   0 失败
unsigned char ReadSysPara(stStorageData_Def *s)
{
    unsigned char i;
	if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE)
  { 
	   s->flag=0;
	  for(i=0;i<SYS_PARA_CYCLE_TIMES;i++)
    { 
			ReadFlash((unsigned char *)s,sizeof(stStorageData_Def),i*SYS_PARA_SIZE);
			if(s->flag==SYS_PARA_FLAG)
			{	
				xSemaphoreGive( xSemaphore );
				return 1;
			}
		}
		
		  xSemaphoreGive( xSemaphore );
			return 0;
	}
    return 0;
}
 

//将系统参数备份到指定的存储区中
//1。成功   0 失败
unsigned char BackupSysPara(stStorageData_Def *s)
{
    unsigned char i;
	  stStorageData_Def  t;
		if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE)
		{
			for(i=0;i<SYS_PARA_CYCLE_TIMES;i++)
			{ 
			ReadFlash((unsigned char *)&t,sizeof(stStorageData_Def),i*SYS_PARA_SIZE);
			if(t.flag==SYS_PARA_FLAG)
				break;
			}
			if(i>=(SYS_PARA_CYCLE_TIMES-1))
			i=0;		
			else i++;
			SectorErase(0);
			SectorErase(0x1000);
			FlashProgram((unsigned char *)s,sizeof(stStorageData_Def),i*SYS_PARA_SIZE);
			xSemaphoreGive( xSemaphore );
			return 1;
	 }
		return 0;
} 
 
//从数据存储区域读取历史记录
//1。成功   0 失败
unsigned char ReadSnoreData(SnoreData_def *p)
{   stStorageData_Def *s;
	
	   s=&gStorageData;
	if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE)
	{
		if(s->SPIFlashAddress==s->SPIFlashOutAddress)
		{    xSemaphoreGive( xSemaphore );
		     return 0; //没有可以读取的数据
		}
		ReadFlash((unsigned char *)p,SNORE_DATA_SIZE,s->SPIFlashOutAddress*SNORE_DATA_SIZE+SNORE_DATA_ADDRESS);
		if(s->SPIFlashOutAddress>=(SNORE_DATA_CNT-1))
	   s->SPIFlashOutAddress=0;
	  else s->SPIFlashOutAddress++;
		xSemaphoreGive( xSemaphore );
		BackupSysPara(s);
    return 1;
	}
	return 0;
}

//将数据写入存储区域中
//1。成功   0 失败
unsigned char WriteSnoreData(SnoreData_def *p)
{   stStorageData_Def *s;
	  s=&gStorageData;
	if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE)
	{
		//检查是否是已经存储满数据，如果存满则将最早的数据推出一个
		  if(s->SPIFlashAddress>=(SNORE_DATA_CNT-1))
			{
				if(s->SPIFlashOutAddress==0)
				{
					s->SPIFlashOutAddress++;	
				}
			}
			else if((s->SPIFlashOutAddress-s->SPIFlashAddress)==1)
			{
				s->SPIFlashOutAddress++;	
				if(s->SPIFlashOutAddress>=SNORE_DATA_CNT)
					s->SPIFlashOutAddress=0;
			}
	/////////////检查是否存储到一个新的SECTOR，如果是则要对它先做擦除////////////////////////////
			if((s->SPIFlashAddress*SNORE_DATA_SIZE)%0x1000 == 0) //??SECTOR??0x000000~0x000FFF
			{

       			if(s->SPIFlashOutAddress>s->SPIFlashAddress) //检查读出的位置是不是在待擦除的区域内
       			{
							if((s->SPIFlashOutAddress-s->SPIFlashAddress)<(4096/SNORE_DATA_SIZE))
							{	
								s->SPIFlashOutAddress=s->SPIFlashAddress+(4096/SNORE_DATA_SIZE);
        				if(s->SPIFlashOutAddress>=SNORE_DATA_CNT)
									s->SPIFlashOutAddress=0;			

							}


						}
						SectorErase((s->SPIFlashAddress*SNORE_DATA_SIZE)+SNORE_DATA_ADDRESS);
			}
		  ////写入相应的数据，然后备份数据的指针
			FlashProgram((unsigned char *)p,SNORE_DATA_SIZE,s->SPIFlashAddress*SNORE_DATA_SIZE+SNORE_DATA_ADDRESS);
			s->SPIFlashAddress++;
			if(s->SPIFlashAddress>= SNORE_DATA_CNT)
			{
				s->SPIFlashAddress = 0;
			}	
		xSemaphoreGive( xSemaphore );
		 BackupSysPara(s);	
	
    return 1;
		}
	return 0;
}
//获取睡眠数据记录数量
int GetSnoreDataCnt(void)  
{    stStorageData_Def *s;
	   s=&gStorageData;
		if(s->SPIFlashAddress==s->SPIFlashOutAddress)
		{	
			return 0 ;
		}
    if(s->SPIFlashAddress>s->SPIFlashOutAddress)
    {
			return s->SPIFlashAddress-s->SPIFlashOutAddress;
	  } 
			return (SNORE_DATA_CNT-s->SPIFlashOutAddress)+s->SPIFlashAddress;
}
//从数据存储区域读写学习的鼾声数据,MFCC特征数据
//1。成功   0 失败

int SnoreLibWR(float * data,unsigned int len,unsigned char wr)
{  int offset=0,i;
	 unsigned char *pt;
  if((len*4) > PRIVATE_SNORE_LIB_MAX_SIZE_64k)
		return 0;
	pt=(unsigned char *)data;
	if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE)
	{
		if(wr)   //write
		{  
			offset=PRIVATE_SNORE_LIB_OFFSET_32K;
			for(i=0;i<((len*4)/256);i++)
			{
				if(offset%0x1000 == 0)			
					SectorErase(offset);
				FlashProgram((unsigned char *)pt,256,offset);
				offset+=256;
				pt+=256;
			}
	
		}
		else     //read
		{
		  ReadFlash((unsigned char *)pt,len*4,PRIVATE_SNORE_LIB_OFFSET_32K);
		}
		xSemaphoreGive( xSemaphore );
		return 1;
	}

return 0;

}

//从数据存储区域读写学习的鼾声数据
//1。成功   0 失败

int PositionInitWR(unsigned char * pt,unsigned int len,unsigned char wr)
{  int offset=0,i;

  if(len > 4096)
		return 0;
	
	if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE)
	{
		if(wr)   //write
		{  
			offset=POSITION_DATA_OFFSET;
			SectorErase(offset);
			for(i=0;i<((len+255)/256);i++)
			{
				FlashProgram((unsigned char *)pt,256,offset);
				offset+=256;
				pt+=256;
			}
	
		}
		else     //read
		{
		  ReadFlash((unsigned char *)pt,len,POSITION_DATA_OFFSET);
		}
		xSemaphoreGive( xSemaphore );
		return 1;
	}

return 0;

}
