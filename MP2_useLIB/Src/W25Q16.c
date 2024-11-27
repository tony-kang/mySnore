/****************************************************************    
    ģ������(Filename):       w25q16.c  
    ��Ŀ����(Projectname):       
    �汾��(Version):	      
    ��������(Date):                  
    ����(Author):             
    ��������(Description):    
    ����˵��(Others):          
    �޸ļ�¼(History):
****************************************************************/
#include <stdio.h> /* for memcpy() */
#include "tuwan.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "W25Q16.h"



//extern DMA_HandleTypeDef hdma_spi1_tx;
//extern DMA_HandleTypeDef hdma_spi1_rx;

#define SPIFLASH_START_ADDR 			0x000000
//#define SPIFLASH_START_BLOCK_ADDR 	0x000000
//#define SPIFLASH_START_SECTOR_ADDR 0x000000

#define SPIFLASH_BLOCK_SIZE 			0x10000
#define SPIFLASH_SECTOR_SIZE 			0x1000  //4096 Bytes
#define SPIFLASH_PAGE_SIZE 				0x100

#define SPIFLASH_CURENT_BLOCK 			0xAAAA
#define SPIFLASH_CURENT_SECTOR 		0xAA55

#define SPIFLASH_CURENT_PAGE 			0xAA
#define SPIFLASH_RECORD_USED 			0x55

#define SPIFLASH_REUSE_TIMES 			0x20//32//(SPIFLASH_SIZE - 1) / (SPIFLASH_BLOCK_LEN + 1)
#define SPIFLASH_BLOCK_REUSE_TIMES 	0x10//16//(SPIFLASH_BLOCK_SIZE - 1) / (SPIFLASH_SECTOR_LEN + 1)
#define SPIFLASH_SECTOR_REUSE_TIMES 	0x10//16//(SPIFLASH_SECTOR_SIZE - 1) / (SPIFLASH_PAGE_LEN + 1)
#define SPIFLASH_PAGE_REUSE_TIMES 	(SPIFLASH_PAGE_SIZE - 1) / (SPIFLASH_DATA_LEN + 1)//ÿһҳ�����ҳд��־λ��ÿһ������֮�������ݱ�־λ

#define SPIFLASH_PROTECTION 			0xFF
#define SPIFLASH_ERROR_OUT_OF_RANGE 	0x01
extern volatile uint32_t TimerTick;
void delay(unsigned char t) 
{
     osDelay(t);
}

unsigned char SPISendByte (unsigned char data)
{
	unsigned char d_read,d_send=data;
 if (HAL_SPI_TransmitReceive(&hspi1,&d_send,&d_read,1,0x1000)!=HAL_OK)
 d_read=0;

 return d_read;
}

unsigned char SPIReceiveByte(void)
{
    return 	SPISendByte (0xff);
}

/******************************************************************************/
/* �������ƣ�uint8 check_flash_busy(void)                                     */
/* �������������flash�Ƿ�æ                                                  */
/* ����˵������                                                               */
/* ��������: ���æ����1����æ����0                                           */
/******************************************************************************/ 
unsigned char Check_Flash_Busy(void)
{  	unsigned char busy_flag;
	  FLASH_SPI_CS_ENABLE();
  	SPISendByte(0x05);		//��״̬�Ĵ���
    //���޹����ݲ���SCK
  	busy_flag=SPIReceiveByte(); //���޹����ݲ���SCK
	  delay(10);
	  FLASH_SPI_CS_DISABLE();
  	if(busy_flag & 0x01)
      		busy_flag = 1;
  	else
      		busy_flag = 0;  
  	return busy_flag;  
  	
}

/******************************************************************************/
/* �������ƣ�uint8 check_flash_busy(void)                                     */
/* �������������flash�Ƿ�æ                                                  */
/* ����˵������                                                               */
/* ��������: ���æ����1����æ����0                                           */
/******************************************************************************/ 
unsigned char Check_Flash_WEL(void)
{
  	unsigned char busy_flag;
	  FLASH_SPI_CS_ENABLE();
  	SPISendByte(0x05);
  	busy_flag=SPIReceiveByte();
	  delay(10);
	  FLASH_SPI_CS_DISABLE();
  	if(busy_flag & 0x02)
      		busy_flag = 1;
  	else
      		busy_flag = 0;
  	return busy_flag;     	  
}

/********************************************
**�� �� ����ReadUniqueID
**������������ȡоƬΨһID�ţ��ܹ�8�ֽ�
**��    �������ݻ�����
 0xEF4015 W25Q16  2M byte
 0XEF4017 W25Q64  8M byte
 0XEF4018 W25Q128 16M byte
*********************************************/
int ReadUniqueID(unsigned char *databuffer)
{ 
  	unsigned char i;
	  FLASH_SPI_CS_ENABLE();
  	SPISendByte(0x9F);
  	for(i=0;i<3;i++)
  	{
		databuffer[i]=SPISendByte(0XFF);
  	}
		FLASH_SPI_CS_DISABLE();
		if(((databuffer[0]==0xef)&&(databuffer[1]==0x40)&&(databuffer[2]==0x15))||((databuffer[0]==0xef)&&(databuffer[1]==0x30)&&(databuffer[2]==0x15)))
			return 1;
		return 0;
}

