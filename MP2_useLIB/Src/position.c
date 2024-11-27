/********************************************************************************/
/* Filename	: position.c                                                        */
/*                                                                              */
/* Content	:                                                                   */
/* Note    	:                                                                   */
/*                                                                              */
/* Change History   :                                                           */
/*        V.0.00  2017/2/22                                                     */
/********************************************************************************/
#include "cmsis_os.h"
#include "tuwan.h"
#include "pillow.h"
#include "position.h"
#include "snore.h"
#include "snortask.h"
#include <SpiFlashFileSys.h>

void ADCstart1(void);
void ADCstart2(void);
void ADCstart3(void);
void GetSnoreDataInSPIFlashDebug(void);

//stimestructure
 HeadPosition_def HeadPosition={BODY_NO,HEAD_NO,HEAD_NO},LastHeadPosition={BODY_NO,HEAD_NO,HEAD_NO},LastPosition,position={BODY_NO,HEAD_NO,HEAD_NO};
 PillowControl_def PillowControl={false,false,false,false};
// RequestControlValve  0������λ�ü�⣬1������λ�ü�⣬2��������ֹͣ
 uint8_t Body_Left_Right_Confirmed=1;
 uint8_t Side_Sleep=0;
 
 uint8_t Position_State_Before_Recheck=0;
 uint8_t Ten_minutes_first_personon_event_flag=0;
 unsigned char person_on_hour=0xff;
 unsigned char person_on_minute=0xff;
 unsigned char person_on_second=0xff;
 unsigned char person_off_hour=0xff;
 unsigned char person_off_minute=0xff;
 unsigned char person_off_second=0xff;
 unsigned char debug_factoryreset_enable=0;
 unsigned int snore_times_every_sleep=0;
 unsigned int snore_times_for_antisnore=0;
 unsigned int snore_times_for_snore=0;
 
 extern uint8_t Error_Code;
 
 uint8_t AirSensor_Position_Changed=0;
 uint8_t cSensor_ErrorCode,Body_Left_Right_Willbesend,PillowMaxOne_Willbesend; 
 
 uint8_t TIM7_operation_stepnum=0;
 //uint16_t TIM7_operation_counter_time[10];
 //uint8_t TIM7_operation_code[10];
 uint8_t TIM7_operation_finished_flag=0;
 
 
 short MIC_data_for_Walve_debug[MIC_data_for_Walve_debug_length]; //����5��ÿ����8��16bit�Ĳ�������
 uint16_t MIC_data_for_Walve_debug_pointer=0;
 uint8_t MIC_data_save_enable=0;
 short MIC_data_MAX[MIC_point_repeat_times], MIC_data_Min[MIC_point_repeat_times];
 uint8_t MIC_point_repeat_step=0;
 uint16_t Walve_poweron_time=0;
 uint16_t Walve_poweroff_time=0;
 uint8_t MIC_data_stop_detect_flag =0;
 
 uint8_t Tt_Is_Sleep_Time=0;
 
 //��������״̬����  1����ͨ�����ã�0����ͨ������
 
 uint8_t Valve_State[4]={0,0,0,0};

//******************************************** 
 uint8_t AirSensor_Position_Checking=0; //1:������ͷ��λ������ 0:δ�����
 uint8_t Snore_MaxDB_Detected;
 uint8_t Snore_detected_type_flag;
 extern TIM_HandleTypeDef htim7;
 void TIM_wait_operation_finish(void);
 extern unsigned char AppTest;
 extern SemaphoreHandle_t xTimeMute;
 extern SemaphoreHandle_t xPositionMute;
 extern SemaphoreHandle_t xLogoMute;
 extern uint8_t person_on_for_save_data_flag;
 extern uint8_t Upgrading_On;
 extern uint8_t log_newfcc_flag,log_weight_add1_flag;
 
 extern uint16_t dynamic_weight,max_vectorweight_inLIB;
 
 extern uint8_t rechecktime_minute,rechecktime_hour,latestsnoretime_minute,latestsnoretime_hour,personontime_minute,personontime_hour;

 uint8_t Received_BLE_NAME_flag=0;
 extern void SetBleName(void);
 extern uint8_t rcvbufdata[50];
 extern void VerifyBleName(void);
 extern uint8_t BleNameVerifyOK_flag;
 extern unsigned char SetBleNameBuffer[14];
 
 uint8_t app_settime_flag=0;
 extern unsigned char study_hour_start,study_min_start,study_hour_end,study_min_end;
 static unsigned int checkposition_count_timer = 0;
//static unsigned int noperson_hour_count_timer = 0;
 unsigned int Antisnore_count_timer = 0;
 unsigned int workdelay_count_timer = 0;
 unsigned int update_request_counter_timer = 0;
 extern uint8_t Firmware_update_received_flag;
 extern uint8_t Firmware_update_keypressed;
 
 extern uint8_t pump_operation_time[20][13];//20����ÿ����һ���ֽ�Ϊ0��������Ч��1��������Ч
 extern uint8_t pump_operation_time_write_pointer;
 extern uint8_t pump_operation_time_read_pointer;//��¼�ͱ���λ��ָ��
 extern SnoreData_def detailed_snore;
 
 extern volatile uint8_t LEDFlashTimer20msOn;
 extern int ResetToFactory(void);

extern uint8_t workdelay_time_cycle_flag;
uint8_t BT_Recieved_Data_From_App_Buffer[4000];

uint16_t BT_Recieved_Data_From_App_Buffer_Pointer=0;
uint8_t BT_Sent_Data_To_App_Buffer[3000];
uint16_t BT_Sent_Data_To_App_Buffer_Pointer=0;

uint8_t reply_app_for_update_keypress_flag = 0;

extern osThreadId SnoreTaskHandle;
extern osThreadId MainTaskHandle;
extern osThreadId ForthTemialTaskHandle;
extern osThreadId PositionTaskHandle;
extern osThreadId ActionTaskHandle;
extern osThreadId LedFlashTaskHandle;

#ifdef TUWAN_BLANKET


//*************************************************************************
//                 ����ֹ��̺��ѹλ��ʶ�����

//*************************************************************************


#define AirSensor_threshold_person_on_sumMAX 0x0a0 //�����ѹ����ͷ��ֵ


#define AirSensor_Positionbag_OffAirTime 8000 //ms  ������̬һ�����ʱ��
#define AirSensor_Positionbag_EnAirTime   3000 //ms ������������ʱ��
#define AirSensor_Calibration_DelayTime_ReadValue 1000 //ms ����ʱ������ȴ������ȥ��ȡ��ѹֵ
#define AirSensor_position_check_cycle     25  //20ms
#define AirSensor_calibration_times        2   //����ʱ����������ֵ����ƽ��ֵ�Ĵ���
#define AirSensor_Airbag_Reinit_Threhold   0x800 //λ��������ѹС�ڸ�ֵ���³��������

uint8_t AirSensor_factory_calibration_flag=0;
/**************************************************************************/
/*                                                                        */
/**************************************************************************/
extern volatile uint8_t Timer20msOn;
extern volatile uint8_t Timer10msOn;


extern int PositionInitWR(unsigned char *pt, unsigned int len, unsigned char wr);
extern uint16_t    udirdata;
uint8_t     dirdata=0;
uint8_t     HeadStatus = 0;
extern unsigned int PressureADC_ConvertedValue[6];



uint8_t AirSensor_cp_calibrationflag = 0;
uint8_t AirSensor_positionbag_init_flag=0;

uint16_t AirSensor_calibration_PressureL,AirSensor_calibration_PressureR,AirSensor_calibration_Pressure[2];
uint16_t AirSensor_Baseline_PressureL,AirSensor_Baseline_PressureR;
uint16_t Airsensor.AirSensor_PressureR[5],Airsensor.AirSensor_PressureL[5],Airsensor.AirSensor_PressureR_Avg,Airsensor.AirSensor_PressureL_Avg,Airsensor.AirSensor_PressureL_Avg_Buffer[3],Airsensor.AirSensor_PressureR_Avg_Buffer[3];
uint8_t AirSensor_Avg_Buffer_Counter=0;
uint8_t AirSensor_readdata_ready = 0;
uint16_t AirSensor_PositionL_DiffP,AirSensor_PositionL_DiffN,AirSensor_PositionR_DiffP,AirSensor_PositionR_DiffN;
uint16_t AirSensor_Position_DiffMax,AirSensor_Diff_BetweenLR;
uint8_t Body_Left_Right=0,Body_Left_Right_Buffer[5];
uint8_t SendData[60];
uint16_t Airsensor_readvalue_50ms;
uint16_t Airsensor.AirSensor_PressureL_Avg_1,Airsensor.AirSensor_PressureR_Avg_1;


extern volatile uint8_t Timer20msOn;
extern int PositionInitWR(unsigned char *pt, unsigned int len, unsigned char wr);
extern uint16_t    udirdata;

extern unsigned int PressureADC_ConvertedValue[6];
extern unsigned int InhaleADC_ConvertedValue[6],ExhaleADC_ConvertedValue[6];
extern unsigned char ADok2,ADok1,ADok3;



/**************************************************************************/
/*                                                                        */
/**************************************************************************/
/*
void AirPressureR_check(void)
{
	    uint8_t i;
	    for(i=0;i<5;i++)
	     {
	      ADok1=0;
		    ADCstart1();
		    do
		     {
			    osDelay(1);
		     }while(ADok1==0);
	      Airsensor.AirSensor_PressureR[i]=ExhaleADC_ConvertedValue[0];
			 }
			
			
			Airsensor.AirSensor_PressureR_Avg=0; 
      for(i=0;i<5;i++)
       {
				
				Airsensor.AirSensor_PressureR_Avg+=Airsensor.AirSensor_PressureR[i];
			 }
      
			Airsensor.AirSensor_PressureR_Avg=	Airsensor.AirSensor_PressureR_Avg/5;
      		 
		
}
void AirPressureL_check(void)
{
	    uint8_t i;
	    
			for(i=0;i<5;i++)
			 {
			  ADok2=0;
		    ADCstart2();
			 
		    do
		     {
			    osDelay(1);
		     }while(ADok2==0);
	      Airsensor.AirSensor_PressureL[i]=InhaleADC_ConvertedValue[0];
       }
			Airsensor.AirSensor_PressureL_Avg=0;
			
      for(i=0;i<5;i++)
       {
				Airsensor.AirSensor_PressureL_Avg+=Airsensor.AirSensor_PressureL[i];
				
			 }
      Airsensor.AirSensor_PressureL_Avg=	Airsensor.AirSensor_PressureL_Avg/5;
					 
		
}
*/
void AirPressure_check(void)
{
	    uint8_t i;
	    for(i=0;i<5;i++)
	     {
	      ADok1=0;
		    ADCstart1();
		    do
		     {
			    osDelay(1);
		     }while(ADok1==0);
	      Airsensor.AirSensor_PressureR[i]=ExhaleADC_ConvertedValue[0];
			 }
			for(i=0;i<5;i++)
			 {
			  ADok2=0;
		    ADCstart2();
			 
		    do
		     {
			    osDelay(1);
		     }while(ADok2==0);
	      Airsensor.AirSensor_PressureL[i]=InhaleADC_ConvertedValue[0];
       }
			Airsensor.AirSensor_PressureL_Avg=0;
			Airsensor.AirSensor_PressureR_Avg=0; 
      for(i=0;i<5;i++)
       {
				Airsensor.AirSensor_PressureL_Avg+=Airsensor.AirSensor_PressureL[i];
				Airsensor.AirSensor_PressureR_Avg+=Airsensor.AirSensor_PressureR[i];
			 }
      Airsensor.AirSensor_PressureL_Avg=	Airsensor.AirSensor_PressureL_Avg/5;
			Airsensor.AirSensor_PressureR_Avg=	Airsensor.AirSensor_PressureR_Avg/5;
      AirSensor_readdata_ready=1;			 
		/*	 
			 ADok3=0;
		  ADCstart3();
		  do
		   {
			  osDelay(1);
		   }while(ADok3==0);
	     Airsensor.AirSensor_Pressure3=PressureADC_ConvertedValue[0];
		*/	
}



void Airsensor_SendlogtoPC(void)
{

    

        SendData[0] = Body_Left_Right_Willbesend;
	      SendData[1] = Body_Left_Right;
//        SendData[1] = Body_Left_Right_Confirmed;

	      SendData[2] = 0xFF;

        SendData[3] = (Airsensor.AirSensor_PressureL_Avg&0xff00)>>8;
				SendData[4] = Airsensor.AirSensor_PressureL_Avg&0xff;
				SendData[5] = (Airsensor.AirSensor_PressureR_Avg&0xff00)>>8;
				SendData[6] = Airsensor.AirSensor_PressureR_Avg&0xff;
	
	      SendData[7] = 0xFF;
	      SendData[8] = (AirSensor_Position_DiffMax&0xff00)>>8;	
				SendData[9] = AirSensor_Position_DiffMax&0xff;
        SendData[10] = (AirSensor_Diff_BetweenLR&0xff00)>>8;
	      SendData[11] = AirSensor_Diff_BetweenLR&0xff;
				SendData[12] = 0xFF;
	      SendData[13] = (AirSensor_PositionL_DiffP&0xff00)>>8;	
				SendData[14] = AirSensor_PositionL_DiffP&0xff;	
//        SendData[13] = (AirSensor_PositionL_DiffN&0xff00)>>8;	
//				SendData[14] = AirSensor_PositionL_DiffN&0xff;	
        SendData[15] = (AirSensor_PositionR_DiffP&0xff00)>>8;	
				SendData[16] = AirSensor_PositionR_DiffP&0xff;	
 //       SendData[17] = (AirSensor_PositionR_DiffN&0xff00)>>8;	
//				SendData[18] = AirSensor_PositionR_DiffN&0xff;
        SendData[17] = 0xFF;
				SendData[18] = (AirSensor_Baseline_PressureL&0xff00)>>8;	
				SendData[19] = AirSensor_Baseline_PressureL&0xff;
        SendData[20] = (AirSensor_Baseline_PressureR&0xff00)>>8;	
				SendData[21] = AirSensor_Baseline_PressureR&0xff;


        //-----------------------------------------        
        SendData[22] = 0xAA;
        SendData[23] = ActionSteps;
        SendData[24] = PillowControl.SnoreOn;
        SendData[25] = PVEvol8;     //real
        //------------
        SendData[26] = SnoreType+(gStorageData.SensorLevel<<4);
		SendData[27] = SnoreNose;   //nose mid
		SendData[28] = Avol8;       //snore max vol
		SendData[29] = SnoreAver;   //snore max aver
		SendData[30] = SnoreNoseLow;  //nose low
		SendData[31] = SnoreNoseHigh; //nose hight
        //------------
//		SendData[34] = stimestructure.Hours;
//        SendData[35] = stimestructure.Minutes;
//        SendData[36] = stimestructure.Seconds;
        SendData[32] = 0x55;
				
//				SendData[38]= (Airsensor.AirSensor_PressureL_Avg_1&0xff00)>>8;
//				SendData[39]= Airsensor.AirSensor_PressureL_Avg_1&0xff;
//				SendData[40]= (Airsensor.AirSensor_PressureR_Avg_1&0xff00)>>8;
//				SendData[41]= Airsensor.AirSensor_PressureR_Avg_1&0xff;
        //------------
        Avol8 = 0;
        SnoreType = 0;
        SnoreNose = 0;
        SnoreAver = 0;
        SnoreNoseLow = 0;
        SnoreNoseHigh = 0;
        //-------------------------------------------
        #ifndef SEND_PCM 
            #ifndef PRINT_TASK_STACK 
                uart_send(SendData, 33);
            #endif 
        #endif 
}



/**************************************************************************/
/*                                                                        */
/**************************************************************************/
void SendDataToPCorSTM32() 
{



        LastPosition.body = Body_Left_Right_Willbesend;
        LastPosition.head1 = 0x10;
        LastPosition.head2 = 0x10;
        LastPosition.sleepOneSide = 0;
        LastPosition.workon = 0;
	
	
        position = LastPosition;
	     
        if (PillowControl.PillowAction == false)
        {
            if ((HeadPosition.body == BODY_NO) && (position.body != BODY_NO))
            {
              //  SnoreInit();
            }
            HeadPosition = position;
        }
 
        
 

	

}




//#define HEAD_R_0_VALUE    1   ������  �������ڶ˵ķ�����Ӧ
//#define HEAD_R_1_VALUE    2   û��
//#define HEAD_R_2_VALUE    3   λ����
//#define HEAD_R_3_VALUE    4   λ����
//#define BODY_R_VALUE      5   ������
//#define BODY_L_VALUE      6   ������
//#define EXHALE_VALUE      7   ������
//ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);�ضϷ��ţ���û��ʱһ����
//ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);�򿪷���
//InhalePump
//ExhalePump


void AirSensor_OffAir_PositionbagLR(void)
{
    ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);      //λ����
//		ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);    //û��
		ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);    //������
		ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);    //λ����
		ValueDriver(BODY_R_VALUE,GPIO_PIN_SET);        //������
		ValueDriver(BODY_L_VALUE,GPIO_PIN_SET);        //������
		ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);      //������
	  osDelay(500);
		InhalePump(1); 
		osDelay(AirSensor_Positionbag_OffAirTime);
		InhalePump(0); 
