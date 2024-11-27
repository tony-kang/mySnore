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

//SPI Flash擦除是以sector为最小单位进行，一个sector为4kB,写入一次最多为一个page(256字节，一次性不能跨page写入
//系统参数只有64个字节，由于经常更新，所以选择了前8K的空间来不断变化位置保存，避免总写在一个地方容易损坏：marked by zjp

//从系统参数备份区域中将参数读出
//1。成功   0 失败

#if 0
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
 
#endif

unsigned char ReadSysPara(stStorageData_Def *s)
{
    unsigned char i;
	while(xSemaphoreTake( xSemaphore, portMAX_DELAY ) != pdTRUE)
	{
		osDelay(1);
	}
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

}


//将系统参数备份到指定的存储区中
//1。成功   0 失败

#if 0
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
#endif 

#if 0 //调整之前最新
unsigned char BackupSysPara(stStorageData_Def *s)
{
    unsigned char i;
	  stStorageData_Def  t;
		while(xSemaphoreTake( xSemaphore, portMAX_DELAY ) != pdTRUE)
		{
		  osDelay(1);
		}
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
		
} 
#endif

//为防止更新时刚擦除就断电，先写到新地址再擦除老地址，以免整个SPI数据区无效，如果新地址写入没完成断电或者老地址为擦除就断电，则本次更新不成功，依然是上次的数据，比所有数据丢失要强！
unsigned char BackupSysPara(stStorageData_Def *s)
{
    unsigned char i;
	  stStorageData_Def  t;
		while(xSemaphoreTake( xSemaphore, portMAX_DELAY ) != pdTRUE)
		{
		  osDelay(1);
		}
		{
			for(i=0;i<SYS_PARA_CYCLE_TIMES;i++)
			{ 
			ReadFlash((unsigned char *)&t,sizeof(stStorageData_Def),i*SYS_PARA_SIZE);
			if(t.flag==SYS_PARA_FLAG)
				break;
			}
			if(i>=(SYS_PARA_CYCLE_TIMES-1))
			{
				i=0;	
				SectorErase(0);
				FlashProgram((unsigned char *)s,sizeof(stStorageData_Def),i*SYS_PARA_SIZE);
				SectorErase(0x1000);
			}
			else if(i<SYS_PARA_CYCLE_TIMES/2)
			{
				i=i+SYS_PARA_CYCLE_TIMES/2;
//				SectorErase(0x1000);
				FlashProgram((unsigned char *)s,sizeof(stStorageData_Def),i*SYS_PARA_SIZE);
				SectorErase(0x0);
			}
			else if(i<SYS_PARA_CYCLE_TIMES-1)
			{
				i=i-SYS_PARA_CYCLE_TIMES/2+1;
//				SectorErase(0);
				FlashProgram((unsigned char *)s,sizeof(stStorageData_Def),i*SYS_PARA_SIZE);
				SectorErase(0x1000);
			}
			
			xSemaphoreGive( xSemaphore );
			return 1;
	 }
		
} 

//从数据存储区域读取历史记录
//1。成功   0 失败
#if 0
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
#endif

unsigned char ReadSnoreData(SnoreData_def *p)
{   stStorageData_Def *s;
	
	   
	while(xSemaphoreTake( xSemaphore, portMAX_DELAY ) != pdTRUE)
	{
		osDelay(1);
	}
	s=&gStorageData;
	{
		if(s->SPIFlashAddress==s->SPIFlashOutAddress)
		{    xSemaphoreGive( xSemaphore );
		     return 0; //没有可以读取的数据
		}
//		ReadFlash((unsigned char *)p,SNORE_DATA_SIZE,s->SPIFlashOutAddress*SNORE_DATA_SIZE+SNORE_DATA_ADDRESS);
		ReadFlash((unsigned char *)p,sizeof(SnoreData_def),s->SPIFlashOutAddress*SNORE_DATA_SIZE+SNORE_DATA_ADDRESS);
		if(s->SPIFlashOutAddress>=(SNORE_DATA_CNT-1))
	   s->SPIFlashOutAddress=0;
	  else s->SPIFlashOutAddress++;
		xSemaphoreGive( xSemaphore );
		BackupSysPara(s);
    return 1;
	}
	
}



