#include "tuwan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"
#include "I2s.h"
#include "pdm_filter.h"
#include "position.h"


QDef  SnoreQ;
extern I2S_HandleTypeDef hi2s2;
//extern osMessageQId PcmQHandle;
unsigned short InternalBuffer[INTERNAL_BUFF_SIZE];  //1ms的数据
unsigned short RecBuf[PCM_OUT_SIZE],volume,error=0;  //转换成 //1ms的音频
unsigned short RecTemp[8];
unsigned char StartSend=0,SnoreDetectTime=0,CntTime=0;
PDMFilter_InitStruct   Filter;
extern UART_HandleTypeDef huart5;
unsigned long RecordTime=0;
unsigned int recordTick=0;
extern unsigned char StartAllAction,StartCountAntSnoreTime;
unsigned short kpi=0;
extern ProcessControl_def ProcessControl;


extern short MIC_data_for_Walve_debug[MIC_data_for_Walve_debug_length];//保存10秒MIC 8K采样单声道声音数据
extern uint16_t MIC_data_for_Walve_debug_pointer;
extern uint8_t MIC_data_save_enable;
extern short MIC_data_MAX[MIC_point_repeat_times], MIC_data_Min[MIC_point_repeat_times];
extern uint8_t MIC_point_repeat_step;


void StartI2S_2(void)
{

	HAL_I2S_Receive_DMA(&hi2s2,InternalBuffer,INTERNAL_BUFF_SIZE);

}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{   //portBASE_TYPE xStatus;
	  int i;
	
		if(hi2s==&hi2s2)
		{    /*
			    #ifndef USE_RTOS_Q
		      if(inputDataToQDef((uint8_t *)InternalBuffer,INTERNAL_BUFF_SIZE*2,&PDMData)<0)
					
					{
				     error=1;
					
					}
					#else
		      xStatus = xQueueSendToBackFromISR(PcmQHandle,(uint8_t *)&InternalBuffer, 0 );
					if( xStatus != pdPASS )
					{
							 DebugLog("PCM in Q error!");
					}
					#endif
			  
					*/
					//  StartI2S_2();
			    
			    for(i=0;i<INTERNAL_BUFF_SIZE;i++)
			    {
					 InternalBuffer[i] = HTONS(InternalBuffer[i]);
            					
					
					}
					CalculatePCM((uint8_t *)&InternalBuffer);
			
		
		
		}
}

void InitPDMLib(void)
{ RCC->AHB1ENR |= RCC_AHB1ENR_CRCEN;
	Filter.LP_HZ=4000;
	Filter.HP_HZ=10;
	Filter.Fs=16000;
	Filter.In_MicChannels=1;
	Filter.Out_MicChannels=1;
	PDM_Filter_Init((PDMFilter_InitStruct*)&Filter);
	volume = 1;
}

void CalculatePCM(unsigned char *Buffer)
{   
//	  portBASE_TYPE xHigherPriorityTaskWoken; 
    uint8_t i=0;
	  short j;
	  	volume = 30;
		PDM_Filter_64_LSB((uint8_t *)Buffer, (uint16_t *)RecBuf, volume ,(PDMFilter_InitStruct *)&Filter);
	#if 1
		RecTemp[0]=RecBuf[1];
		RecTemp[1]=RecBuf[3];
		RecTemp[2]=RecBuf[5];
		RecTemp[3]=RecBuf[7];
		RecTemp[4]=RecBuf[9];
		RecTemp[5]=RecBuf[11];
		RecTemp[6]=RecBuf[13];
		RecTemp[7]=RecBuf[15];
	#else 
		unsigned char *pt;
	pt=(unsigned char *)RecTemp;
	pt[0]=kpi++;
	pt[1]=kpi++;
	pt[2]=kpi++;
	pt[3]=kpi++;
	pt[4]=kpi++;
	pt[5]=kpi++;
	pt[6]=kpi++;
	pt[7]=kpi++;
	pt[8]=kpi++;
	pt[9]=kpi++;
	pt[10]=kpi++;
	pt[11]=kpi++;
	pt[12]=kpi++;
	pt[13]=kpi++;
	pt[14]=kpi++;
	pt[15]=kpi++;
	#endif
	// if(StartSend)
	// {
	
	//为调试检测气阀开关声音
	if(MIC_data_save_enable==1)
	{
	  for(i=0;i<8;i++)
	   {
		  
		  MIC_data_for_Walve_debug[MIC_data_for_Walve_debug_pointer]=RecTemp[i];
			j=RecTemp[i];
			if(MIC_data_MAX[MIC_point_repeat_step]<j)
				MIC_data_MAX[MIC_point_repeat_step]=j;
			if(MIC_data_Min[MIC_point_repeat_step]>j)
				MIC_data_Min[MIC_point_repeat_step]=j;
		  MIC_data_for_Walve_debug_pointer++;
		  if(MIC_data_for_Walve_debug_pointer>=MIC_data_for_Walve_debug_length)
			  MIC_data_for_Walve_debug_pointer=0;
	   }		 
  }
	
	memcpy((unsigned char *)&SnoreBuf.SnoreBuffer[SnoreBuf.num],RecTemp,PCM_OUT_SIZE);//copy 16个字节
	SnoreBuf.num+=8;
    	if(SnoreBuf.num>=256)	
	{  
			//xQueueSendFromISR( SnoreBufferQHandle, SnoreBuf.SnoreBuffer, &xHigherPriorityTaskWoken );  
		inputDataToQDef((unsigned char *)SnoreBuf.SnoreBuffer,256*2,&SnoreQ);
		SnoreBuf.num=0;
		if(gStorageData.Standby==0)    //(StartCountAntSnoreTime)
		{
			if(PillowControl.SnoreOn==true)
				CntTime++;
			else StartCountAntSnoreTime=0;
			if(CntTime>=31)
			{	
				CntTime=0;
			}
		}
		else CntTime=0;
	}
	#ifdef SEND_PCM 
		 //往上位机发送PCM的数据
		if(StartSend)
		{
		//	if(RecordTime<(16000*30))
			{
				uart_send((unsigned char *)RecTemp,PCM_OUT_SIZE);
			//	RecordTime+=PCM_OUT_SIZE;
			}
		}
	#endif
	
		
	//}

}

/*
void DoPDMFilter(void)
{
	unsigned short Buffer[INTERNAL_BUFF_SIZE];  //1ms的数据
	portBASE_TYPE xStatus;
	#ifndef USE_RTOS_Q
		if(PDMData.in!=PDMData.out)
	#else
	if( uxQueueMessagesWaiting( PcmQHandle ) != 0 )
	#endif
	{
	#ifndef USE_RTOS_Q
		outputDataFromQDef((unsigned char *)Buffer,&PDMData);
	#else	
		
		xStatus = xQueueReceive( PcmQHandle, &Buffer, 0 );
		if( xStatus == pdPASS )
		{
			//	DebugLog( "Rec PCM Q ERROR= ", lReceivedValue );
		}
		else
		{
				DebugLog( "Rec PCMQ ERROR!\r\n" );
			  return ;
		}
		
   #endif
		CalculatePCM((uint8_t *) &Buffer);
	
		
	}
}
*/

