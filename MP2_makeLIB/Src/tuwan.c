#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_pwr.h"
#include "cmsis_os.h"
#include "tuwan.h"
#include <stdio.h>
#include <stdlib.h>
 #include <string.h>
#include <SpiFlashFileSys.h>
#include <semphr.h>
#include <portmacro.h>
#include <projdefs.h>
#include "stdbool.h"
 #include "I2s.h"
#include "snore.h"
#include "InputOutput.h"
#include "W25Q16.h"
#include "cmsis_os.h"
#include "task.h"
#include "flash.h"
#include "position.h"

#define CHIP_ID_CNT 300
unsigned ShowModeOn=0;
void ADCstart3(void);
void ADCstart1(void);
void Error_Handler(void);
void MX_USB_HOST_Process(void);
RTC_DateTypeDef sdatestructure;
RTC_TimeTypeDef stimestructure;
bool U3Interrupt=false,U5Interrupt=false;
upgrade_control_def UpgradeCtrl;
unsigned int mac_Code;
unsigned char Uart2Buffer[10],GetChar=0,Uart3Buffer,TestMode=0,AppTest=0;
unsigned char Uart5RXBuffer[10],tempAirBag;
uint16_t    udirdata=0;
unsigned int StartLedCount=1000;
short tick=0,keyTimer=0;
NeedUpgradePara_def NeedUpgradePara;
//extern unsigned char buff2[20];
extern ProcessControl_def ProcessControl;
extern RTC_HandleTypeDef hrtc;
extern HeadPosition_def position;
//QDef  PositionQ;
unsigned char pcWriteBuffer[600],KeyCnt=0;
extern unsigned char CurrentRom,StartSend,StartAllAction,NeedReturnToDefault,step,ADok1,ADok3,needWork;
extern uint8_t AirSensor_position_checked_by_fastmode_flag;
extern unsigned short kpi;
extern SemaphoreHandle_t xSemaphore;
extern unsigned int AutoStartTimer;
void ResetValuePump(void);
void ExhaleAirHead(void);
unsigned char CalculateExhaleOver(bool CheckSnore);
void ExhaleAirBody(void);
extern short int PCMVol;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc3;
Read_Pos_def  ReadPos;
unsigned char Masaji=0;
extern struct SnoreConfig Config;
extern unsigned int PressureADC_ConvertedValue[6],InhaleADC_ConvertedValue[6],ExhaleADC_ConvertedValue[6];
SnoreData_def snoreData;
unsigned char   TestSendWaveData[24*2];

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin==GPIO_PIN_0)
    {
        HAL_Delay(20);/* ???????,???? */
        if (HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)==GPIO_PIN_RESET) {

        }
    }
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
}

#if 0
//==============================================
void TestAddSonreData(void)
{
    unsigned int i;
    unsigned char Date,Hours,Minutes;
    unsigned char Airbag1 = 0,sst = 0,mst = 0,lst = 0,j=0;

    Date=0;
    Hours=8;
    Minutes=60;
    
    for(i=0;i<SNORE_DATA_CNT;i++)
    {
        //WrokingLed(OFF_LED);

        Minutes = Minutes + 10;
        if(Minutes >= 60)
        {
            //WrokingLed(GREE_LED);

            Minutes = 0;
            Hours = Hours + 2;
            if(Hours >= 16)
            {
                Hours = 0;
                Date = Date + 1;
            }
        }

        Airbag1 = Airbag1 + 1;
        if(Airbag1 >= 16)
        {
            Airbag1 = 1;
        }

        if(j < 50)
        {
            sst = j*3;
            mst = 0;
            lst = 0;
        }
        else if(j < 100)
        {
            sst = 0;
            mst = j*3;
            lst = 0;
        }
        else if(j < 150)
        {
            sst = 0;
            mst = 0;
            lst = j*3;
        }

        j++;
        if(j>=150)
        {
            j = 0;
        }
            


        memset(&gStorageData.Snore,0,sizeof(SnoreData_def));
        gStorageData.Snore.UsedRecord=0x55aa;
        gStorageData.Snore.StudyMode = Date % 2;
        gStorageData.Snore.HeadPosion = 1;
//        gStorageData.Snore.ApneaTimes = 0;
				gStorageData.Snore.HeadMovingTimes = 0;
        gStorageData.Snore.AVGSnoreTime = 40;
        gStorageData.Snore.MaxDB = 60;
        gStorageData.Snore.Airbag1Times = Airbag1;
        gStorageData.Snore.SSnoreTimes = sst;
        gStorageData.Snore.MSnoreTimes = mst;
        gStorageData.Snore.LSnoreTimes = lst;

        gStorageData.Snore.StartTime[0]=18;
        gStorageData.Snore.StartTime[1]=6;
        gStorageData.Snore.StartTime[2]=Date;
        gStorageData.Snore.StartTime[3]=Hours;
        gStorageData.Snore.StartTime[4]=Minutes;

        //gStorageData.Snore.temp7 = (unsigned char)(i >> 8);
        //gStorageData.Snore.temp8 = (unsigned char)i;
        //uart_send((unsigned char *)&gStorageData.Snore,sizeof(SnoreData_def));

        WriteSnoreData(&gStorageData.Snore); //保存上次

        
        memset(&gStorageData.Snore,0,sizeof(SnoreData_def));
        gStorageData.Snore.UsedRecord=0x55aa;

        //osDelay(10);
    }    
    //WrokingLed(GREE_LED);
}


#endif




void EntryStandbyMode()
{

    //打开wakeup引脚，待机后来个上升沿就可以唤醒cpu了
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    //清除wakeup_flag,这里很重要，假如没有，就不能重复的让cpu唤醒再进入待机
    //即HAL_PWR_EnterSTANDBYMode()将不起作用！！
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    //cpu进入待机模式，NREST Pin，IWDG，RTC alarm，WakeUp引脚上升沿，都可以唤醒cpu
    HAL_PWR_EnterSTANDBYMode();


}

void MX_UART5_Init_Baudrate(unsigned int baudrate)
{

  huart5.Instance = UART5;
  huart5.Init.BaudRate = baudrate;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }

}
void gStorageDataResetDefault(void)
{
		  gStorageData.flag=SYS_PARA_FLAG;
		  gStorageData.SPIFlashAddress=0;
		  gStorageData.SPIFlashOutAddress=0;
	    gStorageData.ActionLevel=1;    //0:low 1:med 2:high
	gStorageData.SensorLevel=MID_CHECK; //1:高，2：中 3：低
		  gStorageData.StudyMode=0;
		  gStorageData.Standby=1;       //处于待机的状态
		  gStorageData.WorkDelay=0;
		  //gStorageData.InhaleOver=0;
		  gStorageData.ExhaleOver=0;
			//gStorageData.workTimeOn=0;
	    //gStorageData.workTime[0][0]=21;
			//gStorageData.workTime[0][1]=30;
			//gStorageData.workTime[0][2]=9;
			//gStorageData.workTime[0][3]=0;
	
			//gStorageData.workTime[1][0]=12;
			//gStorageData.workTime[1][1]=30;
			//gStorageData.workTime[1][2]=15;
			//gStorageData.workTime[1][3]=0;
		  memset(&gStorageData.Snore,0,32);
		  firstSnoreInit();
}
GPIO_PressState CheckKeyPressState(void)
{
	unsigned char tick=0,i=0;
	while(1){
        if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)==GPIO_PIN_RESET) {
            i++;
            if (i > 1) {
                break;
            }
        }
        else i=0;
		tick++;
		osDelay(100);
		if(tick>=20)
			return LONG_PRESS;
	}
	if(tick>=20)
		return LONG_PRESS;
	return SHORT_PRESS;
}


