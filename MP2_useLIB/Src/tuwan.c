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
#include "bluetooth.h"
#include "time.h"

#define CHIP_ID_CNT 300
unsigned ShowModeOn=0;
void ADCstart3(void);
void ADCstart1(void);
void Error_Handler(void);
void MX_USB_HOST_Process(void);
RTC_DateTypeDef sdatestructure,sdatestructure_temp;
RTC_TimeTypeDef stimestructure,stimestructure_temp;
bool U3Interrupt=false,U5Interrupt=false;
upgrade_control_def UpgradeCtrl;
unsigned int mac_Code;
unsigned char Uart2Buffer[10],GetChar=0,Uart3Buffer,TestMode=0,AppTest=0;
unsigned char Uart5RXBuffer[10],tempAirBag,tempholdingtime;
unsigned char vectors_num,valid_vcetors_num;
uint16_t valid_vectors_weight[7];
uint16_t    udirdata=0;
unsigned int StartLedCount=1000;
short tick=0,keyTimer=0;

uint8_t Upgrading_On=0;
NeedUpgradePara_def NeedUpgradePara;
//extern unsigned char buff2[20];
extern ProcessControl_def ProcessControl;
extern uint8_t Error_Code;
extern RTC_HandleTypeDef hrtc;
extern HeadPosition_def position;
//QDef  PositionQ;
unsigned char pcWriteBuffer[600],KeyCnt=0;
extern unsigned char CurrentRom,StartSend,StartAllAction,NeedReturnToDefault,step,ADok1,ADok3,needWork;
//extern uint8_t AirSensor_position_checked_by_fastmode_flag;
extern unsigned short kpi;
extern SemaphoreHandle_t xSemaphore;
extern SemaphoreHandle_t xTimeMute;
extern SemaphoreHandle_t xPositionMute;
extern SemaphoreHandle_t xLogoMute;

extern unsigned int AutoStartTimer;
void ResetValuePump(void);
void ExhaleAirHead(void);
int ResetToFactory(void);
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

extern uint8_t pump_operation_time[20][13];//20����ÿ����һ���ֽ�Ϊ0��������Ч��1��������Ч
extern uint8_t pump_operation_time_write_pointer,pump_operation_time_read_pointer;//��¼�ͱ���λ��ָ��
extern SnoreData_def detailed_snore;
extern unsigned char person_on_hour;
extern unsigned char person_on_minute;
extern unsigned char person_on_second;
extern unsigned char person_off_hour;
extern unsigned char person_off_minute;
extern unsigned char person_off_second;
extern uint8_t Ten_minutes_first_personon_event_flag;

extern unsigned int snore_times_every_sleep;
extern unsigned int Antisnore_count_timer;

extern uint8_t BT_Recieved_Data_From_App_Buffer[2000];//����LOG��ʾ���յ���APP���� 
extern uint16_t BT_Recieved_Data_From_App_Buffer_Pointer;

extern uint8_t BT_Sent_Data_To_App_Buffer[2000];
extern uint16_t BT_Sent_Data_To_App_Buffer_Pointer;

extern void User_set_studytime_process(void); 
extern void Position_Init(void);
extern void snore_mindb_Init(void);
extern void User_set_positionvalue(void);
uint8_t person_on_for_save_data_flag=0;

uint8_t workdelay_time_cycle_flag = 0;
extern unsigned int workdelay_count_timer;
extern unsigned int update_request_counter_timer;

unsigned char buf_for_BLE_SN_modify[100];
uint8_t buf_for_BLE_SN_modify_pointer=0;
char * BleSNHeader="BLE:";
uint8_t rechecktime_minute,rechecktime_hour=0xff,latestsnoretime_minute,latestsnoretime_hour=0xff,personontime_minute,personontime_hour=0xff;

uint8_t Firmware_update_received_flag = 0;
uint8_t Firmware_update_keypressed = 0; //0:nokey 1:shortpress 2:longpress 3S
extern uint8_t reply_app_for_update_keypress_flag;

uint8_t APP_setworkstate = 0;
uint8_t Scankey_setworkstate = 0;
uint8_t ValidTimeOkFlag = 1;

extern void SetBleName(void);
extern uint8_t rcvbufdata[50];
extern uint8_t Received_BLE_NAME_flag;
extern uint8_t app_settime_flag;
extern unsigned int snore_times_for_antisnore;
extern unsigned int snore_times_for_snore;
extern void SetBLERESET(unsigned char highlow);
extern void SetINTinput(void);
extern void Body_Detect_Init(void);
extern void LOGO_Reset_Output(void);

extern SnoreTime_def SnoreTime_forAPP;

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
        gStorageData.Snore.HeadPosion = 0x0ff;
//        gStorageData.Snore.ApneaTimes = 0;
//				gStorageData.Snore.HeadMovingTimes = 0;
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

        WriteSnoreData(&gStorageData.Snore); //�����ϴ�

        
        memset(&gStorageData.Snore,0,sizeof(SnoreData_def));
        gStorageData.Snore.UsedRecord=0x55aa;

        //osDelay(10);
    }    
    //WrokingLed(GREE_LED);
}


#endif




void EntryStandbyMode()
{

    //��wakeup���ţ����������������ؾͿ��Ի���cpu��
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    //���wakeup_flag,�������Ҫ������û�У��Ͳ����ظ�����cpu�����ٽ������
    //��HAL_PWR_EnterSTANDBYMode()���������ã���
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    //cpu�������ģʽ��NREST Pin��IWDG��RTC alarm��WakeUp���������أ������Ի���cpu
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
		Error_Code=19;
    Error_Handler();
  }

}
void gStorageDataResetDefault(void)
{
		  gStorageData.flag=SYS_PARA_FLAG;
		  gStorageData.SPIFlashAddress=0;
		  gStorageData.SPIFlashOutAddress=0;
	    gStorageData.ActionLevel=38;    //0:low 1:lower 2:mid 3:higher 4:high
	    gStorageData.SensorLevel=MID_CHECK; //�ϵ�(38�룩MID_CHECK; //1:�ߣ�2���� 3����
	    gStorageData.HeadpositionLevel=HIGH_CHECK;	//��������
	    gStorageData.MinSnoredB = LOW_SNOREDB;
	    gStorageData.PositionValue_Onebag = AirSensor_threshold_person_on_HIGH; //ͷ��λ�ü�ⵥ������ֵ
	    gStorageData.PositionValue_Twobag = AirSensor_threshold_person1_on_HIGH;//ͷ��λ�ü��˫����֮�ͷ�ֵ
	    gStorageData.Body_detect_on_threshold = person_on_threshold;
	    gStorageData.Body_detect_off_threshold = stable_down_threshold;
	    gStorageData.One_hour_auto_detect_onoff = 0x1; //ȱʡΪ1Сʱ��������һ��ͷ��λ�ü�⹦�ܴ�
			gStorageData.auto_detect_timecycle = 60; //ȱʡΪ1Сʱ��������һ��ͷ��λ�ü�⹦�ܴ�
		  gStorageData.StudyMode=0;
			gStorageData.Modifystudymodestamptime = 0;
//		  gStorageData.Standby=1;       //���ڴ�����״̬
	    gStorageData.Standby=1;       
		  gStorageData.WorkDelay=0;
		  //gStorageData.InhaleOver=0;
		  //gStorageData.ExhaleOver=0;
			//gStorageData.workTimeOn=0;
	    //gStorageData.workTime[0][0]=21;
			//gStorageData.workTime[0][1]=30;
			//gStorageData.workTime[0][2]=9;
			//gStorageData.workTime[0][3]=0;
	
			//gStorageData.workTime[1][0]=12;
			//gStorageData.workTime[1][1]=30;
			//gStorageData.workTime[1][2]=15;
			//gStorageData.workTime[1][3]=0;
			
		#if 1	
	    #ifdef UserSetSleepTime
	      gStorageData.UserSleepTime[0]=UserSleepTimeStartHour;
			  gStorageData.UserSleepTime[1]=UserSleepTimeStartMin;
			  gStorageData.UserSleepTime[2]=UserSleepTimeEndHour;
			  gStorageData.UserSleepTime[3]=UserSleepTimeEndMin;
			#endif
			BackupSysPara(&gStorageData);
			
//	    User_set_studytime_process();  
		#endif	
			if( USERKEYCODE_ENABLE==1)
			{
				Build_UserkeycodeAutoSet_Flag();
			}
			
		  memset(&gStorageData.Snore,0,sizeof(SnoreData_def));
			gStorageData.Snore.HeadPosion = 0x0ff;
		  firstSnoreInit();
}
void gStorageDataResetDefault1(void)
{
			
		  gStorageData.flag=SYS_PARA_FLAG;
		  gStorageData.SPIFlashAddress=0;
		  gStorageData.SPIFlashOutAddress=0;
			if(gStorageData.ActionLevel == 0)
				gStorageData.ActionLevel = 30;
			else if(gStorageData.ActionLevel == 1)
				gStorageData.ActionLevel = 38;
			else if(gStorageData.ActionLevel == 2)
				gStorageData.ActionLevel = 45;
			else if(gStorageData.ActionLevel == 3)
				gStorageData.ActionLevel = 53;
		  else if(gStorageData.ActionLevel == 4)
				gStorageData.ActionLevel = 60;
			else
				gStorageData.ActionLevel = 38;
	    //gStorageData.ActionLevel=38;    //0:low 1:lower 2:mid 3:higher 4:high
	    gStorageData.SensorLevel=MID_CHECK; //�ϵ�(38�룩MID_CHECK; //1:�ߣ�2���� 3����
	    gStorageData.HeadpositionLevel=HIGH_CHECK;	//��������
	    gStorageData.MinSnoredB = LOW_SNOREDB;
	    gStorageData.PositionValue_Onebag = AirSensor_threshold_person_on_HIGH; //ͷ��λ�ü�ⵥ������ֵ
	    gStorageData.PositionValue_Twobag = AirSensor_threshold_person1_on_HIGH;//ͷ��λ�ü��˫����֮�ͷ�ֵ
	    gStorageData.Body_detect_on_threshold = person_on_threshold;
	    gStorageData.Body_detect_off_threshold = stable_down_threshold;
	    gStorageData.One_hour_auto_detect_onoff = 0x1; //ȱʡΪ1Сʱ��������һ��ͷ��λ�ü�⹦�ܴ�
			gStorageData.auto_detect_timecycle = 60; //ȱʡΪ1Сʱ��������һ��ͷ��λ�ü�⹦�ܴ�
		  gStorageData.StudyMode=0;
//		  gStorageData.Standby=1;       //���ڴ�����״̬
	    gStorageData.Standby=1;       
		  gStorageData.WorkDelay=0;
		  //gStorageData.InhaleOver=0;
		  //gStorageData.ExhaleOver=0;
			//gStorageData.workTimeOn=0;
	    //gStorageData.workTime[0][0]=21;
			//gStorageData.workTime[0][1]=30;
			//gStorageData.workTime[0][2]=9;
			//gStorageData.workTime[0][3]=0;
	
			//gStorageData.workTime[1][0]=12;
			//gStorageData.workTime[1][1]=30;
			//gStorageData.workTime[1][2]=15;
			//gStorageData.workTime[1][3]=0;
			
		#if 1	
	    #ifdef UserSetSleepTime
	      gStorageData.UserSleepTime[0]=UserSleepTimeStartHour;
			  gStorageData.UserSleepTime[1]=UserSleepTimeStartMin;
			  gStorageData.UserSleepTime[2]=UserSleepTimeEndHour;
			  gStorageData.UserSleepTime[3]=UserSleepTimeEndMin;
			#endif
			BackupSysPara(&gStorageData);
			
//	    User_set_studytime_process();  
		#endif	
			
		  //memset(&gStorageData.Snore,0,sizeof(SnoreData_def));
		  //firstSnoreInit();
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
//		if(tick>=20)   //20
				
		if(Firmware_update_received_flag==1)
    {
			if(tick >= 30)
		  {
				Firmware_update_keypressed = 2;
				//return UPDATE_LONG_PRESS;
		  }
			
		}	
    else
		{
			if(tick >= 9 && tick < 30)
			{	
				return LONG_PRESS;
			}
		}			
		
	}
//	if(tick>=20) //20
	if(Firmware_update_received_flag==1)
  {
	  if(tick >= 30)
		{
			Firmware_update_keypressed = 2;
			return UPDATE_LONG_PRESS;
		}
		else
		{
			Firmware_update_keypressed = 1;
			return UPDATE_SHORT_PRESS;
		}
	}		
  else
	{
		if(tick >= 9 && tick <= 30)
		{	
			return LONG_PRESS;
		}
		else
		{
			return SHORT_PRESS;
		}
	}			
		
}
/*
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
//		if(tick>=20)   //20
			if(tick>=9)   //20	
			  return LONG_PRESS;
	}
//	if(tick>=20) //20
	  if(tick>=9) //20
		  return LONG_PRESS;
	return SHORT_PRESS;
}
*/
void InitStorage(void)
{  	unsigned char buf[4];
	  unsigned short tick;
	ReadUniqueID(buf);
   if(ReadSysPara(&gStorageData)==0)  //�洢����δ����ʼ����
	 {
			gStorageDataResetDefault();
			BackupSysPara(&gStorageData);
	 }
	 else if((gStorageData.SPIFlashAddress>=SNORE_DATA_CNT)||(gStorageData.SPIFlashOutAddress>=SNORE_DATA_CNT))
	 {
			gStorageDataResetDefault();
			BackupSysPara(&gStorageData);
	 }
//	 User_set_studytime_process();
	 
    /*
	 else if((gStorageData.InhaleOver==0xffff)||(gStorageData.ExhaleOver==0xffff))
	 {
	 
	 	  gStorageDataResetDefault();
		  BackupSysPara(&gStorageData);
	 
	 }
	*/
	// if( gStorageData.Standby==0)  //��������״̬,���Ӷ����ݵĴ洢����
	// {	
	//   
	 //   	  WriteSnoreData(&gStorageData.Snore); //�����ϴ�
	//    	  gStorageData.Standby=1;
	// }
	//	OnOffLed(1);
	SetINTinput();
	SetBLEWAKEUP(1);
	SetBLERESET(1); 
  BlueToolthEnabled(1);
	WrokingLed(RED_LED,1);
	 
	 
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
	
#if 0	
#ifdef MP1_PLUS	
	 WrokingLed(RED_LED,1);
//	#ifndef UPGRADE_TEST
	 tick=0;
	 
	 //�޸�Ϊ��������
	  while(1)
		{
			if(GetPowerKey()==GPIO_PIN_SET)
			{
//						WrokingLed(BLUE_LED_HIGH,1);
//					  CheckKeyPressState();
				    if(CheckKeyPressState()==LONG_PRESS)
						{	
						  break;
						}
//						else
//						{
//							EntryStandbyMode();
//						}
			}
			else if(tick>500) 	EntryStandbyMode();
			
			tick++;
			osDelay(10);
		}
		WrokingLed(BLUE_LED_HIGH,1);
		while(GetPowerKey()==GPIO_PIN_SET)
  		osDelay(10);
	
#endif	
#endif

		
//		WrokingLed(BLUE_LED_HIGH,1);
		 StartLedCount=1000;
		 BlueToolthEnabled(1);//ϵͳ�����ϵ�
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

//				gStorageData.Standby=1;
        gStorageData.Standby=1;
        Airsensor.It_Is_Standby_State=1;				
//				gStorageData.WorkDelay=0;
				BackupSysPara(&gStorageData);
}

//unsigned int supportChipID[CHIP_ID_CNT]={0x24AC3EA9,0,0};
unsigned int Get_ChipSeriesID(void)
{
    unsigned int ChipSeriesID;  
	  ChipSeriesID=*(unsigned int *)(0xe0042000);  
		return ChipSeriesID&0xffff;
}

