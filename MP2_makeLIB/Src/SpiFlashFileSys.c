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

//SPI Flash��������sectorΪ��С��λ���У�һ��sectorΪ4kB
//ϵͳ����ֻ��64���ֽڣ����ھ������£�����ѡ����ǰ8K�Ŀռ������ϱ仯λ�ñ��棬������д��һ���ط������𻵣�marked by zjp

//��ϵͳ�������������н���������
//1���ɹ�   0 ʧ��
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
 

//��ϵͳ�������ݵ�ָ���Ĵ洢����
//1���ɹ�   0 ʧ��
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
 
//�����ݴ洢�����ȡ��ʷ��¼
//1���ɹ�   0 ʧ��
unsigned char ReadSnoreData(SnoreData_def *p)
{   stStorageData_Def *s;
	
	   s=&gStorageData;
	if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE)
	{
		if(s->SPIFlashAddress==s->SPIFlashOutAddress)
		{    xSemaphoreGive( xSemaphore );
		     return 0; //û�п��Զ�ȡ������
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

//������д��洢������
//1���ɹ�   0 ʧ��
unsigned char WriteSnoreData(SnoreData_def *p)
{   stStorageData_Def *s;
	  s=&gStorageData;
	if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE)
	{
		//����Ƿ����Ѿ��洢�����ݣ��������������������Ƴ�һ��
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
	/////////////����Ƿ�洢��һ���µ�SECTOR���������Ҫ������������////////////////////////////
			if((s->SPIFlashAddress*SNORE_DATA_SIZE)%0x1000 == 0) //??SECTOR??0x000000~0x000FFF
			{

       			if(s->SPIFlashOutAddress>s->SPIFlashAddress) //��������λ���ǲ����ڴ�������������
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
		  ////д����Ӧ�����ݣ�Ȼ�󱸷����ݵ�ָ��
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
//��ȡ˯�����ݼ�¼����
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
//�����ݴ洢�����дѧϰ����������,MFCC��������
//1���ɹ�   0 ʧ��

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

//�����ݴ洢�����дѧϰ����������
//1���ɹ�   0 ʧ��

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