void InitStorage(void)
{  	unsigned char buf[4];
	  unsigned short tick;
	ReadUniqueID(buf);
   if(ReadSysPara(&gStorageData)==0)  //存储区域未被初始化过
	 {
			gStorageDataResetDefault();
			BackupSysPara(&gStorageData);
	 }
	 else if((gStorageData.SPIFlashAddress>512)||(gStorageData.SPIFlashOutAddress>512))
	 {
			gStorageDataResetDefault();
			BackupSysPara(&gStorageData);
	 }
    /*
	 else if((gStorageData.InhaleOver==0xffff)||(gStorageData.ExhaleOver==0xffff))
	 {
	 
	 	  gStorageDataResetDefault();
		  BackupSysPara(&gStorageData);
	 
	 }
	*/
	// if( gStorageData.Standby==0)  //正常工作状态,增加对数据的存储部份
	// {	
	//   
	 //   	  WriteSnoreData(&gStorageData.Snore); //保存上次
	//    	  gStorageData.Standby=1;
	// }
	//	OnOffLed(1);
  // BlueToolthEnabled(1);
//	WrokingLed(RED_LED,1);
	/* 
	#ifndef UPGRADE_TEST
	 tick=0;
		while(1)
		{
			if(GetPowerKey()==GPIO_PIN_SET)
			{
//						WrokingLed(BLUE_LED_HIGH,1);
					  CheckKeyPressState();
						break;
			}
			else if(tick>500) 	EntryStandbyMode();
			
			tick++;
			osDelay(10);
		}
		while(GetPowerKey()==GPIO_PIN_SET)
		osDelay(10);
		
		#endif
	*/ 
//		WrokingLed(BLUE_LED_HIGH,1);
		 StartLedCount=1000;
		 BlueToolthEnabled(1);//系统外设上电
//			gStorageData.Standby = 0;
//			gStorageData.Snore.UsedRecord = 0x55aa;
//			gStorageData.Snore.AVG_Snore = 0;
//			gStorageData.Snore.InterfereTimes = 0;
//			gStorageData.Snore.MAX_Snore = 0;

//			gStorageData.Snore.total_Snore = 0;
//			gStorageData.Snore.StopSnoreTime = 0;
//			gStorageData.Snore.StudyMode = gStorageData.StudyMode;
//			gStorageData.Snore.ContinuousSnore = 0;
//			gStorageData.Snore.StopSnoreTime = 0;
//			gStorageData.Snore.ApneaTimes = 0;
//			gStorageData.Snore.DoAntApneaTimes = 0;
//			gStorageData.Snore.SelfBodyMove = 0;
//
//			gStorageData.Snore.StartTime[0] = sdatestructure.Year;
//			gStorageData.Snore.StartTime[1] = sdatestructure.Month;
//			gStorageData.Snore.StartTime[2] = sdatestructure.Date;
//			gStorageData.Snore.StartTime[3] = stimestructure.Hours;
//			gStorageData.Snore.StartTime[4] = stimestructure.Minutes;
//
//			gStorageData.Snore.EndTime[0] = 0;
//			gStorageData.Snore.EndTime[1] = 0;
//			gStorageData.Snore.EndTime[2] = 0;
//			gStorageData.Snore.EndTime[3] = 0;
//			gStorageData.Snore.EndTime[4] = 0;

				gStorageData.Standby=1;
				gStorageData.WorkDelay=0;
				BackupSysPara(&gStorageData);
}

//unsigned int supportChipID[CHIP_ID_CNT]={0x24AC3EA9,0,0};
void Get_F407ChipID(void)
{
    unsigned int ChipUniqueID[3];  //地址从小到大，先放低字节，再放高字节：小端模式
	  ChipUniqueID[0]=*(unsigned int *)(0x1fff7a10);  
    ChipUniqueID[1]=*(unsigned int *)(0x1fff7a14);  
    ChipUniqueID[2]=*(unsigned int *)(0x1fff7a18); 
    mac_Code = (ChipUniqueID[0]>>1)+(ChipUniqueID[1]>>2)+(ChipUniqueID[2]>>3);  
}

void BCD2Int(unsigned char *time,int cnt)
{  
	int i;
	for(i = 0;i < cnt;i++)				// ?BCD???????
		time[i] = (time[i] >> 4)*10 + (time[i] & 0x0f);
}

void Int2BCD(unsigned char *time,int cnt)
{  
	int i;
	for(i = 0;i < cnt;i++)				// ????BCD
		time[i] = ((time[i]/10) << 4) + (time[i]%10);
}


void HAL_SYSTICK_Callback(void)
{ 
	tick++;
	if((tick>=0)&&(tick<500))
	{
			//Led(BLUE,1);
	}
	else if(tick<1000)
	{
		//Led(BLUE,0);
	}
	else tick=0;
}

void Uart3InterruptOn(void)
{  unsigned char ret,i=0;
	do{
	ret=HAL_UART_Receive_IT(&huart3,&Uart3Buffer,1);
		i++;
	if(i>10)
		break;
 }while(ret != HAL_OK);  
	if(i>=10)
		U3Interrupt=false;
	else U3Interrupt=true;
	
	//HAL_UART_Receive_DMA(&huart3, &Uart3Buffer, 1);
}

void uart3Send(unsigned char *data,unsigned int len)
{
	
	HAL_UART_Transmit_DMA(&huart3, data, len);
 
}


void Uart5InterruptOn(void)
{unsigned char ret,i=0;
	do{
	ret=HAL_UART_Receive_IT(&huart5,Uart2Buffer,1);
	i++;
	if(i>10)
		break;
}while(ret != HAL_OK);  
	
  if(i>=10)
		U5Interrupt=false;
	else U5Interrupt=true;
	

	//HAL_UART_Receive_DMA(&huart5, Uart2Buffer, 1);
}
HAL_StatusTypeDef uart_send(unsigned char *data,unsigned int len)
{
return HAL_UART_Transmit_DMA(&huart5, data, len);
}