void Get_F407ChipID(void)
{
    unsigned int ChipUniqueID[3];  //��ַ��С�����ȷŵ��ֽڣ��ٷŸ��ֽڣ�С��ģʽ
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
	uint16_t i=0;
	HAL_UART_Transmit_DMA(&huart3, data, len);
	if(App_COMMAND_LOG==1)
	{
	  for(i=0;i<len;i++)
	  {
      BT_Sent_Data_To_App_Buffer[BT_Sent_Data_To_App_Buffer_Pointer]=data[i];
      BT_Sent_Data_To_App_Buffer_Pointer++;
      if(BT_Sent_Data_To_App_Buffer_Pointer>=3000)
		  {
        BT_Sent_Data_To_App_Buffer_Pointer=0;	
		  }		
		}			
	}
	
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

uint8_t headerok_flag=0;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{ 
  portBASE_TYPE xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;  	
  static int cnt=0;
	uint8_t j;
  
  if(huart==&huart5)
  { 
		//�¼�Ϊ�������кŸ���
		
		buf_for_BLE_SN_modify[buf_for_BLE_SN_modify_pointer]=Uart2Buffer[0];
		buf_for_BLE_SN_modify_pointer++;
		if(buf_for_BLE_SN_modify_pointer>=100)
			buf_for_BLE_SN_modify_pointer=0;
		
		//����Ƿ�����S/N��д��
		if(buf_for_BLE_SN_modify_pointer==4)
		{
		  if(buf_for_BLE_SN_modify[0]==BleSNHeader[0])
		  {
			  if(buf_for_BLE_SN_modify[1]==BleSNHeader[1])
			  {
				  if(buf_for_BLE_SN_modify[2]==BleSNHeader[2])
			    {
				    if(buf_for_BLE_SN_modify[3]==BleSNHeader[3])
					  {
						  //��ʶͷ��ȷ BLE:
							headerok_flag=1;							
					  }
					  else
						{
						  buf_for_BLE_SN_modify_pointer=0;
						}
			    }
				  else
					{
					  buf_for_BLE_SN_modify_pointer=1;
						buf_for_BLE_SN_modify[0]=buf_for_BLE_SN_modify[3];
					}
			  }
			  else
				{
				  buf_for_BLE_SN_modify_pointer=2;
					buf_for_BLE_SN_modify[0]=buf_for_BLE_SN_modify[2];
				  buf_for_BLE_SN_modify[1]=buf_for_BLE_SN_modify[3];
				}
		  }			
		  else
			{
			  buf_for_BLE_SN_modify_pointer=3;
				buf_for_BLE_SN_modify[0]=buf_for_BLE_SN_modify[1];
				buf_for_BLE_SN_modify[1]=buf_for_BLE_SN_modify[2];
				buf_for_BLE_SN_modify[2]=buf_for_BLE_SN_modify[3];
			}
		}	
		if(headerok_flag==1 && buf_for_BLE_SN_modify_pointer==18)
		{
			//����BLE S/N	
			for(j=8;j<22;j++)
		  {
			  rcvbufdata[j]= buf_for_BLE_SN_modify[j-4];
		  }
			Received_BLE_NAME_flag=1;
//      SetBleName();			
			headerok_flag=0;
			buf_for_BLE_SN_modify_pointer=0;
			
			
		}
		
		
		
		
		//debug serial port ��λַ��
		
		
		
		GetChar=1;
			
		#ifdef SEND_PCM 
		if(Uart2Buffer[0]==0x55)  //��ʼ
		{
			   StartSend=1;
			   kpi=0;
		}
		else if(Uart2Buffer[0]==0xaa) //����
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
     
		else if (Uart2Buffer[0]==0x80)  //�ָ���������ΪĬ��ֵ
		{	
			      //while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				    //{
            //  osDelay(1);
				    //}
			      Airsensor.AirSensor_Checkposition_Mode=SLOW_MODE;	
            //xSemaphoreGive( xPositionMute );
			
						AppCtrlBag.enable=1;
						AppCtrlBag.tick=900;
			      ActionSteps=0;
						ValueDriver1(HEAD_R_0_VALUE,GPIO_PIN_RESET);
			      ValueDriver1(HEAD_R_1_VALUE,GPIO_PIN_RESET);
						ValueDriver1(HEAD_R_2_VALUE,GPIO_PIN_RESET);
						ValueDriver1(HEAD_R_3_VALUE,GPIO_PIN_RESET);
            Valve_State[0]=0;
			      Valve_State[1]=0;
            Valve_State[2]=0;
            Valve_State[3]=0;  			
						InhalePump(0);
						
		
		}
		else if (Uart2Buffer[0]==0x81)  //�������1
		{
							AppCtrlBag.enable=1;
							AppCtrlBag.tick=900;
							ActionSteps=1;
							ValueDriver1(HEAD_R_0_VALUE,GPIO_PIN_SET);
			        Valve_State[0]=1;
			
		
		}
		else if (Uart2Buffer[0]==0x82)  //�������2
		{
							AppCtrlBag.enable=1;
							AppCtrlBag.tick=900;
							ActionSteps=1;
							ValueDriver1(HEAD_R_1_VALUE,GPIO_PIN_SET);
			        Valve_State[1]=0;
		
		}
		else if (Uart2Buffer[0]==0x83)  //�������3
		{
							AppCtrlBag.enable=1;
							AppCtrlBag.tick=900;
							ActionSteps=1;		
							ValueDriver1(HEAD_R_2_VALUE,GPIO_PIN_SET);
			        Valve_State[2]=1;
		
		}
		else if (Uart2Buffer[0]==0x84)  //�������4
		{
							AppCtrlBag.enable=1;
							AppCtrlBag.tick=900;
							ActionSteps=1;
							ValueDriver1(HEAD_R_3_VALUE,GPIO_PIN_SET);
		          Valve_State[3]=1;
		}
/*
		else if (Uart2Buffer[0]==0x85)  //�������
		{       //  AppCtrlBag.enable=1;
						//	AppCtrlBag.tick=900;
						//	ActionSteps=1;
						//	InhalePump(1);
						//	osDelay(1000);
		
		}
		else if (Uart2Buffer[0]==0x86)  //���MIC
		{
		
		
		}
*/		
		else if(Uart2Buffer[0]==0x85)  //���λ��
		{
//						  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
//							ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
//							ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
//							ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
//							InhalePump(0);
							AppCtrlBag.enable=1;
							AppCtrlBag.tick=900;
							ActionSteps=0;
//							Airsensor.AirSensor_Checkposition_Mode=ONCE_TIME;
			
			        //while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				      //{
              //osDelay(1);
				     // }
			        Airsensor.AirSensor_Checkposition_Mode=FAST_MODE;	
              //xSemaphoreGive( xPositionMute );
							
			        RequestControlValve=0; 
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
	else if(huart==&huart3)  //������
	{
		//---------------------
    if(CheckBLETask == 1)
    {
      if(usart3_rx_count >= 50)
      {
        usart3_rx_count = 50-1;
      }
      usart3_rx_buf[usart3_rx_count] = Uart3Buffer;
      usart3_rx_count++;
    }
		else
		{
		  xQueueSendFromISR( UART3QHandle, &Uart3Buffer, &xHigherPriorityTaskWoken );
      //for debug	
      if(App_COMMAND_LOG==1)
			{				
			  BT_Recieved_Data_From_App_Buffer[BT_Recieved_Data_From_App_Buffer_Pointer]=Uart3Buffer;
			  BT_Recieved_Data_From_App_Buffer_Pointer++;
			  if(BT_Recieved_Data_From_App_Buffer_Pointer>=4000)
			  {
				  BT_Recieved_Data_From_App_Buffer_Pointer=0;
			  }
		  }
			//
		}
		Uart3InterruptOn();
	  
	}
}


void SoftWareReset(void)
{
	   uint32_t I_delay=0;
	   RCC->CFGR |=RCC_CFGR_HPRE_DIV4;//GD32F405�������
	   for(I_delay=0xffff;I_delay>0;I_delay--);
		 __set_FAULTMASK(1);
		 HAL_NVIC_SystemReset();
}
extern unsigned char SendData[1000];



void Upgrade_Finish_LOG(void)
{
	int offset=0;
	offset += sprintf(SendData+offset,"Upgrade Finished,will restart firmware again...\n");  	 
	uart_send(SendData, strlen(SendData));
}

void Upgrad_LOG_Display(void)
{
	int offset=0;
	//if(SCB->VTOR==0x08080000)
	if( SCB->VTOR==IMAGE_B_ADDRESS)	
		
	{
		   offset += sprintf(SendData+offset,"IMAGE A Upgrade To: %1d.%1d.%3d\n",NeedUpgradePara.ver3,NeedUpgradePara.ver2,NeedUpgradePara.ver1);  
	 }
  else
	{
			 offset += sprintf(SendData+offset,"IMAGE B Upgrade To: %1d.%1d.%3d\n",NeedUpgradePara.ver3,NeedUpgradePara.ver2,NeedUpgradePara.ver1); 
	 }
	
	offset += sprintf(SendData+offset,"Firmware Size: %7d\n",NeedUpgradePara.firmwareSize); 
	offset += sprintf(SendData+offset,"Checksum: %7x\n",NeedUpgradePara.checksum);	
  offset += sprintf(SendData+offset,"Receiving data and upgrading...\n");  	 
	uart_send(SendData, strlen(SendData));									
										
	
}


void Formware_Version_LOG(void)
{
//	 uint8_t i;
	 int offset=0;
 	 	 offset += sprintf(SendData+offset,"System Poweron or Software Restart!\n");
//     if(SCB->VTOR==0x08080000)
		 if( SCB->VTOR==IMAGE_B_ADDRESS) 
		 {
		   offset += sprintf(SendData+offset,"RUN IMAGE B\n"); 
		  }
		 else
		 {
			 offset += sprintf(SendData+offset,"RUN IMAGE A\n");
		  }
		  
			offset += sprintf(SendData+offset,"IMAGE A VERSION: %4d\n",g_systemParameter.imgAver); 
      offset += sprintf(SendData+offset,"IMAGE B VERSION: %4d\n",g_systemParameter.imgBver);
			offset += sprintf(SendData+offset,"Latest Upgrade Version: %4d\n",g_systemParameter.upgradeType);
			
		 
		 
	   uart_send(SendData, strlen(SendData));
		 app_settime_flag=0;//ֻ����һ�������־
		 
}

int ResetToFactory(void)
{    
		gStorageDataResetDefault();
		BackupSysPara(&gStorageData);
		osDelay(10);
	  if(SENSORLEVEL_HIGH_FOR_24HOURS_ENABLED==1)
		{
	    Build_SensorLevelAutoSet_Flag(); //added by zjp for korea senseor level auto set	
		}
//		SoftWareReset();
		return 1;
}

int ResetToFactory1(void)
{    
		gStorageDataResetDefault1();
		BackupSysPara(&gStorageData);
		osDelay(10);
	  if(SENSORLEVEL_HIGH_FOR_24HOURS_ENABLED==1)
		{
	    Build_SensorLevelAutoSet_Flag(); //added by zjp for korea senseor level auto set	
		}
//		SoftWareReset();
		return 1;
}

void CheckSystemPoweronParameter(void)
{	
		#define OptionByteAddress1 0x1fffc000
		//#define OptionByteAddress1 0x8000000
		uint16_t old_option_byte,new_option_byte,k;
	if(Get_ChipSeriesID() == 0x6413) //STM32:6413 GD:0413
		{
	
		  old_option_byte = *(uint16_t *)(OptionByteAddress1);
	    new_option_byte = old_option_byte&0x0c;//0xaafb;
		  if((old_option_byte&0x0c) != 4)
		  {
			  HAL_FLASH_OB_Unlock();
			  if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) == RESET)
			  {
				  //FLASH->OPTCR = (FLASH->OPTCR&0xffff0003)|0xaae4;
					FLASH->OPTCR = (FLASH->OPTCR&0xfffffff3)|0x4;
				  SET_BIT(FLASH->OPTCR,FLASH_OPTCR_OPTSTRT);
				  while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET)
				  {
					  __NOP;
				  }
				
			  }
			  HAL_FLASH_OB_Lock();
		  }
		}
		
}

void debug_Formware_Version_LOG_1(void)
{
//	 uint8_t i;
	 int offset=0;
 	 	 offset += sprintf(SendData+offset,"RUN1\n");

		  
			offset += sprintf(SendData+offset,"g_systemParameter.upgradeType: %4d\n",g_systemParameter.upgradeType); 
      offset += sprintf(SendData+offset,"SOFTWARE_VERSION: %4d\n",SOFTWARE_VERSION);
			
	   uart_send(SendData, strlen(SendData));
		 app_settime_flag=0;//ֻ����һ�������־
		 
}
void debug_Formware_Version_LOG_2(void)
{
//	 uint8_t i;
	 int offset=0;
 	 	 offset += sprintf(SendData+offset,"RUN2\n");

		  
			offset += sprintf(SendData+offset,"g_systemParameter.imgBver: %4d\n",g_systemParameter.imgBver); 
      offset += sprintf(SendData+offset,"SOFTWARE_VERSION: %4d\n",SOFTWARE_VERSION);
			
	   uart_send(SendData, strlen(SendData));
		 app_settime_flag=0;//ֻ����һ�������־
		 
}

void debug_Formware_Version_LOG_3(void)
{
//	 uint8_t i;
	 int offset=0;
 	 	 offset += sprintf(SendData+offset,"RUN3\n");

		  
			offset += sprintf(SendData+offset,"g_systemParameter.imgAver: %4d\n",g_systemParameter.imgAver); 
      offset += sprintf(SendData+offset,"SOFTWARE_VERSION: %4d\n",SOFTWARE_VERSION);
			
	   uart_send(SendData, strlen(SendData));
		 app_settime_flag=0;//ֻ����һ�������־
		 
}

void CheckSystemParameter(void)
{	
	 int cureent_version;
	 //�ȼ���Ƿ���ϵͳ�����ı�������
	 if(GetSystemParameter1()!=1)
	 {
		 //�Ƚ���ϵͳ����������  ��ַ0x800c000
		 GetSystemParameter();
		 g_systemParameter1=g_systemParameter;
		 BackupSystemParameter1();
	 }
	 else
	 {
		 //���ϵͳ���������Ƿ�����
		 if(GetSystemParameter()==1)
		 {
			 //������������������ͱ������Ƿ�һ��
			 if(g_systemParameter.upgradeType!=g_systemParameter1.upgradeType || g_systemParameter.imgAver!=g_systemParameter1.imgAver ||	g_systemParameter.imgBver!=g_systemParameter1.imgBver)
			 {
         //��������ȫһ��
				 g_systemParameter1=g_systemParameter;
		     BackupSystemParameter1();
			 }				 
			 
		 }
		 else
		 {
			 //��������������
			 g_systemParameter=g_systemParameter1;
		   BackupSystemParameter();
		 }
		 
	 }
	 
	 osDelay(20);
	 
	 Formware_Version_LOG();
	 osDelay(20);
//	 SoftWareReset();
	 
	 
	 
	 //����Ƿ����������汾��ʵ�ʰ汾��һ�µ����������汾ʱ�汾����ȷ���߹�����а汾����
	 //Ҫ���������һ�������������𻵵������Ҳ�ܼ���������ʹ�����
	 
	 if(g_systemParameter.upgradeType!=SOFTWARE_VERSION && g_systemParameter1.upgradeType!=SOFTWARE_VERSION)
	 {
		 debug_Formware_Version_LOG_1();
		 g_systemParameter.upgradeType=SOFTWARE_VERSION;
		 
//     if( SCB->VTOR==0x08080000)
		 if( SCB->VTOR==IMAGE_B_ADDRESS) 	 
			g_systemParameter.imgBver=SOFTWARE_VERSION;
		 else g_systemParameter.imgAver=SOFTWARE_VERSION;
		 BackupSystemParameter();
		 g_systemParameter1=g_systemParameter;
		 BackupSystemParameter1();
		 //ResetToFactory1();//�汾�����ȶ����������ٻָ�����ֵ�����gStorageData�ֶ��б䣬�����ָ���������
		 SoftWareReset();

	 }
	 else
	 {
//		 if( SCB->VTOR==0x08080000)
		 if( SCB->VTOR==IMAGE_B_ADDRESS) 
		 {
       if(g_systemParameter.imgBver!=SOFTWARE_VERSION && g_systemParameter1.imgBver!=SOFTWARE_VERSION)
       {	
				 debug_Formware_Version_LOG_2();
			   g_systemParameter.imgBver=SOFTWARE_VERSION;
				 BackupSystemParameter();
				 g_systemParameter1.imgBver=SOFTWARE_VERSION;
				 BackupSystemParameter1();
				 //ResetToFactory1();//�汾�����ȶ����������ٻָ�����ֵ�����gStorageData�ֶ��б䣬�����ָ���������
				 //SoftWareReset();
//				 SoftWareReset();
			 }
		 }
		 else 
		 { 
			if(g_systemParameter.imgAver!=SOFTWARE_VERSION && g_systemParameter1.imgAver!=SOFTWARE_VERSION)
       { 
				 debug_Formware_Version_LOG_3();
			   g_systemParameter.imgAver=SOFTWARE_VERSION;
		     BackupSystemParameter();
				 g_systemParameter1.imgAver=SOFTWARE_VERSION;
		     BackupSystemParameter1();
				 //ResetToFactory1();//�汾�����ȶ����������ٻָ�����ֵ�����gStorageData�ֶ��б䣬�����ָ���������
				 //SoftWareReset();
//				 SoftWareReset();
			 }
		 }
	 }
	 //׷�Ӷ�Ver5.1.61�����⴦��
	 /*
	 if( SCB->VTOR==IMAGE_B_ADDRESS) 
	 {
			cureent_version = g_systemParameter.imgBver;
	 }
	 else if ( SCB->VTOR==IMAGE_A_ADDRESS)
	 {
		  cureent_version = g_systemParameter.imgAver;
	 }		 
	 if(cureent_version == 62)
	 {
		 if (gStorageData.ActionLevel < 10)
		 {
				gStorageData.flag=SYS_PARA_FLAG;
				gStorageData.SPIFlashAddress=0;
				gStorageData.SPIFlashOutAddress=0;
				if(gStorageData.ActionLevel == 0)
					gStorageData.ActionLevel = 30;
				else if(gStorageData.ActionLevel == 1)
					gStorageData.ActionLevel = 38;
				else if(gStorageData.ActionLevel == 2)
					gStorageData.ActionLevel = 45;
				else if(gStorageData.ActionLevel == 3)
					gStorageData.ActionLevel = 53;
				else if(gStorageData.ActionLevel == 4)
					gStorageData.ActionLevel = 60;
				else
					gStorageData.ActionLevel = 38;
				gStorageData.auto_detect_timecycle = 60; //ȱʡΪ1Сʱ��������һ��ͷ��λ�ü�⹦�ܴ�
				
				//gStorageData.ActionLevel=38;    //0:low 1:lower 2:mid 3:higher 4:high
				//gStorageData.SensorLevel=MID_CHECK; //�ϵ�(38�룩MID_CHECK; //1:�ߣ�2���� 3����
				//gStorageData.HeadpositionLevel=HIGH_CHECK;	//��������
				//gStorageData.MinSnoredB = LOW_SNOREDB;
				//gStorageData.PositionValue_Onebag = AirSensor_threshold_person_on_HIGH; //ͷ��λ�ü�ⵥ������ֵ
				//gStorageData.PositionValue_Twobag = AirSensor_threshold_person1_on_HIGH;//ͷ��λ�ü��˫����֮�ͷ�ֵ
				//gStorageData.Body_detect_on_threshold = person_on_threshold;
				//gStorageData.Body_detect_off_threshold = stable_down_threshold;
				//gStorageData.One_hour_auto_detect_onoff = 0x1; //ȱʡΪ1Сʱ��������һ��ͷ��λ�ü�⹦�ܴ�
				
				//gStorageData.StudyMode=0;
				//gStorageData.Standby=1;       //���ڴ�����״̬
	      //gStorageData.Standby=1;       
		    //gStorageData.WorkDelay=0;
		    //gStorageData.InhaleOver=0;
		    //gStorageData.ExhaleOver=0;
			  //gStorageData.workTimeOn=0;
	      //gStorageData.workTime[0][0]=21;
			  //gStorageData.workTime[0][1]=30;
			  //gStorageData.workTime[0][2]=9;
			  //gStorageData.workTime[0][3]=0;
	
			  //gStorageData.workTime[1][0]=12;
			  //gStorageData.workTime[1][1]=30;
			  //gStorageData.workTime[1][2]=15;
			  //gStorageData.workTime[1][3]=0;
			
			
	    
	      //gStorageData.UserSleepTime[0]=UserSleepTimeStartHour;
			  //gStorageData.UserSleepTime[1]=UserSleepTimeStartMin;
			  //gStorageData.UserSleepTime[2]=UserSleepTimeEndHour;
			  //gStorageData.UserSleepTime[3]=UserSleepTimeEndMin;
			
			  BackupSysPara(&gStorageData);
				
		 }
	 }
	 */
	 if (gStorageData.ActionLevel < 10 || gStorageData.ActionLevel > 60)
	 {
				gStorageData.flag=SYS_PARA_FLAG;
				gStorageData.SPIFlashAddress=0;
				gStorageData.SPIFlashOutAddress=0;
				if(gStorageData.ActionLevel == 0)
					gStorageData.ActionLevel = 30;
				else if(gStorageData.ActionLevel == 1)
					gStorageData.ActionLevel = 38;
				else if(gStorageData.ActionLevel == 2)
					gStorageData.ActionLevel = 45;
				else if(gStorageData.ActionLevel == 3)
					gStorageData.ActionLevel = 53;
				else if(gStorageData.ActionLevel == 4)
					gStorageData.ActionLevel = 60;
				else
					gStorageData.ActionLevel = 38;
				gStorageData.auto_detect_timecycle = 60; //ȱʡΪ1Сʱ��������һ��ͷ��λ�ü�⹦�ܴ�
				BackupSysPara(&gStorageData);
		}
	 if (gStorageData.auto_detect_timecycle < 10 || gStorageData.auto_detect_timecycle > 60)
	 {
		 gStorageData.auto_detect_timecycle =60;
		 BackupSysPara(&gStorageData);
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
unsigned char year,month,date,hour,minute,second,add_bit;
void Datetime_LOG(void)
{
//	 uint8_t i;
	 int offset=0; 	 	 
		  
			
	    offset += sprintf(SendData+offset,"SPI Year: %4d\n",sdatestructure.Year); 
	    offset += sprintf(SendData+offset,"SPI Month: %4d\n",sdatestructure.Month);
			offset += sprintf(SendData+offset,"SPI Day: %4d\n",sdatestructure.Date);
	
      
			
		 
		 
	   uart_send(SendData, strlen(SendData));
		 app_settime_flag=0;//ֻ����һ�������־
		 
}
unsigned char check_if_leapyeaar(uint16_t year)
{
	  uint16_t x;
		x = year % 400;
		if( (x % 4)==0 && x != 100 && x != 200 && x != 300)
		{
			return 1;
		}
		else
		{
			return 0;
		}	
}

uint8_t GetDaysofMonth(uint16_t year,uint8_t month)
{
		uint8_t leapdays[12] = {31,29,31,30,31,30,31,31,30,31,30,31};
		uint8_t normaldays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
		if(check_if_leapyeaar(year) == 1)
		{
			return leapdays[month-1];
		}
		else
		{
			return normaldays[month-1];
		}	
}

uint16_t GetDaysofYear(uint16_t year)
{
		if(check_if_leapyeaar(year) == 1)
		{
			return 366;
		}
		else
		{
			return 365;
		}	
}
void RTC_reset_process(void)
{
		uint16_t d1_year,d2_year,year;
	  uint8_t d1_month,d2_month,month;
	  uint8_t d1_day,d2_day,day;
	  uint8_t d1_hour,d2_hour,hour;
		uint8_t d1_minute,d2_minute,minute;
		uint8_t d1_second,d2_second,second;
	  unsigned int sum_day;
	  uint8_t add_bit;
		uint16_t i;
	
		//��ȡRTCʱ�䣬�ж��Ƿ��б���λ�����ñ����ʱ�����д��RTC
		HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);
	
		//Datetime_LOG();
		 
		if(sdatestructure.Year < 22)
		{
			d1_year = sdatestructure.Year;
			d1_month = sdatestructure.Month;
			d1_day = sdatestructure.Date;
			d1_hour = stimestructure.Hours;
			d1_minute = stimestructure.Minutes;
			d1_second = stimestructure.Seconds;
			
			//d1_year = 1;
			//d1_month = 3;
			//d1_day = 1;
			//d1_hour = 0;
			//d1_minute = 5;
			//d1_second = 30;
			
			Read_Backup_RTC(&sdatestructure,&stimestructure);
			
			//Datetime_LOG();
			
			d2_year = sdatestructure.Year+2000;
			d2_month = sdatestructure.Month;
			d2_day = sdatestructure.Date;
			d2_hour = stimestructure.Hours;
			d2_minute = stimestructure.Minutes;
			d2_second = stimestructure.Seconds; 
			
			//d2_year = 1;
			//d2_month = 3;
			//d2_day = 1;
			//d2_hour = 11;
			//d2_minute = 59;
			//d2_second = 55; 
			
			add_bit = 0;
			sum_day = 0;
			
			//�ȼ���ʱ��
			if( d1_second+d2_second >= 60)
			{
					second = d1_second+d2_second -60;
					add_bit = 1;
			}
			else
			{
					second = d1_second+d2_second;
			}

			if( d1_minute+d2_minute+add_bit >= 60)
			{
					minute = d1_minute+d2_minute+add_bit -60;
					add_bit = 1;
			}
			else
			{
					minute = d1_minute+d2_minute+add_bit;
					add_bit = 0;
			}

			if( d1_hour+d2_hour+add_bit >= 24)
			{
					hour = d1_hour+d2_hour+add_bit -24;
					add_bit = 1;
			}
			else
			{
					hour = d1_hour+d2_hour+add_bit;
					add_bit = 0;
			}
			 
			//����RTC��λ�����е���		
			for(i=0;i<d1_year;i++)
			{
					sum_day += GetDaysofYear(i);
					
			}
			if( d1_month > 1)
			{
					for(i=1;i<d1_month;i++)
					{
							sum_day += GetDaysofMonth(d1_year,i);
					}
			}
			sum_day = sum_day +d1_day-1; 
			
			//�ۼӱ������ڵ��������
			if( d2_month > 1)
			{
					for(i=1;i<d2_month;i++)
					{
							sum_day += GetDaysofMonth(d2_year,i);
					}
			}
			sum_day = sum_day +d2_day-1+add_bit;
			
			//������ӵ�����
			while(1)
			{  
					if( sum_day >= GetDaysofYear(d2_year))
					{
							sum_day = sum_day - GetDaysofYear(d2_year);
							d2_year = d2_year + 1;
					}
					else
					{
							break;
					}
			}

			d2_month = 1;
			while(1)
			{    
					if( sum_day >= GetDaysofMonth(d2_year,d2_month))
					{
							sum_day = sum_day - GetDaysofMonth(d2_year,d2_month);
							d2_month = d2_month + 1;
					}
					else
					{
              break;
					}
      }			
			d2_day=sum_day+1;
			
			sdatestructure.Year = d2_year - 2000;
			sdatestructure.Month = d2_month;
			sdatestructure.Date = d2_day;
			stimestructure.Hours = d2_hour;
			stimestructure.Minutes = d2_minute;
			stimestructure.Seconds = d2_second; 
			
			sdatestructure.WeekDay=RTC_WEEKDAY_MONDAY;
      stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		  stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;									
			HAL_RTC_SetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
      HAL_RTC_SetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);				 
			 			
		}	
	
}


