#include "upgrade.h"
#include "flash.h"
#include <string.h>
#include "stm32f4xx_hal.h"
#include "../Inc/flash.h"

unsigned int t1=0;
typedef void (*iapfun)(void); 
iapfun jump2app;
  //跳转到应用程序段
  //appxaddr:用户代码的起始地址
  void iap_load_app(unsigned int  appxaddr)
  {
      if(((*(unsigned int*)appxaddr)&0x2FFE0000)==0x20000000) //检查栈顶指针是否合法
			{ __set_PRIMASK(1);
				SCB->VTOR=appxaddr;
				jump2app = (iapfun)*(unsigned int *)(appxaddr+4);//用户代码区第二个字为程序开始地址
				__set_MSP(*(volatile unsigned int*) appxaddr);//初始化APP堆栈指针，用户区的第一个字用于存放栈的指针
				jump2app(); //跳转到APP，执行复位中断程序
			}
  }





unsigned char JumpToImage(unsigned char type)
{
	if(type==IMAGEA)
	{
		iap_load_app(IMAGE_A_ADDRESS);
	
	}
	else if(type==IMAGEB)
	{
		iap_load_app(IMAGE_B_ADDRESS);
	
	}
	else return 0;

return 1;
}

void RunLed(unsigned char s)
{
	if(s)
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOA,GPIO_PIN_0, GPIO_PIN_RESET);
}


void BlueToolLed(unsigned char s)
{
	if(s)
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3, GPIO_PIN_RESET);
}

void OnOffLed(unsigned char s)
{
	if(s)
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0, GPIO_PIN_RESET);
}
void PowerLed(unsigned char s)
{
	if(s)
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1, GPIO_PIN_RESET);
}

void BootProcess(void)
{
	if(GetSystemParameter()==1) //是否有可以正常启动的IMAGE
	{
		  if(g_systemParameter.upgradeType==g_systemParameter.imgAver)//检查当前要运行哪一个版本的ROM
			{
				   JumpToImage(IMAGEA);
			}
			else if(g_systemParameter.upgradeType==g_systemParameter.imgBver)
			{
					 JumpToImage(IMAGEB);
			
			}
		  JumpToImage(IMAGEA);
			//g_systemParameter.upgradeType=  我的测试代码
	}
	if(GetSystemParameter1()==1) //是否有可以正常启动的IMAGE
	{
		  if(g_systemParameter1.upgradeType==g_systemParameter1.imgAver)//检查当前要运行哪一个版本的ROM
			{
				   JumpToImage(IMAGEA);
			}
			else if(g_systemParameter1.upgradeType==g_systemParameter1.imgBver)
			{
					 JumpToImage(IMAGEB);
			
			}
		  JumpToImage(IMAGEA);
			//g_systemParameter.upgradeType=  我的测试代码
	}
	
	else  
	{
		while(1)
		{
			RunLed(0);
		  BlueToolLed(0);
		  OnOffLed(0);
		  PowerLed(0);
		  t1=0;
			while(t1<1000);
	    RunLed(1);
		  BlueToolLed(1);
		  OnOffLed(1);
		  PowerLed(1);
			t1=0;
			while(t1<1000);
	
		}
	
	}




}