void uart5Send(unsigned char *data,unsigned int len)
{
	HAL_UART_Transmit(&huart5, data, len, 1000);

}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{ 
  portBASE_TYPE xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;  	
  static int cnt=0;
  
  if(huart==&huart5)
  { //debug serial port 从位址板
		
		GetChar=1;
			
		#ifdef SEND_PCM 
		if(Uart2Buffer[0]==0x55)  //开始
		{
			   StartSend=1;
			   kpi=0;
		}
		else if(Uart2Buffer[0]==0xaa) //结束
		{
				 StartSend=0;
		}
		#else
		//xQueueSendFromISR( UART5QHandle, &Uart2Buffer[0], &xHigherPriorityTaskWoken );  


    //--------------------
    if(Uart2Buffer[0] == 0xaa)
    {
            cnt = 0;
            Uart5RXBuffer[cnt] = 0xaa;
            cnt++;
    }
     
		else if (Uart2Buffer[0]==0x80)  //恢复所有外设为默认值
		{	
						AppCtrlBag.enable=1;
						AppCtrlBag.tick=900;
			      ActionSteps=0;
						ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
						ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
						ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
						ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
						InhalePump(0);
						
		
		}
		else if (Uart2Buffer[0]==0x81)  //检测气阀1
		{
							AppCtrlBag.enable=1;
							AppCtrlBag.tick=900;
							ActionSteps=1;
							ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
		
		}
		else if (Uart2Buffer[0]==0x82)  //检测气阀2
		{
							AppCtrlBag.enable=1;
							AppCtrlBag.tick=900;
							ActionSteps=1;
							ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
		
		}
		else if (Uart2Buffer[0]==0x83)  //检测气阀3
		{
							AppCtrlBag.enable=1;
							AppCtrlBag.tick=900;
							ActionSteps=1;		
							ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
		
		}
		else if (Uart2Buffer[0]==0x84)  //检测气阀4
		{
							AppCtrlBag.enable=1;
							AppCtrlBag.tick=900;
							ActionSteps=1;
							ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
		
		}
		else if (Uart2Buffer[0]==0x85)  //检测气泵
		{       //  AppCtrlBag.enable=1;
						//	AppCtrlBag.tick=900;
						//	ActionSteps=1;
						//	InhalePump(1);
						//	osDelay(1000);
		
		}
		else if (Uart2Buffer[0]==0x86)  //检测MIC
		{
		
		
		}
		else if(Uart2Buffer[0]==0x87)  //检测位置
		{
						  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
							ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
							ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
							ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
							InhalePump(0);
							AppCtrlBag.enable=1;
							AppCtrlBag.tick=900;
							ActionSteps=0;
							AirSensor_Checkposition_Mode=ONCE_TIME;
		}
		else
        {
            //--------------------
            if(cnt == 7)
            {
                if(Uart2Buffer[0] == 0x55)
                {
                    udirdata = Uart5RXBuffer[5];
                    udirdata = udirdata << 8;
                    udirdata = udirdata + Uart5RXBuffer[6];
                }
                Uart5RXBuffer[cnt]=0;
                cnt=0;
            }
            //--------------------
            if(Uart5RXBuffer[0] == 0xaa)
            {
                Uart5RXBuffer[cnt] = Uart2Buffer[0];
                cnt++;
            }
            //--------------------
        }

        
		#endif
	//	xQueueSendFromISR( UART5QHandle, &Uart2Buffer[1], &xHigherPriorityTaskWoken );  
		//xQueueSendFromISR( UART5QHandle, &Uart2Buffer[2], &xHigherPriorityTaskWoken );  
		//xQueueSendFromISR( UART5QHandle, &Uart2Buffer[3], &xHigherPriorityTaskWoken );  
		//xQueueSendFromISR( UART5QHandle, &Uart2Buffer[4], &xHigherPriorityTaskWoken );  
		/*
		inputDataToQDef(&Uart2Buffer[0],1,&PositionQ);
		inputDataToQDef(&Uart2Buffer[1],1,&PositionQ);
		inputDataToQDef(&Uart2Buffer[2],1,&PositionQ);
		inputDataToQDef(&Uart2Buffer[3],1,&PositionQ);
		inputDataToQDef(&Uart2Buffer[4],1,&PositionQ);
		*/
	//	buff2[cnt++]=Uart2Buffer[0];
		//buff2[cnt++]=Uart2Buffer[1];
		//buff2[cnt++]=Uart2Buffer[2];
		//buff2[cnt++]=Uart2Buffer[3];
		//buff2[cnt++]=Uart2Buffer[4];
		//if(cnt>=20)
		//	cnt=0;
		
		 Uart5InterruptOn();
		
	}
	else if(huart==&huart3)  //从蓝牙
	{
		
		xQueueSendFromISR( UART3QHandle, &Uart3Buffer, &xHigherPriorityTaskWoken ); 
		Uart3InterruptOn();
	  
	}
}

void SoftWareReset(void)
{
		 __set_FAULTMASK(1);
		 HAL_NVIC_SystemReset();
}
void CheckSystemParameter(void)
{
	 GetSystemParameter();
	 if(g_systemParameter.upgradeType!=SOFTWARE_VERSION)
	 {
		 g_systemParameter.upgradeType=SOFTWARE_VERSION;
		 
     if( SCB->VTOR==0x08080000)
			g_systemParameter.imgBver=SOFTWARE_VERSION;
		 else g_systemParameter.imgAver=SOFTWARE_VERSION;
		 BackupSystemParameter();
		 
		 SoftWareReset();

	 }
}

void InitBle(void)
{
//	uart3Send("AT+DBM=8\r\n",strlen("AT+DBM=8\r\n"));
//	osDelay(500);
//	uart3Send("AT+CONNINT=10\r\n",strlen("AT+CONNINT=10\r\n"));
//	osDelay(500);
//	uart3Send("AT+ADVINT=50\r\n",strlen("AT+ADVINT=50\r\n"));
//	osDelay(500);
	
	//uart3Send("AT#WM=0\r\n",strlen("AT#WM=0\r\n"));
	osDelay(500);

}

//float temp32,tem33;
//char tem35[100];
//unsigned short tem36;

void userStart(void)
{   
	   LOGO_ON=false;
		Config.SnoreAverTime = 4000; 
		
	   InitStorage();
	  
			SnoreInit();
	  // InitBle();
	  // CheckSystemParameter();
	  /* 
	   temp32=10.2;
	   tem33=-1;
	   tem36=0xffff;
	   tem33=(short)tem36/10.0;
	   temp32+=tem33;
	   sprintf(tem35,":%.1f",temp32);
	*/
	 // DebugLog("debug log 1");
	//	DebugLog("debug log 2");
		//DebugLog("debug log 3");
		//DebugLog("debug log 4");
	 
	  
	  //SetFlashRDProtection();
	  //uart_send("Hello world!",strlen("Hello world!"));


}

void OSTimeDly(unsigned short t)
{
	 osDelay(t);

}

/*
*********************************************************************************************************
*    ? ? ?: vApplicationStackOverflowHook
*    ????: ????????
*    ?    ?: xTask        ????
*             pcTaskName   ???
*    ? ? ?: ?
*********************************************************************************************************
*/
void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName )
{    char str[40];
	sprintf(str," task %s\n",pcTaskName);
	uart_send((unsigned char *)str,strlen(str));
}
#ifdef PRINT_TASK_STACK
void vPrintfTaskStatus(void)
{

	 uart_printf("=================================================\r\n");
   uart_printf("任务名  任务状态   优先级   剩余栈   任务序号\r\n");
   vTaskList((char *)&pcWriteBuffer);
	 uart_send(pcWriteBuffer,strlen(pcWriteBuffer));
 //  uart_printf("%s\r\n", pcWriteBuffer); 
   uart_printf("\r\n");	
   //uart_printf("任务名  运行计数   使用率\r\n");
 //  vTaskGetRunTimeStats((char *)&pcWriteBuffer);
  // uart_printf("%s\r\n", pcWriteBuffer);
}
#endif


unsigned char OK_Command(unsigned char *buf,unsigned char cmd)
{
	buf[0]=0x02;
	buf[1]=0x10;
	buf[2]=cmd;
	buf[3]=0x03;
	buf[4]=0x0;
  return 4;
}

unsigned char ERROR_Command(unsigned char *buf,unsigned char cmd)
{
	buf[0]=0x02;
	buf[1]=0x11;
	buf[2]=cmd;
	buf[3]=0x03;
	buf[4]=0x0;
  return 4;
}

int ResetToFactory(void)
{    
		gStorageDataResetDefault();
		BackupSysPara(&gStorageData);
		osDelay(10);
	
	  Build_SensorLevelAutoSet_Flag(); //added by zjp for korea senseor level auto set	
	
			//SoftWareReset();
		return 1;
}