void userStart(void)
{   
	   LOGO_ON=false;
		 Config.SnoreAverTime = 4000; 
		 
	   InitStorage();
	   
	   if(Date_Time_Save_Enable==1)
		 {
		   RTC_reset_process();	
		 }
	
		 SnoreInit();
//	   Position_Init();
	   snore_mindb_Init();
	   User_set_positionvalue();
	   Body_Detect_Init();
	
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
   uart_printf("������  ����״̬   ���ȼ�   ʣ��ջ   �������\r\n");
   vTaskList((char *)&pcWriteBuffer);
	 uart_send(pcWriteBuffer,strlen(pcWriteBuffer));
 //  uart_printf("%s\r\n", pcWriteBuffer); 
   uart_printf("\r\n");	
   //uart_printf("������  ���м���   ʹ����\r\n");
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




unsigned int GetNeedUpgradeImageType(void)
{
   if(SCB->VTOR==0x08020000)
   {
	   return 2;   //B
   }
	 
//   else if(SCB->VTOR==0x08080000)
	 else if( SCB->VTOR==IMAGE_B_ADDRESS)	 
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


//extern uint8_t SendData[60];
void Airsensor_Print_RTC_Time(void)
{
	      int offset=0;
 	 
	 
		    offset += sprintf(SendData+offset,"%2d ",sdatestructure.Year);
		    offset += sprintf(SendData+offset,"%2d ",sdatestructure.Month);	
		    offset += sprintf(SendData+offset,"%d ",sdatestructure.Date);
	      offset += sprintf(SendData+offset,"%2d ",stimestructure.Hours);
		    offset += sprintf(SendData+offset,"%2d ",stimestructure.Minutes);	
		    offset += sprintf(SendData+offset,"%d ",stimestructure.Seconds);		    	
	      offset += sprintf(SendData+offset,"#\n");		 
	      uart_send(SendData, strlen(SendData));
	
}

void Airsensor_Print_Tuwan_Date(void)
{
	      int offset=0;
 	 
	      offset += sprintf(SendData+offset,"\n\n");
		    offset += sprintf(SendData+offset,"%2d ",gStorageData.Snore.StartTime[0]);
		    offset += sprintf(SendData+offset,"%2d ",gStorageData.Snore.StartTime[1]);	
		    offset += sprintf(SendData+offset,"%2d ",gStorageData.Snore.StartTime[2]);
	      offset += sprintf(SendData+offset,"%2d ",gStorageData.Snore.StartTime[3]);
		    offset += sprintf(SendData+offset,"%2d ",gStorageData.Snore.StartTime[4]);	
		    offset += sprintf(SendData+offset,"%2d ",0);		    	
//	      offset += sprintf(SendData+offset,"\n");		 
	      uart_send(SendData, strlen(SendData));
	
}




void Airsensor_Print_Tuwan_Time(void)
{
	      int offset=0;
 	 
	 
		    offset += sprintf(SendData+offset,"%2d ",detailed_snore.StartTime[0]);
		    offset += sprintf(SendData+offset,"%2d ",detailed_snore.StartTime[1]);	
		    offset += sprintf(SendData+offset,"%2d ",detailed_snore.StartTime[2]);
	      offset += sprintf(SendData+offset,"%2d ",detailed_snore.StartTime[3]);
	      if(detailed_snore.StartTime[4]!=0 || detailed_snore.SSnoreTimes!=0 || detailed_snore.MSnoreTimes!=0 || detailed_snore.LSnoreTimes!=0)
				{
					offset += sprintf(SendData+offset,"%2d ",detailed_snore.StartTime[4]);
		      offset += sprintf(SendData+offset,"%2d ",detailed_snore.SSnoreTimes);	
		      offset += sprintf(SendData+offset,"%2d ",detailed_snore.MSnoreTimes);
	        offset += sprintf(SendData+offset,"%2d ",detailed_snore.LSnoreTimes);
					if(detailed_snore.Airbag1Times!=0 || detailed_snore.Airbag2Times!=0 || detailed_snore.Airbag3Times!=0 || detailed_snore.Airbag4Times!=0)
				  {
					  offset += sprintf(SendData+offset,"%2d ",detailed_snore.Airbag1Times);
		        offset += sprintf(SendData+offset,"%2d ",detailed_snore.Airbag2Times);	
		        offset += sprintf(SendData+offset,"%2d ",detailed_snore.Airbag3Times);
	          offset += sprintf(SendData+offset,"%2d ",detailed_snore.Airbag4Times);	
            						
					  if(detailed_snore.person_on_minute!=0 || detailed_snore.person_on_second!=0 || detailed_snore.person_off_minute!=0 || detailed_snore.person_off_second!=0)
				    {
					  offset += sprintf(SendData+offset,"%2d ",detailed_snore.person_on_minute);
		        offset += sprintf(SendData+offset,"%2d ",detailed_snore.person_on_second);	
		        offset += sprintf(SendData+offset,"%2d ",detailed_snore.person_off_minute);
	          offset += sprintf(SendData+offset,"%2d ",detailed_snore.person_off_second);
						}							
				  }					
				}
	    	
	      offset += sprintf(SendData+offset,"\n");		 
	      uart_send(SendData, strlen(SendData));
	
}


uint8_t pump_record_process_repaet_flag=0;

void Airsensor_Print_Tuwan_Var(void)
{
	      int offset=0;
 	 
	      offset += sprintf(SendData+offset,"\n");
		    offset += sprintf(SendData+offset,"%2d ",pump_record_process_repaet_flag);
		    offset += sprintf(SendData+offset,"%2d ",pump_operation_time_read_pointer);	
		    offset += sprintf(SendData+offset,"%2d ",pump_operation_time_write_pointer);
	      offset += sprintf(SendData+offset,"%2d ",pump_operation_time[pump_operation_time_read_pointer][0]);
	      if((pump_operation_time_read_pointer+1)==20)
					offset += sprintf(SendData+offset,"%2d ",pump_operation_time[0][0]);
				else
	        offset += sprintf(SendData+offset,"%2d ",pump_operation_time[pump_operation_time_read_pointer+1][0]);
				if((pump_operation_time_read_pointer+2)==20)
					offset += sprintf(SendData+offset,"%2d ",pump_operation_time[0][0]);
				else if(pump_operation_time_read_pointer+2==21)
					offset += sprintf(SendData+offset,"%2d ",pump_operation_time[1][0]);
	      else 
					offset += sprintf(SendData+offset,"%2d ",pump_operation_time[pump_operation_time_read_pointer+2][0]);
	      offset += sprintf(SendData+offset,"\n");
	      
	    	
//	      offset += sprintf(SendData+offset,"\n");		 
	      uart_send(SendData, strlen(SendData));
	
}

                      

void print_geton_getup_time_log_for_app(void)
{
	
	      int offset=0;
	      unsigned char minute,second;
 	 
	      offset += sprintf(SendData+offset,"Device Date&Time: ");
		    offset += sprintf(SendData+offset,"%2d ",snoreData.StartTime[0]);
		    offset += sprintf(SendData+offset,"%2d ",snoreData.StartTime[1]);	
		    offset += sprintf(SendData+offset,"%2d ",snoreData.StartTime[2]);
	      offset += sprintf(SendData+offset,"%2d ",snoreData.StartTime[3]);
	      offset += sprintf(SendData+offset,"%2d ",snoreData.StartTime[4]);
	      
	      offset += sprintf(SendData+offset,"Ontime: ");
	
	      if(snoreData.person_on_second==0 && snoreData.person_on_minute==0)
				{
					offset += sprintf(SendData+offset,"No ");
				}
	      else
				{
					  if(snoreData.person_on_second<=0)
					  {
						   second =0-snoreData.person_on_second;
							 if(snoreData.person_on_minute<=snoreData.StartTime[4])
							  minute=snoreData.StartTime[4]-snoreData.person_on_minute;
							 else
							  minute=snoreData.StartTime[4]+60-snoreData.person_on_minute;
						}
					  else
					  {
							 second=0+60-snoreData.person_on_second;
							 if((snoreData.person_on_minute+1)<= snoreData.StartTime[4])
							  minute=snoreData.StartTime[4]-snoreData.person_on_minute-1;
							 else
								minute=snoreData.StartTime[4]+59-snoreData.person_on_minute;
						 }
            offset += sprintf(SendData+offset,"%2d ",minute);
	          offset += sprintf(SendData+offset,"%2d ",second);						
				}
	      
	
	      offset += sprintf(SendData+offset,"Offtime: ");
				
				
				if(snoreData.person_off_second==0 && snoreData.person_off_minute==0)
				{
					offset += sprintf(SendData+offset,"No ");
				}
	      else
				{
					  if(snoreData.person_off_second<=0)
					  {
						   second =0-snoreData.person_off_second;
							 if(snoreData.person_off_minute<=snoreData.StartTime[4])
							  minute=snoreData.StartTime[4]-snoreData.person_off_minute;
							 else
							  minute=snoreData.StartTime[4]+60-snoreData.person_off_minute;
						}
					  else
					  {
							 second=0+60-snoreData.person_off_second;
							 if((snoreData.person_off_minute+1)<= snoreData.StartTime[4])
							  minute=snoreData.StartTime[4]-snoreData.person_off_minute-1;
							 else
								minute=snoreData.StartTime[4]+59-snoreData.person_off_minute;
						 }
            offset += sprintf(SendData+offset,"%2d ",minute);
	          offset += sprintf(SendData+offset,"%2d ",second);						
				}
	
//				offset += sprintf(SendData+offset,"%2d ",snoreData.person_on_minute);
//	      offset += sprintf(SendData+offset,"%2d ",snoreData.person_off_minute);		
				
	      offset += sprintf(SendData+offset,"\n");	      
	    	
//	      offset += sprintf(SendData+offset,"\n");		 
	      uart_send(SendData, strlen(SendData));	
	
}

typedef struct times
{
	unsigned char year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second; 
}times;

int time_to_stamptime(times now_time)
{
	times temptime;	
	int stamptime;	
	temptime = now_time;
	struct tm tempstm;
	memset(&tempstm,0,sizeof(tempstm));
	tempstm.tm_year = temptime.year + 2000 -1900;
	tempstm.tm_mon =  temptime.month  - 1;
	tempstm.tm_mday = temptime.day;
	tempstm.tm_hour = temptime.hour;
	tempstm.tm_min = temptime.minute;
	tempstm.tm_sec = temptime.second;
	stamptime = (int)mktime(&tempstm);	
	return stamptime;
}

int time_add_time_to_stamptime(times now_time,unsigned char time_hours)
{
	times temptime;	
	int stamptime;	
	temptime = now_time;
	struct tm tempstm;
	memset(&tempstm,0,sizeof(tempstm));
	tempstm.tm_year = temptime.year + 2000 -1900;
	tempstm.tm_mon =  temptime.month  - 1;
	tempstm.tm_mday = temptime.day;
	tempstm.tm_hour = temptime.hour;
	tempstm.tm_min = temptime.minute;
	tempstm.tm_sec = temptime.second;
	stamptime = (int)mktime(&tempstm);
	stamptime += time_hours*3600;	
	return stamptime;
}

times time_add_time(times now_time,unsigned char time_hours)
{
	times temptime;
	time_t tick;
	int stamptime;
	char s[100];
	temptime = now_time;
	struct tm tempstm;
	memset(&tempstm,0,sizeof(tempstm));
	tempstm.tm_year = temptime.year + 2000 -1900;
	tempstm.tm_mon =  temptime.month  - 1;
	tempstm.tm_mday = temptime.day;
	tempstm.tm_hour = temptime.hour;
	tempstm.tm_min = temptime.minute;
	tempstm.tm_sec = temptime.second;
	stamptime = (int)mktime(&tempstm);
	stamptime += time_hours*3600;
	tick = (time_t)stamptime;
	tempstm = *localtime(&tick);
	temptime.year = tempstm.tm_year + 1900 - 2000;
	temptime.month = tempstm.tm_mon + 1;
	temptime.day = tempstm.tm_mday;
	temptime.hour = tempstm.tm_hour;
	temptime.minute = tempstm.tm_min;
	temptime.second = tempstm.tm_sec; 
	return temptime;
}

//int aa_stamptime;
/* MainTaskFunction function */
 void MainTaskFunction(void const * argument)
{
  /* USER CODE BEGIN 5 */
  unsigned char   SensorLevelbak,CheckSumDone=0,timelen; 
 	unsigned short temp,snoreAver=0,recordTotal=100;
	unsigned char buf[30],m=0xff,t1=0,*pt=NULL;
	unsigned int  flashBuf[4],i;
	uint8_t pump_time_read_temp,j,k,state,pump_temp;	
	times datetime_now;	
	
	/*
	timelen = 100;
  datetime_now.year = 23;
  datetime_now.month = 10;
  datetime_now.day = 23;
	datetime_now.hour = 21;
	datetime_now.minute = 41;
	datetime_now.second = 32;
	
	gStorageData.Modifystudymodestamptime = time_add_time_to_stamptime(datetime_now,timelen);
	
	datetime_now.year = 23;
  datetime_now.month = 10;
  datetime_now.day = 28;
	datetime_now.hour = 1;
	datetime_now.minute = 41;
	datetime_now.second = 33;
	
	aa_stamptime = time_to_stamptime(datetime_now);
	*/
	
	//OnOffLed(1);
	xSemaphore = xSemaphoreCreateMutex();
	xTimeMute = xSemaphoreCreateMutex(); //������ȡϵͳʱ��Ļ����ź���
	xPositionMute = xSemaphoreCreateMutex();//λ�ü���������֮��Ļ����ź���
	xLogoMute = xSemaphoreCreateMutex();//λ�ü���������֮��Ļ����ź��
	
	
//  BlueToolthEnabled(1);//ϵͳ�����ϵ�
	CheckSystemPoweronParameter();
//  CheckSystemParameter();
	Uart3InterruptOn();
//  CheckBleName(); //���������豸��ʶ
	BLEInit();
	
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
	if(SENSORLEVEL_HIGH_FOR_24HOURS_ENABLED==1)
	{
    check_sensorlevel_autoset(); //added by zjp for v.3.1.29 for korea
	}
	
	if(USERKEYCODE_ENABLE==1)
	{
    check_userkeycode_autoset(); //added by zjp for v.3.1.29 for korea
	}
	
	if(USERVALIDTIME_ENABLE==1)
	{
    check_validtime_autoset(); //added by zjp for v.3.1.29 for korea
	}
	//gStorageDataResetDefault();
	//check_validtime_autoset();
	//check_userkeycode_autoset();
	//claer_validtimeflag();
	userStart();
	CheckSystemParameter();	
	
	InitPDMLib();
	Get_F407ChipID();
//	Uart3InterruptOn();
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
	
	//*****************��ʾ�ϵ��������debug��Ϣ*******************************
//	LOGO_Reset_Output();
//	osDelay(20);
	//*************************************************************************
	
	
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
										if(buf[2]>60 || buf[2]< 10)
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
										if(buf[2]== 0 || (buf[2] >= 30 && buf[2] <= 120))
										{
											gStorageData.WorkDelay=buf[2];
											OK_Command(buf,WORK_DELAY);
											uart3Send((unsigned char *)buf,4);
											BackupSysPara(&gStorageData);
											//�������˯��״̬������ˢ���ӳ�ʱ��
											if(Airsensor.Tt_Is_Sleep_Time==1 && buf[2] >= 30 && buf[2] <= 120)
											{
												workdelay_count_timer = 0;
												workdelay_time_cycle_flag = 1;												
											}
										}
										else
										{
											ERROR_Command(buf,WORK_DELAY);
											uart3Send((unsigned char *)buf,4);											
										}
										break;
										
									case SET_WORK_MODE:
										if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[4], 100)==pdFALSE)
												continue;
										
										if((buf[4]==0x03) && (buf[2]==0))
										{
											//����ֹ��ģʽ
											
											gStorageData.StudyMode = 0;
											
											gStorageData.Modifystudymodestamptime = 0;
											
                      BackupSysPara(&gStorageData);
										  OK_Command(buf,SET_WORK_MODE);											
										  uart3Send((unsigned char *)buf,4);
											SnoreInit_personon();
                      continue;											
										}
										else if((buf[4]==0x03) && (buf[2]==1))
										{
											//��������ѧϰģʽ����ֹ��
//											
										  gStorageData.StudyMode = 1;	

											if(buf[3] != 0 )	
											{
												timelen = buf[3];
												datetime_now.year = sdatestructure.Year;
												datetime_now.month = sdatestructure.Month;
												datetime_now.day = sdatestructure.Date;
												datetime_now.hour = stimestructure.Hours;
												datetime_now.minute = stimestructure.Minutes;
												datetime_now.second = stimestructure.Seconds;
		
												gStorageData.Modifystudymodestamptime = time_add_time_to_stamptime(datetime_now,timelen);
											}
											else
											{
												gStorageData.Modifystudymodestamptime = 0;
											}
												
                      BackupSysPara(&gStorageData);
										  OK_Command(buf,SET_WORK_MODE);
										  uart3Send((unsigned char *)buf,4);
											SnoreInit_personon();	
                      continue;											
										}										
										else
										{
											ERROR_Command(buf,SET_WORK_MODE);
											uart3Send((unsigned char *)buf,4);
											continue;											
										}
										break;									
								/*		
									case SET_USER_KEYCODE:
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
										if(buf[6] != 0x03)
										{
										  ERROR_Command(buf,SET_WORK_MODE);
											uart3Send((unsigned char *)buf,4);
											continue;	
										}
										else
										{
											UserkeycodeAutoSetData.userkeycode1 = buf[2];
											UserkeycodeAutoSetData.userkeycode2 = buf[3];
											UserkeycodeAutoSetData.userkeycode3 = buf[4];
											UserkeycodeAutoSetData.userkeycode4 = buf[5];
											update_userkeycode_autoset();
											OK_Command(buf,SET_USER_KEYCODE);
										  uart3Send((unsigned char *)buf,4);
											continue;	
											
										}
										break;
								*/
								/*		
									case INPUT_USER_KEYCODE:
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
										if(buf[6] != 0x03)
										{
										  ERROR_Command(buf,INPUT_USER_KEYCODE);
											uart3Send((unsigned char *)buf,4);
											continue;	
										}
										else
										{
											if(UserkeycodeAutoSetData.userkeycode1 == buf[2] && UserkeycodeAutoSetData.userkeycode2 == buf[3] && UserkeycodeAutoSetData.userkeycode3 == buf[4] && UserkeycodeAutoSetData.userkeycode4 == buf[5])
											{
											  OK_Command(buf,INPUT_USER_KEYCODE);
										    uart3Send((unsigned char *)buf,4);
											  continue;
											}
											else
											{
												ERROR_Command(buf,INPUT_USER_KEYCODE);
											  uart3Send((unsigned char *)buf,4);
											  continue;	
											}
										}
										break;
									*/	
									case SET_VALID_TIME:
										if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[4], 100)==pdFALSE)
												continue;
									  if(xQueueReceive(UART3QHandle, &buf[5], 100)==pdFALSE)
												continue;
										
										if(buf[5] != 0x03)
										{
										  ERROR_Command(buf,SET_VALID_TIME);
											uart3Send((unsigned char *)buf,4);
											continue;	
										}
										else
										{
											UserkeycodeAutoSetData.validtime_year = buf[2];
											UserkeycodeAutoSetData.validtime_month = buf[3];
											UserkeycodeAutoSetData.validtime_day = buf[4];
											
											update_validtime_autoset();
											OK_Command(buf,SET_VALID_TIME);
										  uart3Send((unsigned char *)buf,4);
											continue;	
											
										}
										break;
									case GET_VALID_TIME:
										if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										
										if(buf[2] != 0x03)
										{
										  ERROR_Command(buf,GET_VALID_TIME);
											uart3Send((unsigned char *)buf,4);
											continue;	
										}
										else
										{
											
											buf[0]=0x02;
									  	buf[1]=GET_VALID_TIME;
									  	buf[2]=UserkeycodeAutoSetData.validtime_year;
									  	buf[3]=UserkeycodeAutoSetData.validtime_month;
											buf[4]=UserkeycodeAutoSetData.validtime_day;
									  	buf[5]=3;
											uart3Send((unsigned char *)buf,6);
											
										}
										break;	
										
		#if 0							
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
											//��ʾģʽ
                      if(buf[2] != 2)
                      {
                        gStorageData.StudyMode = 0;     //ǿ��Ϊ����ģʽ
									      RequestControlValve=0;  //added by zjp 2020.06.08�޸�����
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
                    #if 0 //����20200608
                    if(gStorageData.Standby==0)
										{
											//����ģʽ
											AppCtrlBag.enable=1;
											AppCtrlBag.tick=500;											
											ResetValuePump();
//											gStorageData.Standby=1;
										} 
                    #endif									
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
												memset(&gStorageData.Snore,0,sizeof(SnoreData_def));
												gStorageData.Snore.HeadPosion = 0x0ff;
												gStorageData.Snore.UsedRecord=0x55aa;
//												gStorageData.Snore.ApneaTimes=0;
//												gStorageData.Snore.HeadMovingTimes = 0;
												gStorageData.Snore.StudyMode=gStorageData.StudyMode;
//												gStorageData.Standby=0;
												BackupSysPara(&gStorageData);
											}
											else if((gStorageData.Standby==0)&&(buf[2]==1))
											{
												AppCtrlBag.enable=1;
												AppCtrlBag.tick=500;
//												gStorageData.Standby=1;
												ResetValuePump();
											}
											
											
											
											OK_Command(buf,SET_WORK_STATUS);
											uart3Send((unsigned char *)buf,4);

											BackupSysPara(&gStorageData);
											break;
		#endif									
											
									case RESET_FACTORY_DEFAULT:
										

											
									
											//InitStorage();