//    ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);        //������
//		ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET); 
    osDelay(500);	
}
void AirSensor_EnAir_RightAirbag(void)
{
	  
		do{
		 osDelay(1);
		}while(ActionSteps != 0x00);
	  ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);      //λ����
		ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);        //������
		ValueDriver(BODY_L_VALUE,GPIO_PIN_SET);        //������
		ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);      //������
	  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);    //������
    ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);    //λ����
	  osDelay(500);
		InhalePump(1); 
		osDelay(9*60000);
		InhalePump(0);
	  ValueDriver(BODY_R_VALUE,GPIO_PIN_SET);      //������
	  osDelay(500);
    ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);        //������
	  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);      //������
		osDelay(100);
}
/*
//��������Ŀ����ѹ�ĳ���ʱ����
void AirSensor_EnAir_PositionbagLR_Fixed_Pressure(void)
{
	  uint8_t i;
	  ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);      //λ����
//		ValueDriver(BODY_R_VALUE,GPIO_PIN_SET);        //������
//		ValueDriver(BODY_L_VALUE,GPIO_PIN_SET);        //������
		ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);      //������
	  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);    //������
    ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);    //λ����
	  osDelay(500);
	  InhalePump(1);
	  i=0;
    do
		 {
     	AirPressureL_check();
      if(Airsensor.AirSensor_PressureL_Avg>=0x870)
			 {				
		    i=1;
				Airsensor.AirSensor_PressureL_Avg_1= Airsensor.AirSensor_PressureL_Avg;
			 }
		 }
		while(i==0);
		InhalePump(0);
	  ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);      //λ����
	  osDelay(500);
		 
		 

    ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);        //λ����
	  osDelay(500);
 	  InhalePump(1);
		 
//		osDelay(AirSensor_Positionbag_EnAirTime);
		i=0;
    do
		 {
     	AirPressureR_check();
      if(Airsensor.AirSensor_PressureR_Avg>=0x870)
			 {				
		    i=1;
				Airsensor.AirSensor_PressureR_Avg_1= Airsensor.AirSensor_PressureR_Avg;
			 }
		 }
		while(i==0);
		InhalePump(0);
	  ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);      //λ����
	  osDelay(500);
	
    ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);        //������
		ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);        //������
    ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);        //������
	  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);      //������
		osDelay(100);
}
*/

//��Ԥ��ʱ�����
void AirSensor_EnAir_PositionbagLR(void)
{
	  ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);      //λ����
		ValueDriver(BODY_R_VALUE,GPIO_PIN_SET);        //������
		ValueDriver(BODY_L_VALUE,GPIO_PIN_SET);        //������
		ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);      //������
	  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);    //������
    ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);    //λ����
	  osDelay(500);
		InhalePump(1); 
		osDelay(AirSensor_Positionbag_EnAirTime);
		InhalePump(0);
	  ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);      //λ����
	  osDelay(500);

    ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);        //λ����
	  osDelay(500);
 	  InhalePump(1); 
		osDelay(AirSensor_Positionbag_EnAirTime);
		InhalePump(0);
	  ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);      //λ����
	  osDelay(500);
	
    ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);        //������
		ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);        //������
    ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);        //������
	  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);      //������
		osDelay(100);
}



void Airsensor_factory_calibration_operation(void)
{
  uint8_t i,wrret=0;
	if (ActionSteps == 0x00 && RequestControlValve==0)
	{
    if (AirSensor_factory_calibration_flag == 1)
     {
        AirSensor_cp_calibrationflag = 1;
        AirSensor_calibration_PressureL=0;
			  AirSensor_calibration_PressureR=0;
        AirSensor_factory_calibration_flag = 0;
			  
			  for(i=0;i<AirSensor_calibration_times;i++)
			   {
					AirSensor_OffAir_PositionbagLR();
				  AirSensor_EnAir_PositionbagLR();
					osDelay(AirSensor_Calibration_DelayTime_ReadValue);
          AirPressure_check();
					AirSensor_calibration_PressureL += Airsensor.AirSensor_PressureL_Avg;
					AirSensor_calibration_PressureR += Airsensor.AirSensor_PressureR_Avg; 
				 }	
        AirSensor_calibration_PressureL = AirSensor_calibration_PressureL/AirSensor_calibration_times;
        AirSensor_calibration_PressureR = AirSensor_calibration_PressureR/AirSensor_calibration_times;
				AirSensor_calibration_Pressure[0]= AirSensor_calibration_PressureL;
				AirSensor_calibration_Pressure[1]= AirSensor_calibration_PressureR;
				AirSensor_Baseline_PressureL=AirSensor_calibration_PressureL;
		    AirSensor_Baseline_PressureR=AirSensor_calibration_PressureR; 
        AirSensor_cp_calibrationflag = 0;	
        AirSensor_positionbag_init_flag=1;
        AirSensor_readdata_ready=0; 				 
        while(wrret==0)
         {
				  wrret = PositionInitWR((uint8_t*)AirSensor_calibration_Pressure,sizeof(uint16_t)*2,1);
         }
				 
		}
    
	}	
}

		
							
void AirSensor_Positionbag_Init(void)
{
	  if(AirSensor_positionbag_init_flag==0)
		 {
	    if(ActionSteps == 0x00 && RequestControlValve==0)
		   {
//	      AirSensor_OffAir_PositionbagLR();
				AirSensor_EnAir_PositionbagLR();

			  AirSensor_positionbag_init_flag=1;
		   }
		 }
		else
		 {
			//���λ�ü�������Ƿ���ѹ������Ҫ���³�������
			if(ActionSteps == 0x00 && RequestControlValve==0)      
			 {				
			  if(AirSensor_Baseline_PressureL<AirSensor_Airbag_Reinit_Threhold||AirSensor_Baseline_PressureR<AirSensor_Airbag_Reinit_Threhold)
		     {
	        AirSensor_OffAir_PositionbagLR();
				  AirSensor_EnAir_PositionbagLR();
					AirSensor_Baseline_PressureL=AirSensor_calibration_PressureL;
		      AirSensor_Baseline_PressureR=AirSensor_calibration_PressureR;   
		     }
			 }
		 } 
}



/**************************************************************************/
/*                                                                        */
/**************************************************************************/
void Time20MSCheck(void)
{
//    static unsigned char status2 = 0;
    

	
    static uint8_t Airsensor_Count_20ms = 0;
//    static  uint8_t uart2[2] = {0xA5,0x51};
    if (Timer20msOn == 1)
    {
        Timer20msOn = 0;
			  Airsensor_Count_20ms++;

				if(Airsensor_Count_20ms >= 50)
        {
          Airsensor_Count_20ms=0;  
					Airsensor_SendlogtoPC();
        }
	      //--------------------------------
        Airsensor_readvalue_50ms++;
				if(AirSensor_positionbag_init_flag==1)
				 {	
          if (Airsensor_readvalue_50ms > AirSensor_position_check_cycle)
            {
 
             Airsensor_readvalue_50ms = 0;
             AirPressure_check();
//						 Airsensor_SendlogtoPC();
            }
					}
				 
	
    }
}
//AirSensor_calibration_PressureL
//Airsensor.AirSensor_PressureL_Avg
void AirSensor_position_process(void)
{
	uint8_t i,j,k,m;
  uint16_t lk;
	if(AirSensor_readdata_ready==1)
	 {
		if(AirSensor_positionbag_init_flag==1)
		 {
			 Airsensor.AirSensor_PressureL_Avg_Buffer[0]=Airsensor.AirSensor_PressureL_Avg_Buffer[1];
			 Airsensor.AirSensor_PressureL_Avg_Buffer[1]=Airsensor.AirSensor_PressureL_Avg_Buffer[2];
			 Airsensor.AirSensor_PressureL_Avg_Buffer[2]=Airsensor.AirSensor_PressureL_Avg;
			 Airsensor.AirSensor_PressureR_Avg_Buffer[0]=Airsensor.AirSensor_PressureR_Avg_Buffer[1];
			 Airsensor.AirSensor_PressureR_Avg_Buffer[1]=Airsensor.AirSensor_PressureR_Avg_Buffer[2];
			 Airsensor.AirSensor_PressureR_Avg_Buffer[2]=Airsensor.AirSensor_PressureR_Avg;			 
			 if(AirSensor_Avg_Buffer_Counter<3)
				AirSensor_Avg_Buffer_Counter++;
       else
			  {				 
			   if(Airsensor.AirSensor_PressureL_Avg_Buffer[0]<AirSensor_Baseline_PressureL && Airsensor.AirSensor_PressureL_Avg_Buffer[1]<AirSensor_Baseline_PressureL 
					  && Airsensor.AirSensor_PressureL_Avg_Buffer[2]<AirSensor_Baseline_PressureL)
				  {
					 if(Airsensor.AirSensor_PressureL_Avg_Buffer[0]>=Airsensor.AirSensor_PressureL_Avg_Buffer[1])
					  { 
						 if(Airsensor.AirSensor_PressureL_Avg_Buffer[0]>=Airsensor.AirSensor_PressureL_Avg_Buffer[2])
							lk= Airsensor.AirSensor_PressureL_Avg_Buffer[0];
						 else 
							lk= Airsensor.AirSensor_PressureL_Avg_Buffer[2];
						}
					 else
					  {
						 if(Airsensor.AirSensor_PressureL_Avg_Buffer[1]>=Airsensor.AirSensor_PressureL_Avg_Buffer[2])
							lk= Airsensor.AirSensor_PressureL_Avg_Buffer[1];
						 else 
							lk= Airsensor.AirSensor_PressureL_Avg_Buffer[2];
					  }
					 if(lk<AirSensor_Baseline_PressureL)
					  AirSensor_Baseline_PressureL=lk;	
					}
			   if(Airsensor.AirSensor_PressureR_Avg_Buffer[0]<AirSensor_Baseline_PressureR && Airsensor.AirSensor_PressureR_Avg_Buffer[1]<AirSensor_Baseline_PressureR 
					  && Airsensor.AirSensor_PressureR_Avg_Buffer[2]<AirSensor_Baseline_PressureR)
				  {
					 if(Airsensor.AirSensor_PressureR_Avg_Buffer[0]>=Airsensor.AirSensor_PressureR_Avg_Buffer[1])
					  { 
						 if(Airsensor.AirSensor_PressureR_Avg_Buffer[0]>=Airsensor.AirSensor_PressureR_Avg_Buffer[2])
							lk= Airsensor.AirSensor_PressureR_Avg_Buffer[0];
						 else 
							lk= Airsensor.AirSensor_PressureR_Avg_Buffer[2];
						}
					 else
					  {
						 if(Airsensor.AirSensor_PressureR_Avg_Buffer[1]>=Airsensor.AirSensor_PressureR_Avg_Buffer[2])
							lk= Airsensor.AirSensor_PressureR_Avg_Buffer[1];
						 else 
							lk= Airsensor.AirSensor_PressureR_Avg_Buffer[2];
					  }
					 if(lk<AirSensor_Baseline_PressureR)
					  AirSensor_Baseline_PressureR=lk;	
					} 
			   
			  }				 
			 

	    //�������߱仯ֵ֮��
      if(Airsensor.AirSensor_PressureL_Avg>AirSensor_Baseline_PressureL)
			 AirSensor_PositionL_DiffP=Airsensor.AirSensor_PressureL_Avg-AirSensor_Baseline_PressureL;
			else
			 AirSensor_PositionL_DiffP=0;
			if(Airsensor.AirSensor_PressureR_Avg>AirSensor_Baseline_PressureR)
			 AirSensor_PositionR_DiffP=Airsensor.AirSensor_PressureR_Avg-AirSensor_Baseline_PressureR;
			else
			 AirSensor_PositionR_DiffP=0;
			AirSensor_Position_DiffMax=AirSensor_PositionL_DiffP+AirSensor_PositionR_DiffP;
			
			if(AirSensor_PositionL_DiffP>=AirSensor_PositionR_DiffP)
			 AirSensor_Diff_BetweenLR=AirSensor_PositionL_DiffP-AirSensor_PositionR_DiffP;
			else
			 AirSensor_Diff_BetweenLR=AirSensor_PositionR_DiffP-AirSensor_PositionL_DiffP;
			
		
      if(AirSensor_Position_DiffMax >= AirSensor_threshold_person_on_sumMAX)
			 {
			  if(AirSensor_PositionL_DiffP>=AirSensor_PositionR_DiffP)
				 Body_Left_Right=3;
				else
				 Body_Left_Right=1; 
			 }	
      else
			 {
				Body_Left_Right=0; 
			 }	
			 
			for(i=0;i<4;i++)
			 {
				Body_Left_Right_Buffer[i]= Body_Left_Right_Buffer[i+1];
			 }
      Body_Left_Right_Buffer[4]= Body_Left_Right;
			j=0;
			k=0;
			for(i=0;i<5;i++)
			 {
				if(Body_Left_Right_Buffer[i]==1)
				 j++;
        if(Body_Left_Right_Buffer[i]==3)
         k++;	
        if(Body_Left_Right_Buffer[i]==0)
         m++;					
			 }
			if(j>=3)
			 	Body_Left_Right_Willbesend=1;
			else if(k>=3)
				Body_Left_Right_Willbesend=3;
      else if(m>=3)
        Body_Left_Right_Willbesend=0;				
		 }			 
		AirSensor_readdata_ready=0;
    SendDataToPCorSTM32();		 
	 }
 }

/**************************************************************************/
/*                                                                        */
/**************************************************************************/
void PositionTaskFunction(void const *argument)
{
//	  uint8_t i;
    uint8_t wrret=0;
    
    //unsigned char status2 = 0;
    //------------init-----------------------------

    while(StartAllAction==0)
 		 osDelay(10);
    
	
    LastPosition.body = 0;
    LastPosition.head1 = 0x10;
    LastPosition.head2 = 0x10;

		
	  osDelay(500);  //��ʱ���ϵͳ�ϵ��ʼ�����ȶ����������300ms
//    Airsensor.AirSensor_Checkposition_Mode=2;
//		AirSensor_fast_check_position();
//		Airsensor.AirSensor_Checkposition_Mode=1;
//		PillowMaxOne_Lastsend=0x10; 
//		Pillow2ndMaxOne_Lastsend=0x10;
//    AirSensor_fast_check_position();
    wrret=0;	  
		while(wrret==0)
	  {
	    wrret = PositionInitWR((uint8_t*)AirSensor_calibration_Pressure,sizeof(uint16_t)*2,0);
     }
//		AirSensor_EnAir_RightAirbag();
		AirSensor_calibration_PressureL=AirSensor_calibration_Pressure[0];
		AirSensor_calibration_PressureR=AirSensor_calibration_Pressure[1]; 
		AirSensor_Baseline_PressureL=AirSensor_calibration_PressureL;
		AirSensor_Baseline_PressureR=AirSensor_calibration_PressureR; 
    //---------------------------------------------
    /* Infinite loop */
    for (;;)
    {
        //PowerLed(status2);
        //status2 = !status2;
        osDelay(1); 
        #ifdef PRINT_TASK_STACK
            temp++;
            if (temp % 10 == 0)
            {
                vPrintfTaskStatus();
            }
        #endif 
        //---------------------------------------------
					if(RequestControlValve==1)
				   {
					  Airsensor_readvalue_50ms=0;
            RequestControlValve=2;             
				    }
					else
					 {
					  Airsensor_factory_calibration_operation();
					  AirSensor_Positionbag_Init();
					 }						 
          Time20MSCheck();
					AirSensor_position_process();

					 

        //---------------------------------------------
    }
    /* USER CODE END PositionTaskFunction */
}






#else



//*************************************************************************
//                 ����ֹ������ѹλ��ʶ�����

//*************************************************************************






extern volatile uint8_t Timer20msOn;
extern volatile uint8_t Timer10msOn;
extern volatile uint32_t TimerTick;
extern volatile uint32_t AirSensor_Personon_Timer;
extern volatile uint32_t Led_OnOff_Timer;
extern int PositionInitWR(unsigned char *pt, unsigned int len, unsigned char wr);
extern uint16_t    udirdata;
uint8_t     dirdata=0;
uint8_t     HeadStatus = 0;
extern unsigned int PressureADC_ConvertedValue[6];
extern unsigned int InhaleADC_ConvertedValue[6],ExhaleADC_ConvertedValue[6];
extern unsigned char ADok2,ADok1,ADok3;


//only for korea 3.1.29
//start

SensorLevelAutoSet_Def SensorLevelAutoSetData;
UserkeycodeAutoSet_Def UserkeycodeAutoSetData;

Airsensor_struct_Def Airsensor;
Valve_struct_Def Valve_Control;

uint8_t time_start_ok_flag=0;

uint8_t SensorLevelAutoSetData_timeout_flag=0;
//end

 
 
 




//uint8_t AirSensor_readpressuredata_counter=0;
uint8_t AirSensor_position_check_finish_flag;//,SendData[1000],;
unsigned char SendData[4000];
uint8_t Pillow2ndMaxOne_Willbesend,secondtest;
//uint8_t AirSensor_Checkposition_Mode;// 1:slow mode 2:fast mode 3:once time(fast) 4:person check other:no check
uint16_t Airsensor_readvalue_50ms = 0;
//uint8_t AirSensor_Check_Mode; //0:no operation 1:slow check position  2:fast check position 3:fast check position once time 4:check person on/off
//uint8_t AirSensor_position_checked_by_fastmode_flag=0;//λ������02 fast mode����λ��

uint16_t AirSensor_checkposition_pumpon_time_everyone[4]={400,400,400,400};
#define AirSensor_ValueDriverDelayOnTime 5
#define AirSensor_ValueDriverDelayOffTime 1

//*******************���������********************************************************************
//#define Test_pump_time 10000      //�������м�2�������ĳ���ʱ��


#if 0
//�������������ı����Ͳ���
//**************************************************
unsigned char  Valve_Number_Operate=1;

#define Airbag_To_Pump_PowerOn_Time  910
#define Airbag_To_Pump_PowerOff_Time 210
#define Airbag_To_Air_PowerOn_Time   430
#define Airbag_To_Air_PowerOff_Time  460
#endif

//***************************************************




/**************************************************************************/
/*                                                                        */
/**************************************************************************/
void AirPressure_check(void)
{
	    ADok3=0;
		  ADCstart3();
		  do
		   {
			  osDelay(1);
		   }while(ADok3==0);
	     Airsensor.AirSensor_Pressure=PressureADC_ConvertedValue[0];
}

void Print_Walve_noise_volume(void)
{
	int offset=0;
 	 	 
	   
		 
		 
		 offset += sprintf(SendData+offset,"%6d ",MIC_data_MAX[0]);
	   offset += sprintf(SendData+offset,"%6d ",MIC_data_Min[0]);
	   offset += sprintf(SendData+offset,"%6d ",MIC_data_MAX[1]);
	   offset += sprintf(SendData+offset,"%6d ",MIC_data_Min[1]);
	   
	   offset += sprintf(SendData+offset,"%6d ",Walve_poweron_time);
	   offset += sprintf(SendData+offset,"%6d ",Walve_poweroff_time);
	
		 
		 
		 offset += sprintf(SendData+offset,"|\n");
		 
   
	   uart_send(SendData, strlen(SendData));
	
}

void LOGO_system_error(void)
{
	int offset=0;
	
	offset += sprintf(SendData+offset,"\nSystem Power ON or Software Reset Error! Error_Code=%3d\n\n",Error_Code);	 
	uart_send(SendData, strlen(SendData));	
	
	
}


void LOGO_Reset_Output(void)
{
	 int offset=0;
  
	 offset += sprintf(SendData+offset,"System Power ON or Software Restart!\n");	 
	 uart_send(SendData, strlen(SendData));	
	
}


void Airsensor_SendlogtoPC(void)
{
	 uint16_t i;
	 int offset=0;
 	 if(Upgrading_On==0)
	 {		 
	   if(Airsensor.AirSensor_PersonOn==1)
			offset += sprintf(SendData+offset,"1");
		 else
			offset += sprintf(SendData+offset,"0");
		 
		 offset += sprintf(SendData+offset,"|");
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.Body_Moved_Times);
//		 offset += sprintf(SendData+offset,"# "); 
		 
		 offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_Avgvalue);
		 offset += sprintf(SendData+offset,"%4d",Airsensor.AirSensor_pressure_sample_avg);
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_Avgvalue1);
//		 offset += sprintf(SendData+offset,"# ");
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_Maxvalue);
//     offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_Minvalue);
		 
		 offset += sprintf(SendData+offset,"|");
//		 offset += sprintf(SendData+offset,"%d ",Airsensor.Max_or_Min_detected);
//		 offset += sprintf(SendData+offset,"| ");
//		 offset += sprintf(SendData+offset,"%d ",Airsensor.AirSensor_stable_min_detected_flag);
//		 offset += sprintf(SendData+offset,"| ");
		 offset += sprintf(SendData+offset,"%d",Airsensor.AirSensor_step_stable_flag);
		 offset += sprintf(SendData+offset,"| ");
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_pressure_stable_value_last);
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_pressure_stable_value);
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_Pressure_from_noperson_to_Personon);
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.Pressure_increase_several_times);
		 
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_pressure_stable_value_noperson_last);	
//     offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_pressure_stable_value_noperson_MAX);	
 //    offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_pressure_stable_value_noperson_MIN);	

		 offset += sprintf(SendData+offset,"%4d ",Airsensor.pressure_stable_value_diff_between_now_before);
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.Body_Pressure_Value);
		 offset += sprintf(SendData+offset,"%4d",Airsensor.AirSensor_stepnum_between_twostablestate);	 
		 
//		 offset += sprintf(SendData+offset,"# ");		 
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.down_stepnum);
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.down_max);
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.down_total);
//		 offset += sprintf(SendData+offset,"%8.3f ",Airsensor.person_onoff_factor2);		 

		 offset += sprintf(SendData+offset,"|");
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.down_MAX);
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.down_MIN);
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.last_diff_MaxMin);		 
		 
//		 offset += sprintf(SendData+offset,"| ");		 
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_pressure_sample);	
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_pressure_sample_avg);	
//		 offset += sprintf(SendData+offset,"| ");
//		 offset += sprintf(SendData+offset,"%2d ",Airsensor.onoff_result_reason);
//     offset += sprintf(SendData+offset,"| ");
//		 offset += sprintf(SendData+offset,"%2d ",Airsensor.AirSensor_PersonOn1);
//		 offset += sprintf(SendData+offset,"%2d ",Airsensor.AirSensor_PersonOn2);
//		 offset += sprintf(SendData+offset,"%2d ",Airsensor.AirSensor_PersonOn3);
//		 offset += sprintf(SendData+offset,"| ");	
		 offset += sprintf(SendData+offset,"%1d",Airsensor.personon_by_positioncheck_flag);
		 offset += sprintf(SendData+offset,"|");
//		 offset += sprintf(SendData+offset,"%4x ",gStorageData.Snore.HeadMovingTimes);
		 offset += sprintf(SendData+offset,"%3d",PVEvol8*2);     //real����
		 offset += sprintf(SendData+offset,"%2d",PillowControl.SnoreOn);
		 
//		 offset += sprintf(SendData+offset,"%2d ",Airsensor.AirSensor_BufferRefresh);
//		 offset += sprintf(SendData+offset,"%2d ",Airsensor.power_on_set_value_flag);
		 
		 
//		 offset += sprintf(SendData+offset,"|");
		 
//		 offset += sprintf(SendData+offset,"%4d ",PillowControl.SnoreOn);
//		 offset += sprintf(SendData+offset,"%2d ",SnoreFlag);
//		 offset += sprintf(SendData+offset,"%2d ",Tt_Is_Sleep_Time);
		 offset += sprintf(SendData+offset,"|");
		 
		 offset += sprintf(SendData+offset,"%1d",ActionSteps);		 
		 offset += sprintf(SendData+offset,"%2d",AppCtrlBag.enable);
		 
		 offset += sprintf(SendData+offset,"|");
		 
//		 offset += sprintf(SendData+offset,"%4d ",gStorageData.Standby);
     offset += sprintf(SendData+offset,"%4d",sdatestructure.Year);
		 offset += sprintf(SendData+offset,"/");
		 offset += sprintf(SendData+offset,"%2d",sdatestructure.Month);
		 offset += sprintf(SendData+offset,"/");
		 offset += sprintf(SendData+offset,"%2d",sdatestructure.Date);
		 offset += sprintf(SendData+offset," ");
     offset += sprintf(SendData+offset,"%2d",stimestructure.Hours); 
     offset += sprintf(SendData+offset,":"); 		 
     offset += sprintf(SendData+offset,"%2d",stimestructure.Minutes);
		 offset += sprintf(SendData+offset,":");
		 offset += sprintf(SendData+offset,"%2d",stimestructure.Seconds);
//		 offset += sprintf(SendData+offset,"%2d",app_settime_flag);
		 offset += sprintf(SendData+offset,"|");
		 
     offset += sprintf(SendData+offset,"%1d ",gStorageData.SensorLevel); 
     offset += sprintf(SendData+offset,"%1d",gStorageData.HeadpositionLevel);		 
//     offset += sprintf(SendData+offset,"%2d ",time_start_ok_flag);	
//		 offset += sprintf(SendData+offset,"%2d ",SensorLevelAutoSetData.times);		 
//		 offset += sprintf(SendData+offset,"%2d ",Config.level);
		 offset += sprintf(SendData+offset,"|");
//		 offset += sprintf(SendData+offset,"%2d ",Airsensor.personon_by_positioncheck_flag);
//		 offset += sprintf(SendData+offset,"%2d ",Airsensor.AirSensor_log_state);
     
		 if(gStorageData.SPIFlashAddress >= gStorageData.SPIFlashOutAddress)
		 {
       offset += sprintf(SendData+offset,"%5d",gStorageData.SPIFlashAddress-gStorageData.SPIFlashOutAddress);
		 }
     else
		 {
       offset += sprintf(SendData+offset,"%5d",gStorageData.SPIFlashAddress+SNORE_DATA_CNT-gStorageData.SPIFlashOutAddress);
		 }			 
		 offset += sprintf(SendData+offset,"|");
		 
#if 0		 
		 
		 offset += sprintf(SendData+offset,"%3d ",person_on_hour); 
		 offset += sprintf(SendData+offset,"%3d ",person_on_minute); 
		 offset += sprintf(SendData+offset,"%3d",person_on_second);
		 offset += sprintf(SendData+offset,"|");
		 offset += sprintf(SendData+offset,"%3d ",person_off_hour);
		 offset += sprintf(SendData+offset,"%3d ",person_off_minute); 
		 offset += sprintf(SendData+offset,"%3d",person_off_second);
		 
#else
		 offset += sprintf(SendData+offset,"%2d",study_hour_start);
		 offset += sprintf(SendData+offset,":");
		 offset += sprintf(SendData+offset,"%2d ",study_min_start);
		 offset += sprintf(SendData+offset,"%2d",study_hour_end);
		 offset += sprintf(SendData+offset,":");
		 offset += sprintf(SendData+offset,"%2d ",study_min_end);
		 
		 offset += sprintf(SendData+offset,"%2d ",Airsensor.Sleep_snore_study_flag);
		 
#endif		 
		 
		 
		 
		 offset += sprintf(SendData+offset,"|");
#if 0		 
		 offset += sprintf(SendData+offset,"%2d ",Airsensor.AirSensor_Checkposition_Mode);
		 offset += sprintf(SendData+offset,"%2d ",Airsensor.AirSensor_Check_Mode);
		 offset += sprintf(SendData+offset,"%2d ",RequestControlValve);
#endif	
     //����ʶ���⵽������������������Ȩlog��ʾ		 
//		 offset += sprintf(SendData+offset,"%2d ",log_newfcc_flag);
//		 offset += sprintf(SendData+offset,"%2d ",log_weight_add1_flag);
		 log_newfcc_flag=0;log_weight_add1_flag=0;
		 //ʵʱ������ʾ�������е�������������Ч��������
		 int i;
		 uint8_t vectors_num_temp,valid_vcetors_num_temp;
		 vectors_num_temp=0;
	   valid_vcetors_num_temp=0;
	   	
	   for(i=0;i<MELBuffer+1;i++)
     {
       if(Config.MfccMat[i][MELFILTER]!=0)
         vectors_num_temp++;
     		
       if(Config.MfccMat[i][MELFILTER]>=dynamic_weight)
		   {
			   valid_vcetors_num_temp++;
      			
		   }			
     }
		 offset += sprintf(SendData+offset,"%2d ",vectors_num_temp);
		 offset += sprintf(SendData+offset,"%2d",valid_vcetors_num_temp);
		 offset += sprintf(SendData+offset,"|");
		 offset += sprintf(SendData+offset,"%5d",snore_times_every_sleep);
		 offset += sprintf(SendData+offset,"|");
		 offset += sprintf(SendData+offset,"%1d ",Config.autostudy);
		 offset += sprintf(SendData+offset,"%5d ",max_vectorweight_inLIB);
		 offset += sprintf(SendData+offset,"%4d ",dynamic_weight);
		 offset += sprintf(SendData+offset,"%3d",Snore_MaxDB_Detected);
//		 offset += sprintf(SendData+offset,"%3d ",Snore_detected_type_flag);
//		 offset += sprintf(SendData+offset,"%3d ",Config.HaveCheckSnore);
//		 offset += sprintf(SendData+offset,"%3d ",gStorageData.Snore.SnoreAvilable);
		 offset += sprintf(SendData+offset,"|");
		 
		 /*
		 offset += sprintf(SendData+offset,"%3d ",gStorageData.Snore.SSnoreTimes);
		 offset += sprintf(SendData+offset,"%3d ",gStorageData.Snore.MSnoreTimes);
		 offset += sprintf(SendData+offset,"%3d",gStorageData.Snore.LSnoreTimes);
		 offset += sprintf(SendData+offset,"|");
		 offset += sprintf(SendData+offset,"%1d",gStorageData.StudyMode);
		 offset += sprintf(SendData+offset,"|");
		 offset += sprintf(SendData+offset,"%3d ",gStorageData.Snore.Airbag1Times);
		 offset += sprintf(SendData+offset,"%3d ",gStorageData.Snore.Airbag2Times);
		 offset += sprintf(SendData+offset,"%3d ",gStorageData.Snore.Airbag3Times);
		 offset += sprintf(SendData+offset,"%3d",gStorageData.Snore.Airbag4Times);
		 offset += sprintf(SendData+offset,"|");
		 
		 offset += sprintf(SendData+offset,"%4d ",snore_times_for_antisnore);
		 offset += sprintf(SendData+offset,"%4d ",snore_times_for_snore);
     offset += sprintf(SendData+offset,"%3d ",gStorageData.PositionValue_Onebag);	
     offset += sprintf(SendData+offset,"%3d ",gStorageData.PositionValue_Twobag);
		 offset += sprintf(SendData+offset,"%3d ",gStorageData.Body_detect_on_threshold);
     offset += sprintf(SendData+offset,"%3d ",gStorageData.Body_detect_off_threshold);
		 offset += sprintf(SendData+offset,"%2d ",gStorageData.MinSnoredB);
		 */
		 if(gStorageData.Standby==1)
			 offset += sprintf(SendData+offset,"0 ");
		 else if(gStorageData.Standby==0 && Airsensor.Tt_Is_Sleep_Time==0)
			 offset += sprintf(SendData+offset,"1 ");
		 else if(gStorageData.Standby==0 && Airsensor.Tt_Is_Sleep_Time==1)
			 offset += sprintf(SendData+offset,"2 ");
		 offset += sprintf(SendData+offset,"%1d ",Airsensor.period_snore_checkposition_counter);
		 
		 
//		 offset += sprintf(SendData+offset,"%3d ",checkposition_count_timer/50);
//		 offset += sprintf(SendData+offset,"%4d ",Airsensor.onehour_snore_checkposition_counter/50);
		 
		 /*
		 offset += sprintf(SendData+offset,"%2d",gStorageData.UserSleepTime[0]);
		 offset += sprintf(SendData+offset,":");
		 offset += sprintf(SendData+offset,"%2d ",gStorageData.UserSleepTime[1]);
		 offset += sprintf(SendData+offset,"%2d",gStorageData.UserSleepTime[2]);
		 offset += sprintf(SendData+offset,":");
		 offset += sprintf(SendData+offset,"%2d ",gStorageData.UserSleepTime[3]);
		 */
		 offset += sprintf(SendData+offset,"V");
		 offset += sprintf(SendData+offset,"%1d",BOARD_VERSION);
		 offset += sprintf(SendData+offset,".");
		 offset += sprintf(SendData+offset,"%1d",MASTER_VERSION);
		 offset += sprintf(SendData+offset,"."); 
		 offset += sprintf(SendData+offset,"%2d",SOFTWARE_VERSION);
		 offset += sprintf(SendData+offset,"\n");
	 }
//for debug

     //*********************************************
		 
		 if(BT_Recieved_Data_From_App_Buffer_Pointer!=0)
		 {
			 offset += sprintf(SendData+offset,"\n");
			 for(i=0;i<BT_Recieved_Data_From_App_Buffer_Pointer;i++)
			 {
				 if(offset>3900)
				 {
					 break;
				 }
				 if(i>=1)
				 {
					 if(BT_Recieved_Data_From_App_Buffer[i]==2 && BT_Recieved_Data_From_App_Buffer[i-1]==3)
					 {
             
						 offset += sprintf(SendData+offset,"  (R)\n");
						
					 }
				 }
				 
				 offset += sprintf(SendData+offset,"%3x",BT_Recieved_Data_From_App_Buffer[i]);
				 
			 }
			 offset += sprintf(SendData+offset,"  (R)\n\n");
			 BT_Recieved_Data_From_App_Buffer_Pointer=0;
		 }
		 
		 if(BT_Sent_Data_To_App_Buffer_Pointer!=0)
		 {
			 offset += sprintf(SendData+offset,"\n");
			 for(i=0;i<BT_Sent_Data_To_App_Buffer_Pointer;i++)
			 {
				 if(offset>3900)
				 {
					 break;
				 }
				 if(i>=1)
				 {
					 if(BT_Sent_Data_To_App_Buffer[i]==2 && BT_Sent_Data_To_App_Buffer[i-1]==3)
					 {

						 offset += sprintf(SendData+offset,"  (S)\n");
						
					 }
				 }
				 
				 offset += sprintf(SendData+offset,"%3x",BT_Sent_Data_To_App_Buffer[i]);
				 
			 }
			 offset += sprintf(SendData+offset,"  (S)\n\n");
			 BT_Sent_Data_To_App_Buffer_Pointer=0;
		 }
		 
		 //*********************************************

     if(offset!=0)
		 {
		 
	     uart_send(SendData, offset);//strlen(SendData));
		 }
		 
		 app_settime_flag=0;//ֻ����һ�������־
	 
}