unsigned int GetNeedUpgradeImageType(void)
{
   if(SCB->VTOR==0x08020000)
   {
	   return 2;   //B
   }
   else if(SCB->VTOR==0x08080000)
   {
	   return 1;   //A
   }
		

	return 0;

}
unsigned char send_buf[100];



void ChangeToTestWav(void)
{
    int i;
    int MaxWaveData;
    int MinWaveData;
    int Maxdev;
    int Mindev;
    int Okdev;

    MaxWaveData = TestOkWaveData[0];
    MinWaveData = TestOkWaveData[0];    
    for(i=0;i<24;i++)
    {
        if(MaxWaveData < TestOkWaveData[i]) MaxWaveData = TestOkWaveData[i];
        if(MinWaveData > TestOkWaveData[i]) MinWaveData = TestOkWaveData[i];
    }
    
    if((MaxWaveData>250) && (MinWaveData < (-250)))
    {
        Maxdev = MaxWaveData / 250;
        Mindev = abs(MinWaveData) / 250;

        if(MaxWaveData > abs(MinWaveData))
        {
            Okdev = Maxdev + 1;
        }
        else
        {
            Okdev = Mindev + 1;
        }

        for(i=0;i<24;i++)
        {
            TestSendWaveData[i*2+0] =  (TestOkWaveData[i]/Okdev+250) & 0xff;
            TestSendWaveData[i*2+1] = ((TestOkWaveData[i]/Okdev+250) & 0xff00) >> 8;
        }
    }
    else
    {
        for(i=0;i<24;i++)
        {
            TestSendWaveData[i*2+0] =  (TestOkWaveData[i]+250) & 0xff;
            TestSendWaveData[i*2+1] = ((TestOkWaveData[i]+250) & 0xff00) >> 8;
        }
    }

}








/* MainTaskFunction function */
 void MainTaskFunction(void const * argument)
{
  /* USER CODE BEGIN 5 */
  unsigned char   SensorLevelbak,CheckSumDone=0; 
 	unsigned short temp,snoreAver=0,recordTotal=100;
	unsigned char buf[30],m=0xff,t1=0,*pt=NULL;
	unsigned int  flashBuf[4],i;
	
	//OnOffLed(1);
	xSemaphore = xSemaphoreCreateMutex();

  CheckSystemParameter();

//	if(ResetToFactory())
//	{
//			gStorageDataResetDefault();
//		  BackupSysPara(&gStorageData);

//		  OnOffLed(0);
//		  while(GetPowerKey()==GPIO_PIN_RESET);
//
//
//	}

	
	//uart_printf("Power On \n\r");
	HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN); 
	#ifdef SEND_PCM
	MX_UART5_Init_Baudrate(256000);
	#else
	MX_UART5_Init_Baudrate(UART5_BAUDRATE);
	#endif
	check_sensorlevel_autoset(); //added by zjp for v.3.1.29 for korea
	userStart();
	InitPDMLib();
	Get_F407ChipID();
	Uart3InterruptOn();
	Uart5InterruptOn();