//											SnoreInit();	
									    OK_Command(buf,RESET_FACTORY_DEFAULT);
											uart3Send((unsigned char *)buf,4);	
											ResetToFactory();
    									SoftWareReset();
											break;
									
									case GET_DEVICE_READY:										
									
									    buf[0]=0x02;
									  	buf[1]=GET_DEVICE_READY;
									    if(gStorageData.Standby==1)
											{
												buf[2]=0;
											}
									    else if(gStorageData.Standby==0 && Airsensor.Tt_Is_Sleep_Time==0)
											{
												buf[2]=1;
											}
									    else if(gStorageData.Standby==0 && Airsensor.Tt_Is_Sleep_Time==1)
											{
                        buf[2]=2;        
											}												
									    else
											{
												buf[2]=0; 
											}									  	
									  	buf[3]=3;
										  uart3Send((unsigned char *)buf,4);
									
//											OK_Command(buf,GET_DEVICE_READY);
//											uart3Send((unsigned char *)buf,4);											
											break;
									
									case SET_WORK_STATE:
											if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
											if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
											if((buf[3]!=0x03)||(buf[2]>1))
											{
												ERROR_Command(buf,SET_WORK_STATE);
												uart3Send((unsigned char *)buf,4);
												continue;
											}
											if(buf[2] == 0)
											{
												APP_setworkstate = 1;//app���ô���ģʽ
											}
											else if(buf[2] == 1)
											{
												APP_setworkstate = 2;//app����˯��ģʽ
											}												
											else
											{
												APP_setworkstate = 0;//app�����ù���״̬
											}
											OK_Command(buf,SET_WORK_STATE);
											uart3Send((unsigned char *)buf,4);										
											break;
											
									case GET_SLEEP_DATA_CNT:
										  temp=GetSnoreDataCnt();
									    if(temp>SNORE_DATA_CNT)
										  {	
												temp=0;
										  }
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
										
//									  recordTotal--;
										//�жϼ�¼��˯�����ݻ��Ǳó����ʱ������
										
										if(snoreData.StudyMode==0x80)
										{
											//�����ó����ʱ�����Ϣ
											
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
											buf[11]=snoreData.Airbag1Times;
											buf[12]=snoreData.Airbag2Times;
											buf[13]=snoreData.Airbag3Times;
											buf[14]=snoreData.Airbag4Times;
											buf[15]= snoreData.person_on_minute;
											buf[16]= snoreData.person_on_second;
											buf[17]= snoreData.person_off_minute;
											buf[18]= snoreData.person_off_second;
											buf[19]= 0x03;
											
										}
										else
										{
											//����˯������
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
//										buf[16]= snoreData.HeadMovingTimes; //��ʱ���
										
										//���������ò���ʱ�仹���ϴ����´�ʱ��
										
										  buf[16]= ((snoreData.person_on_minute<<2)&0x0fc) | ((snoreData.person_on_second>>4)&0x03);
										  buf[17]= ((snoreData.person_on_second<<4)&0x0f0) | ((snoreData.person_off_minute>>2)&0x0f);
										  buf[18]= ((snoreData.person_off_minute<<6)&0x0c0) | ((snoreData.person_off_second&0x03f));
										
										  buf[19]=3;
											
											#if 0  //��ӡ���´�ʱ����Ϣ�Ա�˲���Ϣ��ʧ���ߴ���
											
											osDelay(20);
											print_geton_getup_time_log_for_app();
											osDelay(20);
											
											#endif
									  }
									 	uart3Send((unsigned char *)buf,20);	
//									uart3Send((unsigned char *)buf,17);	
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
										if(xQueueReceive(UART3QHandle, &buf[7], 100)==pdFALSE)
												continue;
										
										sdatestructure.Year=buf[2];
										sdatestructure.Month=buf[3];
										sdatestructure.Date=buf[4];
										sdatestructure.WeekDay=RTC_WEEKDAY_MONDAY;
					
										stimestructure.Hours=buf[5];
										stimestructure.Minutes=buf[6];										
//										stimestructure.Seconds=10;
										stimestructure.Seconds=buf[7];
					
										stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
										stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;
										
										if(Upgrading_On==0)
										{
										  if (HAL_RTC_SetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN) != HAL_OK)
										  {
											//Error_Handler();
										  }
										  if (HAL_RTC_SetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN) != HAL_OK)
										  {
												//Error_Handler();
										  }
										}
										OK_Command(buf,SET_TIME);
										uart3Send((unsigned char *)buf,4);
										app_settime_flag=1;
										//if(gStorageData.workTimeOn==0)
										//{		
										//	gStorageData.workTimeOn=1;
										//	BackupSysPara(&gStorageData);	
										//}
										break;
										
									case GET_TIME:
										  buf[0]=0x02;
										  buf[1]=GET_TIME;
									    buf[2]=sdatestructure.Year;										
									    buf[3]=sdatestructure.Month;
										  buf[4]=sdatestructure.Date;
                      buf[5]=0x0;									
										  buf[6]=stimestructure.Hours;
										  buf[7]=stimestructure.Minutes;
									    buf[8]=stimestructure.Seconds;
									    buf[9]=3;
									    uart3Send((unsigned char *)buf,10);
										break;
										
										
									case GET_FIRMWARE_VERSION:
										buf[0]=0x02;
									  	buf[1]=GET_FIRMWARE_VERSION; 
									  	buf[2]=SOFTWARE_VERSION;  //2
									  	buf[3]=MASTER_VERSION;  //1
									  	buf[4]=BOARD_VERSION; //5  5.1.2
									  	buf[5]=3;
										uart3Send((unsigned char *)buf,6);
									  Upgrading_On=0;
										break;
									case REQUEST_UPGRADE:
										if(gStorageData.Standby == 0)
										{
											ERROR_Command(buf,REQUEST_UPGRADE);
											uart3Send((unsigned char *)buf,4);
											break;
										}
										if(Update_Presskey3S_Enable==1)
										{
											//****** ֧������ʱ����3��
											Firmware_update_keypressed = 0;
											Firmware_update_received_flag = 1;
											//******
										}
										else
										{
											buf[0]=0x02;
											buf[1]=REQUEST_UPGRADE;
											buf[2]=GetNeedUpgradeImageType();
											buf[3]=3;
											uart3Send((unsigned char *)buf,4);
										}
                    
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
										UpgradeCtrl.sector_sequence=0;
										UpgradeCtrl.block_package_number=32; //����512K����Ӧ��32����
										memset(UpgradeCtrl.package_number_ever_recieved,0,32);
										memset(UpgradeCtrl.UpgradeBuffer,0,512);										