//uint16_t tempdbebug;

void Airsensor_SendlogtoPC_CheckPosition(void)
{
//	 uint8_t i;
	 int offset=0;
 	 if(Upgrading_On==0)
	 {
	 
		    offset += sprintf(SendData+offset,"%2d ",PillowMaxOne_Willbesend);
		    offset += sprintf(SendData+offset,"%2d ",Pillow2ndMaxOne_Willbesend);
	
		    offset += sprintf(SendData+offset,"%d ",secondtest);
		    offset += sprintf(SendData+offset,"# ");
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValueAvg[0]);
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValueAvg[1]);
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValueAvg[2]);
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValueAvg[3]);
		    offset += sprintf(SendData+offset,"# ");
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff[0]);
	      offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff[1]);
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff[2]);
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff[3]);
	      offset += sprintf(SendData+offset,"| ");
/*	
	      offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff_last_noperson[0]);
	      offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff_last_noperson[1]);
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff_last_noperson[2]);
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff_last_noperson[3]);
	      offset += sprintf(SendData+offset,"| ");
				
				offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff_filtered_new[0]);
	      offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff_filtered_new[1]);
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff_filtered_new[2]);
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff_filtered_new[3]);
	      offset += sprintf(SendData+offset,"| ");
*/	
	      offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff_filtered[0]);
	      offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff_filtered[1]);
	      offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff_filtered[2]);
				offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_PressureValue_Diff_filtered[3]);	
	      offset += sprintf(SendData+offset,"| ");
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AvgDiff);
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AvgDiff1);
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AvgDiff_Total);
		    offset += sprintf(SendData+offset,"# ");	
				
		    offset += sprintf(SendData+offset,"%4d ",Airsensor.AirSensor_fast_check_mode);

        offset += sprintf(SendData+offset,"%4d ",PVEvol8);     //real����

        offset += sprintf(SendData+offset,"%2d ",Airsensor.personon_by_positioncheck_flag);
				
				offset += sprintf(SendData+offset,"$*#\n");		 
	      
//				tempdbebug=strlen(SendData);
	      uart_send(SendData, strlen(SendData)); //�ܹ�106��ASCII�ֽ�
			}
}


/**************************************************************************/
/*                                                                        */
/**************************************************************************/
void SendDataToPCorSTM32() 
{
      uint8_t tt;
//    if (AirSensor_position_check_finish_flag == 1)
//    {
 //       AirSensor_position_check_finish_flag = 0;


 
        
 

        LastPosition.head1 = PillowMaxOne_Willbesend;
        LastPosition.head2 = Pillow2ndMaxOne_Willbesend;
//        PillowMaxOne_Lastsend = PillowMaxOne_Willbesend;
//			  Pillow2ndMaxOne_Lastsend=Pillow2ndMaxOne_Willbesend;
			
        position = LastPosition;
#if 0			
        if (PillowControl.PillowAction == false)
        {
            if ((HeadPosition.head1 == HEAD_NO) && (position.head1 != HEAD_NO))
            {
             //   SnoreInit();
            }
            HeadPosition = position;
        }
	
				else
				{
					tt=0;
				}
#endif
        HeadPosition = position;			
        //-----------------------------------------
//				Airsensor_SendlogtoPC();
//    }
}






void AirSensor_Set_AirGate(uint8_t airgate_number)
{
     if(airgate_number==0)
		 {
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
		 }
		 else if(airgate_number==1)
		 {
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
		 }
		 else if(airgate_number==2)
		 {
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
		 }
		 else
		 {
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
		 }
}

void AirSensor_Reset_AirGate(uint8_t airgate_number)
{
     if(airgate_number==0)
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
		 else if(airgate_number==1)
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
		 else if(airgate_number==2)
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
		 else
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
}


void us_delay(uint16_t time_us)
{
	uint16_t i,j;
	while(time_us--)
  {
      i=50;  //????
      while(i--) ;    
  }	
}
/*
void AirSensor_Set_AirGate(uint8_t airgate_number)
{
     if(airgate_number==0)
		 {
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
			  osDelay(AirSensor_ValueDriverDelayOnTime);
			  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
			  us_delay(100);
			  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
//			  osDelay(AirSensor_ValueDriverDelayOnTime);
//			  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
//			  osDelay(AirSensor_ValueDriverDelayOffTime);
//			  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
		 }
		 else if(airgate_number==1)
		 {
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
			  osDelay(AirSensor_ValueDriverDelayOnTime);
			  ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
			  us_delay(100);
			  ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
//			  osDelay(AirSensor_ValueDriverDelayOnTime);
//			  ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
//			  osDelay(AirSensor_ValueDriverDelayOffTime); 
//			  ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
		 }
		 else if(airgate_number==2)
		 {
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
			  osDelay(AirSensor_ValueDriverDelayOnTime);
			  ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
			  us_delay(100);
			  ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
//			  osDelay(AirSensor_ValueDriverDelayOnTime);
//			  ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
//			  osDelay(AirSensor_ValueDriverDelayOffTime); 
//			  ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
		 }
		 else
		 {
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
			  osDelay(AirSensor_ValueDriverDelayOnTime);
			  ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
			  us_delay(100);
			  ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
//			  osDelay(AirSensor_ValueDriverDelayOnTime);
//			  ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
//			  osDelay(AirSensor_ValueDriverDelayOffTime); 
//			  ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
		 }
}

void AirSensor_Reset_AirGate(uint8_t airgate_number)
{
     if(airgate_number==0)
		 {
				
			  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
			  osDelay(AirSensor_ValueDriverDelayOnTime);
			  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
			  us_delay(100);
			  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
//			  osDelay(AirSensor_ValueDriverDelayOnTime); 
//			  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
//			  osDelay(AirSensor_ValueDriverDelayOffTime);
//			  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
		 }
		 else if(airgate_number==1)
		 {
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
			  osDelay(AirSensor_ValueDriverDelayOnTime);
			  ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
			  us_delay(100);
			  ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
//			  osDelay(AirSensor_ValueDriverDelayOnTime);
//			  ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
//			  osDelay(AirSensor_ValueDriverDelayOffTime);
//			  ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
		 }
		 else if(airgate_number==2)
		 {
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
			  osDelay(AirSensor_ValueDriverDelayOnTime);
			  ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
			  us_delay(100);
			  ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
//			  osDelay(AirSensor_ValueDriverDelayOnTime);
//			  ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
//			  osDelay(AirSensor_ValueDriverDelayOffTime);
//			  ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
		 }
		 else
		 {
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
			  osDelay(AirSensor_ValueDriverDelayOnTime);
			  ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
			  us_delay(100);
			  ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
//			  osDelay(AirSensor_ValueDriverDelayOnTime); 
//			  ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
//			  osDelay(AirSensor_ValueDriverDelayOffTime);
//			  ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
		 }
}
*/
/**************************************************************************/
/*                                                                        */
/**************************************************************************/

void AirSensor_check_position_init(void)
{
	  AirSensor_Reset_AirGate(3);	
    osDelay(300);	
	  AirSensor_Reset_AirGate(2);
	  osDelay(300);
	  AirSensor_Reset_AirGate(1);
	  osDelay(300);
    AirSensor_Reset_AirGate(0);
	  osDelay(300);
	  
	  
	 
//	  osDelay(500);
}

void AirSensor_fast_check_position(void)
{
	int8_t i,j,stop;
	uint16_t airsensor_pressure[7];
	uint16_t max,min;
	uint8_t max_pos,min_pos,m,n;
	MIC_data_stop_detect_flag=1;
	osDelay(200);
  AirSensor_check_position_init();
	Airsensor.AirSensor_fast_check_mode=Airsensor.AirSensor_Checkposition_Mode;
	if(ActionSteps == 0x00)
	 {	
    stop=0;
		for(i=3;i>=0;i--)
     {
      if((Airsensor.AirSensor_Checkposition_Mode==2||Airsensor.AirSensor_Checkposition_Mode==3)&&RequestControlValve==0)
			 {				
			  AirSensor_Set_AirGate(i);
			  osDelay(300);//50
			  InhalePump(1);				
    	  osDelay(AirSensor_checkposition_pumpon_time_everyone[i]);
    	  InhalePump(0);
				osDelay(200);//������Ҫ200ms
			  for(j=0;j<7;j++)
			  {
				  osDelay(5);
//			    ADok3=0;
//		      ADCstart3();
					ADok2=0;
					ADCstart2();
//          ADok1=0;
//					ADCstart1();
		      do
		       {
			      osDelay(2);
//		       }while(ADok3==0);
          }while(ADok2==0);	
//           }while(ADok1==0);
//           airsensor_pressure[j]=ExhaleADC_ConvertedValue[0]; //ADok1			 
					 airsensor_pressure[j]=InhaleADC_ConvertedValue[0]; //ADOK2
					 
//				 airsensor_pressure[j]=PressureADC_ConvertedValue[0];//ADok3

					 
					 
					 //	        Airsensor.AirSensor_PressureValue[3-i][j]=PressureADC_ConvertedValue[0];
			  }
//				osDelay(300);
//				InhalePump(0);
//				osDelay(300);
			  AirSensor_Reset_AirGate(i);
				max=0;
				min=0xffff;
				max_pos=0;
				min_pos=0;
				for(m=0;m<7;m++)
				{
					if(airsensor_pressure[m]>=max)
					{
						max=airsensor_pressure[m];
						max_pos=m;
					}
					if(airsensor_pressure[m]<min)
					{
						min=airsensor_pressure[m];
						min_pos=m;
					}					
				}
				n=0;
				for(m=0;m<7;m++)
				{
					if(m!=max_pos && m!=min_pos)
					{
						Airsensor.AirSensor_PressureValue[3-i][n]=airsensor_pressure[m];
						n++;
					}
					
				} 
				 
        osDelay(300);//50
				
       }
      else
			 {
        stop=1;
        break;				 
			 }				 
		 }
//		 osDelay(200);//50
		 if(stop == 0)
		  { 
				//ȥ�����ֵ����Сֵ
				
				
				
			  AirSensor_position_process(&Airsensor);
			  SendDataToPCorSTM32();
			  Airsensor_SendlogtoPC_CheckPosition();
			  osDelay(200);//��һ����ʾλ�ã����ӳ�500ms������
			  if(Airsensor.AirSensor_Checkposition_Mode==3)
 			  {	
					Airsensor.personon_by_positioncheck_flag_forpensononoff=1;
					Airsensor.AirSensor_position_checked_by_fastmode_flag=0;//Ϊtest mode
          if(PillowMaxOne_Willbesend!=0x10)
					{						
 			      Airsensor.personon_by_positioncheck_flag=0;
						Airsensor.period_snore_checkposition_counter=0;
						Airsensor.period_snore_checkposition_flag=0;
 			      Airsensor.AirSensor_PersonOn=1;
					}
					else
					{
						Airsensor.period_snore_checkposition_counter++;
						if(Airsensor.period_snore_checkposition_counter >= Checkposition_times_whennoperson)
						{
							Airsensor.period_snore_checkposition_flag=1;
							Airsensor.period_snore_checkposition_counter=0;
							
						}
						
						
//						Airsensor.personon_by_positioncheck_flag=1;//��ʱ����
						Airsensor.personon_by_positioncheck_flag=0;//��ʱ����
 			      Airsensor.AirSensor_PersonOn=0;
//						Airsensor.AirSensor_recheck_times=2;
//						PillowControl.SnoreOn = false; //������ֻ���һ�ξ�ֹͣ�����������������
						
					}
		  	}
				else
				{
					Airsensor.AirSensor_position_checked_by_fastmode_flag=1;
//					Airsensor.AirSensor_PersonOn=0;
					if(Airsensor.AirSensor_vale_exceed_upvalue_istrue==0)
					{
					  Airsensor.Airsensor_checkposition_valueavg_last[0]=Airsensor.AirSensor_PressureValueAvg[0];
					  Airsensor.Airsensor_checkposition_valueavg_last[1]=Airsensor.AirSensor_PressureValueAvg[1];
					  Airsensor.Airsensor_checkposition_valueavg_last[2]=Airsensor.AirSensor_PressureValueAvg[2];
					  Airsensor.Airsensor_checkposition_valueavg_last[3]=Airsensor.AirSensor_PressureValueAvg[3];			
					
					  Airsensor.AirSensor_PressureValue_Diff_filtered_last[0]=Airsensor.AirSensor_PressureValue_Diff_filtered[0];
            Airsensor.AirSensor_PressureValue_Diff_filtered_last[1]=Airsensor.AirSensor_PressureValue_Diff_filtered[1];
					  Airsensor.AirSensor_PressureValue_Diff_filtered_last[2]=Airsensor.AirSensor_PressureValue_Diff_filtered[2];
					  Airsensor.AirSensor_PressureValue_Diff_filtered_last[3]=Airsensor.AirSensor_PressureValue_Diff_filtered[3];
						Airsensor.AirSensor_vale_exceed_upvalue_flag=0;
					}
					else
						Airsensor.AirSensor_vale_exceed_upvalue_flag=1;
					
//					Airsensor.AirSensor_PersonOn=0;
				}
		  }
		 else
		  {
			 if(RequestControlValve==1)
			  {
			   Airsensor_readvalue_50ms=0;
         RequestControlValve=2;
         Airsensor.AirSensor_Checkposition_Mode=5;
        }				 
			}
	 } 
	 MIC_data_stop_detect_flag=0;
}		 

#if 0
void AirSensor_fast_check_position(void)
{
	int8_t i,j,stop;
	
  AirSensor_check_position_init();
	Airsensor.AirSensor_fast_check_mode=Airsensor.AirSensor_Checkposition_Mode;
	if(ActionSteps == 0x00)
	 {	
    stop=0;
		for(i=3;i>=0;i--)
     {
      if((Airsensor.AirSensor_Checkposition_Mode==2||Airsensor.AirSensor_Checkposition_Mode==3)&&RequestControlValve==0)
			 {				
			  AirSensor_Set_AirGate(i);
			  osDelay(300);//50
			  InhalePump(1);				
    	  osDelay(AirSensor_checkposition_pumpon_time_everyone[i]);
    	  InhalePump(0);
				osDelay(200);//������Ҫ200ms
			  for(j=0;j<5;j++)
			   {
				  osDelay(5);
			    ADok3=0;
		      ADCstart3();
		      do
		       {
			      osDelay(1);
		       }while(ADok3==0);
	        Airsensor.AirSensor_PressureValue[3-i][j]=PressureADC_ConvertedValue[0];
			   }
//				osDelay(300);
//				InhalePump(0);
//				osDelay(300);
			  AirSensor_Reset_AirGate(i);
        osDelay(200);//50
				
       }
      else
			 {
        stop=1;
        break;				 
			 }				 
		 }
		 if(stop == 0)
		  { 
			  AirSensor_position_process();
			  SendDataToPCorSTM32();
			  Airsensor_SendlogtoPC_CheckPosition();
			  osDelay(200);//��һ����ʾλ�ã����ӳ�500ms������
			  if(Airsensor.AirSensor_Checkposition_Mode==3)
 			  {	
					AirSensor_position_checked_by_fastmode_flag=0;//Ϊtest mode
          if(PillowMaxOne_Willbesend!=0x10)
					{						
 			      Airsensor.personon_by_positioncheck_flag=0; //�¸Ĳ��ö�ʱ���죬��������ʱ����
 			      Airsensor.AirSensor_PersonOn=1;
					}
					else
					{
						Airsensor.personon_by_positioncheck_flag=1;
 			      Airsensor.AirSensor_PersonOn=0;
						
					}
		  	}
				else
				{
					AirSensor_position_checked_by_fastmode_flag=1;
				}
		  }
		 else
		  {
			 if(RequestControlValve==1)
			  {
			   Airsensor_readvalue_50ms=0;
         RequestControlValve=2;
         Airsensor.AirSensor_Checkposition_Mode=0;
        }				 
			}
	 } 
}		 
#endif