//	ADok3=0;
	//ADCstart3();
	//osDelay(500);
	//if(PressureADC_ConvertedValue[0]<10)
	 //   	ErrorCode|=0x01;
  	StartAllAction=1;
	NeedUpgradePara.upgradeOk=0;
	NeedUpgradePara.ver1=0;
	NeedUpgradePara.ver2=0;
	NeedUpgradePara.ver3=0;
									
 	 NeedUpgradePara.firmwareType=0;
  	NeedUpgradePara.firmwareSize=0;
	NeedUpgradePara.checksum=0;
									
	NeedUpgradePara.upgradeOk=0;
	NeedUpgradePara.timer=0;
	NeedUpgradePara.countCheckSum=0;
									
  	NeedUpgradePara.address=0;
	NeedUpgradePara.startupgrade=0;
	NeedUpgradePara.upgradeOk=0;
	
	
   /*
	osDelay(20000);
	BlueToolthEnabled(0);
	
	 BlueToolLed(1);
	 OnOffLed(1);
	 PowerLed(1);
	 RunLed(1);
	 ResetValuePump();
	 EntryStandbyMode();
	 )
	 */
  /* Infinite loop */
  for(;;)
  {
		osDelay(2);
	
		//send_buf[0]=2;
		//send_buf[1]=GET_FIRMWARE_VERSION;
		//send_buf[2]=3;
		//uart3Send((unsigned char *)send_buf,3);	
		if(U3Interrupt==false)
			Uart3InterruptOn();
		
		if(U5Interrupt==false)
			Uart5InterruptOn();
			
		if(xQueueReceive(UART3QHandle, &buf[0], 100)==pdTRUE)
		{
						if(buf[0]==0x02)
						{  if(xQueueReceive(UART3QHandle, &buf[1], 100)==pdFALSE)
								continue;
								switch(buf[1])
								{
									case SNORE_SENSITIVITY:
										if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										if(buf[3]!=0x03)
										{
											ERROR_Command(buf,SNORE_SENSITIVITY);
											uart3Send((unsigned char *)buf,4);
											continue;
										}
                                        SensorLevelbak = gStorageData.SensorLevel;     
										if(buf[2]==0x00)
										 gStorageData.SensorLevel=LOW_CHECK;
										else if(buf[2]==0x01)
										 gStorageData.SensorLevel=MID_CHECK;
										else if(buf[2]==0x02)	
										 gStorageData.SensorLevel=HIGH_CHECK;
										else continue;										
                                        if(SensorLevelbak != gStorageData.SensorLevel)
                                        {
                                          //  SnoreInit();
																					  SnoreInit_personon(); 
                                        }
										OK_Command(buf,SNORE_SENSITIVITY);
										uart3Send((unsigned char *)buf,4);
										BackupSysPara(&gStorageData);
										break;
								  case ACTION_LEVEL:
									  	if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										if(buf[3]!=0x03)
										{
											ERROR_Command(buf,ACTION_LEVEL);
											uart3Send((unsigned char *)buf,4);
											continue;
										}
										if(buf[2]>0x02)
										{
											ERROR_Command(buf,ACTION_LEVEL);
											uart3Send((unsigned char *)buf,4);
											continue;
											
										}
										gStorageData.ActionLevel=buf[2];
										OK_Command(buf,ACTION_LEVEL);
										uart3Send((unsigned char *)buf,4);
										BackupSysPara(&gStorageData);
										break;
									case WORK_DELAY:
										if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										if(buf[3]!=0x03)
										{
											ERROR_Command(buf,WORK_DELAY);
											uart3Send((unsigned char *)buf,4);
											continue;
										}
										gStorageData.WorkDelay=buf[2];
										OK_Command(buf,WORK_DELAY);
										uart3Send((unsigned char *)buf,4);
										BackupSysPara(&gStorageData);
										break;
									case SET_WORK_MODE:
										if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										if((buf[3]!=0x03)||((ActionSteps!=0)&&(gStorageData.StudyMode!=2)))
										{
											  	ERROR_Command(buf,SET_WORK_MODE);
												uart3Send((unsigned char *)buf,4);
												continue;
										}
                                        //====================================
                                        if(gStorageData.StudyMode == 2)
                                        {
                                            if(buf[2] != 2)
                                            {
                                                gStorageData.StudyMode = 0;     //强制为工作模式
																							  RequestControlValve=0;  //added by zjp
                                            }
                                            else
                                            {
                                                gStorageData.StudyMode = 2;
                                            }
                                        }
                                        else
                                        {
                                            gStorageData.StudyMode=buf[2];
                                        }
                                        //====================================

                                        if(gStorageData.Standby==0)
										{
											AppCtrlBag.enable=1;
											AppCtrlBag.tick=500;											
											ResetValuePump();
											gStorageData.Standby=1;
										}                
                                    //    SnoreInit();
										                      SnoreInit_personon();
										BackupSysPara(&gStorageData);
										OK_Command(buf,SET_WORK_MODE);
										uart3Send((unsigned char *)buf,4);
										break;
											
									case SET_WORK_STATUS:
											if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
											if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
											if((buf[3]!=0x03)||(buf[2]>1))
											{
												ERROR_Command(buf,SET_WORK_STATUS);
												uart3Send((unsigned char *)buf,4);
												continue;
											}
							
											if((gStorageData.Standby==1)&&(buf[2]==0))
										  {
												memset(&gStorageData.Snore,0,32);
												gStorageData.Snore.UsedRecord=0x55aa;
//												gStorageData.Snore.ApneaTimes=0;
												gStorageData.Snore.HeadMovingTimes = 0;
												gStorageData.Snore.StudyMode=gStorageData.StudyMode;
												gStorageData.Standby=0;
												BackupSysPara(&gStorageData);
											}
											else if((gStorageData.Standby==0)&&(buf[2]==1))
											{
												AppCtrlBag.enable=1;
												AppCtrlBag.tick=500;
												gStorageData.Standby=1;
												ResetValuePump();
											}
											
											
											
											OK_Command(buf,SET_WORK_STATUS);
											uart3Send((unsigned char *)buf,4);

											BackupSysPara(&gStorageData);
											break;
									case RESET_FACTORY_DEFAULT:
										
											OK_Command(buf,RESET_FACTORY_DEFAULT);
											uart3Send((unsigned char *)buf,4);		
											ResetToFactory();
											//InitStorage();
											SnoreInit();
											break;
										  
									case GET_SLEEP_DATA_CNT:
										  temp=GetSnoreDataCnt();
											//temp=recordTotal;
									  	buf[0]=0x02;
									  	buf[1]=GET_SLEEP_DATA_CNT;
									  	buf[2]=temp&0xff;
									  	buf[3]=(temp&0xff00)>>8;
									  	buf[4]=3;
										uart3Send((unsigned char *)buf,5);
										break;
									case GET_SLEEP_DATA:
										
										temp=GetSnoreDataCnt();
									 	 if(temp==0)
										{	
									 			ERROR_Command(buf,GET_SLEEP_DATA);
									 			uart3Send((unsigned char *)buf,4);
									 			break;
										}
										/*
									  if(recordTotal==0)
										{
												ERROR_Command(buf,GET_SLEEP_DATA);
									 			uart3Send((unsigned char *)buf,4);
									 			break;
										
										
										}
										*/
										snoreData.UsedRecord=0;
										ReadSnoreData(&snoreData);
										if(snoreData.UsedRecord!=0x55aa)
										{	
											ERROR_Command(buf,GET_SLEEP_DATA);
											uart3Send((unsigned char *)buf,4);
											break;
										}	
										
									  recordTotal--;
										
										buf[0]=0x02;
										buf[1]=GET_SLEEP_DATA;
									  buf[2]=snoreData.StudyMode;
									  buf[3]=snoreData.StartTime[0];
										buf[4]=snoreData.StartTime[1];
										buf[5]=snoreData.StartTime[2];
										buf[6]=snoreData.StartTime[3];
										buf[7]=snoreData.StartTime[4];
										
									  buf[8]=snoreData.SSnoreTimes;
									  buf[9]=snoreData.MSnoreTimes;
										buf[10]=snoreData.LSnoreTimes;
										buf[11]=(snoreData.Airbag1Times<<4)|snoreData.Airbag2Times;
										buf[12]=(snoreData.Airbag3Times<<4)|snoreData.Airbag4Times;;
										if(snoreData.HeadPosion==0x10)
											buf[13]=0;
										else
											buf[13]=snoreData.HeadPosion+1;
										buf[14]=snoreData.MaxDB;
										buf[15]=snoreData.AVGSnoreTime;
										buf[16]=3;
										uart3Send((unsigned char *)buf,17);	
										osDelay(30);
										break;
									case SET_TIME:
										if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[4], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[5], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[6], 100)==pdFALSE)
												continue;
										sdatestructure.Year=buf[2];
										sdatestructure.Month=buf[3];
										sdatestructure.Date=buf[4];
										sdatestructure.WeekDay=RTC_WEEKDAY_MONDAY;
					
										stimestructure.Hours=buf[5];
										stimestructure.Minutes=buf[6];
										stimestructure.Seconds=10;
					
										stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
										stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;
										if (HAL_RTC_SetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN) != HAL_OK)
										{
											//Error_Handler();
										}
										if (HAL_RTC_SetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN) != HAL_OK)
										{
												//Error_Handler();
										}
										OK_Command(buf,SET_TIME);
										uart3Send((unsigned char *)buf,4);
										//if(gStorageData.workTimeOn==0)
										//{		
										//	gStorageData.workTimeOn=1;
										//	BackupSysPara(&gStorageData);	
										//}
										break;
									case GET_FIRMWARE_VERSION:
										buf[0]=0x02;
									  	buf[1]=GET_FIRMWARE_VERSION;
									  	buf[2]=SOFTWARE_VERSION;
									  	buf[3]=MASTER_VERSION;
									  	buf[4]=BOARD_VERSION;
									  	buf[5]=3;
										uart3Send((unsigned char *)buf,6);
										break;
									case REQUEST_UPGRADE:						
									  	buf[0]=0x02;
									  	buf[1]=REQUEST_UPGRADE;
									  	buf[2]=GetNeedUpgradeImageType();
									  	buf[3]=3;
										uart3Send((unsigned char *)buf,4);
										break;
									case SEND_FIRMWARE_PARA:
										NeedUpgradePara.upgradeOk=0;
										if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[4], 100)==pdFALSE)
												continue;
								  		if(xQueueReceive(UART3QHandle, &buf[5], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[6], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[7], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[8], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[9], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[10], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[11], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[12], 100)==pdFALSE)
												continue;
										if(buf[12]!=0x03)
										{
										  	ERROR_Command(buf,SEND_FIRMWARE_PARA);
												uart3Send((unsigned char *)buf,4);
												break;
										}
										NeedUpgradePara.ver1=buf[2];
										NeedUpgradePara.ver2=buf[3];
										NeedUpgradePara.ver3=buf[4];
									
										NeedUpgradePara.firmwareType=buf[5];
										NeedUpgradePara.firmwareSize=buf[6]|(buf[7]<<8)|(buf[8]<<16)|(buf[9]<<24);
										NeedUpgradePara.checksum=buf[10]|(buf[11]<<8);
									
										if((NeedUpgradePara.ver3!=BOARD_VERSION))
										{
											ERROR_Command(buf,SEND_FIRMWARE_PARA);
											uart3Send((unsigned char *)buf,4);
											break;
										}
									
										if((NeedUpgradePara.ver2<MASTER_VERSION))
										{
											ERROR_Command(buf,SEND_FIRMWARE_PARA);
											uart3Send((unsigned char *)buf,4);
											break;
										}
										if(NeedUpgradePara.ver1<=SOFTWARE_VERSION)
										{
											ERROR_Command(buf,SEND_FIRMWARE_PARA);
											uart3Send((unsigned char *)buf,4);
											break;
										}
									
										if(NeedUpgradePara.firmwareSize==0)
										{
											ERROR_Command(buf,SEND_FIRMWARE_PARA);
											uart3Send((unsigned char *)buf,4);
											break;
										}
									
										if(GetNeedUpgradeImageType()!=NeedUpgradePara.firmwareType)
										{
											ERROR_Command(buf,SEND_FIRMWARE_PARA);
											uart3Send((unsigned char *)buf,4);
											break;
										}
										if(NeedUpgradePara.firmwareType==1)
										{
											EraseImageAFlash();
											NeedUpgradePara.address=IMAGE_A_ADDRESS;
											pt=(unsigned char *)IMAGE_A_ADDRESS;
										}
										else if(NeedUpgradePara.firmwareType==2)
										{
											EraseImageBFlash();
											NeedUpgradePara.address=IMAGE_B_ADDRESS;
											pt=(unsigned char *)IMAGE_B_ADDRESS;
										}
										else
										{
											ERROR_Command(buf,SEND_FIRMWARE_PARA);
											uart3Send((unsigned char *)buf,4);
											break;									
										}
								  		NeedUpgradePara.upgradeOk=1;
										NeedUpgradePara.timer=0;
										NeedUpgradePara.countCheckSum=0;
										UpgradeCtrl.sum=0;
										UpgradeCtrl.pos=0;
										memset(UpgradeCtrl.UpgradeBuffer,0,512);
										OK_Command(buf,SEND_FIRMWARE_PARA);
										uart3Send((unsigned char *)buf,4);
										break;
									case SEND_FIRMWARE_DATA:
										if(NeedUpgradePara.upgradeOk==0)
										{
											ERROR_Command(buf,SEND_FIRMWARE_DATA);
											uart3Send((unsigned char *)buf,4);
											break;
										}
										NeedUpgradePara.startupgrade=1;		
										CheckSumDone=0;
										if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[4], 100)==pdFALSE)
												continue;
								    	if(xQueueReceive(UART3QHandle, &buf[5], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[6], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[7], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[8], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[9], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[10], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[11], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[12], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[13], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[14], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[15], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[16], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[17], 100)==pdFALSE)
												continue;
										
										if(xQueueReceive(UART3QHandle, &buf[18], 100)==pdFALSE)
												continue;
									
										if(buf[18]!=0x03)
									  	{
										  	ERROR_Command(buf,SEND_FIRMWARE_PARA);
												uart3Send((unsigned char *)buf,4);
												break;
									
									
									  	}
										
										UpgradeCtrl.sum=UpgradeCtrl.sum+(buf[2]+buf[3]+buf[4]+buf[5]+buf[6]+buf[7]+buf[8]+buf[9]+buf[10]
										+buf[11]+buf[12]+buf[13]+buf[14]+buf[15]+buf[16]+buf[17]);
											
										if(UpgradeCtrl.pos<=(512-16)){		
										memcpy((unsigned char *)(((unsigned int )(UpgradeCtrl.UpgradeBuffer))+UpgradeCtrl.pos),&buf[2],16);	
										UpgradeCtrl.pos+=16;	
										}	
										//memcpy((unsigned char *)flashBuf,&buf[2],16);
										//ProgramFlashBlock(NeedUpgradePara.address,4,flashBuf);
										//NeedUpgradePara.address+=16;
                    										
										break;
									case SEND_FIRMWARE_CHK:
											if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
											
										if(CheckSumDone==1){
												OK_Command(buf,SEND_FIRMWARE_CHK);
									      uart3Send((unsigned char *)buf,4);
										    break;
										}	
										else if(CheckSumDone==2){
												
												ERROR_Command(buf,SEND_FIRMWARE_CHK);
												uart3Send((unsigned char *)buf,4);
										    break;

										}
											
											
									 if(buf[2]!=UpgradeCtrl.sum){
												
												ERROR_Command(buf,SEND_FIRMWARE_CHK);
												uart3Send((unsigned char *)buf,4);
										   CheckSumDone=2;

										}
										else {
												ProgramFlashBlock(NeedUpgradePara.address,128,UpgradeCtrl.UpgradeBuffer);
												NeedUpgradePara.address+=512;
													
												OK_Command(buf,SEND_FIRMWARE_CHK);
									      uart3Send((unsigned char *)buf,4);
											  CheckSumDone=1;
										}
										
										memset(UpgradeCtrl.UpgradeBuffer,0,512);
										UpgradeCtrl.pos=0;
										UpgradeCtrl.sum=0;
										
										break;
									case SEND_FIRMWARE_END:										
										NeedUpgradePara.countCheckSum=0;
										for(i=0;i<NeedUpgradePara.firmwareSize;i++)
										{
												
											NeedUpgradePara.countCheckSum+=*pt;
											pt++;
										
										}
										if(NeedUpgradePara.countCheckSum==NeedUpgradePara.checksum)
										{
												OK_Command(buf,SEND_FIRMWARE_END);
									      uart3Send((unsigned char *)buf,4);
												osDelay(1000);
												gStorageData.SPIFlashAddress=0;
												gStorageData.SPIFlashOutAddress=0;
												BackupSysPara(&gStorageData);
												#ifndef UPGRADE_TEST
													if(NeedUpgradePara.firmwareType==1)
														JumpToImage(IMAGEA);
													else JumpToImage(IMAGEB);
												#endif
										}
										else
										{
											ERROR_Command(buf,SEND_FIRMWARE_END);
											uart3Send((unsigned char *)buf,4);
										}
											NeedUpgradePara.ver1=0;
									  	NeedUpgradePara.ver2=0;
									  	NeedUpgradePara.ver3=0;
									
									  	NeedUpgradePara.firmwareType=0;
									  	NeedUpgradePara.firmwareSize=0;
									  	NeedUpgradePara.checksum=0;
									
									  	NeedUpgradePara.upgradeOk=0;
									  	NeedUpgradePara.timer=0;
									  	NeedUpgradePara.countCheckSum=0;
									
									  	NeedUpgradePara.address=0;
									  	NeedUpgradePara.startupgrade=0;
									  	NeedUpgradePara.upgradeOk=0;
																				
										break;
									case READ_SNORE_SENSITIVITY:
										buf[0]=0x02;
									  	buf[1]=READ_SNORE_SENSITIVITY;
									 	if(gStorageData.SensorLevel==LOW_CHECK)
										 buf[2]=0;
										else if(gStorageData.SensorLevel==MID_CHECK)
										 buf[2]=1;
										else 
										 buf[2]=2;
									  	buf[3]=3;
										uart3Send((unsigned char *)buf,4);
										break;
									case READ_ACTION_LEVEL:
										buf[0]=0x02;
									  	buf[1]=READ_ACTION_LEVEL;
									  	buf[2]=gStorageData.ActionLevel;
									  	buf[3]=3;
										uart3Send((unsigned char *)buf,4);
										break;
									case READ_WORK_DELAY:
										buf[0]=0x02;
									  	buf[1]=READ_WORK_DELAY;
									  	buf[2]=gStorageData.WorkDelay;
									  	buf[3]=3;
										uart3Send((unsigned char *)buf,4);
										break;
									case READ_WORK_MODE:
										buf[0]=0x02;
									  	buf[1]=READ_WORK_MODE;
									  	buf[2]=gStorageData.StudyMode;
									  	buf[3]=3;
										uart3Send((unsigned char *)buf,4);
										break;
									case READ_WORK_STATUS:
										buf[0]=0x02;
										buf[1]=READ_WORK_STATUS;
										if(AppTest==0)
											buf[2]=gStorageData.Standby;
										else if(AppTest)
											buf[2]=0;
										buf[3]=3;
										uart3Send((unsigned char *)buf,4);
										break;
									case READ_ERROR_STATUS:
										buf[0]=0x02;
									  	buf[1]=READ_ERROR_STATUS;
									  	buf[2]=ErrorCode;
									  	buf[3]=3;
										uart3Send((unsigned char *)buf,4);
										break;
									case CONTROL_AIR_BAG:
										if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										tempAirBag=buf[2];
