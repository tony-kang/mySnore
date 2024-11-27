
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "stdbool.h"
#include "I2s.h"
#include "snore.h"
#include "InputOutput.h"
#include "tuwan.h"
void uart3Send(unsigned char *data,unsigned int len);
unsigned char g_InhalePumpStatus=0,g_ExhalePumpStatus=0;
void RunLed(unsigned char s)
{
	if(s)
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1, GPIO_PIN_RESET);
}

void ERP_ON(void)
{
HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3, GPIO_PIN_SET);
}
void BlueToolthEnabled(unsigned char s)
{
#if defined(TUWAN_PILLOW_3) || defined(TUWAN_MAT)  //������ͷ
	if(s==0)
	{HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3, GPIO_PIN_SET);
	 HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4, GPIO_PIN_SET);
	}
	else{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3, GPIO_PIN_RESET); //����ģ���ϵ�
	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4, GPIO_PIN_RESET);
	}   //ERP�ϵ�
#else   //ֹ��̺

	if(s==0)
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3, GPIO_PIN_RESET);
	}
	else{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3, GPIO_PIN_RESET);//����ģ���ϵ�
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3, GPIO_PIN_SET); //ERP�ϵ�
	}

#endif
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

GPIO_PinState GetPowerKey(void)
{
			return HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
}
GPIO_PinState GetBlueToothKey(unsigned char key)
{
			return  HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2);	
}
void InhalePump(unsigned char s)
{
	//if(gStorageData.StudyMode)
	//{
	//	return;
	//}
    if(s==GPIO_PIN_RESET)
        s=GPIO_PIN_SET;
    else s=GPIO_PIN_RESET;
	#ifdef TUWAN_BLANKET
	if(s)
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_RESET);
	
	#else
	
	if(s)
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8, GPIO_PIN_RESET);
	#endif
    g_InhalePumpStatus=s;
}
void ExhalePump(unsigned char s)
{
	
	 if(s==GPIO_PIN_RESET)	
	 s=GPIO_PIN_SET;
	else s=GPIO_PIN_RESET;
	
	 
	 #ifdef TUWAN_BLANKET
	if(s)
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13, GPIO_PIN_RESET);
  g_ExhalePumpStatus=s;
	
	 #else
	 return ;
	 #endif
}

void ValueDriver(unsigned char value,GPIO_PinState s)
{

	if(s==GPIO_PIN_RESET)	
	 s=GPIO_PIN_SET;
	else s=GPIO_PIN_RESET;
#ifdef TUWAN_BLANKET
		switch(value)
		{
			case 1:   //ͷ���ұ�1������������
				//	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15, s);
				//	break;
			case 2:    //ͷ���ұ�2������������
				//HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7, s);
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15, s);
				break;
			case 3:   //ͷ�����1������������
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9, s);
				break;
			case 4:  //ͷ�����2������������
				//HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9, s);
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8, s);
				break;
			case 5:  //�ұ�����
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8, s);
				break;
			case 6:   //�������
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9, s);
				break;
			case 7:
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10, s);
				break;
		  default:
				break;
		}		
#else
		switch(value)
		{
			case 1:   //ͷ���ұ�1������������
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10, s);
				break;
			case 2:    //ͷ���ұ�2������������
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9, s);
				break;
			case 3:   //ͷ�����1������������
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8, s);
				break;
			case 4:  //ͷ�����2������������
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9, s);
				break;
		  default:
				break;
		}		

#endif		
}
void HeadLeft(GPIO_PinState s)
{
    ValueDriver(1,s);
}
void HeadRight(GPIO_PinState s)
{
    ValueDriver(2,s);
}
void BodyLeft(GPIO_PinState s)
{
    ValueDriver(3,s);
}
void BodyRigth(GPIO_PinState s)
{
    ValueDriver(4,s);
}



void WrokingLed(unsigned char type,unsigned char state)
{
		if(type==RED_LED)
		{
			OnOffLed(state);
			
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4, 0);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5, 0);
		}
		else if(type==BLUE_LED_LOW)
		{
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4, state);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5, 0);
			OnOffLed(0);
		
		}
		else if(type==BLUE_LED_HIGH)
		{
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5, state);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4, state);
				OnOffLed(0);
		}



}