//										osDelay(20);
//										Upgrad_LOG_Display();
//										osDelay(20);
										OK_Command(buf,SEND_FIRMWARE_PARA);
										uart3Send((unsigned char *)buf,4);
										Upgrading_On=1;
										break;
										

										
									case SEND_FIRMWARE_DATA:
										if(NeedUpgradePara.upgradeOk==0)
										{
											ERROR_Command(buf,SEND_FIRMWARE_PARA);
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
										if(xQueueReceive(UART3QHandle, &buf[19], 100)==pdFALSE)
												continue;
									
										if(buf[19]!=0x03)
									  	{
										  	ERROR_Command(buf,SEND_FIRMWARE_DATA);
												uart3Send((unsigned char *)buf,4);
												break;
									
									
									  	}
										
//										UpgradeCtrl.sum=UpgradeCtrl.sum+(buf[2]+buf[3]+buf[4]+buf[5]+buf[6]+buf[7]+buf[8]+buf[9]+buf[10]
//										+buf[11]+buf[12]+buf[13]+buf[14]+buf[15]+buf[16]+buf[17]);
											
										//�������к��Ƿ���ȷ
										k=buf[18]&0x7f;
										if(k<32)
										{
											  if(UpgradeCtrl.package_number_ever_recieved[k]==0)
												{
													//δ���յ���
													UpgradeCtrl.package_number_ever_recieved[k]=1;
													UpgradeCtrl.sum=UpgradeCtrl.sum+(buf[2]+buf[3]+buf[4]+buf[5]+buf[6]+buf[7]+buf[8]+buf[9]+buf[10]
										                      +buf[11]+buf[12]+buf[13]+buf[14]+buf[15]+buf[16]+buf[17]);
													memcpy((unsigned char *)(((unsigned int )(UpgradeCtrl.UpgradeBuffer))+k*16),&buf[2],16);
										    }
												if((buf[18]&0x80)==0x80)
												{
													//�������һ������
													UpgradeCtrl.block_package_number=k+1;													
												}																						  
										}
										else
										{
											  ERROR_Command(buf,SEND_FIRMWARE_DATA);
												uart3Send((unsigned char *)buf,4);
												break;
										}

											
//										if(UpgradeCtrl.pos<=(512-16))
//										{		
//										  memcpy((unsigned char *)(((unsigned int )(UpgradeCtrl.UpgradeBuffer))+UpgradeCtrl.pos),&buf[2],16);	
//										  UpgradeCtrl.pos+=16;	
//										}	
										
                    										
										break;
									case SEND_FIRMWARE_CHK:
										if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
											
										//Ϊ��������
                    if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										
 										// Ϊ�������ӽ���
										
										if(CheckSumDone==1)
										{
												buf[0]=0x2;
												buf[1]=0x10;
												buf[2]=SEND_FIRMWARE_CHK;
												buf[3]=UpgradeCtrl.sector_sequence;
												buf[4]=0x03;
									      uart3Send((unsigned char *)buf,5);
										    break;
										}	
										else if(CheckSumDone==2)
										{
												
												buf[0]=0x2;
												buf[1]=0x11;
												buf[2]=SEND_FIRMWARE_CHK;
												buf[3]=UpgradeCtrl.sector_sequence;
												buf[4]=0x03;
									      uart3Send((unsigned char *)buf,5);
											  CheckSumDone=2;		

										}
											
											
									  if(buf[2]!=UpgradeCtrl.sum)
									  {
												
												buf[0]=0x2;
												buf[1]=0x11;
												buf[2]=SEND_FIRMWARE_CHK;
												buf[3]=UpgradeCtrl.sector_sequence;
												buf[4]=0x03;
									      uart3Send((unsigned char *)buf,5);
											  CheckSumDone=2;		

									  }
									  else 
									  {
											
											//׷�Ӽ����Ƿ��յ�����
											state=0;
											for(k=0;k<UpgradeCtrl.block_package_number;k++)
											{  
												if(UpgradeCtrl.package_number_ever_recieved[k]==0)
												{
													state=1;
												}
											}
											if(state==0)												
											{
												if(UpgradeCtrl.sector_sequence==buf[3])
												{
												  ProgramFlashBlock(NeedUpgradePara.address,128,UpgradeCtrl.UpgradeBuffer);
												  NeedUpgradePara.address+=512;
													
//												  OK_Command(buf,SEND_FIRMWARE_CHK);
													buf[0]=0x2;
													buf[1]=0x10;
													buf[2]=SEND_FIRMWARE_CHK;
													buf[3]=UpgradeCtrl.sector_sequence;
													buf[4]=0x03;
									        uart3Send((unsigned char *)buf,5);
											    CheckSumDone=1;
													UpgradeCtrl.sector_sequence++;
												} 
                        else
												{													
												  buf[0]=0x2;
													buf[1]=0x11;
													buf[2]=SEND_FIRMWARE_CHK;
													buf[3]=UpgradeCtrl.sector_sequence;
													buf[4]=0x03;
									        uart3Send((unsigned char *)buf,5);
											    CheckSumDone=2;													
												}
												
											}
											else
											{
												buf[0]=0x2;
												buf[1]=0x11;
												buf[2]=SEND_FIRMWARE_CHK;
												buf[3]=UpgradeCtrl.sector_sequence;
												buf[4]=0x03;
											  CheckSumDone=2;		
											}
									  }
										
										memset(UpgradeCtrl.UpgradeBuffer,0,512);
										UpgradeCtrl.pos=0;
										UpgradeCtrl.sum=0;										
										UpgradeCtrl.block_package_number=32;
										memset(UpgradeCtrl.package_number_ever_recieved,0,32);
										
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
											  //���������ɹ�֮ǰ���Ƚ��а汾����ϵͳע�ᣬ����ע��ʱ�û��ϵ�
											  if(NeedUpgradePara.firmwareType==1)
												{
													//��������A�汾
													g_systemParameter.imgAver=NeedUpgradePara.ver1;
													g_systemParameter.upgradeType=NeedUpgradePara.ver1;
												}
												else
												{
													//��������B�汾
													g_systemParameter.imgBver=NeedUpgradePara.ver1;
													g_systemParameter.upgradeType=NeedUpgradePara.ver1;													
												}
											  BackupSystemParameter();
												
												//���������ɹ�֮ǰ���Ƚ��а汾����ϵͳע�ᣬ����ע��ʱ�û��ϵ�
											  if(NeedUpgradePara.firmwareType==1)
												{
													//��������A�汾
													g_systemParameter1.imgAver=NeedUpgradePara.ver1;
													g_systemParameter1.upgradeType=NeedUpgradePara.ver1;
												}
												else
												{
													//��������B�汾
													g_systemParameter1.imgBver=NeedUpgradePara.ver1;
													g_systemParameter1.upgradeType=NeedUpgradePara.ver1;													
												}
											  BackupSystemParameter1();
												
											
												OK_Command(buf,SEND_FIRMWARE_END);
									      uart3Send((unsigned char *)buf,4);
//												osDelay(1000);
//												gStorageData.SPIFlashAddress=0;
//												gStorageData.SPIFlashOutAddress=0;
//												BackupSysPara(&gStorageData);
											  
												while(xSemaphoreTake( xLogoMute, portMAX_DELAY ) != pdTRUE)
	                      {
                          osDelay(1);
		                    } 	
												osDelay(20);
												Airsensor_SendlogtoPC();
//												uint32_t I_delay=0;
//												for(I_delay=0xffff;I_delay>0;I_delay--);
											  osDelay(50);
												
											  Upgrade_Finish_LOG();
											  osDelay(50);
												xSemaphoreGive( xLogoMute );
//												__set_PRIMASK(1);//�ر��ж�
//												ResetToFactory();
												
												
												SoftWareReset();
										#if 0
											  Upgrading_On=0;
											  uint32_t I_delay=0;
											  RCC->CFGR |=RCC_CFGR_HPRE_DIV4;//GD32F405�������
	                      for(I_delay=0xffff;I_delay>0;I_delay--);
											
												#ifndef UPGRADE_TEST
													if(NeedUpgradePara.firmwareType==1)
														JumpToImage(IMAGEA);
													else JumpToImage(IMAGEB);
												#endif
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
											UpgradeCtrl.sector_sequence=0;
                      Upgrading_On=0;										
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
									
								#if 0
									case READ_WORK_MODE:
										
										  buf[0]=0x02;
									  	buf[1]=READ_WORK_MODE;
									    if(gStorageData.Standby==1)
											{
												buf[2]=0;
											}
									    else if(gStorageData.Standby==0 && Airsensor.Tt_Is_Sleep_Time==0)
											{
												buf[2]=1;
											}
									    else if(gStorageData.Standby==0 && Airsensor.Tt_Is_Sleep_Time==1)
											{
                        buf[2]=2;        
											}												
									    else
											{
												buf[2]=0; 
											}									  	
									  	buf[3]=3;
										  uart3Send((unsigned char *)buf,4);						

										break;
								#endif
								#if 0
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
								#endif
									case CONTROL_AIR_BAG:
//									 if(RequestControlValve==3 || Airsensor.AirSensor_Check_Mode!=4 || gStorageData.Standby==0)
									if(RequestControlValve==3 || gStorageData.Standby==0)
									 {
										ERROR_Command(buf,CONTROL_AIR_BAG);
										uart3Send((unsigned char *)buf,4);	
									 }
									 else
									 {
										Airsensor.AirSensor_position_checked_by_fastmode_flag=0;
									  position.head1=HEAD_NO;
										if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;	
										tempAirBag=buf[2];
										tempholdingtime = buf[3];
										
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
//											gStorageData.Standby=1;
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
									}
									  break;
									case GET_BODY_POSITON:
									 if(RequestControlValve==3 || gStorageData.Standby==0)
									 {
										ERROR_Command(buf,GET_BODY_POSITON);
										uart3Send((unsigned char *)buf,4);	
									 }	
									 else
									 {
										#if defined(TUWAN_PILLOW_3) || defined(TUWAN_MAT)
									    if(Airsensor.AirSensor_position_checked_by_fastmode_flag==1)												
											{
											  buf[0]=0x02;
									  	  buf[1]=GET_BODY_POSITON;
									  	  if(position.head1==HEAD_NO)
									  		  buf[2]=0;
										    else
											    buf[2]=position.head1+1;
									  	  buf[3]=3;
									  	  ReadPos.enable=1;												
									  	  ReadPos.tick=500;//500
												
												Airsensor.LED_flash_state_flag=1;
//	                      Airsensor.Led_flash_counter=0;
//	                      Airsensor.Led_yellow_state=1;//�Ƶ�����
												
//												Airsensor.AirSensor_position_checked_by_fastmode_flag=0;
												
											#if 0	//����������������
									  	  if(ActionSteps==0)
												{
													while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				                  {
                            osDelay(1);
				                  } 
									  	    Airsensor.AirSensor_Checkposition_Mode=FAST_MODE;
													xSemaphoreGive( xPositionMute );
												}
										  #endif
											}
											else
											{
											  buf[0]=0x02;
									  	  buf[1]=GET_BODY_POSITON;									  	  
									  		buf[2]=0;
										    buf[3]=3;
									  	  ReadPos.enable=1;												
									  	  ReadPos.tick=500;	//500	
                        Airsensor.LED_flash_state_flag=1;
//	                      Airsensor.Led_flash_counter=0;
//	                      Airsensor.Led_yellow_state=1;//�Ƶ�����
												
                      #if 0			//����������������									
									  	  if(ActionSteps==0)
												{
													while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				                  {
                            osDelay(1);
				                  }
									  	    Airsensor.AirSensor_Checkposition_Mode=FAST_MODE;
													xSemaphoreGive( xPositionMute );
												}	
                      #endif												
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
										
									 }
									 break;
									case GET_SNORE_DB:
										temp = (unsigned short)PCMVol;
										buf[0]=0x02;
									  	buf[1]=GET_SNORE_DB;
									  	buf[2]=temp&0xff;
									  	buf[3]=(temp&0xff00)>>8;
									  	buf[4]=3;
										uart3Send((unsigned char *)buf,5);
									  Airsensor.AirSensor_position_checked_by_fastmode_flag=0;
									  position.head1=HEAD_NO;
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
#if 1									
									#ifdef UserSetSleepTime
									case SET_MAINSLEEP_TIME:
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
									  	
									
											if(buf[6]!=0x03)
									  	{
										  	ERROR_Command(buf,SET_MAINSLEEP_TIME);
												uart3Send((unsigned char *)buf,4);
												break;									
									  	}
											
											gStorageData.UserSleepTime[0]=buf[2];
											gStorageData.UserSleepTime[1]=buf[3];
											gStorageData.UserSleepTime[2]=buf[4];
											gStorageData.UserSleepTime[3]=buf[5];		
														
											
											OK_Command(buf,SET_MAINSLEEP_TIME);
											uart3Send((unsigned char *)buf,4);
											BackupSysPara(&gStorageData);
//											User_set_studytime_process();
											break;
											
									case GET_MAINSLEEP_TIME:
											buf[0]=0x02;
											buf[1]=GET_MAINSLEEP_TIME;
											buf[2]=gStorageData.UserSleepTime[0];
											buf[3]=gStorageData.UserSleepTime[1];
											buf[4]=gStorageData.UserSleepTime[2];
											buf[5]=gStorageData.UserSleepTime[3];											
											buf[6]=3;
											uart3Send((unsigned char *)buf,7);
											break;
									#endif
#endif									
									case GET_SNORE_VECTORS:									
										  buf[0]=0x02;
											buf[1]=GET_SNORE_VECTORS;
											buf[2]=vectors_num;
											buf[3]=valid_vcetors_num;
                      buf[4]=(valid_vectors_weight[0]&0x0ff00)>>8;
                      buf[5]=valid_vectors_weight[0]&0x0ff; 
                      buf[6]=(valid_vectors_weight[1]&0x0ff00)>>8;
                      buf[7]=valid_vectors_weight[1]&0x0ff; 
                      buf[8]=(valid_vectors_weight[2]&0x0ff00)>>8;
                      buf[9]=valid_vectors_weight[2]&0x0ff; 
                      buf[10]=(valid_vectors_weight[3]&0x0ff00)>>8;
                      buf[11]=valid_vectors_weight[3]&0x0ff; 
                      buf[12]=(valid_vectors_weight[4]&0x0ff00)>>8;
                      buf[13]=valid_vectors_weight[4]&0x0ff; 
                      buf[14]=(valid_vectors_weight[5]&0x0ff00)>>8;
                      buf[15]=valid_vectors_weight[5]&0x0ff; 
									    buf[16]=(valid_vectors_weight[6]&0x0ff00)>>8;
                      buf[17]=valid_vectors_weight[6]&0x0ff;                      									
											buf[18]=3;
									    uart3Send((unsigned char *)buf,19);
											break;
											
									case SET_HEADPOSITION_SENSITIVITY:
										if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										if(buf[3]!=0x03)
										{
											ERROR_Command(buf,SET_HEADPOSITION_SENSITIVITY);
											uart3Send((unsigned char *)buf,4);
											continue;
										}
 //                   SensorLevelbak = gStorageData.SensorLevel;     
										if(buf[2]==0x00)
										 gStorageData.HeadpositionLevel=LOW_CHECK;
										else if(buf[2]==0x01)
										 gStorageData.HeadpositionLevel=MID_CHECK;
										else if(buf[2]==0x02)	
										 gStorageData.HeadpositionLevel=HIGH_CHECK;
										else continue;										
                    Position_Init();
										OK_Command(buf,SET_HEADPOSITION_SENSITIVITY);
										uart3Send((unsigned char *)buf,4);
										BackupSysPara(&gStorageData);
										break;
									case GET_HEADPOSITION_SENSITIVITY:
										buf[0]=0x02;
									  	buf[1]=GET_HEADPOSITION_SENSITIVITY;
									 	if(gStorageData.HeadpositionLevel==LOW_CHECK)
										 buf[2]=0;
										else if(gStorageData.HeadpositionLevel==MID_CHECK)
										 buf[2]=1;
										else 
										 buf[2]=2;
									  buf[3]=3;
										uart3Send((unsigned char *)buf,4);
										break;
										
                 case SET_SNORE_MINDB:
										if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										if(buf[3]!=0x03)
										{
											ERROR_Command(buf,SET_SNORE_MINDB);
											uart3Send((unsigned char *)buf,4);
											continue;
										}
										gStorageData.MinSnoredB=buf[2]; 									
                    snore_mindb_Init();
										OK_Command(buf,SET_SNORE_MINDB);
										uart3Send((unsigned char *)buf,4);
										BackupSysPara(&gStorageData);
										break;
										
								case GET_SNORE_MINDB:
										buf[0]=0x02;
									  buf[1]=GET_SNORE_MINDB;									 	
										buf[2]=gStorageData.MinSnoredB;
									  buf[3]=3;
										uart3Send((unsigned char *)buf,4);
										break;

								case SET_HEADPOSITION_SENSITIVITY_PARA:
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
									
											if(buf[6]!=0x03)
									  	{
										  	ERROR_Command(buf,SET_HEADPOSITION_SENSITIVITY_PARA);
												uart3Send((unsigned char *)buf,4);
												break;									
									  	}
											
											gStorageData.PositionValue_Onebag=buf[2]*256+buf[3];  //16����������λ�ں󣬸�λ��ǰ
											gStorageData.PositionValue_Twobag=buf[4]*256+buf[5];  //16����������λ�ں󣬸�λ��ǰ;							
														
											
											OK_Command(buf,SET_HEADPOSITION_SENSITIVITY_PARA);
											uart3Send((unsigned char *)buf,4);
											
											BackupSysPara(&gStorageData);
											User_set_positionvalue();
											break;
											
									case GET_HEADPOSITION_SENSITIVITY_PARA:
											buf[0]=0x02;
											buf[1]=GET_HEADPOSITION_SENSITIVITY_PARA;
											buf[2]=(gStorageData.PositionValue_Onebag & 0x0ff00)>>8;
									    buf[3]= gStorageData.PositionValue_Onebag & 0x0ff;
											buf[4]=(gStorageData.PositionValue_Twobag & 0x0ff00)>>8;
									    buf[5]= gStorageData.PositionValue_Twobag & 0x0ff;
											buf[6]=3;
											uart3Send((unsigned char *)buf,7);
											break;

									
									case SET_BODYON_THRESHOLD:
										  //�����������ϴ���ֵ
									    if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										  if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										  if(buf[3]!=0x03)
										  {
											  ERROR_Command(buf,SET_BODYON_THRESHOLD);
											  uart3Send((unsigned char *)buf,4);
											  continue;
										  }
										  gStorageData.Body_detect_on_threshold=buf[2]; 									
                      Body_Detect_Init();
										  OK_Command(buf,SET_BODYON_THRESHOLD);
										  uart3Send((unsigned char *)buf,4);
										  BackupSysPara(&gStorageData);									
										  break;
									
                  case GET_BODYON_THRESHOLD:
										  //��ȡ�������ϴ���ֵ
									    buf[0]=0x02;
									    buf[1]=GET_BODYON_THRESHOLD;									 	
										  buf[2]=gStorageData.Body_detect_on_threshold;
									    buf[3]=3;
										  uart3Send((unsigned char *)buf,4);									
										  break;
										
									case SET_BODYOFF_THRESHOLD:
										  //�����������𴲷�ֵ
									    if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										  if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										  if(buf[3]!=0x03)
										  {
											  ERROR_Command(buf,SET_BODYOFF_THRESHOLD);
											  uart3Send((unsigned char *)buf,4);
											  continue;
										  }
										  gStorageData.Body_detect_off_threshold=buf[2]; 									
                      Body_Detect_Init();
										  OK_Command(buf,SET_BODYOFF_THRESHOLD);
										  uart3Send((unsigned char *)buf,4);
										  BackupSysPara(&gStorageData);
										  break;
									
                  case GET_BODYOFF_THRESHOLD:
										  //��ȡ�������𴲷�ֵ
									    buf[0]=0x02;
									    buf[1]=GET_BODYOFF_THRESHOLD;									 	
										  buf[2]=gStorageData.Body_detect_off_threshold;
									    buf[3]=3;
										  uart3Send((unsigned char *)buf,4);	
										  break;
									
									case SET_ONEHOUR_AUTODETECT:
										  //����1Сʱ�������Զ���һ��ͷ��λ�ü��
									    if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										  if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										  if(buf[3]!=0x03)
										  {
											  ERROR_Command(buf,SET_ONEHOUR_AUTODETECT);
											  uart3Send((unsigned char *)buf,4);
											  continue;
										  }
											gStorageData.One_hour_auto_detect_onoff=buf[2]; 
                      if(gStorageData.One_hour_auto_detect_onoff==0x01)
                        Airsensor.onehour_snore_checkposition_counter=0; 												
                      OK_Command(buf,SET_ONEHOUR_AUTODETECT);
										  uart3Send((unsigned char *)buf,4);
										  BackupSysPara(&gStorageData);									
										  break;
									
                  case GET_ONEHOUR_AUTODETECT:
										  //��ȡ�������ϴ���ֵ
									    buf[0]=0x02;
									    buf[1]=GET_ONEHOUR_AUTODETECT;									 	
										  buf[2]=gStorageData.One_hour_auto_detect_onoff;
									    buf[3]=3;
										  uart3Send((unsigned char *)buf,4);									
										  break;
									
									//SET_AUTODETECT_CYCLE
									case SET_AUTODETECT_CYCLE:
										  //����1Сʱ�������Զ���һ��ͷ��λ�ü��
									    if(xQueueReceive(UART3QHandle, &buf[2], 100)==pdFALSE)
												continue;
										  if(xQueueReceive(UART3QHandle, &buf[3], 100)==pdFALSE)
												continue;
										  if(buf[3]!=0x03 || buf[2] > 60 || buf[2] < 1)
										  {
											  ERROR_Command(buf,SET_AUTODETECT_CYCLE);
											  uart3Send((unsigned char *)buf,4);
											  continue;
										  }
											gStorageData.auto_detect_timecycle=buf[2]; 
                      if(gStorageData.One_hour_auto_detect_onoff==0x01)
                        Airsensor.onehour_snore_checkposition_counter=0; 												
                      OK_Command(buf,SET_AUTODETECT_CYCLE);
										  uart3Send((unsigned char *)buf,4);
										  BackupSysPara(&gStorageData);									
										  break;
									
                  case GET_AUTODETECT_CYCLE:
										  //��ȡ�������ϴ���ֵ
									    buf[0]=0x02;
									    buf[1]=GET_AUTODETECT_CYCLE;									 	
										  buf[2]=gStorageData.auto_detect_timecycle;
									    buf[3]=3;
										  uart3Send((unsigned char *)buf,4);									
										  break;
									
									case GET_LATEST_SNORE_TIME:
										if(SnoreTime_forAPP.valid == 1)
										{
										  buf[0]=0x02;
									    buf[1]=GET_LATEST_SNORE_TIME;									 	
										  buf[2]=SnoreTime_forAPP.year;
									    buf[3]=SnoreTime_forAPP.month;
											buf[4]=SnoreTime_forAPP.day;
											buf[5]=SnoreTime_forAPP.hour;
									    buf[6]=SnoreTime_forAPP.minute;
											buf[7]=SnoreTime_forAPP.second;
									    buf[8]=3;
										  uart3Send((unsigned char *)buf,9);
											SnoreTime_forAPP.valid = 0;
											continue;
										}
										else
										{
											ERROR_Command(buf,GET_LATEST_SNORE_TIME);
											uart3Send((unsigned char *)buf,4);
											continue;
										}
									  break;
									
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
		
		if(reply_app_for_update_keypress_flag == 2)
		{
			reply_app_for_update_keypress_flag = 0;
			buf[0]=0x02;
			buf[1]=REQUEST_UPGRADE;
			buf[2]=GetNeedUpgradeImageType();
			buf[3]=3;
			uart3Send((unsigned char *)buf,4);
			if(gStorageData.Standby==0 && Airsensor.Tt_Is_Sleep_Time==0)
			{
			   WrokingLed(BLUE_LED_HIGH,1);
			}				
			else if(gStorageData.Standby==0 && Airsensor.Tt_Is_Sleep_Time==1)
			{
			   WrokingLed(BLUE_LED_HIGH,2);
			}
		  if(gStorageData.Standby==1)
			{
				 WrokingLed(RED_LED,1);
			}
		}
		else if(reply_app_for_update_keypress_flag == 1)
		{
			reply_app_for_update_keypress_flag = 0;
			ERROR_Command(buf,REQUEST_UPGRADE);
			uart3Send((unsigned char *)buf,4);
			if(gStorageData.Standby==0 && Airsensor.Tt_Is_Sleep_Time==0)
			{
			   WrokingLed(BLUE_LED_HIGH,1);
			}				
			else if(gStorageData.Standby==0 && Airsensor.Tt_Is_Sleep_Time==1)
			{
			   WrokingLed(BLUE_LED_HIGH,2);
			}
		  if(gStorageData.Standby==1)
			{
				 WrokingLed(RED_LED,1);
			}
		}
		osDelay(20);
		t1++;
		if(t1>6)
		{
			t1=0;
			
			while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
			{
        osDelay(1);
			}
			
			
			while(1)
			{
			  HAL_RTC_GetTime(&hrtc, &stimestructure, RTC_FORMAT_BIN);
			  HAL_RTC_GetDate(&hrtc, &sdatestructure, RTC_FORMAT_BIN);
			  HAL_RTC_GetTime(&hrtc, &stimestructure_temp, RTC_FORMAT_BIN);
			  HAL_RTC_GetDate(&hrtc, &sdatestructure_temp, RTC_FORMAT_BIN);
				if(stimestructure.Hours == stimestructure_temp.Hours && stimestructure.Minutes == stimestructure_temp.Minutes && stimestructure.Seconds == stimestructure_temp.Seconds)
				{
					if(sdatestructure.Year == sdatestructure_temp.Year && sdatestructure.Month == sdatestructure_temp.Month && sdatestructure.Date == sdatestructure_temp.Date)
					{
						if(stimestructure.Hours <= 23 && stimestructure.Minutes <=59 && stimestructure.Seconds <= 59)
						{
							if(sdatestructure.Year <=99 && sdatestructure.Month <=12 && sdatestructure.Date <=31)
						    break;
						}
					}
				}
			}	
			
			//����Ƿ�ûָ�ֹ��ON
			if(gStorageData.StudyMode == 1 && gStorageData.Modifystudymodestamptime != 0)
			{
				datetime_now.year = sdatestructure.Year;
				datetime_now.month = sdatestructure.Month;
				datetime_now.day = sdatestructure.Date;
				datetime_now.hour = stimestructure.Hours;
				datetime_now.minute = stimestructure.Minutes;
				datetime_now.second = stimestructure.Seconds;
		
				if(time_to_stamptime(datetime_now) >= gStorageData.Modifystudymodestamptime)
				{
					gStorageData.Modifystudymodestamptime = 0;
					gStorageData.StudyMode = 0;
				}
			}	
			
			//�����Ч���Ƿ���ڣ�������ر�����
			if(USERVALIDTIME_ENABLE==1)
			{
				if(UserkeycodeAutoSetData.validtime_year > sdatestructure.Year || (UserkeycodeAutoSetData.validtime_month > sdatestructure.Month && UserkeycodeAutoSetData.validtime_year == sdatestructure.Year) || (UserkeycodeAutoSetData.validtime_day > sdatestructure.Date && UserkeycodeAutoSetData.validtime_year == sdatestructure.Year && UserkeycodeAutoSetData.validtime_month == sdatestructure.Month) ||(UserkeycodeAutoSetData.validtime_day == sdatestructure.Date && UserkeycodeAutoSetData.validtime_year == sdatestructure.Year && UserkeycodeAutoSetData.validtime_month == sdatestructure.Month && stimestructure.Hours < 12))
				{
					ValidTimeOkFlag = 1;
				}
				else
				{
					ValidTimeOkFlag = 0;
				}
			}
			
					
			//�����������ں�ʱ���Ա��β��Դ����RTC��λ�Իָ�ϵͳʱ��
			if(Date_Time_Save_Enable==1)
			{
				if(stimestructure.Seconds%Seconds_for_savedateandtime==0)
				{
					
					Backup_RTC(&sdatestructure,&stimestructure);
				}
				
			}
			
			

      //�������ܣ�1��˯��ʱ��֮�⣬2��1Сʱ����������3��1Сʱ����ȷ�϶��� 4��ϵͳ��Ϊ���˵�������ҳ���һ��Сʱ
			//��һ���Ƿ������˵�ȷ�ϣ��Ծ�����������������뿪�˶��豸һֱ��Ϊ�����ڴ�
			
			if(Recheck_when_person_without_snore_longtime_beyond_sleeptime_enable==1)
			{
				if(Airsensor.AirSensor_PersonOn==1)
				{
					uint8_t notin_sleep_time_flag=0;
					//ȷ����˯��ʱ������
					if(gStorageData.UserSleepTime[0]<gStorageData.UserSleepTime[2])
					{
						if(((stimestructure.Hours>gStorageData.UserSleepTime[0]) || ((stimestructure.Minutes>=gStorageData.UserSleepTime[1])  && (stimestructure.Hours==gStorageData.UserSleepTime[0])))			 
				    && ((stimestructure.Hours<gStorageData.UserSleepTime[2]) || ((stimestructure.Minutes<gStorageData.UserSleepTime[3])  && (stimestructure.Hours==gStorageData.UserSleepTime[2]))))
			      {
							//˯��ʱ����
				      notin_sleep_time_flag=0;
			      }	
            else
            {
							notin_sleep_time_flag=1;
						}							
					}
					else 
					{
						if(((stimestructure.Hours>gStorageData.UserSleepTime[0]) || ((stimestructure.Minutes>=gStorageData.UserSleepTime[1])  && (stimestructure.Hours==gStorageData.UserSleepTime[0])))			 
				    || ((stimestructure.Hours<gStorageData.UserSleepTime[2]) || ((stimestructure.Minutes<gStorageData.UserSleepTime[3])  && (stimestructure.Hours==gStorageData.UserSleepTime[2]))))
			      {
				      //˯��ʱ����	
				      notin_sleep_time_flag=0;
			      }			 
						else
            {
							notin_sleep_time_flag=1;
						}	
					}	
         	if(notin_sleep_time_flag==1)
					{
            //��ȷ���Ƿ����������趨��ʱ������
						uint8_t no_snore_recently_flag=0;
						if(latestsnoretime_hour==0xff)
						{
							 //�������ʱ��Ϊ��Чʱ��
							 no_snore_recently_flag=1;
						}
						else
						{
							//��⵱ǰʱ�������һ��������ʱ���Ƿ���һ��Сʱ����
              if(stimestructure.Hours>=latestsnoretime_hour)
							{
								if((stimestructure.Hours*60+stimestructure.Minutes)>=(latestsnoretime_hour*60+latestsnoretime_minute))
								{
									if(((stimestructure.Hours*60+stimestructure.Minutes)-(latestsnoretime_hour*60+latestsnoretime_minute))>=Recheck_when_person_without_snore_longtime_beyond_sleeptime_time)									
									{
										no_snore_recently_flag=1;
									}
								}
								
							}
							else
							{
								if((((stimestructure.Hours+24)*60+stimestructure.Minutes)-(latestsnoretime_hour*60+latestsnoretime_minute))>=Recheck_when_person_without_snore_longtime_beyond_sleeptime_time)									
								{
										no_snore_recently_flag=1;
								}
							}

						}
						if(no_snore_recently_flag==1)
						{
							//��ȷ��ϵͳȷ�������Ƿ���1Сʱ����
							uint8_t personon_enoughtime_flag=0;
							if(personontime_hour==0xff)
							{
								personon_enoughtime_flag=0;
							}
							else
							{
							  if(stimestructure.Hours>=personontime_hour)
							  {
								  if((stimestructure.Hours*60+stimestructure.Minutes)>=(personontime_hour*60+personontime_minute))
								  {
									  if(((stimestructure.Hours*60+stimestructure.Minutes)-(personontime_hour*60+personontime_minute))>=Recheck_when_person_without_snore_longtime_beyond_sleeptime_time)									
									  {
										  personon_enoughtime_flag=1;
									  }
								  }
								
							  }
							  else
							  {
								  if((((stimestructure.Hours+24)*60+stimestructure.Minutes)-(personontime_hour*60+personontime_minute))>=Recheck_when_person_without_snore_longtime_beyond_sleeptime_time)									
								  {
										personon_enoughtime_flag=1;
								  }
							  }
							}
              if(personon_enoughtime_flag==1)
							{
								//��ȷ��һСʱ���Ƿ�������ȷ�϶���
								uint8_t norecheck_enoughtime_flag=0;
								if(rechecktime_hour==0xff)
								{
									//����Чȷ�϶���
									norecheck_enoughtime_flag=1;
								}
								else
								{
							    if(stimestructure.Hours>=rechecktime_hour)
							    {
								    if((stimestructure.Hours*60+stimestructure.Minutes)>=(rechecktime_hour*60+rechecktime_minute))
								    {
									    if(((stimestructure.Hours*60+stimestructure.Minutes)-(rechecktime_hour*60+rechecktime_minute))>=Recheck_when_person_without_snore_longtime_beyond_sleeptime_time)									
									    {
										    norecheck_enoughtime_flag=1;
									    }
								    }								
							    }
							    else
							    {
								    if((((stimestructure.Hours+24)*60+stimestructure.Minutes)-(rechecktime_hour*60+rechecktime_minute))>=Recheck_when_person_without_snore_longtime_beyond_sleeptime_time)									
								    {
										  norecheck_enoughtime_flag=1;
								    }
							    }
							  }
                if(norecheck_enoughtime_flag==1)	
								{									
								  //�������������ñ�־����ϵͳ����ȷ��
									Airsensor.personon_by_positioncheck_flag=1;
									
							  }
								
							}
						}						
						
					}						
					
				}				
			}

			
			
			
			
/*			
			if((m!=stimestructure.Minutes)&&(stimestructure.Minutes%10==2))
			{
				m=stimestructure.Minutes;
				if(Airsensor.AirSensor_PersonOn==1)
				  gStorageData.Snore.HeadMovingTimes = gStorageData.Snore.HeadMovingTimes|0x10;
				else
					gStorageData.Snore.HeadMovingTimes = gStorageData.Snore.HeadMovingTimes&0xef;
			}
			else if((m!=stimestructure.Minutes)&&(stimestructure.Minutes%10==4))
			{
				m=stimestructure.Minutes;
				if(Airsensor.AirSensor_PersonOn==1)
				  gStorageData.Snore.HeadMovingTimes = gStorageData.Snore.HeadMovingTimes|0x08;
				else
					gStorageData.Snore.HeadMovingTimes = gStorageData.Snore.HeadMovingTimes&0xf7;
			}
			else if((m!=stimestructure.Minutes)&&(stimestructure.Minutes%10==6))
			{
				m=stimestructure.Minutes;
				if(Airsensor.AirSensor_PersonOn==1)
				  gStorageData.Snore.HeadMovingTimes = gStorageData.Snore.HeadMovingTimes|0x04;
				else
					gStorageData.Snore.HeadMovingTimes = gStorageData.Snore.HeadMovingTimes&0xfb;
			}
			else if((m!=stimestructure.Minutes)&&(stimestructure.Minutes%10==8))
			{
				m=stimestructure.Minutes;
				if(Airsensor.AirSensor_PersonOn==1)
				  gStorageData.Snore.HeadMovingTimes = gStorageData.Snore.HeadMovingTimes|0x02;
				else
					gStorageData.Snore.HeadMovingTimes = gStorageData.Snore.HeadMovingTimes&0xfd;
			}			
		  
			else if((m!=stimestructure.Minutes)&&(stimestructure.Minutes%10==0))
			{
*/		
      if(Airsensor.Tt_Is_Sleep_Time==1)
//			if(Airsensor.AirSensor_PersonOn==1)	
      {
				person_on_for_save_data_flag=1;
			}
			
			if((m!=stimestructure.Minutes)&&(stimestructure.Minutes%10==0) || Airsensor.Poweroffkey_savedata_flag==1 )
			{
   //*****************************************************************************
   #if 0			
			
			
			#ifdef UserSetSleepTime
			 if(gStorageData.UserSleepTime[0]<=gStorageData.UserSleepTime[2])
			 {
			   if(((stimestructure.Hours>gStorageData.UserSleepTime[0]) || ((stimestructure.Minutes>=gStorageData.UserSleepTime[1])  && (stimestructure.Hours==gStorageData.UserSleepTime[0])))			 
				    && ((stimestructure.Hours<gStorageData.UserSleepTime[2]) || ((stimestructure.Minutes<gStorageData.UserSleepTime[3])  && (stimestructure.Hours==gStorageData.UserSleepTime[2]))))
			   {
				  //��ʱ���ӣ��ж�ʱ���Ƿ��û��趨����˯��ʱ��			
				   Tt_Is_Sleep_Time=1;
			   }			 
			   else
				   Tt_Is_Sleep_Time=0;
			 }
			 else
			 {
				 if(((stimestructure.Hours>gStorageData.UserSleepTime[0]) || ((stimestructure.Minutes>=gStorageData.UserSleepTime[1])  && (stimestructure.Hours==gStorageData.UserSleepTime[0])))			 
				    || ((stimestructure.Hours<gStorageData.UserSleepTime[2]) || ((stimestructure.Minutes<gStorageData.UserSleepTime[3])  && (stimestructure.Hours==gStorageData.UserSleepTime[2]))))
			   {
				  //��ʱ���ӣ��ж�ʱ���Ƿ��û��趨����˯��ʱ��			
				   Tt_Is_Sleep_Time=1;
			   }			 
			   else
				   Tt_Is_Sleep_Time=0;
				 
			 }
			 
			 
			 if((m!=stimestructure.Minutes)&&(stimestructure.Minutes%10==0) || Airsensor.Poweroffkey_savedata_flag==1)
			 {
			
			
			#else
       if((m!=stimestructure.Minutes)&&(stimestructure.Minutes%10==0))
			 { 
				//��ʱ���ӣ��ж�ʱ���Ƿ�Ϊ����11:00������7:00
				
				 if(stimestructure.Hours<=6 && stimestructure.Hours>=0 )					
				   Tt_Is_Sleep_Time=1;
				 else
					 Tt_Is_Sleep_Time=0;
				
			#endif	
			
   #endif
	 //*************************************************************************************
				 
				//��ʱ���ӽ��� 
				//***********************************************
				
				m=stimestructure.Minutes;
/*
				if(Airsensor.AirSensor_PersonOn==1)
				  gStorageData.Snore.HeadMovingTimes = gStorageData.Snore.HeadMovingTimes|0x01;
				else
					gStorageData.Snore.HeadMovingTimes = gStorageData.Snore.HeadMovingTimes&0xfe;
				
				if((gStorageData.Snore.HeadMovingTimes&0x01)==0x1 || (gStorageData.Snore.HeadMovingTimes&0x02)==0x2
					|| (gStorageData.Snore.HeadMovingTimes&0x04)==0x4 || (gStorageData.Snore.HeadMovingTimes&0x08)==0x8
				  || (gStorageData.Snore.HeadMovingTimes&0x10)==0x10)
				{
					gStorageData.Snore.HeadMovingTimes=0x1;
	
*/	

        //ʲô����±���10���ӵ�����
//        if(Airsensor.Tt_Is_Sleep_Time_last==1)	
//        if(Airsensor.AirSensor_PersonOn==1)				
				if(person_on_for_save_data_flag==1 || Airsensor.Poweroffkey_savedata_flag==1)	
				{		
          if(SENSORLEVEL_HIGH_FOR_24HOURS_ENABLED==1)
					{
            //�жϻָ��������ú�ǰ24Сʱʹ������������Ϊ�ߵļ��
				    if(SensorLevelAutoSetData.times!=0)
		        {
				      SensorLevelAutoSetData.times--;
					
					    if(SensorLevelAutoSetData.times==0)
					    {
						    SensorLevelAutoSetData_timeout_flag=1;
						    Delete_SensorLevelAutoSet_Flag();
					    }
					    else
					    {
						    update_sensorlevel_autoset();
					    }					
				    } 			
					}
					gStorageData.Snore.StartTime[0]=sdatestructure.Year;
			    gStorageData.Snore.StartTime[1]=sdatestructure.Month;
			    gStorageData.Snore.StartTime[2]=sdatestructure.Date;
			    gStorageData.Snore.StartTime[3]=stimestructure.Hours;
			    gStorageData.Snore.StartTime[4]=stimestructure.Minutes;
			    gStorageData.Snore.AVGSnoreTime=Config.SnoreAverTime/100;
					
					//���ϴ����뿪ʱ��ת��Ϊ���ʱ��
					if(person_on_second != 0xff || person_on_minute != 0xff)
					{
					  if(person_on_second<=stimestructure.Seconds)
					  {
						   gStorageData.Snore.person_on_second =stimestructure.Seconds-person_on_second;
							 if(person_on_minute<=stimestructure.Minutes)
							  gStorageData.Snore.person_on_minute=stimestructure.Minutes-person_on_minute;
							 else
								gStorageData.Snore.person_on_minute=stimestructure.Minutes+60-person_on_minute;
						 }
					  else
					  {
							 gStorageData.Snore.person_on_second=stimestructure.Seconds+60-person_on_second;
							 if((person_on_minute+1)<= stimestructure.Minutes)
							  gStorageData.Snore.person_on_minute=stimestructure.Minutes-person_on_minute-1;
							 else
								gStorageData.Snore.person_on_minute=stimestructure.Minutes+59-person_on_minute;
						 }	 
					 }
					 else
					 {
						 gStorageData.Snore.person_on_minute=0;
						 gStorageData.Snore.person_on_second=0;
					 }
           if(person_off_second !=0xff || person_off_minute != 0xff)
					 {						 
					   if(person_off_second<=stimestructure.Seconds)
					   {
						   gStorageData.Snore.person_off_second=stimestructure.Seconds-person_off_second;
							 if(person_off_minute<=stimestructure.Minutes)
							  gStorageData.Snore.person_off_minute=stimestructure.Minutes-person_off_minute;
							 else
								gStorageData.Snore.person_off_minute=stimestructure.Minutes+60-person_off_minute;
						  }
						 else
						 {
							 gStorageData.Snore.person_off_second=stimestructure.Seconds+60-person_off_second;
							 if((person_off_minute+1)<= stimestructure.Minutes)
							  gStorageData.Snore.person_off_minute=stimestructure.Minutes-person_off_minute-1;
							 else
								gStorageData.Snore.person_off_minute=stimestructure.Minutes+59-person_off_minute;
						 }
					 }
					 else
					 {
						 gStorageData.Snore.person_off_minute=0;
						 gStorageData.Snore.person_off_second=0;
					 }
					
/*					
					gStorageData.Snore.person_on_minute=person_on_minute;
					gStorageData.Snore.person_on_second=person_on_second;
					gStorageData.Snore.person_off_second=person_off_second;
					gStorageData.Snore.person_off_minute=person_off_minute;
*/				
          person_on_hour=0xff;					 
					person_on_minute=0xff;
					person_on_second=0xff;
					person_off_hour=0xff;	 
					person_off_second=0xff;
					person_off_minute=0xff;
					Ten_minutes_first_personon_event_flag=0; 
					
//					Airsensor_Print_Tuwan_Date();
//  				osDelay(50);
					 
					if(Airsensor.Poweroffkey_savedata_flag==1)
						gStorageData.Snore.StudyMode=0x82;
          else						
					  gStorageData.Snore.StudyMode=gStorageData.StudyMode;
					gStorageData.Snore.UsedRecord=0x55aa; 
					WriteSnoreData(&gStorageData.Snore); //�����ϴ�
					
					 
					
//					gStorageData.Snore.HeadMovingTimes = 0;
					memset(&gStorageData.Snore,0,sizeof(SnoreData_def));
					gStorageData.Snore.UsedRecord=0x55aa;
					gStorageData.Snore.StudyMode=gStorageData.StudyMode;
					Config.SnoreAverTime = 4000; 
					gStorageData.Snore.HeadPosion = 0x0ff;
					person_on_for_save_data_flag=0;
					
					memset(&detailed_snore,0,sizeof(SnoreData_def)); //������20220427
          j=1;
					 pump_record_process_repaet_flag=1;
					 pump_time_read_temp=0;
					 
				   //���ڱ��������ǽṹ��������ѭ������ֻ��һ�����ò���һ�����ش���10�������������2�������¼
					 
				   //�����һ�����ò���
				 do
         {	
					 pump_temp = 0;
					 if(pump_operation_time_read_pointer != pump_operation_time_write_pointer)
					 {
					 if(pump_operation_time[pump_operation_time_read_pointer][0]==1)
					 {
						 //�Ƚ�ʱ�����ʱ���뵱ǰʱ����Ⱥ�
						 if(stimestructure.Hours > pump_operation_time[pump_operation_time_read_pointer][10])
						 {
							 pump_temp = 1;
						 }
						 else if(stimestructure.Hours == pump_operation_time[pump_operation_time_read_pointer][10])
						 {
							 if(stimestructure.Minutes > pump_operation_time[pump_operation_time_read_pointer][11])
							 {
									pump_temp = 1;
							 }
							 else if(stimestructure.Minutes == pump_operation_time[pump_operation_time_read_pointer][11])
							 {
								 if(stimestructure.Seconds > pump_operation_time[pump_operation_time_read_pointer][12])
							   {
									 pump_temp = 1;
							   } 
								 else
							   {
								   pump_temp = 0;
							   }
							 }
							 else
							 {
								 pump_temp = 0;
							 }
						 }
						 else if(stimestructure.Hours < pump_operation_time[pump_operation_time_read_pointer][10])
						 {
							 if( stimestructure.Hours == 0 && pump_operation_time[pump_operation_time_read_pointer][10]==23)
							 {
								 pump_temp = 1;
							 }
							 else
							 {
								 pump_temp = 0;
							 }
						 }
					 }
				 }
					 if(pump_temp == 1 ) 
					 {
						 //ת�����ÿ�ʼ����ʱ��Ϊ���ʱ�䣬��¼����˯�߼�¼ʱ���ȥ����ʱ�����ķֺ�����
						 if(pump_operation_time[pump_operation_time_read_pointer][6]<=stimestructure.Seconds)
						 {
						   detailed_snore.StartTime[1]=stimestructure.Seconds-pump_operation_time[pump_operation_time_read_pointer][6];
							 if(pump_operation_time[pump_operation_time_read_pointer][5]<=stimestructure.Minutes)
							  detailed_snore.StartTime[0]=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][5];
							 else
								detailed_snore.StartTime[0]=stimestructure.Minutes+60-pump_operation_time[pump_operation_time_read_pointer][5];
						 }
						 else
						 {
							 detailed_snore.StartTime[1]=stimestructure.Seconds+60-pump_operation_time[pump_operation_time_read_pointer][6];
							 if((pump_operation_time[pump_operation_time_read_pointer][5]+1)<= stimestructure.Minutes)
							  detailed_snore.StartTime[0]=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][5]-1;
							 else
								detailed_snore.StartTime[0]=stimestructure.Minutes+59-pump_operation_time[pump_operation_time_read_pointer][5];
						 }
						 
						 //ת�����ÿ�ʼ������ʱ���
						 if(pump_operation_time[pump_operation_time_read_pointer][12]<=stimestructure.Seconds)
						 {
						   detailed_snore.StartTime[3]=stimestructure.Seconds-pump_operation_time[pump_operation_time_read_pointer][12];
							 if(pump_operation_time[pump_operation_time_read_pointer][11]<=stimestructure.Minutes)
							  detailed_snore.StartTime[2]=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][11];
							 else
								detailed_snore.StartTime[2]=stimestructure.Minutes+60-pump_operation_time[pump_operation_time_read_pointer][11];
						 }
						 else
						 {
							 detailed_snore.StartTime[3]=stimestructure.Seconds+60-pump_operation_time[pump_operation_time_read_pointer][12];
							 if((pump_operation_time[pump_operation_time_read_pointer][11]+1)<= stimestructure.Minutes)
							  detailed_snore.StartTime[2]=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][11]-1;
							 else
								detailed_snore.StartTime[2]=stimestructure.Minutes+59-pump_operation_time[pump_operation_time_read_pointer][11];
						 }
						 pump_operation_time[pump_operation_time_read_pointer][0]=0;
						 pump_operation_time_read_pointer++;
						 if(pump_operation_time_read_pointer>=20)
							 pump_operation_time_read_pointer=0;
						 
						 pump_time_read_temp=1;//��ʾ�����ò�������Ҫ���浽flash
						 
						 //�����2�����ò���ʱ��
						 pump_temp = 0;
						 if(pump_operation_time_read_pointer != pump_operation_time_write_pointer)
					   {
						 if(pump_operation_time[pump_operation_time_read_pointer][0]==1)
						 {
						 //�Ƚ�ʱ�����ʱ���뵱ǰʱ����Ⱥ�
						 if(stimestructure.Hours > pump_operation_time[pump_operation_time_read_pointer][10])
						 {
							 pump_temp = 1;
						 }
						 else if(stimestructure.Hours == pump_operation_time[pump_operation_time_read_pointer][10])
						 {
							 if(stimestructure.Minutes > pump_operation_time[pump_operation_time_read_pointer][11])
							 {
									pump_temp = 1;
							 }
							 else if(stimestructure.Minutes == pump_operation_time[pump_operation_time_read_pointer][11])
							 {
								 if(stimestructure.Seconds > pump_operation_time[pump_operation_time_read_pointer][12])
							   {
									 pump_temp = 1;
							   } 
								 else
							   {
								   pump_temp = 0;
							   }
							 }
							 else
							 {
								 pump_temp = 0;
							 }
						 }
						 else if(stimestructure.Hours < pump_operation_time[pump_operation_time_read_pointer][10])
						 {
							 if( stimestructure.Hours == 0 && pump_operation_time[pump_operation_time_read_pointer][10]==23)
							 {
								 pump_temp = 1;
							 }
							 else
							 {
								 pump_temp = 0;
							 }
						 }
					 } 
				 }
						 if(pump_temp == 1)
					   {
						    //ת�����ÿ�ʼ����ʱ��Ϊ���ʱ�䣬��¼����˯�߼�¼ʱ���ȥ����ʱ�����ķֺ�����
						   if(pump_operation_time[pump_operation_time_read_pointer][6]<=stimestructure.Seconds)
						   {
						     detailed_snore.SSnoreTimes=stimestructure.Seconds-pump_operation_time[pump_operation_time_read_pointer][6];
							   if(pump_operation_time[pump_operation_time_read_pointer][5]<=stimestructure.Minutes)
							     detailed_snore.StartTime[4]=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][5];
							   else
								   detailed_snore.StartTime[4]=stimestructure.Minutes+60-pump_operation_time[pump_operation_time_read_pointer][5];
						    }
						   else
						   {
							 detailed_snore.SSnoreTimes=stimestructure.Seconds+60-pump_operation_time[pump_operation_time_read_pointer][6];
							 if((pump_operation_time[pump_operation_time_read_pointer][5]+1)<=stimestructure.Minutes)
							  detailed_snore.StartTime[4]=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][5]-1;
							 else
								detailed_snore.StartTime[4]=stimestructure.Minutes+59-pump_operation_time[pump_operation_time_read_pointer][5];
						   }
						 
						 //ת�����ÿ�ʼ������ʱ���
						 if(pump_operation_time[pump_operation_time_read_pointer][12]<=stimestructure.Seconds)
						 {
						   detailed_snore.LSnoreTimes=stimestructure.Seconds-pump_operation_time[pump_operation_time_read_pointer][12];
							 if(pump_operation_time[pump_operation_time_read_pointer][11]<=stimestructure.Minutes)
							  detailed_snore.MSnoreTimes=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][11];
							 else
								detailed_snore.MSnoreTimes=stimestructure.Minutes+60-pump_operation_time[pump_operation_time_read_pointer][11];
						 }
						 else
						 {
							 detailed_snore.LSnoreTimes=stimestructure.Seconds+60-pump_operation_time[pump_operation_time_read_pointer][12];
							 if((pump_operation_time[pump_operation_time_read_pointer][11]+1)<=stimestructure.Minutes)
							  detailed_snore.MSnoreTimes=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][11]-1;
							 else
								detailed_snore.MSnoreTimes=stimestructure.Minutes+59-pump_operation_time[pump_operation_time_read_pointer][11];
						 }
						 pump_operation_time[pump_operation_time_read_pointer][0]=0;
						 pump_operation_time_read_pointer++;
						 if(pump_operation_time_read_pointer>=20)
							 pump_operation_time_read_pointer=0;
						 pump_time_read_temp=2;
					  
					  //�����3�����ò���ʱ��
						 pump_temp = 0;
						 if(pump_operation_time_read_pointer != pump_operation_time_write_pointer)
					 {
						 if(pump_operation_time[pump_operation_time_read_pointer][0]==1)
						 {
						 //�Ƚ�ʱ�����ʱ���뵱ǰʱ����Ⱥ�
						 if(stimestructure.Hours > pump_operation_time[pump_operation_time_read_pointer][10])
						 {
							 pump_temp = 1;
						 }
						 else if(stimestructure.Hours == pump_operation_time[pump_operation_time_read_pointer][10])
						 {
							 if(stimestructure.Minutes > pump_operation_time[pump_operation_time_read_pointer][11])
							 {
									pump_temp = 1;
							 }
							 else if(stimestructure.Minutes == pump_operation_time[pump_operation_time_read_pointer][11])
							 {
								 if(stimestructure.Seconds > pump_operation_time[pump_operation_time_read_pointer][12])
							   {
									 pump_temp = 1;
							   } 
								 else
							   {
								   pump_temp = 0;
							   }
							 }
							 else
							 {
								 pump_temp = 0;
							 }
						 }
						 else if(stimestructure.Hours < pump_operation_time[pump_operation_time_read_pointer][10])
						 {
							 if( stimestructure.Hours == 0 && pump_operation_time[pump_operation_time_read_pointer][10]==23)
							 {
								 pump_temp = 1;
							 }
							 else
							 {
								 pump_temp = 0;
							 }
						 }
					 } 
				 }
						 if(pump_temp == 1) 
					    {
						    //ת�����ÿ�ʼ����ʱ��Ϊ���ʱ�䣬��¼����˯�߼�¼ʱ���ȥ����ʱ�����ķֺ�����
						   if(pump_operation_time[pump_operation_time_read_pointer][6]<=stimestructure.Seconds)
						   {
						     detailed_snore.Airbag2Times=stimestructure.Seconds-pump_operation_time[pump_operation_time_read_pointer][6];
							   if(pump_operation_time[pump_operation_time_read_pointer][5]<=stimestructure.Minutes)
							     detailed_snore.Airbag1Times=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][5];
							   else
								   detailed_snore.Airbag1Times=stimestructure.Minutes+60-pump_operation_time[pump_operation_time_read_pointer][5];
						    }
						 else
						 {
							 detailed_snore.Airbag2Times=stimestructure.Seconds+60-pump_operation_time[pump_operation_time_read_pointer][6];
							 if((pump_operation_time[pump_operation_time_read_pointer][5]+1)<= stimestructure.Minutes)
							  detailed_snore.Airbag1Times=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][5]-1;
							 else
								detailed_snore.Airbag1Times=stimestructure.Minutes+59-pump_operation_time[pump_operation_time_read_pointer][5];
						 }
						 
						 //ת�����ÿ�ʼ������ʱ���
						 if(pump_operation_time[pump_operation_time_read_pointer][12]<=stimestructure.Seconds)
						 {
						   detailed_snore.Airbag4Times=stimestructure.Seconds-pump_operation_time[pump_operation_time_read_pointer][12];
							 if(pump_operation_time[pump_operation_time_read_pointer][11]<=stimestructure.Minutes)
							  detailed_snore.Airbag3Times=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][11];
							 else
								detailed_snore.Airbag3Times=stimestructure.Minutes+60-pump_operation_time[pump_operation_time_read_pointer][11];
						 }
						 else
						 {
							 detailed_snore.Airbag4Times=stimestructure.Seconds+60-pump_operation_time[pump_operation_time_read_pointer][12];
							 if((pump_operation_time[pump_operation_time_read_pointer][11]+1)<=stimestructure.Minutes)
							  detailed_snore.Airbag3Times=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][11]-1;
							 else
								detailed_snore.Airbag3Times=stimestructure.Minutes+59-pump_operation_time[pump_operation_time_read_pointer][11];
						 }
						 pump_operation_time[pump_operation_time_read_pointer][0]=0;
						 pump_operation_time_read_pointer++;
						 if(pump_operation_time_read_pointer>=20)
							 pump_operation_time_read_pointer=0;
						 pump_time_read_temp=3;
						 
						 //�����4�����ò���ʱ��
						 pump_temp = 0;
						 if(pump_operation_time_read_pointer != pump_operation_time_write_pointer)
					 {
						 if(pump_operation_time[pump_operation_time_read_pointer][0]==1)
						 {
						 //�Ƚ�ʱ�����ʱ���뵱ǰʱ����Ⱥ�
						 if(stimestructure.Hours > pump_operation_time[pump_operation_time_read_pointer][10])
						 {
							 pump_temp = 1;
						 }
						 else if(stimestructure.Hours == pump_operation_time[pump_operation_time_read_pointer][10])
						 {
							 if(stimestructure.Minutes > pump_operation_time[pump_operation_time_read_pointer][11])
							 {
									pump_temp = 1;
							 }
							 else if(stimestructure.Minutes == pump_operation_time[pump_operation_time_read_pointer][11])
							 {
								 if(stimestructure.Seconds > pump_operation_time[pump_operation_time_read_pointer][12])
							   {
									 pump_temp = 1;
							   } 
								 else
							   {
								   pump_temp = 0;
							   }
							 }
							 else
							 {
								 pump_temp = 0;
							 }
						 }
						 else if(stimestructure.Hours < pump_operation_time[pump_operation_time_read_pointer][10])
						 {
							 if( stimestructure.Hours == 0 && pump_operation_time[pump_operation_time_read_pointer][10]==23)
							 {
								 pump_temp = 1;
							 }
							 else
							 {
								 pump_temp = 0;
							 }
						 }
					 } 
				 }	 
						 if(pump_temp == 1)
					   {
						 
						    //ת�����ÿ�ʼ����ʱ��Ϊ���ʱ�䣬��¼����˯�߼�¼ʱ���ȥ����ʱ�����ķֺ�����
						   if(pump_operation_time[pump_operation_time_read_pointer][6]<=stimestructure.Seconds)
						   {
						     detailed_snore.person_on_second=stimestructure.Seconds-pump_operation_time[pump_operation_time_read_pointer][6];
							   if(pump_operation_time[pump_operation_time_read_pointer][5]<=stimestructure.Minutes)
							     detailed_snore.person_on_minute=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][5];
							   else
								   detailed_snore.person_on_minute=stimestructure.Minutes+60-pump_operation_time[pump_operation_time_read_pointer][5];
						    }
						 else
						 {
							 detailed_snore. person_on_second=stimestructure.Seconds+60-pump_operation_time[pump_operation_time_read_pointer][6];
							 if((pump_operation_time[pump_operation_time_read_pointer][5]+1)<= stimestructure.Minutes)
							  detailed_snore.person_on_minute=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][5]-1;
							 else
								detailed_snore.person_on_minute=stimestructure.Minutes+59-pump_operation_time[pump_operation_time_read_pointer][5];
						 }
						 
						 //ת�����ÿ�ʼ������ʱ���
						 if(pump_operation_time[pump_operation_time_read_pointer][12]<=stimestructure.Seconds)
						 {
						   detailed_snore.person_off_second=stimestructure.Seconds-pump_operation_time[pump_operation_time_read_pointer][12];
							 if(pump_operation_time[pump_operation_time_read_pointer][11]<=stimestructure.Minutes)
							  detailed_snore.person_off_minute=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][11];
							 else
								detailed_snore.person_off_minute=stimestructure.Minutes+60-pump_operation_time[pump_operation_time_read_pointer][11];
						 }
						 else
						 {
							 detailed_snore.person_off_second=stimestructure.Seconds+60-pump_operation_time[pump_operation_time_read_pointer][12];
							 if((pump_operation_time[pump_operation_time_read_pointer][11]+1)<=stimestructure.Minutes)
							  detailed_snore.person_off_minute=stimestructure.Minutes-pump_operation_time[pump_operation_time_read_pointer][11]-1;
							 else
								detailed_snore.person_off_minute=stimestructure.Minutes+59-pump_operation_time[pump_operation_time_read_pointer][11];
						 }
						 pump_operation_time[pump_operation_time_read_pointer][0]=0;
						 pump_operation_time_read_pointer++;
						 if(pump_operation_time_read_pointer>=20)
							 pump_operation_time_read_pointer=0;
						 pump_time_read_temp=4;
					    }
						 
					   } 
					  }
					 }
					 
					 if(pump_time_read_temp>=1)
					 {
						 
						 
						 
						 detailed_snore.HeadPosion = 0x0;				
					
					   detailed_snore.StudyMode = 0x80; //0x80:�������ʼʱ�����Ϣ
					   detailed_snore.UsedRecord=0x55aa;
					

						 
				     WriteSnoreData(&detailed_snore); //������������  �����⵽������û��⵽�˼�����ֹ��������Ҳ�ᱣ���������ݼ�¼
				
						 Airsensor_Print_Tuwan_Time();
						 osDelay(50);
						 
				     memset(&detailed_snore,0,sizeof(SnoreData_def));
					 }
					 if(pump_time_read_temp==4)
					 {
				     pump_time_read_temp=0;
						 pump_record_process_repaet_flag=1;
					 }
					 else
					 {
						 pump_time_read_temp=0;
						 pump_record_process_repaet_flag=0;
					 }
					 