#ifdef TUWAN_BLANKET
										if(tempAirBag==1)
											tempAirBag=3;

										if(tempAirBag==2)
											tempAirBag=4;
#endif
										if((gStorageData.Standby==0))
										{
											AppCtrlBag.enable=1;
											AppCtrlBag.tick=500;
											gStorageData.Standby=1;
										}
										if((AppTest==0)&&(gStorageData.Standby==1))
										{
											AppTest=1;
											OK_Command(buf,CONTROL_AIR_BAG);
											uart3Send((unsigned char *)buf,4);	
										}
										else
										{
											ERROR_Command(buf,CONTROL_AIR_BAG);
											uart3Send((unsigned char *)buf,4);
										}
										break;
									case GET_BODY_POSITON:
										#if defined(TUWAN_PILLOW_3) || defined(TUWAN_MAT)
									    if(AirSensor_position_checked_by_fastmode_flag==1)
											{
											  buf[0]=0x02;
									  	  buf[1]=GET_BODY_POSITON;
									  	  if(position.head1==HEAD_NO)
									  		  buf[2]=0;
										    else
											    buf[2]=position.head1+1;
									  	  buf[3]=3;
									  	  ReadPos.enable=1;
									  	  ReadPos.tick=200;
//												AirSensor_position_checked_by_fastmode_flag=0;
									  	  if(ActionSteps==0)
									  	   AirSensor_Checkposition_Mode=FAST_MODE;;
											}
											else
											{
											  buf[0]=0x02;
									  	  buf[1]=GET_BODY_POSITON;									  	  
									  		buf[2]=0;
										    buf[3]=3;
									  	  ReadPos.enable=1;
									  	  ReadPos.tick=200;												
									  	  if(ActionSteps==0)
									  	   AirSensor_Checkposition_Mode=FAST_MODE;												
											}
										#else
										buf[0]=0x02;
									  	buf[1]=GET_BODY_POSITON;
									  	if(position.body==0)
									  		buf[2]=0;
										else if(position.body==1)
											buf[2]=2;
										else if(position.body==3)
											buf[2]=1;
										else  buf[2]=0;
									  	buf[3]=3;
										#endif
										uart3Send((unsigned char *)buf,4);
										break;
									case GET_SNORE_DB:
										temp = (unsigned short)PCMVol;
										buf[0]=0x02;
									  	buf[1]=GET_SNORE_DB;
									  	buf[2]=temp&0xff;
									  	buf[3]=(temp&0xff00)>>8;
									  	buf[4]=3;
										uart3Send((unsigned char *)buf,5);
										break;

                                    //------------------------------------------
									case GET_TEST_WAV_DATA0:
                                        ChangeToTestWav();
                                        Config.level =  HIGH_CHECK;
                                        
										buf[0]=0x02;
									  	buf[1]=GET_TEST_WAV_DATA0;

                                        for(i=0;i<16;i++)
                                        {
									  	    buf[2+i] = TestSendWaveData[16*0+i];
                                        }

                                        if((Config.TestState == 2)||(Config.TestState == 1))
                                        {
                                            buf[18] = Config.Testvol * 2;
                                        }
                                        else
                                        {
                                            buf[18] = 0;
                                        }
                                        
									  	buf[19]=3;
										uart3Send((unsigned char *)buf,20);
										break;

									case GET_TEST_WAV_DATA1:
										buf[0]=0x02;
									  	buf[1]=GET_TEST_WAV_DATA1;

                                        for(i=0;i<16;i++)
                                        {
									  	    buf[2+i] = TestSendWaveData[16*1+i];
                                        }
                                        
                                        buf[18]=Config.TestState;
									  	buf[19]=3;
										uart3Send((unsigned char *)buf,20);
										break;
                                    

									case GET_TEST_WAV_DATA2:
										buf[0]=0x02;
									  	buf[1]=GET_TEST_WAV_DATA2;

                                        for(i=0;i<16;i++)
                                        {
									  	    buf[2+i] = TestSendWaveData[16*2+i];
                                        }
                                        
                                        buf[18]=Config.TestState;
									  	buf[19]=3;
                                        if(Config.TestState == 2)
                                        {
                                            Config.level = gStorageData.SensorLevel;
                                        }
										uart3Send((unsigned char *)buf,20);
										break;


                                    //------------------------------------------
                                        
										#ifdef TUWAN_BLANKET
									case INPUT_POSITION_STANDARD:
									
										if((ActionSteps==0)||(ActionSteps==6))
										{
											AirSensor_factory_calibration_flag=1;
											osDelay(300);
											OK_Command(buf,INPUT_POSITION_STANDARD);
											uart3Send((unsigned char *)buf,4);
										}
										else
										{
											ERROR_Command(buf,INPUT_POSITION_STANDARD);
											uart3Send((unsigned char *)buf,4);
										}
									
										break;
									case DO_POSITION_STANDARD_STATUS:
										buf[0]=0x02;
										buf[1]=DO_POSITION_STANDARD_STATUS;
										buf[2]=AirSensor_cp_calibrationflag;
										buf[3]=3;
										uart3Send((unsigned char *)buf,4);
										break;
										#endif
									
									/*
									case SET_WORK_TIME:
											if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[4], 100)==pdFALSE)
												continue;
								    	if(xQueueReceive(UART3QHandle, &buf[5], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[6], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[7], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[8], 100)==pdFALSE)
												continue;
									  	if(xQueueReceive(UART3QHandle, &buf[9], 100)==pdFALSE)
												continue;
									    if(xQueueReceive(UART3QHandle, &buf[10], 100)==pdFALSE)
												continue;
									
											if(buf[10]!=0x03)
									  	{
										  	ERROR_Command(buf,SET_WORK_TIME);
												uart3Send((unsigned char *)buf,4);
												break;
									
									
									  	}
											
											gStorageData.workTime[0][0]=buf[2];
											gStorageData.workTime[0][1]=buf[3];
											gStorageData.workTime[0][2]=buf[4];
											gStorageData.workTime[0][3]=buf[5];
				
											gStorageData.workTime[1][0]=buf[6];
											gStorageData.workTime[1][1]=buf[7];
											gStorageData.workTime[1][2]=buf[8];
											gStorageData.workTime[1][3]=buf[9];
											
											
											
											
											OK_Command(buf,SET_WORK_TIME);
											uart3Send((unsigned char *)buf,4);
											gStorageData.workTimeOn=1;
											BackupSysPara(&gStorageData);
											break;
									case GET_WORK_TIME:
											buf[0]=0x02;
											buf[1]=GET_WORK_TIME;
											buf[2]=gStorageData.workTime[0][0];
											buf[3]=gStorageData.workTime[0][1];
											buf[4]=gStorageData.workTime[0][2];
											buf[5]=gStorageData.workTime[0][3];
				
											buf[6]=gStorageData.workTime[1][0];
											buf[7]=gStorageData.workTime[1][1];
											buf[8]=gStorageData.workTime[1][2];
											buf[9]=gStorageData.workTime[1][3];
											buf[10]=3;
											uart3Send((unsigned char *)buf,11);
											break;
*/
									default:
										ERROR_Command(buf,0xff);
										uart3Send((unsigned char *)buf,4);
										continue;
								}
						}
						continue;
			 
		 }
		osDelay(20);
		t1++;
		if(t1>8)
		{
			t1=0;
			HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
			HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);
			/*
			if(gStorageData.workTimeOn)
			{
				if(gStorageData.workTime[0][0]>=12)
						buf[0]=gStorageData.workTime[0][0]-12;
				else buf[0]=gStorageData.workTime[0][0]+12;
		
				if(gStorageData.workTime[0][2]>=12)
						buf[1]=gStorageData.workTime[0][2]-12;
				else buf[1]=gStorageData.workTime[0][2]+12; 	//第一段时间
				
				flashBuf[0]=buf[0]*60+gStorageData.workTime[0][1]+gStorageData.WorkDelay;
				flashBuf[1]=buf[1]*60+gStorageData.workTime[0][3];
				
				if(gStorageData.workTime[1][0]>=12)
						buf[2]=gStorageData.workTime[1][0]-12;
				else buf[2]=gStorageData.workTime[1][0]+12;
		
				if(gStorageData.workTime[1][2]>=12)
						buf[3]=gStorageData.workTime[1][2]-12;
				else buf[3]=gStorageData.workTime[1][2]+12; 	//第二段时间
				
				flashBuf[2]=buf[2]*60+gStorageData.workTime[1][1]+gStorageData.WorkDelay;
				flashBuf[3]=buf[3]*60+gStorageData.workTime[1][3];
				
				if(stimestructure.Hours>=12)
						buf[4]=stimestructure.Hours-12;
				else buf[4]=stimestructure.Hours+12;
				
				if(((buf[4]*60+stimestructure.Minutes)<flashBuf[0])||((buf[4]*60+stimestructure.Minutes)>flashBuf[1]))
				{
						if(((buf[4]*60+stimestructure.Minutes)<flashBuf[2])||((buf[4]*60+stimestructure.Minutes)>flashBuf[3]))
						{
									gStorageData.Standby=1;  //进入待机的状态
						
						}else gStorageData.Standby=0;

				}else gStorageData.Standby=0;

			}
			*/
			if((m!=stimestructure.Minutes)&&(stimestructure.Minutes%10==0))
			{
				 m=stimestructure.Minutes; 
				 if(gStorageData.Standby==0)   //备份鼾声数据
				 {
//					  gStorageData.Snore.HeadMovingTimes=Airsensor.Body_Moved_Times;
						gStorageData.Snore.StartTime[0]=sdatestructure.Year;
						gStorageData.Snore.StartTime[1]=sdatestructure.Month;
						gStorageData.Snore.StartTime[2]=sdatestructure.Date;
						gStorageData.Snore.StartTime[3]=stimestructure.Hours;
						gStorageData.Snore.StartTime[4]=stimestructure.Minutes;
					  gStorageData.Snore.AVGSnoreTime=Config.SnoreAverTime/100;
					 	WriteSnoreData(&gStorageData.Snore); //保存上次
					  memset(&gStorageData.Snore,0,32);
					  gStorageData.Snore.UsedRecord=0x55aa;
					  gStorageData.Snore.StudyMode=gStorageData.StudyMode;
					  Config.SnoreAverTime = 4000; 
//					  Airsensor.Body_Moved_Times=0;
				 }
			}
		} 	
  }
  /* USER CODE END 5 */ 
}