void AirSensor_slow_check_position(void)
{
	uint8_t i,j,stop;
	uint16_t k;
	if((ActionSteps == 0x00)&&(RequestControlValve==0))
	 {	
//	  ADok2=0;
//	  ADCstart2();
//	  Airsensor.AirSensor_PressureValue[AirSensor_readpressuredata_counter]=InhaleADC_ConvertedValue[0];
		AirSensor_Position_Checking=1;
		stop=0;
		for(i=0;i<4;i++)
     {		 

			AirSensor_Set_AirGate(i);
			//delay 1000ms
			for(k=0;k<200;k++)
			 {
			  osDelay(5);
				if(Airsensor.AirSensor_Checkposition_Mode!=1||RequestControlValve==1)
				 {	
				  stop=1;
					AirSensor_Reset_AirGate(i);
					break;
				 }
			 }
			if(stop==0)
			 {
				//pump on
			  InhalePump(1);
				//delay AirSensor_checkposition_pumpon_time ms 
				for(k=0;k < AirSensor_checkposition_pumpon_time_everyone[i];k++)
			   {
			    osDelay(1);
				  if(Airsensor.AirSensor_Checkposition_Mode!=1||RequestControlValve==1)
				   {	
				    stop=1;
						InhalePump(0);
						AirSensor_Reset_AirGate(i);
					  break;
				   }
			   }
				if(stop==0)
				 {	
//    	    osDelay(AirSensor_checkposition_pumpon_time);
    	    InhalePump(0);
					osDelay(250);
			    for(j=0;j<5;j++)
			     {
				    osDelay(10);
			      ADok3=0;
		        ADCstart3();
		        do
		         {
			        osDelay(1);
		         }while(ADok3==0);
	          Airsensor.AirSensor_PressureValue[3-i][j]=PressureADC_ConvertedValue[0];
			     }
					//delay 1000ms
				  for(k=0;k<200;k++)
			     {
			      osDelay(5);
				    if(Airsensor.AirSensor_Checkposition_Mode!=1||RequestControlValve==1)
				     {	
				      stop=1;
						  AirSensor_Reset_AirGate(i);
					    break;
				     }
			     }
 //       osDelay(1000);
				  if(stop==0)
				   {

					  AirSensor_Reset_AirGate(i); 
						//delay 1000ms
					  for(k=0;k<200;k++)
			       {
			        osDelay(5);
				      if(Airsensor.AirSensor_Checkposition_Mode!=1||RequestControlValve==1)
				       {	
				        stop=1;
					      break;
				       }
			       }
						if(stop!=0)
						 break;
					 }
					else
					 break;
 				 }
        else
         break;					
		   }	
      else
       break;				
	   }
		if(stop==0)
		 {			 
		   AirSensor_position_process(&Airsensor);
       SendDataToPCorSTM32();
			 Airsensor_SendlogtoPC();
		 }
    else
		 {
			if(RequestControlValve==1)
			 {
				Airsensor_readvalue_50ms=0;
        RequestControlValve=2;
        Airsensor.AirSensor_Checkposition_Mode=1;				 
			 }
		 }			 
	}
  AirSensor_Position_Checking=0;	 
}		 



void AirSensor_pump_bag1_bag2(void)
{
	  //��ͨ����1��2���ó�����0��3�Ͽ����ӵ������н��з���
	  AirSensor_Reset_AirGate(0);
	  AirSensor_Reset_AirGate(3);
	  AirSensor_Reset_AirGate(1);
	  AirSensor_Reset_AirGate(2);
	  osDelay(5000);//50
	  AirSensor_Set_AirGate(1);
	  AirSensor_Set_AirGate(2);
	  AirSensor_Set_AirGate(0);
	  AirSensor_Set_AirGate(3);
	
	
	  osDelay(1000);//50
//		InhalePump(1);				
//    osDelay(Test_pump_time);
//    InhalePump(0);
//	  osDelay(1000);//50
}









void AirSensor_person_detect_init(void)
{
	  uint16_t current_airsensor_pressure;
	  
	  //������ģʽ������ʼ��
//	  AirSensor_Reset_AirGate(0);
//	  AirSensor_Reset_AirGate(1);
//	  AirSensor_Reset_AirGate(2);
//	  AirSensor_Reset_AirGate(3);
	  
//	  osDelay(5000);//50
		MIC_data_stop_detect_flag = 1;
		osDelay(200);
#if 1	  
	  AirSensor_Set_AirGate(0);	
    osDelay(200);//50  
	  AirSensor_Set_AirGate(1);
	  osDelay(200);//50
	  AirSensor_Set_AirGate(2);	
    osDelay(200);//50  
	  AirSensor_Set_AirGate(3);	
	  //�ӳ�10�룬ͬʱ����LED������
	  osDelay(200);//50
		
#endif
		MIC_data_stop_detect_flag = 0;
	  ADok3=0;
		ADCstart3();
		do
		{
			osDelay(1);
		}while(ADok3==0);
		current_airsensor_pressure=PressureADC_ConvertedValue[0];
	  Airsensor.AirSensor_Maxvalue=current_airsensor_pressure;
		Airsensor.AirSensor_Minvalue=current_airsensor_pressure;
		Airsensor.AirSensor_pressure_sample_avg=current_airsensor_pressure;
		Airsensor.Max_or_Min_detected=0;
		Airsensor.AirSensor_stable_min_detected_flag=0;
		Airsensor.AirSensor_PressureBuffer_WR_pointer=0;
    Airsensor.AirSensor_PressureTest_WR_pointer=0;
		Airsensor.AirSensor_Avgvalue_Buffer_Pointer=0;
//		AvgMaxMin_pointer=0;
		Airsensor.AirSensor_BufferRefresh=0;
		Airsensor.power_on_set_value_flag=0;
		Airsensor.AirSensor_Maxvalue=0; //�����ѹֵ    
    Airsensor.AirSensor_Minvalue=0xfff;
		Airsensor.pressure_stable_value_last_is_valid=0;
		Airsensor.pressure_stable_value_valid_counter=0;
		Airsensor.AirSensor_stepnum_between_twostablestate=0;
		Airsensor.AirSensor_stable_noresult_flag=0;
//		Airsensor.AirSensor_position_checked_by_fastmode_flag=0;
		position.head1=HEAD_NO;
		
		
}
void AirSensor_fast_check_person(void)
{
	  int8_t i,j;	
//	  osDelay(1000);
	  AirSensor_Reset_AirGate(3);
	  osDelay(30);
	  AirSensor_Reset_AirGate(2);
	   osDelay(30);
	  AirSensor_Reset_AirGate(1);
	  osDelay(30);
    AirSensor_Reset_AirGate(0);
	  
	  
	  
	 
	  
	  //��ʱ10�룬ͬʱ�����ʾ���ڴ����LED����
	  for(i=0;i<10;i++)
	  {
			if(Led_OnOff_Timer>10*1000)
		  {
				if(Airsensor.Led_Person_OnOff!=0)
				{
			    Airsensor.Led_Person_OnOff=0;
			    WrokingLed(RED_LED,0);
				}
			
		  }
	    osDelay(1000);
		}
		for(i=3;i>=0;i--)
    {
      				
			  AirSensor_Set_AirGate(i);
			  osDelay(50);//50
			  InhalePump(1);				
    	  osDelay(AirSensor_checkposition_pumpon_time_everyone[i]);
    	  InhalePump(0);
				osDelay(200);//50
			
			  for(j=0;j<5;j++)
			   {
				  osDelay(5);
			    ADok3=0;
		      ADCstart3();
		      do
		       {
			      osDelay(1);
		       }while(ADok3==0);
	        Airsensor.AirSensor_PressureValue[3-i][j]=PressureADC_ConvertedValue[0];
			   }
//				osDelay(300);
//				InhalePump(0);
//				osDelay(300);
			  AirSensor_Reset_AirGate(i);
        osDelay(200);//50       	 
		 }
		 
		 AirSensor_position_process(&Airsensor);
		 Airsensor_SendlogtoPC_CheckPosition();
     AirSensor_person_detect_init();
		 

}		 




void AirSensor_fast_check_person_poweron(void)
{
	  int8_t i,j;	
	  uint16_t airsensor_pressure[7];
	  uint16_t max,min;
	  uint8_t max_pos,min_pos,m,n;
	  MIC_data_stop_detect_flag=1;
	  osDelay(200);
	  AirSensor_Reset_AirGate(3);	
	  osDelay(300);
	  AirSensor_Reset_AirGate(2);
    osDelay(300);
	  AirSensor_Reset_AirGate(1);
    osDelay(300);
    AirSensor_Reset_AirGate(0);
	  osDelay(300);
	  
	  
	  
	
	  //��ʱ10�룬ͬʱ�����ʾ���ڴ����LED����
	  
//	  osDelay(200);
		Airsensor.AirSensor_fast_check_mode=1;//for log
		for(i=3;i>=0;i--)
    {
      				
			  AirSensor_Set_AirGate(i);
			  osDelay(300);//50
			  InhalePump(1);				
    	  osDelay(AirSensor_checkposition_pumpon_time_everyone[i]);
    	  InhalePump(0);
				osDelay(200);//50
			
/*			
			  for(j=0;j<5;j++)
			   {
				  osDelay(5);
			    ADok3=0;
		      ADCstart3();
		      do
		       {
			      osDelay(1);
		       }while(ADok3==0);
	        Airsensor.AirSensor_PressureValue[3-i][j]=PressureADC_ConvertedValue[0];
			   }
//				osDelay(300);
//				InhalePump(0);
//				osDelay(300);
			  AirSensor_Reset_AirGate(i);
        osDelay(200);//50 

*/
        
      	for(j=0;j<7;j++)
			  {
				  osDelay(5);
//			    ADok3=0;
//		      ADCstart3();
          ADok2=0;
		      ADCstart2();
//          ADok1=0;
//		      ADCstart1();         					
					
		      do
		       {
			      osDelay(2);
//           }while(ADok1==0);						
//						 }while(ADok3==0);
					 }while(ADok2==0);
//					 airsensor_pressure[j]=ExhaleADC_ConvertedValue[0]; //��һ·ADCר��λ�ü�� ADok1
						 airsensor_pressure[j]=InhaleADC_ConvertedValue[0]; //��һ·ADCר��λ�ü��ADok2
//					 airsensor_pressure[j]=PressureADC_ConvertedValue[0];//ADok3

					 
					 
					 //	        Airsensor.AirSensor_PressureValue[3-i][j]=PressureADC_ConvertedValue[0];
			  }
//				osDelay(300);
//				InhalePump(0);
//				osDelay(300);
			  AirSensor_Reset_AirGate(i);
				max=0;
				min=0xffff;
				max_pos=0;
				min_pos=0;
				for(m=0;m<7;m++)
				{
					if(airsensor_pressure[m]>=max)
					{
						max=airsensor_pressure[m];
						max_pos=m;
					}
					if(airsensor_pressure[m]<min)
					{
						min=airsensor_pressure[m];
						min_pos=m;
					}					
				}
				n=0;
				for(m=0;m<7;m++)
				{
					if(m!=max_pos && m!=min_pos)
					{
						Airsensor.AirSensor_PressureValue[3-i][n]=airsensor_pressure[m];
						n++;
					}
					
				} 
				
				osDelay(200);//50 
		 }
		 
		 AirSensor_position_process(&Airsensor);
		 SendDataToPCorSTM32();
		 Airsensor_SendlogtoPC_CheckPosition();
		 
		 Airsensor.AirSensor_fast_check_position_finished_flag=1;
		 Airsensor.AirSensor_position_checked_by_fastmode_flag=0;//Ϊtest mode
		 Airsensor.personon_by_positioncheck_flag_forpensononoff=1;
		 if(PillowMaxOne_Willbesend!=0x10)
		 {
//			 WrokingLed(BLUE_LED_HIGH,2);//������
//			 Led_OnOff_Timer=0;
//			 Airsensor.Led_Person_OnOff=1;
       Airsensor.AirSensor_PersonOn=1;
			 Airsensor.period_snore_checkposition_counter=0;
			 if(RECHECK_NO_DELAY_FLAG==1)
			 {
//				 if(Airsensor.personon_by_positioncheck_flag==1)
//			   { 
//			     Airsensor.personon_by_positioncheck_flag=0;//����м�⣬�����ʱ�����־
//					 
//			   }
			 }
			 
			 else if(Airsensor.personon_by_positioncheck_flag==1 && Position_State_Before_Recheck==1)
			 { 
			   Airsensor.personon_by_positioncheck_flag=0;//����м�⣬�����ʱ�����־
			 }
//			 else if(Airsensor.personon_by_positioncheck_flag==1 && Position_State_Before_Recheck==0)
//			 {
//				 Position_State_Before_Recheck=1;
//			 }
       			 
		 }
		 else
		 {
//			 WrokingLed(RED_LED,1);//�����
//			 Led_OnOff_Timer=0;
//			 Airsensor.Led_Person_OnOff=2;
       Airsensor.AirSensor_PersonOn=0;
			 
		#if 0	 
			 Airsensor.period_snore_checkposition_counter++;
			 if(Airsensor.period_snore_checkposition_counter >= Checkposition_times_whennoperson)
			 {
					Airsensor.period_snore_checkposition_flag=1;
					Airsensor.period_snore_checkposition_counter=0;
							
			 }
		#endif
			 
			 if(RECHECK_NO_DELAY_FLAG==1)
			 {
//				 if(Airsensor.personon_by_positioncheck_flag==1)
//			   { 
//			     Airsensor.personon_by_positioncheck_flag=0;//����м�⣬�����ʱ�����־					 
//			   }
			 }
			 else if(Airsensor.personon_by_positioncheck_flag==1 && Position_State_Before_Recheck==0)
			 { 
				 Airsensor.personon_by_positioncheck_flag=0;//����м�⣬�����ʱ�����־
			 }
//			 else if(Airsensor.personon_by_positioncheck_flag==1 && Position_State_Before_Recheck==1)
//			 {
//				 Position_State_Before_Recheck=0;
//			 }
			 
			 
/*			 
			 if(Airsensor.personon_by_positioncheck_flag==1 && Position_State_Before_Recheck==0)
			 { 
				 if(Airsensor.AirSensor_recheck_times==1)
				 {
			     Airsensor.personon_by_positioncheck_flag=0;//����м�⣬�����ʱ�����־
					 Airsensor.AirSensor_recheck_times=0;
				 }
				 else if(Airsensor.AirSensor_recheck_times==2)
				 {
					 Airsensor.AirSensor_recheck_times=1;
				 }				 
			 }
*/			 
//       Airsensor.personon_by_positioncheck_flag=1;			 
		 } 
		 
  MIC_data_stop_detect_flag=0;
}		 


void sort(uint16_t *a, int l)//aΪ�����ַ,lΪ���鳤��
{
    int i, j;
    uint16_t v;    

    for(i = 0; i < l - 1; i ++)
        for(j = i+1; j < l; j ++)
        {
            if(a[i] > a[j])//���ǰ��ıȺ�����򽻻�
            {
                v = a[i];
                a[i] = a[j];
                a[j] = v;
            }
        }
} 

void AirSensor_monitor_airpressure(void)
{
    uint8_t i;
	  uint16_t sum=0;
	  ADok3=0;
		ADCstart3();
		do
		{
			osDelay(1);
		}while(ADok3==0);
		Airsensor.AirSensor_PressureBuffer[Airsensor.AirSensor_PressureBuffer_WR_pointer]=PressureADC_ConvertedValue[0];
		Airsensor.AirSensor_pressure_sample=Airsensor.AirSensor_PressureBuffer[AirSensor_Pressure_fs-1];//for data view
		Airsensor.AirSensor_PressureBuffer_WR_pointer++;
		if(Airsensor.AirSensor_PressureBuffer_WR_pointer>=AirSensor_Pressure_fs)
		{
			
			Airsensor.AirSensor_PressureBuffer_WR_pointer=0;
			
			//�Բ��������ݽ���������ȡ�м��4�����ݽ������㴦��
			
//			sort(Airsensor.AirSensor_PressureBuffer,10);
			
			sort(Airsensor.AirSensor_PressureBuffer,AirSensor_Pressure_fs);
			
#if 0
      for(i=0;i<AirSensor_Pressure_fs;i++)
      {
        sum=sum+Airsensor.AirSensor_PressureBuffer[i];
			}	
			
      Airsensor.AirSensor_PressureTest[Airsensor.AirSensor_PressureTest_WR_pointer]=sum/Airsensor.AirSensor_Pressure_fs;
			
#else
//			for(i=4;i<6;i++)
//      {
//        sum=sum+Airsensor.AirSensor_PressureBuffer[i];
//			}	
//      Airsensor.AirSensor_PressureTest[Airsensor.AirSensor_PressureTest_WR_pointer]=sum/2;
			  Airsensor.AirSensor_PressureTest[Airsensor.AirSensor_PressureTest_WR_pointer]=Airsensor.AirSensor_PressureBuffer[AirSensor_Pressure_fs-1];
			
			
#endif
			Airsensor.AirSensor_pressure_sample_avg_last=Airsensor.AirSensor_pressure_sample_avg;
			Airsensor.AirSensor_pressure_sample_avg=Airsensor.AirSensor_PressureTest[Airsensor.AirSensor_PressureTest_WR_pointer];//for datat view
	  	Airsensor.AirSensor_PressureTest_WR_pointer++;
  		if( Airsensor.AirSensor_PressureTest_WR_pointer>=AirSensor_PressureTest_buffer_length)
	  	{
				
  			Airsensor.AirSensor_PressureTest_WR_pointer=0;
        Airsensor.AirSensor_BufferRefresh=1;			
  		}	
      
 //     AirSensor_monitor_airpressure_filter();
      AirSensor_monitor_Pressure_AvgValue(&Airsensor);
//			AirSensor_monitor_Pressure_AvgValue1(&Airsensor);
			AirSensor_detect_person_onoff(&Airsensor);
			
		}
		
		

		
}