//将数据写入存储区域中
//1。成功   0 失败

#if 0
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
#endif


unsigned char WriteSnoreData(SnoreData_def *p)
{   stStorageData_Def *s;
	 
	while(xSemaphoreTake( xSemaphore, portMAX_DELAY ) != pdTRUE)
	{
		osDelay(1);
	}
	 s=&gStorageData;
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
//			FlashProgram((unsigned char *)p,SNORE_DATA_SIZE,s->SPIFlashAddress*SNORE_DATA_SIZE+SNORE_DATA_ADDRESS);
			FlashProgram((unsigned char *)p,sizeof(SnoreData_def),s->SPIFlashAddress*SNORE_DATA_SIZE+SNORE_DATA_ADDRESS);
			s->SPIFlashAddress++;
			if(s->SPIFlashAddress>= SNORE_DATA_CNT)
			{
				s->SPIFlashAddress = 0;
			}	
		xSemaphoreGive( xSemaphore );
		 BackupSysPara(s);	
	
    return 1;
		}
//	return 0;
}

//获取睡眠数据记录数量
#if 0
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
#endif


unsigned short GetSnoreDataCnt(void)  
{   stStorageData_Def *s;
	  unsigned short tt=0;
	  
	  while(xSemaphoreTake( xSemaphore, portMAX_DELAY ) != pdTRUE)
	  {
		osDelay(1);
	  }
	  s=&gStorageData;	  
		if(s->SPIFlashAddress==s->SPIFlashOutAddress)
		{	
			xSemaphoreGive( xSemaphore );
			return 0 ;
		}
    if(s->SPIFlashAddress>s->SPIFlashOutAddress)
    {
			tt=s->SPIFlashAddress-s->SPIFlashOutAddress;
			xSemaphoreGive( xSemaphore );
			return tt;
	  } 
		tt = SNORE_DATA_CNT+s->SPIFlashAddress-s->SPIFlashOutAddress;
		xSemaphoreGive( xSemaphore );
		return tt;
}