void ForthTemialFunction(void const * argument)
{   
  /* USER CODE BEGIN ForthTemialFunction */
  /* Infinite loop */
#if defined(TUWAN_PILLOW_3) || defined(TUWAN_MAT)
	unsigned char t,j;
#endif
  	int cnt=0,start=0;
	ReadPos.enable=0;
	AppCtrlBag.enable=0;
	while(StartAllAction==0)
		 osDelay(10);

  	for(;;)
  	{
     	osDelay(10);
     	
     	if(ReadPos.enable)
     	{
     		if(ReadPos.tick>0)
     			ReadPos.tick--;
     		else{
     			ReadPos.enable=0;
					#if defined(TUWAN_PILLOW_3) || defined(TUWAN_MAT)
     			AirSensor_Checkposition_Mode=SLOW_MODE;
					#endif
     		}


     	}

     	if(AppCtrlBag.enable)
     	{
     		if(AppCtrlBag.tick>0)
     			AppCtrlBag.tick--;
     		else
     		    AppCtrlBag.enable=0;
     	}
			#if defined(TUWAN_PILLOW_3) || defined(TUWAN_MAT)
				#ifdef UPGRADE_TEST
					t++;
					if((t%10)==0)
						PowerLed(j=!j);
				#endif
			#endif
		if(NeedUpgradePara.upgradeOk&&(NeedUpgradePara.startupgrade==0))
		{
		
		
			NeedUpgradePara.timer++;
			if(NeedUpgradePara.timer>700)
			{
				NeedUpgradePara.startupgrade=0;
				NeedUpgradePara.upgradeOk=0;
				
			}
		
		}
		if(start==0)
		{
			cnt++;
		 	if(cnt>300)
		 	{ 	StartI2S_2();
			 
		 		start=1;
		 	}
	 	}
///cnt action process overflow time
		if(ProcessControl.on)
		{
			ProcessControl.tick++;
			if(ProcessControl.tick>=(PROCESS_OVER_TIME))
			{ 	ProcessControl.on=false;
				NeedReturnToDefault=1;
				ProcessControl.LastStep=step;
			}
		}
		else ProcessControl.tick=0;
		
	 	if(GetPowerKey()==GPIO_PIN_SET)
	 	{  
			if(CheckKeyPressState()==LONG_PRESS) //长按进入关机状态切换
			{
					if(gStorageData.Standby==0) {  //如果处于工作状态，则先结束目前的工作
							gStorageData.Standby = 1;
							BackupSysPara(&gStorageData);
							NeedReturnToDefault = 1;
					}
//					WrokingLed(BLUE_LED_HIGH,0);
					while(GetPowerKey()==GPIO_PIN_SET)
						osDelay(10);
					BackupSysPara(&gStorageData);
					EntryStandbyMode();
			}
			else
			{
					StartLedCount=1000;
					if(gStorageData.StudyMode==2)
					ShowModeOn=1;
					KeyCnt++;
				  if(KeyCnt>=8)
					{
						Masaji=1;
						AppTest=1;
						KeyCnt=0;
					
					
					}
			}

		}
		if(StartLedCount)
		{
			StartLedCount--;
//			WrokingLed(BLUE_LED_HIGH,1);

		}
		else {
//			WrokingLed(BLUE_LED_LOW,1);
			KeyCnt=0;
		}
		//FT_Kernel();
  }
  /* USER CODE END ForthTemialFunction */
}



void ADCstart1(void)
{
	 HAL_ADC_Start_IT(&hadc1);

}
void ADCstart2(void)
{
	 HAL_ADC_Start_IT(&hadc2);

}
void ADCstart3(void)
{
	 HAL_ADC_Start_IT(&hadc3);

}