void Body_Detect_Init(void)
{
	Airsensor.Body_detect_on_threshold = gStorageData.Body_detect_on_threshold;
	Airsensor.Body_detect_off_threshold = gStorageData.Body_detect_off_threshold;
	
}


void Position_Init(void)
{
	 if(gStorageData.HeadpositionLevel==HIGH_CHECK)
	 {
		 Airsensor.headposition_sensitivity_person_on=AirSensor_threshold_person_on_HIGH;
     Airsensor.headposition_sensitivity_person_snore=AirSensor_threshold_person_snore_HIGH;
     Airsensor.headposition_sensitivity_person1_on=AirSensor_threshold_person1_on_HIGH;
     Airsensor.headposition_sensitivity_person1_snore=AirSensor_threshold_person1_snore_HIGH; 
		 Airsensor.headposition_sensitivity_person_on1=AirSensor_threshold_person_on1_HIGH;
	 }
	 else if(gStorageData.HeadpositionLevel==MID_CHECK)
	 {
		 Airsensor.headposition_sensitivity_person_on=AirSensor_threshold_person_on_MID;
     Airsensor.headposition_sensitivity_person_snore=AirSensor_threshold_person_snore_MID;
     Airsensor.headposition_sensitivity_person1_on=AirSensor_threshold_person1_on_MID;
     Airsensor.headposition_sensitivity_person1_snore=AirSensor_threshold_person1_snore_MID; 
		 Airsensor.headposition_sensitivity_person_on1=AirSensor_threshold_person_on1_MID;
	 }
	 else if(gStorageData.HeadpositionLevel==LOW_CHECK)
	 {
		 Airsensor.headposition_sensitivity_person_on=AirSensor_threshold_person_on_LOW;
     Airsensor.headposition_sensitivity_person_snore=AirSensor_threshold_person_snore_LOW;
     Airsensor.headposition_sensitivity_person1_on=AirSensor_threshold_person1_on_LOW;
     Airsensor.headposition_sensitivity_person1_snore=AirSensor_threshold_person1_snore_LOW; 
		 Airsensor.headposition_sensitivity_person_on1=AirSensor_threshold_person_on1_LOW;
	 }
}

void User_set_positionvalue(void)
{
	   Airsensor.headposition_sensitivity_person_on=gStorageData.PositionValue_Onebag;
     Airsensor.headposition_sensitivity_person_snore=gStorageData.PositionValue_Onebag;
     Airsensor.headposition_sensitivity_person1_on=gStorageData.PositionValue_Twobag;
     Airsensor.headposition_sensitivity_person1_snore=gStorageData.PositionValue_Twobag;
		 Airsensor.headposition_sensitivity_person_on1=gStorageData.PositionValue_Twobag+20;
	
	   if(Airsensor.headposition_sensitivity_person_on==0)
		 {
			 Airsensor.headposition_sensitivity_person_on=125;
			 Airsensor.headposition_sensitivity_person_snore=125;
		 }
		 if(Airsensor.headposition_sensitivity_person1_on==0)
		 {
			 Airsensor.headposition_sensitivity_person1_on=180;
		   Airsensor.headposition_sensitivity_person1_snore=180;
		   Airsensor.headposition_sensitivity_person_on1=200;
	   }
}



/**************************************************************************/
/*                                                                        */
/**************************************************************************/
void Time20MSCheck(void)
{
//    static unsigned char status2 = 0;
    
		//unsigned char buf[30];
	  uint8_t tt=0;
    static uint8_t Airsensor_Count_20ms = 0;
	  static uint8_t Airsensor_Count_timer = 0;
	  
	
	  if (Timer10msOn == 1)
    {
          Timer10msOn = 0;
			  //---------------��ʱ��ȡ��ѹ������ֵ�Լ���Ƿ�����------------------
			  Airsensor_Count_timer++;
			  if(Airsensor_Count_timer>=1)  //100ms����һ��
				{
			    if( Airsensor.AirSensor_Check_Mode==4 )
					{
					  AirSensor_monitor_airpressure(); //10ms����һ����ѹ����
					}
//					AirSensor_monitor_airpressure_filter();
//					AirSensor_monitor_Pressure_AvgValue();
					
					Airsensor_Count_timer=0;
				}			
			
		}
		
		
#if 0		
		//******************��ʱ���******************************
	  if(Airsensor.AirSensor_PersonOn==1 && Airsensor.AirSensor_PersonOn_Last==0)
		{
			 WrokingLed(BLUE_LED_HIGH,2);//������
			 Led_OnOff_Timer=0;
			 Airsensor.Led_Person_OnOff=1;
			
		}
		
		if(Airsensor.AirSensor_PersonOn==0 && Airsensor.AirSensor_PersonOn_Last==1)
		{
			 WrokingLed(RED_LED,1);//�����
			 Led_OnOff_Timer=0;
			 Airsensor.Led_Person_OnOff=2;
			
		}
		
		
		//********************************************************
		if(Led_OnOff_Timer>AirSensor_LED_onoff_time*1000)
		{
			if(Airsensor.Led_Person_OnOff!=0)
			{
			  Airsensor.Led_Person_OnOff=0;
			  WrokingLed(RED_LED,0);
			}
			
		}
		
#endif		
//    static  uint8_t uart2[2] = {0xA5,0x51};
    if (Timer20msOn == 1)
    {
        Timer20msOn = 0;
			  
			
			  //---------------��ʱ��PC����log��Ϣ---------------------------------
			  Airsensor_Count_20ms++;

				if(Airsensor_Count_20ms >= 50)
        {
          Airsensor_Count_20ms=0; 
#if 0					
          if(AppCtrlBag.enable==0)
					{	
            while(xSemaphoreTake( xLogoMute, portMAX_DELAY ) != pdTRUE)
	          {
              osDelay(1);
		        } 						
					  Airsensor_SendlogtoPC();
						xSemaphoreGive( xLogoMute );
					}
#else
				  Airsensor_SendlogtoPC();	
#endif
        }
//				checkposition_count_timer++;
				if(Airsensor.period_snore_checkposition_flag==1)
				{
					checkposition_count_timer++;
					if(checkposition_count_timer>=50*Checkposition_period_whennoperson)
					{
						Airsensor.period_snore_checkposition_flag=0;
						checkposition_count_timer=0;
					}
				}
				if(Airsensor.Tt_Is_Sleep_Time==1 && gStorageData.One_hour_auto_detect_onoff==1)
				{
					Airsensor.onehour_snore_checkposition_counter++;
					if(Airsensor.onehour_snore_checkposition_counter>=gStorageData.auto_detect_timecycle*3000) //����תΪ20ms
					{
						//����������1Сʱ����һ��ͷ��λ�ü�����ṩ���Ƿ��뿪��Ϣ��app
						AirSensor_fast_check_person_poweron();
						//ˢ��������ʱ��
            Airsensor.AirSensor_fast_check_position_finished_flag=0;
					  Airsensor.AirSensor_Checkposition_Mode=4;					
						AirSensor_person_detect_init();
						Airsensor.AirSensor_Check_Mode=Airsensor.AirSensor_Checkposition_Mode;	
						
						Airsensor.onehour_snore_checkposition_counter=0;
						Airsensor.period_snore_checkposition_counter=0;
					}
				}
				
				if(Airsensor.antisonre_timer_flag==1)
				{
					Antisnore_count_timer++;
					if(Antisnore_count_timer >= 50*Airsensor.antisnore_timer_len)
					{
						Airsensor.antisonre_timer_flag=0;
						Antisnore_count_timer=0;
					}
					
				}
				if(Airsensor.Tt_Is_Sleep_Time==1 && workdelay_time_cycle_flag==1)
				{
					workdelay_count_timer++;
				  if(workdelay_count_timer >= 50*60*gStorageData.WorkDelay)
					{
						workdelay_time_cycle_flag=0;
						workdelay_count_timer=0;
					}
				
					
				}
				if(Firmware_update_received_flag == 1)
				{
					update_request_counter_timer++;
					if(gStorageData.Standby==0)
					{
						Firmware_update_received_flag = 0;
						update_request_counter_timer = 0;
						reply_app_for_update_keypress_flag = 1;
					}
					else if(Firmware_update_keypressed == 2)
					{
						reply_app_for_update_keypress_flag = 2;
						Firmware_update_received_flag = 0;
						update_request_counter_timer = 0;
						
						//buf[0]=0x02;
						//buf[1]=REQUEST_UPGRADE;
						//buf[2]=GetNeedUpgradeImageType();
						//buf[3]=3;
						//uart3Send((unsigned char *)buf,4);
						
						WrokingLed(RED_LED,1);//�����
					}
					else if(Firmware_update_keypressed == 1)
					{
						Firmware_update_received_flag = 0;
						update_request_counter_timer = 0;
						reply_app_for_update_keypress_flag = 1;
						
						//ERROR_Command(buf,REQUEST_UPGRADE);
						//uart3Send((unsigned char *)buf,4);
						WrokingLed(RED_LED,1);//�����
					}
					else if(update_request_counter_timer > 50*10) //10S
					{
						Firmware_update_received_flag = 0;
						update_request_counter_timer = 0;
						reply_app_for_update_keypress_flag = 1;
						
						//ERROR_Command(buf,REQUEST_UPGRADE);
						//uart3Send((unsigned char *)buf,4);
						WrokingLed(RED_LED,1);//�����
					}
					else if((update_request_counter_timer % 40)==0) //800ms
					{
						WrokingLed(BLUE_LED_HIGH,2);//������						
					}
					else if((update_request_counter_timer % 20)==0) //800ms
					{
						WrokingLed(RED_LED,1);//�����
					}
					
					
				}
	      //--------------��ʱλ�ü��------------------------------------------
        Airsensor_readvalue_50ms++;
				if(Airsensor.AirSensor_Checkposition_Mode==1)
				 {	
          if (Airsensor_readvalue_50ms > AirSensor_position_slowcheck_cycle)
            {
 
             Airsensor_readvalue_50ms = 0;
//            AirSensor_slow_check_position();
            }
					}
				 if(Airsensor.AirSensor_Checkposition_Mode==2 && gStorageData.Standby==1)
				  {
						if(gStorageData.Standby==1)
						{
 //             if(Airsensor_readvalue_50ms > AirSensor_position_fastcheck_cycle)
 //             {
						    Airsensor.AirSensor_Check_Mode=5;
                Airsensor_readvalue_50ms = 0;
                AirSensor_fast_check_position();
//						    tt=1;
//							}
            }
						else
						{
							Airsensor.AirSensor_Checkposition_Mode=5;						 
						  Airsensor.AirSensor_Check_Mode=5;
						}
					}					
	
    }
		
}

void BLE_SendlogtoPC_OK(void)
{
     uint8_t i=0;
	   int offset=0; 
	   
	   offset += sprintf(SendData+offset,"BLE S/N:"); 
		 
	   for(i=0;i<14;i++)
	   {
	     SendData[offset]=SetBleNameBuffer[i];
			 offset++;
		 }
	   
		 offset += sprintf(SendData+offset,"\n");
		 
		 offset += sprintf(SendData+offset,"BLE S/N NAME Set OK!\n");
		 
	   uart_send(SendData, strlen(SendData));
		 
}

void BLE_SendlogtoPC_NG(void)
{
     uint8_t i=0;
	   int offset=0; 
	   
	   offset += sprintf(SendData+offset,"BLE S/N:"); 
		 
	   for(i=0;i<14;i++)
	   {
	     SendData[offset]=SetBleNameBuffer[i];
			 offset++;
		 }
	   
		 offset += sprintf(SendData+offset,"\n");
		 
		 offset += sprintf(SendData+offset,"BLE S/N NAME Set NG!\n");
		 
	   uart_send(SendData, strlen(SendData));
		 
}

void test1(void)
{
	unsigned short a,b,c,d,e;
	a=10000;
	b=13000;
	c=5000;
	d=c-b+a;
	e=a+c-b;
	osDelay(10);
	
}