int SnoreLibWR(float * data,unsigned int len,unsigned char wr)
{  int offset=0,i;
	 unsigned char *pt;
  if((len*4) > PRIVATE_SNORE_LIB_MAX_SIZE_64K)
		return 0;
	pt=(unsigned char *)data;
	
	while(xSemaphoreTake( xSemaphore, portMAX_DELAY ) != pdTRUE)
	{
		osDelay(1);
	}
	{
		if(wr)   //write
		{  
			offset=PRIVATE_SNORE_LIB_OFFSET_32K;
			BlockErase32K(offset);
			for(i=0;i<((len*4)/256);i++)
			{
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

//return 0;

}



#if 0
//从数据存储区域读写学习的鼾声数据,MFCC特征数据
//1。成功   0 失败

int SnoreLibWR(float * data,unsigned int len,unsigned char wr)
{  int offset=0,i;
	 unsigned char *pt;
  if((len*4) > PRIVATE_SNORE_LIB_MAX_SIZE_64k)
		return 0;
	pt=(unsigned char *)data;
	
	while(xSemaphoreTake( xSemaphore, portMAX_DELAY ) != pdTRUE)
	{
		osDelay(1);
	}
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

//return 0;

}
#endif

/*

unsigned char RTC_test[682][6];

unsigned char Read_Backup_RTC_test(void)
{
	int offset=0,i,j;
	unsigned char tempbuffer[4];
	unsigned char date_time_buffer1[6],date_time_buffer2[6],date_time_buffer3[6],date_time_buffer_temp[8];
	uint8_t cmpok=0;
	while(xSemaphoreTake( xSemaphore, portMAX_DELAY ) != pdTRUE)
	{
		osDelay(1);
	}
	j=0;
	for(offset=0;offset<4096;offset=offset+8)
	{
		  ReadFlash((unsigned char *)&date_time_buffer_temp,8,DATE_TIME_OFFSET+offset);
		  for(i=0;i<6;i++)		
        RTC_test[j][i]	= date_time_buffer_temp[i+2];	
			j++;
			
	}
	j=0;
	for(offset=0;offset<4096;offset=offset+8)
	{
		  ReadFlash((unsigned char *)&date_time_buffer_temp,8,DATE_TIME_OFFSET+offset+0x1000);
		  for(i=0;i<6;i++)		
        RTC_test[j][i]	= date_time_buffer_temp[i+2];	
			j++;
			
	}
	j=0;
	for(offset=0;offset<4096;offset=offset+8)
	{
		  ReadFlash((unsigned char *)&date_time_buffer_temp,8,DATE_TIME_OFFSET+offset+0x2000);
		  for(i=0;i<6;i++)		
        RTC_test[j][i]	= date_time_buffer_temp[i+2];	
			j++;
			
	}
	xSemaphoreGive( xSemaphore );
	return 0;
	
}

*/

unsigned char Read_Backup_RTC(RTC_DateTypeDef *date,RTC_TimeTypeDef *time)
{
	int offset=0,i;	
	unsigned char date_time_buffer1[6],date_time_buffer2[6],date_time_buffer3[6],date_time_buffer_temp[8];
	uint8_t cmpok=0;
	while(xSemaphoreTake( xSemaphore, portMAX_DELAY ) != pdTRUE)
	{
		osDelay(1);
	}
	ReadFlash((unsigned char *)&date_time_buffer_temp,8,DATE_TIME_OFFSET);
	if(date_time_buffer_temp[0]==0x55 || date_time_buffer_temp[1]==0xAA)
	{
		for(offset=0;offset<4096;offset=offset+8)
		{
		  ReadFlash((unsigned char *)&date_time_buffer_temp,8,DATE_TIME_OFFSET+offset);			
			if(date_time_buffer_temp[0]!=0xFF)
			{
				for(i=0;i<6;i++)
				  date_time_buffer1[i] = date_time_buffer_temp[i+2];
			}
			else
				break;
		}
		//读取备份2
		ReadFlash((unsigned char *)&date_time_buffer_temp,8,DATE_TIME_OFFSET+0x1000);
		if(date_time_buffer_temp[0]==0x55 || date_time_buffer_temp[1]==0xAA)
		{
			for(offset=0;offset<4096;offset=offset+8)
			{
				ReadFlash((unsigned char *)&date_time_buffer_temp,8,DATE_TIME_OFFSET+0x1000+offset);			
				if(date_time_buffer_temp[0]!=0xFF)
				{
					for(i=0;i<6;i++)
						date_time_buffer2[i] = date_time_buffer_temp[i+2];
				}
				else
					break;
			}
			//读取备份3
			ReadFlash((unsigned char *)&date_time_buffer_temp,8,DATE_TIME_OFFSET+0x2000);
			if(date_time_buffer_temp[0]==0x55 || date_time_buffer_temp[1]==0xAA)
			{
				for(offset=0;offset<4096;offset=offset+8)
				{
					ReadFlash((unsigned char *)&date_time_buffer_temp,8,DATE_TIME_OFFSET+0x2000+offset);			
					if(date_time_buffer_temp[0]!=0xFF)
					{
						for(i=0;i<6;i++)
							date_time_buffer3[i] = date_time_buffer_temp[i+2];
					}
					else
						break;
				}
				//比较三个备份的数据，两者相同则为有效
				cmpok=1;
				for(i=0;i<6;i++)
				{
					if(date_time_buffer1[i]!=date_time_buffer2[i])
					{
						cmpok=0;
						break;
					}
				}
				if(cmpok==1)
				{
					date->Year = date_time_buffer1[0];
					date->Month = date_time_buffer1[1];
					date->Date = date_time_buffer1[2];
					time->Hours = date_time_buffer1[3];
					time->Minutes = date_time_buffer1[4];
					time->Seconds = date_time_buffer1[5];
					xSemaphoreGive( xSemaphore );
					return 1;
				}
				else
				{
					cmpok=1;
					for(i=0;i<6;i++)
					{
						if(date_time_buffer1[i]!=date_time_buffer3[i])
						{
							cmpok=0;
							break;
						}
					}
					if(cmpok==1)
					{
						date->Year = date_time_buffer1[0];
						date->Month = date_time_buffer1[1];
						date->Date = date_time_buffer1[2];
						time->Hours = date_time_buffer1[3];
						time->Minutes = date_time_buffer1[4];
						time->Seconds = date_time_buffer1[5];
						xSemaphoreGive( xSemaphore );
						return 1;
					}
					else
					{
						cmpok=1;
						for(i=0;i<6;i++)
						{
							if(date_time_buffer2[i]!=date_time_buffer3[i])
							{
								cmpok=0;
								break;
							}
						}
						if(cmpok==1)
						{
							date->Year = date_time_buffer2[0];
							date->Month = date_time_buffer2[1];
							date->Date = date_time_buffer2[2];
							time->Hours = date_time_buffer2[3];
							time->Minutes = date_time_buffer2[4];
							time->Seconds = date_time_buffer2[5];
							xSemaphoreGive( xSemaphore );
							return 1;
						}						
					}					
				}
			}
		}	
	}	
	xSemaphoreGive( xSemaphore );
	return 0;
}

unsigned char Backup_RTC(RTC_DateTypeDef *date,RTC_TimeTypeDef *time)
{
	int offset=0,i;
	unsigned char date_time_buffer[8];
	while(xSemaphoreTake( xSemaphore, portMAX_DELAY ) != pdTRUE)
	{
		osDelay(1);
	}
	//备份1
	ReadFlash((unsigned char *)&date_time_buffer,8,DATE_TIME_OFFSET);
	if(date_time_buffer[0]==0x55 || date_time_buffer[1]==0xAA)
	{
		for(offset=8;offset<4096;offset=offset+8)
		{
		  ReadFlash((unsigned char *)&date_time_buffer,8,DATE_TIME_OFFSET+offset);
			if(date_time_buffer[0]==0xFF)
				break;
		}
		if(offset<4096)
		{
			date_time_buffer[0] = 0x55;
			date_time_buffer[1] = 0xAA;	
			date_time_buffer[2] = date->Year;
			date_time_buffer[3] = date->Month;
			date_time_buffer[4] = date->Date;
			date_time_buffer[5] = time->Hours;
			date_time_buffer[6] = time->Minutes;
			date_time_buffer[7] = time->Seconds;
			FlashProgram((unsigned char *)date_time_buffer,8,DATE_TIME_OFFSET+offset);			
		}
		else
		{
			SectorErase(DATE_TIME_OFFSET);
			date_time_buffer[0] = 0x55;
			date_time_buffer[1] = 0xAA;		
			date_time_buffer[2] = date->Year;
			date_time_buffer[3] = date->Month;
			date_time_buffer[4] = date->Date;
			date_time_buffer[5] = time->Hours;
			date_time_buffer[6] = time->Minutes;
			date_time_buffer[7] = time->Seconds;
			FlashProgram((unsigned char *)date_time_buffer,8,DATE_TIME_OFFSET);		
		}
		
	}
	else
	{
		SectorErase(DATE_TIME_OFFSET);
		date_time_buffer[0] = 0x55;
		date_time_buffer[1] = 0xAA;		
		date_time_buffer[2] = date->Year;
		date_time_buffer[3] = date->Month;
		date_time_buffer[4] = date->Date;
		date_time_buffer[5] = time->Hours;
		date_time_buffer[6] = time->Minutes;
		date_time_buffer[7] = time->Seconds;
		FlashProgram((unsigned char *)date_time_buffer,8,DATE_TIME_OFFSET);	
	}
	
	
	//备份2
		ReadFlash((unsigned char *)&date_time_buffer,8,DATE_TIME_OFFSET+0x1000);
	if(date_time_buffer[0]==0x55 || date_time_buffer[1]==0xAA)
	{
		for(offset=8;offset<4096;offset=offset+8)
		{
		  ReadFlash((unsigned char *)&date_time_buffer,8,DATE_TIME_OFFSET+0x1000+offset);
			if(date_time_buffer[0]==0xFF)
				break;
		}
		if(offset<4096)
		{
			date_time_buffer[0] = 0x55;
			date_time_buffer[1] = 0xAA;	
			date_time_buffer[2] = date->Year;
			date_time_buffer[3] = date->Month;
			date_time_buffer[4] = date->Date;
			date_time_buffer[5] = time->Hours;
			date_time_buffer[6] = time->Minutes;
			date_time_buffer[7] = time->Seconds;
			FlashProgram((unsigned char *)date_time_buffer,8,DATE_TIME_OFFSET+0x1000+offset);			
		}
		else
		{
			SectorErase(DATE_TIME_OFFSET+0x1000);
			date_time_buffer[0] = 0x55;
			date_time_buffer[1] = 0xAA;		
			date_time_buffer[2] = date->Year;
			date_time_buffer[3] = date->Month;
			date_time_buffer[4] = date->Date;
			date_time_buffer[5] = time->Hours;
			date_time_buffer[6] = time->Minutes;
			date_time_buffer[7] = time->Seconds;
			FlashProgram((unsigned char *)date_time_buffer,8,DATE_TIME_OFFSET+0x1000);		
		}
		
	}
	else
	{
		SectorErase(DATE_TIME_OFFSET+0x1000);
		date_time_buffer[0] = 0x55;
		date_time_buffer[1] = 0xAA;		
		date_time_buffer[2] = date->Year;
		date_time_buffer[3] = date->Month;
		date_time_buffer[4] = date->Date;
		date_time_buffer[5] = time->Hours;
		date_time_buffer[6] = time->Minutes;
		date_time_buffer[7] = time->Seconds;
		FlashProgram((unsigned char *)date_time_buffer,8,DATE_TIME_OFFSET+0x1000);	
	}
	
	//备份3
		ReadFlash((unsigned char *)&date_time_buffer,8,DATE_TIME_OFFSET+0x2000);
	if(date_time_buffer[0]==0x55 || date_time_buffer[1]==0xAA)
	{
		for(offset=8;offset<4096;offset=offset+8)
		{
		  ReadFlash((unsigned char *)&date_time_buffer,8,DATE_TIME_OFFSET+0x2000+offset);
			if(date_time_buffer[0]==0xFF)
				break;
		}
		if(offset<4096)
		{
			date_time_buffer[0] = 0x55;
			date_time_buffer[1] = 0xAA;	
			date_time_buffer[2] = date->Year;
			date_time_buffer[3] = date->Month;
			date_time_buffer[4] = date->Date;
			date_time_buffer[5] = time->Hours;
			date_time_buffer[6] = time->Minutes;
			date_time_buffer[7] = time->Seconds;
			FlashProgram((unsigned char *)date_time_buffer,8,DATE_TIME_OFFSET+0x2000+offset);			
		}
		else
		{
			SectorErase(DATE_TIME_OFFSET+0x2000);
			date_time_buffer[0] = 0x55;
			date_time_buffer[1] = 0xAA;		
			date_time_buffer[2] = date->Year;
			date_time_buffer[3] = date->Month;
			date_time_buffer[4] = date->Date;
			date_time_buffer[5] = time->Hours;
			date_time_buffer[6] = time->Minutes;
			date_time_buffer[7] = time->Seconds;
			FlashProgram((unsigned char *)date_time_buffer,8,DATE_TIME_OFFSET+0x2000);		
		}
		
	}
	else
	{
		SectorErase(DATE_TIME_OFFSET+0x2000);
		date_time_buffer[0] = 0x55;
		date_time_buffer[1] = 0xAA;		
		date_time_buffer[2] = date->Year;
		date_time_buffer[3] = date->Month;
		date_time_buffer[4] = date->Date;
		date_time_buffer[5] = time->Hours;
		date_time_buffer[6] = time->Minutes;
		date_time_buffer[7] = time->Seconds;
		FlashProgram((unsigned char *)date_time_buffer,8,DATE_TIME_OFFSET+0x2000);	
	}
	
	
	xSemaphoreGive( xSemaphore );
	return 1;
	
}

void spitest(void)
{
	unsigned char tempbuffer[4];
	ReadFlash((unsigned char *)&tempbuffer,4,DATE_TIME_OFFSET);
	SectorErase(DATE_TIME_OFFSET);
	ReadFlash((unsigned char *)&tempbuffer,4,DATE_TIME_OFFSET);
}


//从数据存储区域读写学习的鼾声数据
//1。成功   0 失败

int PositionInitWR(unsigned char * pt,unsigned int len,unsigned char wr)
{  int offset=0,i;

  if(len > 4096)
		return 0;
	
	while(xSemaphoreTake( xSemaphore, portMAX_DELAY ) != pdTRUE)
	{
		osDelay(1);
	}
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

//return 0;

}