//					 Airsensor_Print_Tuwan_Var();
//					 osDelay(50);
					 
				 } 
				 while(pump_record_process_repaet_flag==1);
         Airsensor.Poweroffkey_savedata_flag=0;
				 
         




				}	
        else
				{
					memset(&gStorageData.Snore,0,sizeof(SnoreData_def));
					gStorageData.Snore.UsedRecord=0x55aa;
					gStorageData.Snore.StudyMode=gStorageData.StudyMode;
					Config.SnoreAverTime = 4000; 
					gStorageData.Snore.HeadPosion = 0x0ff;	
//          gStorageData.Snore.HeadMovingTimes=0x0;
					person_on_hour=0xff;
					person_on_minute=0xff;
          person_on_second=0xff;
					person_off_hour=0xff;
          person_off_minute=0xff;
          person_off_second=0xff;
				}	
				
			}	
			else
			{
				m=stimestructure.Minutes;
			}
/*			
      gStorageData.Snore.StartTime[0]=sdatestructure.Year;
			gStorageData.Snore.StartTime[1]=sdatestructure.Month;
			gStorageData.Snore.StartTime[2]=sdatestructure.Date;
			gStorageData.Snore.StartTime[3]=stimestructure.Hours;
			gStorageData.Snore.StartTime[4]=stimestructure.Minutes;
			gStorageData.Snore.AVGSnoreTime=Config.SnoreAverTime/100;
			if((m!=stimestructure.Minutes)&&(stimestructure.Minutes%10==0))
			{
				 m=stimestructure.Minutes; 
				 if(gStorageData.Standby==0)   //������������
				 {
//					  gStorageData.Snore.HeadMovingTimes=Airsensor.Body_Moved_Times;
//					  gStorageData.Snore.HeadMovingTimes= Airsensor.AirSensor_PersonOn;
					  if(Airsensor.AirSensor_PersonOn==1)
				       gStorageData.Snore.HeadMovingTimes = gStorageData.Snore.HeadMovingTimes|0x01;
				    else
					     gStorageData.Snore.HeadMovingTimes = gStorageData.Snore.HeadMovingTimes&0xfe;
//						gStorageData.Snore.StartTime[0]=sdatestructure.Year;
//						gStorageData.Snore.StartTime[1]=sdatestructure.Month;
//						gStorageData.Snore.StartTime[2]=sdatestructure.Date;
//						gStorageData.Snore.StartTime[3]=stimestructure.Hours;
//						gStorageData.Snore.StartTime[4]=stimestructure.Minutes;
//					  gStorageData.Snore.AVGSnoreTime=Config.SnoreAverTime/100;
					 	WriteSnoreData(&gStorageData.Snore); //�����ϴ�
					  gStorageData.Snore.HeadMovingTimes = 0;
					  memset(&gStorageData.Snore,0,sizeof(SnoreData_def));
					  gStorageData.Snore.UsedRecord=0x55aa;
					  gStorageData.Snore.StudyMode=gStorageData.StudyMode;
					  Config.SnoreAverTime = 4000; 
//					  Airsensor.Body_Moved_Times=0;
				 }
			}
*/			
		xSemaphoreGive( xTimeMute );	
		} 	
  }
  /* USER CODE END 5 */ 
}