uint32_t  timer_test1,timer_test2;
/**************************************************************************/
/*                                                                        */
/**************************************************************************/
void PositionTaskFunction(void const *argument)
{
	  uint8_t i,j;
	  uint8_t pump_record_process_repaet_flag=0;
	  uint8_t pump_time_read_temp;
	  uint8_t no_snore_recently_flag1=0;
	
//		spitest();
	
//    uint16_t wrret=0;
    
    //unsigned char status2 = 0;
    //------------init-----------------------------
    //MP2�������������ߵ�ƽ����������δͨ��״̬�����ɴ�������״̬����ʱ����ͨ������á�
    while(StartAllAction==0)
 		 osDelay(10);   


	  //���Դ���
		
//		GetEnableStudy();
		
		
//		test1();
	  Airsensor.AirSensor_PersonOn=0;
		Airsensor.AirSensor_PersonOn_Last=0;
		Airsensor.period_snore_checkposition_flag=0;
		Airsensor.period_snore_checkposition_counter=0;
		Airsensor.Poweroffkey_savedata_flag=0;
    LastPosition.body = 0;
    LastPosition.head1 = 0x10;
    LastPosition.head2 = 0x10;
		PillowMaxOne_Willbesend=0x10;
		Pillow2ndMaxOne_Willbesend=0x10;
		
		
	  osDelay(500);  //��ʱ���ϵͳ�ϵ��ʼ�����ȶ����������300ms
		
		//��ͨ���õ��������������úͷ���֮����������һ�£���Ϊһ�ϵ�͹ر�����
#if 1
		AirSensor_Set_AirGate(0);
		osDelay(200);
	  AirSensor_Set_AirGate(1);
		osDelay(200);
	  AirSensor_Set_AirGate(2);
		osDelay(200);
	  AirSensor_Set_AirGate(3);
		
//		osDelay(2000); 		
		
#endif	

    Airsensor.personon_by_positioncheck_flag_forpensononoff=0;
		Airsensor.personon_by_positioncheck_flag=0;
		Airsensor.AirSensor_noperson_state_checked_istrue=0;
		
//    WrokingLed(RED_LED,1);
//    WrokingLed(BLUE_LED_HIGH,0);
#if 0
		//�ϵ����Ƿ�����
    AirSensor_fast_check_person_poweron();
		if(PillowMaxOne_Willbesend!=0x10)
		{
			WrokingLed(BLUE_LED_HIGH,2);//������
			Led_OnOff_Timer=0;
			Airsensor.Led_Person_OnOff=1;	
		}
		else
		{
			WrokingLed(RED_LED,1);//������
			Led_OnOff_Timer=0;
			Airsensor.Led_Person_OnOff=2;	
			
		}
		
#endif
		
		Airsensor.AirSensor_Check_Mode=5;
		Airsensor.AirSensor_Checkposition_Mode=0;
//		AirSensor_person_detect_init();
		
		for (;;)
    {
			  osDelay(10);
			  timer_test1=AirSensor_Personon_Timer;
			
			  //����������
			  if(Received_BLE_NAME_flag==1)
				{
					
					SetBleName();
					VerifyBleName();
					if(BleNameVerifyOK_flag==1)
					{
						BLE_SendlogtoPC_OK();
					}
					else
					{
						BLE_SendlogtoPC_NG();
					}						
					Received_BLE_NAME_flag=0;	
					osDelay(3000);
					
				}
				if(gStorageData.Standby==1 && Airsensor.Sleep_time_flag==1)
				{
					Airsensor.AirSensor_Check_Mode=5;
					Airsensor.AirSensor_Checkposition_Mode=0;
//		      Airsensor.AirSensor_Checkposition_Mode=5;
					Airsensor.Sleep_time_flag=0;
//					AirSensor_Reset_AirGate(0);
//					AirSensor_Reset_AirGate(1);
//					AirSensor_Reset_AirGate(2);
//					AirSensor_Reset_AirGate(3);
					AirSensor_Set_AirGate(0);
					AirSensor_Set_AirGate(1);
					AirSensor_Set_AirGate(2);
					AirSensor_Set_AirGate(3);

          Airsensor.Poweroffkey_savedata_flag=1;

#if 0
					
					//�����𴲰���ʱ��
					while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
					{
              osDelay(1);
					}	
//					person_off_hour=stimestructure.Hours;
//					person_off_minute=stimestructure.Minutes;
//					person_off_second=stimestructure.Seconds;
//					Airsensor.Special_savesnoredata_flag=1;
					
					gStorageData.Snore.StartTime[0]=sdatestructure.Year;
			    gStorageData.Snore.StartTime[1]=sdatestructure.Month;
			    gStorageData.Snore.StartTime[2]=sdatestructure.Date;
			    gStorageData.Snore.StartTime[3]=stimestructure.Hours;
			    gStorageData.Snore.StartTime[4]=stimestructure.Minutes;
					gStorageData.Snore.StudyMode=0x82;
					gStorageData.Snore.UsedRecord=0x55aa; 
					WriteSnoreData(&gStorageData.Snore); //�����ϴ�					
					memset(&gStorageData.Snore,0,sizeof(SnoreData_def));
					gStorageData.Snore.UsedRecord=0x55aa;
					gStorageData.Snore.StudyMode=gStorageData.StudyMode;
					Config.SnoreAverTime = 4000;
					gStorageData.Snore.HeadPosion = 0x0ff;	
					person_on_for_save_data_flag=0;
					xSemaphoreGive( xTimeMute );
					
					//�������ù�����¼
					j=1;
					pump_record_process_repaet_flag=1;
					pump_time_read_temp=0;
					 
				   
				  do
          {					 
					  if(pump_operation_time[pump_operation_time_read_pointer][0]==1)
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
						 
						  if(pump_operation_time[pump_operation_time_read_pointer][0]==1)
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
						 
						    if(pump_operation_time[pump_operation_time_read_pointer][0]==1)
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
						 
						      if(pump_operation_time[pump_operation_time_read_pointer][0]==1)
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
					
//						 Airsensor_Print_Tuwan_Time();
//						 osDelay(50);
						 
				     WriteSnoreData(&detailed_snore); //������������  �����⵽������û��⵽�˼�����ֹ��������Ҳ�ᱣ���������ݼ�¼
				
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
				 person_on_hour=0xff;					 
				 person_on_minute=0xff;
			   person_on_second=0xff;
				 person_off_hour=0xff;	 
				 person_off_second=0xff;
				 person_off_minute=0xff;
				 Ten_minutes_first_personon_event_flag=0; 

#endif
				 
					
				}
				if(Airsensor.Sleep_time_flag==0 && gStorageData.Standby==0)
				{
					
//					Airsensor.AirSensor_Check_Mode=4;
//		      Airsensor.AirSensor_Checkposition_Mode=4;
//					AirSensor_person_detect_init();
//					Airsensor.AirSensor_pressure_sample_avg_diffsum=0;
					//���濪ʼ˯�߰���ʱ��
					while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
					{
              osDelay(1);
					}	
//					person_off_hour=stimestructure.Hours;
//					person_off_minute=stimestructure.Minutes;
//					person_off_second=stimestructure.Seconds;
//					Airsensor.Special_savesnoredata_flag=1;
					memset(&gStorageData.Snore,0,sizeof(SnoreData_def));
					gStorageData.Snore.StartTime[0]=sdatestructure.Year;
			    gStorageData.Snore.StartTime[1]=sdatestructure.Month;
			    gStorageData.Snore.StartTime[2]=sdatestructure.Date;
			    gStorageData.Snore.StartTime[3]=stimestructure.Hours;
			    gStorageData.Snore.StartTime[4]=stimestructure.Minutes;
					gStorageData.Snore.StudyMode=0x81;
					gStorageData.Snore.UsedRecord=0x55aa; 
					WriteSnoreData(&gStorageData.Snore); //�����ϴ�					
					memset(&gStorageData.Snore,0,sizeof(SnoreData_def));
					gStorageData.Snore.UsedRecord=0x55aa;
					gStorageData.Snore.StudyMode=gStorageData.StudyMode;
					Config.SnoreAverTime = 4000;
					
					gStorageData.Snore.HeadPosion = 0x0ff;
					person_on_for_save_data_flag=0;
					
					xSemaphoreGive( xTimeMute );

					Airsensor.Sleep_time_flag=1;
					
					
				}
				if(Airsensor.Sleep_time_flag==1 && Airsensor.Tt_Is_Sleep_Time==1 && Airsensor.Tt_Is_Sleep_Time_last==0)
//				if(Airsensor.Sleep_time_flag==1 && Airsensor.AirSensor_PersonOn==1 && Airsensor.AirSensor_PersonOn_Last==0)	
				{
//				  Airsensor.AirSensor_Check_Mode=5;
//		      Airsensor.AirSensor_Checkposition_Mode=5;
//					AirSensor_Reset_AirGate(0);
//					AirSensor_Reset_AirGate(1);
//					AirSensor_Reset_AirGate(2);
//					AirSensor_Reset_AirGate(3);
					Airsensor.Tt_Is_Sleep_Time_last=1;
					WrokingLed(BLUE_LED_LOW,1);//������
					//����ѧϰʱ�����估�����ϴ���ʱ��
					while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
					{
              osDelay(1);
					}	
//					person_on_hour=stimestructure.Hours;
//					person_on_minute=stimestructure.Minutes;
//					person_on_second=stimestructure.Seconds;
					uint16_t hourtemp=0,mintemp=0;
					mintemp=stimestructure.Minutes+StudyDelayTime;
					study_min_start=mintemp%60;					
					study_hour_start=stimestructure.Hours+mintemp/60;
					if(study_hour_start>=24)
						study_hour_start=study_hour_start-24;
					study_min_end=study_min_start;
					study_hour_end=study_hour_start+3;
					if(study_hour_end>=24)
						study_hour_end=study_hour_end-24;									
					xSemaphoreGive( xTimeMute );
					
					gStorageData.Snore.LSnoreTimes=0;
				  gStorageData.Snore.SSnoreTimes=0;
				  gStorageData.Snore.MSnoreTimes=0;
					gStorageData.Snore.Airbag1Times=0;
					gStorageData.Snore.Airbag2Times=0;
					gStorageData.Snore.Airbag3Times=0;
					gStorageData.Snore.Airbag4Times=0;
					
					while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
					{
						osDelay(1);
					}
					pump_operation_time[0][0]=0;
					pump_operation_time[1][0]=0;
					pump_operation_time[2][0]=0;
					pump_operation_time[3][0]=0;
					pump_operation_time[4][0]=0;
					pump_operation_time[5][0]=0;
					pump_operation_time[6][0]=0;
					pump_operation_time[7][0]=0;
					pump_operation_time[8][0]=0;
					pump_operation_time[9][0]=0;
					pump_operation_time[10][0]=0;
					pump_operation_time[11][0]=0;
					pump_operation_time[12][0]=0;
					pump_operation_time[13][0]=0;
					pump_operation_time[14][0]=0;
					pump_operation_time[15][0]=0;
					pump_operation_time[16][0]=0;
					pump_operation_time[17][0]=0;
					pump_operation_time[18][0]=0;
					pump_operation_time[19][0]=0;
					
					pump_operation_time_read_pointer = 0;
					pump_operation_time_write_pointer = 0;
					xSemaphoreGive( xTimeMute );	
#if 0					
					//�����˯��ʱ����1һ��Сʱ������
					while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
					{
            osDelay(1);
					}	
				  latestsnoretime_minute=stimestructure.Minutes;
					latestsnoretime_hour=stimestructure.Hours;										
				  xSemaphoreGive( xTimeMute );
#endif
//					Airsensor.onehour_snore_checkposition_flag=1;
					Airsensor.onehour_snore_checkposition_counter=0;
					if(gStorageData.WorkDelay >= 30 && gStorageData.WorkDelay <= 120)
					{
						workdelay_time_cycle_flag = 1;
						workdelay_count_timer = 0;
					}					
					
				}
				
				if((Airsensor.AirSensor_PersonOn==1 && Airsensor.AirSensor_PersonOn_Last==0))
				{
					while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
					{
              osDelay(1);
					}	
					if(Ten_minutes_first_personon_event_flag==0)
					{
						person_on_hour=stimestructure.Hours;
					  person_on_minute=stimestructure.Minutes;
					  person_on_second=stimestructure.Seconds;
						Ten_minutes_first_personon_event_flag=1;
					}
					personontime_minute=stimestructure.Minutes;
					personontime_hour=stimestructure.Hours;
					rechecktime_hour=0xff;

					xSemaphoreGive( xTimeMute );
					snore_times_every_sleep=0;
					Airsensor.period_snore_checkposition_flag=0;				
					checkposition_count_timer=0;
					Airsensor.onehour_snore_checkposition_counter=0;
					
				}	
				
				else if(Airsensor.AirSensor_PersonOn==0 && Airsensor.AirSensor_PersonOn_Last==1)					
				{
					while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
					{
              osDelay(1);
					}	
					person_off_hour=stimestructure.Hours;
					person_off_minute=stimestructure.Minutes;
					person_off_second=stimestructure.Seconds;
					xSemaphoreGive( xTimeMute );					
				}
				
        Airsensor.AirSensor_PersonOn_Last=Airsensor.AirSensor_PersonOn;
				
				
				while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				{
              osDelay(1);
				} 
//			  if( AppTest==0 && AppCtrlBag.enable==0 && ReadPos.enable==0 && gStorageData.StudyMode!=2 && Airsensor.AirSensor_Checkposition_Mode!=Airsensor.AirSensor_Check_Mode && Airsensor.AirSensor_Checkposition_Mode!=2 && Airsensor.AirSensor_Checkposition_Mode!=3 && ActionSteps == 0x00 &&RequestControlValve==0 && PillowControl.SnoreOn==0 && Airsensor.Tt_Is_Sleep_Time==1)
        if( AppTest==0 && AppCtrlBag.enable==0 && ReadPos.enable==0 && gStorageData.StudyMode!=2 && Airsensor.AirSensor_Checkposition_Mode!=Airsensor.AirSensor_Check_Mode && Airsensor.AirSensor_Checkposition_Mode!=2 && Airsensor.AirSensor_Checkposition_Mode!=3 && ActionSteps == 0x00 &&RequestControlValve==0 && PillowControl.SnoreOn==0 && Airsensor.Sleep_time_flag==1)
				{
					  if(Airsensor.AirSensor_fast_check_position_finished_flag==0)
						{
//					    AirSensor_fast_check_person_poweron();//ֻ���ϵ����03����������������������ǰ��λ�ü��ȷ��һ���Ƿ����� ��2022��5��10�رգ���ֹѧϰ��������Ϊ����
							Airsensor.AirSensor_fast_check_position_finished_flag=0;
						}
						else
							 Airsensor.AirSensor_fast_check_position_finished_flag=0;
					  Airsensor.AirSensor_Checkposition_Mode=4;					
						AirSensor_person_detect_init();
						Airsensor.AirSensor_Check_Mode=Airsensor.AirSensor_Checkposition_Mode;
						
				}  				 
			  xSemaphoreGive( xPositionMute );
				
				if(RequestControlValve==1)
				 {
					Airsensor_readvalue_50ms=0;
          RequestControlValve=2;
          Airsensor.AirSensor_Checkposition_Mode=0;	
          Airsensor.AirSensor_Check_Mode=5;					 
				 }
				else
				{		
           if(Airsensor.AirSensor_Checkposition_Mode==3 && Airsensor.Tt_Is_Sleep_Time==1)					
//          if(Airsensor.AirSensor_Checkposition_Mode==3 && Airsensor.AirSensor_PersonOn==1)
				   {
					  AirSensor_fast_check_position();					  
						RequestControlValve=3; //��ʱ����
						if(RequestControlValve==3)
              osDelay(100);							
						Airsensor.AirSensor_Checkposition_Mode=5;						 
						Airsensor.AirSensor_Check_Mode=5;
					  Airsensor_readvalue_50ms=0;
				   }
					 else if(Airsensor.AirSensor_Checkposition_Mode==3 && Airsensor.Tt_Is_Sleep_Time==0)
//          else if(Airsensor.AirSensor_Checkposition_Mode==3 && Airsensor.AirSensor_PersonOn==0)	
           {
             RequestControlValve=3; //��ʱ����
             Airsensor.AirSensor_Checkposition_Mode=5;						 
						 Airsensor.AirSensor_Check_Mode=5;						
					 }						
				}
				
				//����Ƿ���1��Сʱ������
#if 0				
				if(Airsensor.Tt_Is_Sleep_Time==1)
				{
//					uint8_t no_snore_recently_flag1=0;
					no_snore_recently_flag1=0;
//					while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
//				  {
//            osDelay(1);
//				  } 
					//��⵱ǰʱ�������һ��������ʱ���Ƿ���һ��Сʱ����
          if(stimestructure.Hours>=latestsnoretime_hour)
					{
						if((stimestructure.Hours*60+stimestructure.Minutes)>=(latestsnoretime_hour*60+latestsnoretime_minute))
						{
							if(((stimestructure.Hours*60+stimestructure.Minutes)-(latestsnoretime_hour*60+latestsnoretime_minute))>=Recheck_when_person_without_snore_longtime_beyond_sleeptime_time)									
							{
							  no_snore_recently_flag1=1;
							}
						}
								
					}
					else
					{
					  if((((stimestructure.Hours+24)*60+stimestructure.Minutes)-(latestsnoretime_hour*60+latestsnoretime_minute))>=Recheck_when_person_without_snore_longtime_beyond_sleeptime_time)									
						{
							no_snore_recently_flag1=1;
						}
					}
          if(no_snore_recently_flag1==1)
					{
						//����������1Сʱ����һ��ͷ��λ�ü�����ṩ���Ƿ��뿪��Ϣ��app
						AirSensor_fast_check_person_poweron();
						//ˢ��������ʱ��
            latestsnoretime_minute=stimestructure.Minutes;
					  latestsnoretime_hour=stimestructure.Hours;
            Airsensor.AirSensor_fast_check_position_finished_flag=0;
					  Airsensor.AirSensor_Checkposition_Mode=4;					
						AirSensor_person_detect_init();
						Airsensor.AirSensor_Check_Mode=Airsensor.AirSensor_Checkposition_Mode;						
					}
					
//					xSemaphoreGive( xTimeMute );
				}

#endif	
        if( AppTest==0 && AppCtrlBag.enable==0 && gStorageData.Standby==1 && ReadPos.enable==0 && Airsensor.AirSensor_Checkposition_Mode!=Airsensor.AirSensor_Check_Mode && Airsensor.AirSensor_Checkposition_Mode!=2 && Airsensor.AirSensor_Checkposition_Mode!=3 && ActionSteps == 0x00 &&RequestControlValve==0 && PillowControl.SnoreOn==0)
				{
          Airsensor.AirSensor_Checkposition_Mode=4;					
					AirSensor_person_detect_init();
					Airsensor.AirSensor_Check_Mode=Airsensor.AirSensor_Checkposition_Mode;
					Airsensor.AirSensor_PersonOn=0;
					Airsensor.AirSensor_PersonOn_Last=0;
				}
				Time20MSCheck();
				timer_test2=AirSensor_Personon_Timer;
				
			}	

			
		}				
				
		
		
/*				
			  if((Airsensor.AirSensor_PersonOn==1 && Airsensor.AirSensor_PersonOn_Last==0) && Tt_Is_Sleep_Time ==0 )
				{
					SnoreInit_personon();
//					Airsensor.AirSensor_PersonOn_init_snore_flag=1;					
				}	
				
				if((Airsensor.AirSensor_PersonOn==1 && Airsensor.AirSensor_PersonOn_Last==0))
				{
					while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
					{
              osDelay(1);
					}	
					if(Ten_minutes_first_personon_event_flag==0)
					{
						person_on_hour=stimestructure.Hours;
					  person_on_minute=stimestructure.Minutes;
					  person_on_second=stimestructure.Seconds;
						Ten_minutes_first_personon_event_flag=1;
					}
					personontime_minute=stimestructure.Minutes;
					personontime_hour=stimestructure.Hours;
					rechecktime_hour=0xff;
					latestsnoretime_hour=0xff;
					xSemaphoreGive( xTimeMute );
					snore_times_every_sleep=0;
				}	
				
				else if(Airsensor.AirSensor_PersonOn==0 && Airsensor.AirSensor_PersonOn_Last==1)					
				{
					while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
					{
              osDelay(1);
					}	
					person_off_hour=stimestructure.Hours;
					person_off_minute=stimestructure.Minutes;
					person_off_second=stimestructure.Seconds;
					xSemaphoreGive( xTimeMute );					
				}
				
        Airsensor.AirSensor_PersonOn_Last=Airsensor.AirSensor_PersonOn;

        


			  if(RequestControlValve==1)
				 {
					Airsensor_readvalue_50ms=0;
          RequestControlValve=2;
          Airsensor.AirSensor_Checkposition_Mode=0;	
          Airsensor.AirSensor_Check_Mode=5;					 
				 }
				else if(gStorageData.StudyMode==2)
				{
					Airsensor.AirSensor_Checkposition_Mode=0;	
          Airsensor.AirSensor_Check_Mode=5;	
				}
				else
				{				  
          if(Airsensor.AirSensor_Checkposition_Mode==3)
				   {
//						AirSensor_check_position_init();
						
					  AirSensor_fast_check_position();					  
						RequestControlValve=3; //��ʱ����
						if(RequestControlValve==3)
              osDelay(100);							
						Airsensor.AirSensor_Checkposition_Mode=0;						 
						Airsensor.AirSensor_Check_Mode=5;
					  Airsensor_readvalue_50ms=0;
//						Airsensor.AirSensor_fast_check_position_finished_flag=1;//�¸Ļ�������ͳһ���ϵ��⸴��
				   } 
				}
				
#if 0				
				
        while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				{
              osDelay(1);
				} 
			  if( AppTest==0 && AppCtrlBag.enable==0 && ReadPos.enable==0 && gStorageData.StudyMode!=2 && Airsensor.AirSensor_Checkposition_Mode!=Airsensor.AirSensor_Check_Mode && Airsensor.AirSensor_Checkposition_Mode!=2 && Airsensor.AirSensor_Checkposition_Mode!=3 && ActionSteps == 0x00 &&RequestControlValve==0 && PillowControl.SnoreOn==0)
				{
					  if(Airsensor.AirSensor_fast_check_position_finished_flag==0)
						{
					    AirSensor_fast_check_person_poweron();//ֻ���ϵ����03����������������������ǰ��λ�ü��ȷ��һ���Ƿ�����
							Airsensor.AirSensor_fast_check_position_finished_flag=0;
						}
						else
							 Airsensor.AirSensor_fast_check_position_finished_flag=0;
					  Airsensor.AirSensor_Checkposition_Mode=4;					
						AirSensor_person_detect_init();
						Airsensor.AirSensor_Check_Mode=Airsensor.AirSensor_Checkposition_Mode;
						
				 }  				 
			   xSemaphoreGive( xPositionMute );
#endif
				 
				 while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				{
              osDelay(1);
				} 
				xSemaphoreGive( xPositionMute ); 
				
				 
			   Time20MSCheck();
			   GetSnoreDataInSPIFlashDebug();//�����ڵ��ԣ�����app��ȡ˯�����ݲ鿴	
		}
	}

*/	
void LedFlashTaskFunction(void const *argument)
{
	
	for(;;)
	{
		osDelay(10);		
	  if (LEDFlashTimer20msOn == 1)
    {
      LEDFlashTimer20msOn = 0;
			if(Airsensor.LED_flash_state_flag==1)
			{
				//���������ҳ����������Ի���ֹ�������ڷ������Ƶ���
				Airsensor.Led_flash_counter++;
				if(Airsensor.Led_flash_counter>=15)
				{
				  if(Airsensor.Led_yellow_state==1)
					{
						WrokingLed(RED_LED,0);
						Airsensor.Led_yellow_state=0;
					}
					else
					{
							WrokingLed(RED_LED,1);
							Airsensor.Led_yellow_state=1;
					}
					Airsensor.Led_flash_counter=0;
				}
			}				
    }
	}
}


//***************************************************************************************************

	
//�ô����ȡ�豸���˯������,�����ڵ���

SnoreData_def snoreData_temp;
DetailedSnoreData_Def DetailedSnoreData;
	
uint8_t debug_sleepdata_enable=0;
uint32_t snore_number=0;
uint32_t operation_number=0;


void Airsensor_Print_RTC_Time_Date(void)
{
	      int offset=0;
 	 
	      offset += sprintf(SendData+offset,"\n\n");
		    offset += sprintf(SendData+offset,"%2d ",snoreData_temp.StartTime[0]);
		    offset += sprintf(SendData+offset,"%2d ",snoreData_temp.StartTime[1]);	
		    offset += sprintf(SendData+offset,"%2d ",snoreData_temp.StartTime[2]);
	      offset += sprintf(SendData+offset,"%2d ",snoreData_temp.StartTime[3]);
		    offset += sprintf(SendData+offset,"%2d ",snoreData_temp.StartTime[4]);	
		    offset += sprintf(SendData+offset,"%2d ",0);		    	
//	      offset += sprintf(SendData+offset,"\n");		 
	      uart_send(SendData, strlen(SendData));
	
}


void Airsensor_Print_RTC_Time_Time(void)
{
	      int offset=0;
 	 
	 
		    offset += sprintf(SendData+offset,"%2d ",snoreData_temp.StartTime[0]);
		    offset += sprintf(SendData+offset,"%2d ",snoreData_temp.StartTime[1]);	
		    offset += sprintf(SendData+offset,"%2d ",snoreData_temp.StartTime[2]);
	      offset += sprintf(SendData+offset,"%2d ",snoreData_temp.StartTime[3]);
	      if(snoreData_temp.StartTime[4]!=0 || snoreData_temp.SSnoreTimes!=0 || snoreData_temp.MSnoreTimes!=0 || snoreData_temp.LSnoreTimes!=0)
				{
					offset += sprintf(SendData+offset,"%2d ",snoreData_temp.StartTime[4]);
		      offset += sprintf(SendData+offset,"%2d ",snoreData_temp.SSnoreTimes);	
		      offset += sprintf(SendData+offset,"%2d ",snoreData_temp.MSnoreTimes);
	        offset += sprintf(SendData+offset,"%2d ",snoreData_temp.LSnoreTimes);
					if(snoreData_temp.Airbag1Times!=0 || snoreData_temp.Airbag2Times!=0 || snoreData_temp.Airbag3Times!=0 || snoreData_temp.Airbag4Times!=0)
				  {
					  offset += sprintf(SendData+offset,"%2d ",snoreData_temp.Airbag1Times);
		        offset += sprintf(SendData+offset,"%2d ",snoreData_temp.Airbag2Times);	
		        offset += sprintf(SendData+offset,"%2d ",snoreData_temp.Airbag3Times);
	          offset += sprintf(SendData+offset,"%2d ",snoreData_temp.Airbag4Times);			
					  if(snoreData_temp.person_on_minute!=0 || snoreData_temp.person_on_second!=0 || snoreData_temp.person_off_minute!=0 || snoreData_temp.person_off_second!=0)
				    {
					    offset += sprintf(SendData+offset,"%2d ",snoreData_temp.person_on_minute);
		          offset += sprintf(SendData+offset,"%2d ",snoreData_temp.person_on_second);	
		          offset += sprintf(SendData+offset,"%2d ",snoreData_temp.person_off_minute);
	            offset += sprintf(SendData+offset,"%2d ",snoreData_temp.person_off_second);
            }							
				  }
					
				}
	    	
//	      offset += sprintf(SendData+offset,"\n");		 
	      uart_send(SendData, strlen(SendData));
	
}



void Airsensor_Print_RTC_Time_Blank(void)
{
	      int offset=0;		    	    	
	      offset += sprintf(SendData+offset,"\n");		 
	      uart_send(SendData, strlen(SendData));
	
}






void GetSnoreDataInSPIFlashDebug(void)
{
	   uint8_t tt=0;
	   if(debug_sleepdata_enable==1)
		 {
	     DetailedSnoreData.SnoreDataItemNumber=GetSnoreDataCnt();
			 if(DetailedSnoreData.SnoreDataItemNumber!=0)
			 {
	       ReadSnoreData(&snoreData_temp);
				 
				 if(snoreData_temp.StudyMode!=0x80)
				 {
					 Airsensor_Print_RTC_Time_Date();
					 osDelay(50);
					 tt=1;
				 }
				 else
				 {
					 Airsensor_Print_RTC_Time_Time();
					 osDelay(50);
           tt=2;
					 
//				   tt++;
				 }
			 }
			 else
			 {
				 debug_sleepdata_enable=0;
			 }
			 
		 }
		 if(debug_factoryreset_enable==1)
		 {
			 ResetToFactory();
			 debug_factoryreset_enable=0;
		 }
	
}
	
	


void TIM_wait_operation_finish(void)
{
	TIM7_operation_stepnum=0;
	TIM7_operation_finished_flag=0;
	HAL_TIM_Base_Start_IT(&htim7);
	while(TIM7_operation_finished_flag==0)
	{
		osDelay(1);
	}
	HAL_TIM_Base_Stop_IT(&htim7);
}


#if 0
void Switch_Valve_Do_Airbag_To_Air(unsigned char valve)
{
	//������������������ͨ������й��
	
	      Valve_Number_Operate = valve;
	
	      TIM7_operation_counter_time[0]=1;
			  TIM7_operation_code[0]=2;
			  TIM7_operation_counter_time[1]=Airbag_To_Air_PowerOff_Time;//461
			
			  TIM7_operation_code[1]=1;
	
			  TIM7_operation_counter_time[2]=Airbag_To_Air_PowerOn_Time;
			  TIM7_operation_code[2]=2;
			  TIM7_operation_counter_time[3]=4001;
			  TIM7_operation_code[3]=2;
			  TIM7_operation_code[4]=0;
			

//			  MIC_data_for_Walve_debug_pointer=0;
//			  MIC_data_save_enable=1;
	      MIC_data_save_enable=0;
			  TIM_wait_operation_finish();
//			  MIC_data_save_enable=0;
	
}

void Switch_Valve_Do_Airbag_To_Pump(unsigned char valve)
{
	//������������������ͨ���������ɽ��б������߳���
	
	      Valve_Number_Operate = valve;
	
	      TIM7_operation_counter_time[0]=1;
			  TIM7_operation_code[0]=1;
			  TIM7_operation_counter_time[1]=Airbag_To_Pump_PowerOn_Time;//461
		
			
			  TIM7_operation_code[1]=2;		
			
			
			  TIM7_operation_counter_time[2]=Airbag_To_Pump_PowerOff_Time;
			  TIM7_operation_code[2]=1;
			  TIM7_operation_counter_time[3]=4001;
			  TIM7_operation_code[3]=1;
			  TIM7_operation_code[4]=0;
			

//			  MIC_data_for_Walve_debug_pointer=0;
//			  MIC_data_save_enable=1;
        MIC_data_save_enable=0;
			  TIM_wait_operation_finish();
//			  MIC_data_save_enable=0;	
	
}
#endif

void Switch_Valve_Do_Airbag_To_Air(unsigned char valve)
{
	//������������������ͨ������й��
	      uint8_t tt;
	
	      Switch_Valve_Do_Airbag_To_Air_Sub(&Valve_Control,valve);
	      MIC_data_save_enable=0;
			  TIM_wait_operation_finish();
	      
	      tt=0;

	
}

void Switch_Valve_Do_Airbag_To_Pump(unsigned char valve)
{
	//������������������ͨ���������ɽ��б������߳���
	      uint8_t tt;
	      
	      Switch_Valve_Do_Airbag_To_Pump_Sub(&Valve_Control,valve);
	      
        MIC_data_save_enable=0;
			  TIM_wait_operation_finish();	
	
	      tt=0;
}





void INT_TIM7_Service_Code(void)
{
	
	Valve_Control.TIM7_operation_counter_time[TIM7_operation_stepnum]--;
	
	if(Valve_Control.TIM7_operation_counter_time[TIM7_operation_stepnum]==0)
	{
		switch(Valve_Control.TIM7_operation_code[TIM7_operation_stepnum])
	  {
		  case 0: 
			{
				break;
			}
			case 1:
			{
//				ValueDriver1(HEAD_R_1_VALUE,GPIO_PIN_SET); //���������
				ValueDriver1(Valve_Control.Valve_Number_Operate,GPIO_PIN_SET); //���������
//				InhalePump(1);
				break;
			}
			case 2:
			{
//				ValueDriver1(HEAD_R_1_VALUE,GPIO_PIN_RESET); //�޵����
				ValueDriver1(Valve_Control.Valve_Number_Operate,GPIO_PIN_RESET); //�޵����
//				InhalePump(0);
				
				break;
			}
			case 3:
			{
				
				break;
			}
			case 4:
			{
				break;
			}
			default:
				break;		  
	  }
		if(Valve_Control.TIM7_operation_code[TIM7_operation_stepnum+1]==0)
		{
			HAL_TIM_Base_Stop_IT(&htim7);
			TIM7_operation_finished_flag=1;
		}
		else
			TIM7_operation_stepnum++;
  }
}

//�Զ���ʼ���û�����ͱ����û���������
void Build_UserkeycodeAutoSet_Flag(void)
{
	/*
	UserkeycodeAutoSetData.flag = USERKEYCODE_FLAG;
	UserkeycodeAutoSetData.userkeycode1 = 0;
	UserkeycodeAutoSetData.userkeycode2 = 0;
	UserkeycodeAutoSetData.userkeycode3 = 0;
	UserkeycodeAutoSetData.userkeycode4 = 0;
	UserkeycodeAutoSetData.userkeytype = 0;
	      
	while(PositionInitWR((uint8_t*)&UserkeycodeAutoSetData,16,1)==0);
	*/
}

void update_userkeycode_autoset(void)
{
	   while(PositionInitWR((uint8_t*)&UserkeycodeAutoSetData,16,1)==0);	
}

void check_userkeycode_autoset(void)
{
	/*
	   while(PositionInitWR((uint8_t*)&UserkeycodeAutoSetData,16,0)==0);
	   if(UserkeycodeAutoSetData.flag==USERKEYCODE_FLAG)
			 return; 
		 Build_UserkeycodeAutoSet_Flag();
		 
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
	*/	 
}

//�Զ���ʼ���û���Ч�ںͱ����û���Ч��
void Build_validtimeAutoSet_Flag(void)
{
	UserkeycodeAutoSetData.validtimeflag = USERKEYCODE_FLAG;
	UserkeycodeAutoSetData.validtime_year = 99;
	UserkeycodeAutoSetData.validtime_month = 12;
	UserkeycodeAutoSetData.validtime_day = 31;
	      
	while(PositionInitWR((uint8_t*)&UserkeycodeAutoSetData,7,1)==0);
}

//ֻ���ڵ���
void claer_validtimeflag(void)
{
	UserkeycodeAutoSetData.validtimeflag = 0x00000000;
	UserkeycodeAutoSetData.validtime_year = 00;
	UserkeycodeAutoSetData.validtime_month = 00;
	UserkeycodeAutoSetData.validtime_day = 00;
	      
	while(PositionInitWR((uint8_t*)&UserkeycodeAutoSetData,7,1)==0);
}

void update_validtime_autoset(void)
{
	   while(PositionInitWR((uint8_t*)&UserkeycodeAutoSetData,7,1)==0);	
}

void check_validtime_autoset(void)
{
	   while(PositionInitWR((uint8_t*)&UserkeycodeAutoSetData,7,0)==0);
	   if(UserkeycodeAutoSetData.validtimeflag==USERKEYCODE_FLAG)
			 return; 
		 Build_validtimeAutoSet_Flag();
		 
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
		 
}

//Ϊ�����涨��V3.1.29
void Build_SensorLevelAutoSet_Flag(void)
{
	   SensorLevelAutoSetData.flag=SENSORLEVEL_HIGH_TO_MED_FLAG;
	   SensorLevelAutoSetData.times=SENSORLEVEL_HIGH_TO_MED_Times;
	      
	   while(PositionInitWR((uint8_t*)&SensorLevelAutoSetData,5,1)==0);
}

void Delete_SensorLevelAutoSet_Flag(void)
{
	   SensorLevelAutoSetData.flag=SENSORLEVEL_HIGH_TO_MED_FINISHED_FLAG;
	   SensorLevelAutoSetData.times=0;
	   while(PositionInitWR((uint8_t*)&SensorLevelAutoSetData,5,1)==0);	
}
void update_sensorlevel_autoset(void)
{
	   while(PositionInitWR((uint8_t*)&SensorLevelAutoSetData,5,1)==0);	
}
void check_sensorlevel_autoset(void)
{
	   while(PositionInitWR((uint8_t*)&SensorLevelAutoSetData,5,0)==0);
	   if(SensorLevelAutoSetData.flag==SENSORLEVEL_HIGH_TO_MED_FLAG || SensorLevelAutoSetData.flag==SENSORLEVEL_HIGH_TO_MED_FINISHED_FLAG)
			 return; 
		 Build_SensorLevelAutoSet_Flag();//����һ��������sensor highǿ�Ʋ���
		 
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
		 else
		 {
			 //�޸�sensor levelĬ��ֵΪ��
			 gStorageData.SensorLevel=MID_CHECK;
		   BackupSysPara(&gStorageData);
		 }
}

/*	old version
//Ϊ�����涨��V3.1.29
void Build_SensorLevelAutoSet_Flag(void)
{
	   SensorLevelAutoSetData.flag=SENSORLEVEL_HIGH_TO_MED_FLAG;
	   SensorLevelAutoSetData.days=SENSORLEVEL_HIGH_TO_MED_DAYS;
	   SensorLevelAutoSetData.checkposition=CHECK_POSITION_AT_SensorLevelAutoSet;
	   SensorLevelAutoSetData.timestart=STARTTIME_AT_SensorLevelAutoSet;
	   SensorLevelAutoSetData.timeend=ENDTIME_AT_SensorLevelAutoSet;
	   while(PositionInitWR((uint8_t*)&SensorLevelAutoSetData,9,1)==0);
}

void Delete_SensorLevelAutoSet_Flag(void)
{
	   SensorLevelAutoSetData.flag=SENSORLEVEL_HIGH_TO_MED_FINISHED_FLAG;
	   SensorLevelAutoSetData.days=0;
	   SensorLevelAutoSetData.checkposition=0;
	   SensorLevelAutoSetData.timestart=0;
	   SensorLevelAutoSetData.timeend=0;
	   while(PositionInitWR((uint8_t*)&SensorLevelAutoSetData,9,1)==0);	
}
void update_sensorlevel_autoset(void)
{
	   while(PositionInitWR((uint8_t*)&SensorLevelAutoSetData,9,1)==0);	
}
void check_sensorlevel_autoset(void)
{
	   while(PositionInitWR((uint8_t*)&SensorLevelAutoSetData,9,0)==0);
	   if(SensorLevelAutoSetData.flag==SENSORLEVEL_HIGH_TO_MED_FLAG || SensorLevelAutoSetData.flag==SENSORLEVEL_HIGH_TO_MED_FINISHED_FLAG)
			 return; 
		 Build_SensorLevelAutoSet_Flag();//����һ��������sensor highǿ�Ʋ���
		 
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
		 else
		 {
			 //�޸�sensor levelĬ��ֵΪ��
			 gStorageData.SensorLevel=MID_CHECK;
		   BackupSysPara(&gStorageData);
		 }
}

*/

    



#endif