/******************************************
** �� �� ����WriteEnable
** ����������дʹ��
** ��    ������
** ��    �أ���
*******************************************/
void WriteEnable(void)
{
	//static unsigned char i;               
	//��cs��ѡ��
	  FLASH_SPI_CS_ENABLE();
   	SPISendByte(0x06);
	  delay(10);
	  FLASH_SPI_CS_DISABLE();
}
/********************************************************
** �� �� ����SectorErase
** ������������������
** ��    ����������ַ
** ��    �أ���  
*********************************************************/
void SectorErase(unsigned int address)
{
	  unsigned short i=0;
	 
   	WriteEnable();					//дʹ��	
    FLASH_SPI_CS_ENABLE();	//��cs��ѡ��
   	SPISendByte(0x20);
	  SPISendByte((unsigned char)(address>>16));
   	SPISendByte((unsigned char)(address>>8));
   	SPISendByte((unsigned char)address);
   	delay(10); 	
	  FLASH_SPI_CS_DISABLE();
	  delay(10); 	
   	while(Check_Flash_Busy())
   	{
		i++;
		if(i>=200)
			break;
   	}
		
}      

/********************************************************
** �� �� ����BlockErase
** ���������������
** ��    �������ַ
** ��    �أ���  
*********************************************************/
void BlockErase64K(unsigned int address)
{
   	WriteEnable();					//дʹ��	              
    FLASH_SPI_CS_ENABLE();	//��cs��ѡ��
   	SPISendByte(0xD8);
	  SPISendByte((unsigned char)(address>>16));
   	SPISendByte((unsigned char)(address>>8));
   	SPISendByte((unsigned char)address);
   	delay(10);
		FLASH_SPI_CS_DISABLE();
}      
/********************************************************
** �� �� ����BlockErase
** ���������������
** ��    �������ַ
** ��    �أ���  
*********************************************************/
void BlockErase32K(unsigned int address)
{
	  unsigned short i=0;
   	WriteEnable();					//дʹ��	              
    FLASH_SPI_CS_ENABLE();	//��cs��ѡ��
   	SPISendByte(0x52);
	  SPISendByte((unsigned char)(address>>16));
   	SPISendByte((unsigned char)(address>>8));
   	SPISendByte((unsigned char)address);
   	delay(10);
		FLASH_SPI_CS_DISABLE();
	  TimerTick=0;
	  while(Check_Flash_Busy())
   	{
//		i++;
//		TimerTick++;	
		if(TimerTick>=900)
			break;
   	}
		i=TimerTick;
}      

/********************************************************
** �� �� ����ChipErase
** ����������оƬ����
** ��    ����������ַ
** ��    �أ���  
*********************************************************/
void ChipErase(void)
{
	static unsigned char i;
	
   	WriteEnable();				//дʹ��
	  FLASH_SPI_CS_ENABLE();          //��cs��ѡ��
   	SPISendByte(0xc7);
	  delay(10);
  	FLASH_SPI_CS_DISABLE();		                 //��cs�߲�ѡ��
   	while(Check_Flash_Busy())
   	{
	  //�ȴ�����д����� 
		i++;
		if(i>=255)
			break;
   	} 
  
}   



#if 1
/******************************************************************************
** �� �� ����FlashProgram                               
** ��������: ��ָ���ĵ�ַд��ָ���������ֽ�   ���һ��ֻ��д��һ��PAGE��256���ֽ�                                                              
** ��    �������ݻ��������ֽ�������ʼ��ַ                             
** ��    �أ���                       
******************************************************************************/ 
void FlashProgram(unsigned char *databuffer,unsigned int datasize,unsigned int address)
{ 
	unsigned short i=0,index;
   	WriteEnable();			         //дʹ��
   	delay(10);
    FLASH_SPI_CS_ENABLE();             //��cs��ѡ��
   	SPISendByte(0x02);
	  SPISendByte((unsigned char)(address>>16));
   	SPISendByte((unsigned char)(address>>8));
   	SPISendByte((unsigned char)(address)); 
	  for(index=0;index<datasize;index++)
   	{
	 	SPISendByte(*databuffer++);
	 
   	}
	  delay(10);
    FLASH_SPI_CS_DISABLE();		
    index=0;
		
//		TimerTick=0;
	  while(Check_Flash_WEL())
   	{
	
		index++;
		if(index>=300)
			break;
   	}
   	i=0;
   	
   	while(Check_Flash_Busy())
   	{
	
		i++;
		if(i>=300)
			break;
   	}
	 
}

#endif
/*****************************************************************
** �� �� ����ReadRFlash
** ������������ָ���ĵ�ַ��ȡָ���������ֽ�
** ��    �������ݻ��������ֽ�������ʼ��ַ
** ��    �أ��� 
******************************************************************/
void ReadFlash(unsigned char *databuffer,unsigned int datasize,unsigned int address)
{
	    static unsigned int index; 	
	    FLASH_SPI_CS_ENABLE();  
    	SPISendByte(0x03);
	    SPISendByte((unsigned char)(address>>16));
    	SPISendByte((unsigned char)(address>>8));
    	SPISendByte((unsigned char)(address));  
    	for(index=0;index<datasize;index++)
    	{
		    databuffer[index]=SPIReceiveByte();
    	} 
			index=1000;
			while(index--);
			//delay(10);  
      FLASH_SPI_CS_DISABLE();					
}    