uint8_t UserStopAtAutoOnTimeFlag=0;
uint8_t UserStartAtAutoOffTimeFlag=0;

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
					Airsensor.LED_flash_state_flag=0;
		      WrokingLed(RED_LED,1);
		      Airsensor.Led_yellow_state=1;
					#if defined(TUWAN_PILLOW_3) || defined(TUWAN_MAT)
					while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				  {
              osDelay(1);
				  } 
     			Airsensor.AirSensor_Checkposition_Mode=SLOW_MODE;	
					xSemaphoreGive( xPositionMute );
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
	#ifdef MP1_PLUS	
		
	 #ifdef UserSetTimeAutoONOFF
		//�����û��ɵ��������Զ������͹رյĹ��� ��ʼ����
		if(gStorageData.Standby==1)
		{
		  //�ȵƹػ�״̬������Ƿ񵽿���ʱ��
			//����Ƿ��û��������Զ�����ʱ��
			if(gStorageData.UserSleepTime[0]<=23 && gStorageData.UserSleepTime[1] <= 59 && (gStorageData.UserSleepTime[0]!=gStorageData.UserSleepTime[2] || gStorageData.UserSleepTime[1]!=gStorageData.UserSleepTime[3]))
			{
				//�û����ÿ���ʱ����Ч������Ƿ񵽿���ʱ��
				if(stimestructure.Hours==gStorageData.UserSleepTime[0] && stimestructure.Minutes==gStorageData.UserSleepTime[1])
				{
				  if(UserStopAtAutoOnTimeFlag==0)
					{
					   //�Զ�����ʱ�䵽��ʵʩ����
					   //������ڲ���ģʽ�����������˯�߽׶�
						  if(AppTest==1 || ReadPos.enable==1 || ActionSteps !=0)
							{
							}
							
							else
							{
								if(UserkeycodeAutoSetData.validtime_year > sdatestructure.Year || (UserkeycodeAutoSetData.validtime_month > sdatestructure.Month && UserkeycodeAutoSetData.validtime_year == sdatestructure.Year) || (UserkeycodeAutoSetData.validtime_day > sdatestructure.Date && UserkeycodeAutoSetData.validtime_year == sdatestructure.Year && UserkeycodeAutoSetData.validtime_month == sdatestructure.Month) ||(UserkeycodeAutoSetData.validtime_day == sdatestructure.Date && UserkeycodeAutoSetData.validtime_year == sdatestructure.Year && UserkeycodeAutoSetData.validtime_month == sdatestructure.Month && stimestructure.Hours < 12))
								{
										//����˯�߹���״̬�����������˵�״̬
										Airsensor.AirSensor_Check_Mode=4;
										Airsensor.AirSensor_Checkposition_Mode=4;
										AirSensor_person_detect_init();
										Airsensor.AirSensor_pressure_sample_avg_diffsum=0;
										Airsensor.Sleep_rapidly_check_flag=1;
										WrokingLed(BLUE_LED_HIGH,1);//������
//						      while(GetPowerKey()==GPIO_PIN_SET)
//						      	osDelay(10);
							
							
									//�ر�����������app����
//									BlueToolthEnabled(0);							
//									SetBLERESET(0);
										gStorageData.Standby = 0;
										Airsensor.It_Is_Standby_State=0;
										Airsensor.AirSensor_PersonOn=0;
										Airsensor.Sleep_time_flag=0;
										Airsensor.period_snore_checkposition_flag=0;
										Airsensor.period_snore_checkposition_counter=0;
										Airsensor.onehour_snore_checkposition_counter=0;
										Airsensor.Check_again_if_person_on=0;
										Airsensor.Poweroffkey_savedata_flag=0;
									}	
								}								
							}
					
				}
				else
				{
					UserStopAtAutoOnTimeFlag=0;
				}
			}		
		}
		else
		{
			//�̵�����״̬������Ƿ񵽹ر�ʱ��
			//����Ƿ��û��������Զ��ر�ʱ��
			if(gStorageData.UserSleepTime[2]<=23 && gStorageData.UserSleepTime[3] <= 59 && (gStorageData.UserSleepTime[0]!=gStorageData.UserSleepTime[2] || gStorageData.UserSleepTime[1]!=gStorageData.UserSleepTime[3]))
			{
				//�û����ùر�ʱ����Ч������Ƿ񵽹ر�ʱ��
				if(stimestructure.Hours==gStorageData.UserSleepTime[2] && stimestructure.Minutes==gStorageData.UserSleepTime[3])
				{
					if(UserStartAtAutoOffTimeFlag==0)
					{
					    //�Զ��ر�ʱ�䵽��ʵʩ�ر�
					    WrokingLed(RED_LED,1);
						  Airsensor.AirSensor_PersonOn=0;
							Airsensor.AirSensor_PersonOn_Last=0;
							Airsensor.period_snore_checkposition_flag=0;
							Airsensor.Sleep_rapidly_check_flag=0;
//					    while(GetPowerKey()==GPIO_PIN_SET)
//						    osDelay(10);							
//							BlueToolthEnabled(1);
//							SetBLERESET(1);
							gStorageData.Standby = 1;
							Airsensor.It_Is_Standby_State=1;
							PillowControl.SnoreOn=0;
						  snore_times_every_sleep=0;
							snore_times_for_antisnore=0;
							snore_times_for_snore=0;
							Airsensor.Sleep_snore_study_flag=0;
							Airsensor.Sleep_snore_study_control=0;
							Airsensor.Tt_Is_Sleep_Time=0;
							Airsensor.Tt_Is_Sleep_Time_last=0;
							snore_times_for_antisnore=0;
							snore_times_for_snore=0;
							snore_times_every_sleep=0;
							Airsensor.Check_again_if_person_on=0;
						}
				}
				else
				{
					UserStartAtAutoOffTimeFlag=0;
				}
			}
		}
		
		
		
		
		
		//�����û��ɵ��������Զ������͹رյĹ��� ����
	#endif
		
		
	 	if(GetPowerKey()==GPIO_PIN_SET)
	 	{  
			if(CheckKeyPressState()==LONG_PRESS) //��������ػ�״̬�л�
			{
					if(gStorageData.Standby==0) 
					{  //�������˯�߹���״̬�����Ƚ���Ŀǰ�Ĺ���
							
							Scankey_setworkstate = 1;						
					}
					else
					{
							Scankey_setworkstate = 2;						
					}
//					WrokingLed(BLUE_LED_HIGH,0);
					
//					BackupSysPara(&gStorageData);
//					EntryStandbyMode();
			}
			else
			{
//				WrokingLed(BLUE_LED_HIGH,2);//������
//        osDelay(500);
//        WrokingLed(RED_LED,0);//������ 				
#if 0				
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
#endif					
			}

		}
		
		
		if(Scankey_setworkstate == 1 || APP_setworkstate ==1)
		{
							Scankey_setworkstate = 0;
							APP_setworkstate = 0;
			//						  Airsensor.Sleep_time_flag=0;
						  WrokingLed(RED_LED,1);
						  Airsensor.AirSensor_PersonOn=0;
							Airsensor.AirSensor_PersonOn_Last=0;
							Airsensor.period_snore_checkposition_flag=0;
							Airsensor.Sleep_rapidly_check_flag=0;
					    while(GetPowerKey()==GPIO_PIN_SET)
						    osDelay(10);							
//							BlueToolthEnabled(1);
//							SetBLERESET(1);
							gStorageData.Standby = 1;
							Airsensor.It_Is_Standby_State=1;
							PillowControl.SnoreOn=0;
						  snore_times_every_sleep=0;
							snore_times_for_antisnore=0;
							snore_times_for_snore=0;
							Airsensor.Sleep_snore_study_flag=0;
							Airsensor.Sleep_snore_study_control=0;
							Airsensor.Tt_Is_Sleep_Time=0;
							Airsensor.Tt_Is_Sleep_Time_last=0;
							snore_times_for_antisnore=0;
							snore_times_for_snore=0;
							snore_times_every_sleep=0;
							Airsensor.Check_again_if_person_on=0;
							
							workdelay_time_cycle_flag = 0;
							SnoreTime_forAPP.valid = 0;
							
							//Ϊ�Զ��������
							if(stimestructure.Hours==gStorageData.UserSleepTime[0] && stimestructure.Minutes==gStorageData.UserSleepTime[1])
							{
							  UserStopAtAutoOnTimeFlag=1;
							}
			
			
		}
		if(Scankey_setworkstate == 2 || APP_setworkstate ==2)
		{
							
							Scankey_setworkstate = 0;
							APP_setworkstate = 0;
							//������ڲ���ģʽ�����������˯�߽׶�
						  if(AppTest==1 || ReadPos.enable==1 || ActionSteps !=0)
							{
							}
							
							else
							{
						    if(UserkeycodeAutoSetData.validtime_year > sdatestructure.Year || (UserkeycodeAutoSetData.validtime_month > sdatestructure.Month && UserkeycodeAutoSetData.validtime_year == sdatestructure.Year) || (UserkeycodeAutoSetData.validtime_day > sdatestructure.Date && UserkeycodeAutoSetData.validtime_year == sdatestructure.Year && UserkeycodeAutoSetData.validtime_month == sdatestructure.Month) ||(UserkeycodeAutoSetData.validtime_day == sdatestructure.Date && UserkeycodeAutoSetData.validtime_year == sdatestructure.Year && UserkeycodeAutoSetData.validtime_month == sdatestructure.Month && stimestructure.Hours < 12))
								{	
										//����˯�߹���״̬�����������˵�״̬
										Airsensor.AirSensor_Check_Mode=4;
										Airsensor.AirSensor_Checkposition_Mode=4;
										AirSensor_person_detect_init();
										Airsensor.AirSensor_pressure_sample_avg_diffsum=0;
										Airsensor.Sleep_rapidly_check_flag=1;
										WrokingLed(BLUE_LED_HIGH,1);//������
										while(GetPowerKey()==GPIO_PIN_SET)
											osDelay(10);
							
							
										//�ر�����������app����
//									BlueToolthEnabled(0);							
//									SetBLERESET(0);
										gStorageData.Standby = 0;
										Airsensor.It_Is_Standby_State=0;
										Airsensor.AirSensor_PersonOn=0;
										Airsensor.Sleep_time_flag=0;
										Airsensor.period_snore_checkposition_flag=0;
										Airsensor.period_snore_checkposition_counter=0;
										Airsensor.onehour_snore_checkposition_counter=0;
										Airsensor.Check_again_if_person_on=0;
										Airsensor.Poweroffkey_savedata_flag=0;
								
										workdelay_time_cycle_flag = 0;
										SnoreTime_forAPP.valid = 0;
								
										if(stimestructure.Hours==gStorageData.UserSleepTime[2] && stimestructure.Minutes==gStorageData.UserSleepTime[3])
										{
											//�û����Զ��ػ�ʱ���˹�����
											UserStartAtAutoOffTimeFlag=1;
										}
								
									}
							}
						}
		
	#endif
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
