#include "tuwan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"
#include "I2s.h"
extern unsigned char AppTest;
extern unsigned int recordTick;
extern unsigned char StartSend,SnoreDetectTime,StartAllAction;
extern int CheckSnoreSub(short int * snore,uint8_t *SnoreMaxVol);
extern float PVE(short int *WaveData);
extern uint8_t PillowMaxOne_Willbesend,Body_Left_Right_Willbesend;
extern uint8_t AirSensor_PersonOn_init_snore_flag;
extern QDef  SnoreQ;
short buf_snore[256];  
extern uint8_t AirSensor_PersonOn;
volatile uint32_t TimerTick     = 0; 
volatile uint32_t AirSensor_Personon_Timer    = 0;
volatile uint32_t Led_OnOff_Timer    = 0;
volatile uint8_t  Timer20msOn   = 0;
volatile uint8_t LEDFlashTimer20msOn=0;
volatile uint8_t  Timer10msOn   = 0;
extern unsigned int snore_times_every_sleep;
extern SemaphoreHandle_t xTimeMute;
extern uint8_t latestsnoretime_minute,latestsnoretime_hour;
extern uint8_t workdelay_time_cycle_flag;
extern SnoreTime_def SnoreTime_forAPP;
int SnoreFlag;
extern uint8_t ValidTimeOkFlag;

void SnoreTaskFunction(void const * argument)
{
	static int cnt=0;
    static int cnt1=0;
//    int SnoreFlag;
    uint8_t SnoreMaxVol;
	while(StartAllAction==0)
		 osDelay(10);
	while(1)
	{	   	
        //HAL_IWDG_Refresh(&hiwdg);
        //------------------
	    if(SnoreQ.in!=SnoreQ.out)
    	{
            //outputDataFromQDef(&dat,&PositionQ)>0)//
    		outputDataFromQDef((unsigned char *)buf_snore,&SnoreQ);
    		recordTick++;
            //||||||||||||||||
				
//		原始的直接调用鼾声处理	    
//           SnoreFlag = CheckSnoreSub(buf_snore,&SnoreMaxVol);
				
				
//  2022年5月10日修改，位置检测时屏蔽鼾声识别
#if 1				
			if(MIC_data_stop_detect_flag== 0)
			{
				SnoreFlag = CheckSnoreSub(buf_snore,&SnoreMaxVol);
			}				
			else
			{
				SnoreFlag=0;
			}
#endif		
/*				
//    modifed by zjp
//    增加对是否人的判断，如果有人则调用正常的鼾声处理，如果无人，则调用音量的处理以便test mode下检测MIC的显示

            				
				    if(AirSensor_PersonOn==1)	
            {							
				      if(AirSensor_PersonOn_init_snore_flag==0)
							  SnoreFlag = CheckSnoreSub(buf_snore,&SnoreMaxVol);
							else
							{
								AirSensor_PersonOn_init_snore_flag=0;
								SnoreInit();
							}
						}
						else
							PVE(buf_snore);
//				  SnoreFlag = 1;   //added by zjp 临时屏蔽鼾声检测或者强制设置检测到鼾声
*/

/*
#ifdef TUWAN_BLANKET
            if(Body_Left_Right_Willbesend != 0x00)
#else
            if(PillowMaxOne_Willbesend    != 0x10)
#endif
            {
                if(stimestructure.Hours < 6)
                {
                    if( (cnt1 > 0) && (PillowControl.SnoreOn == false) && (SnoreFlag==2) )
                    {
                        SnoreFlag = 1;
                    }
                }               
            }
*/

            //---------------------------------------------

//            if((SnoreFlag==1)||(SnoreFlag==2))
            if(SnoreFlag==1)							
            {
/*                
                if(gStorageData.SensorLevel == LOW_CHECK) 
                {
                    if((SnoreFlag==1) && (SnoreMaxVol > 50))
                    {
                        PillowControl.SnoreOn = true;
                    }
                }
                else
*/                    
                {
//                    if(SnoreFlag==1 || SnoreFlag==2) //add SnoreFlag by zjp
									  snore_times_every_sleep++;
								#if 0	
									  while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
					          {
                      osDelay(1);
					          }	
									  latestsnoretime_minute=stimestructure.Minutes;
					          latestsnoretime_hour=stimestructure.Hours;										
										xSemaphoreGive( xTimeMute );
                #endif
//										Airsensor.onehour_snore_checkposition_flag=1;
										Airsensor.onehour_snore_checkposition_counter=0;
//										if((SnoreFlag==1||SnoreFlag==2) && AppCtrlBag.enable==0 && ReadPos.enable==0 && AppTest==0) 	//20201116added flag==2
                    if(SnoreFlag==1 && AppCtrlBag.enable==0 && ReadPos.enable==0 && AppTest==0 && workdelay_time_cycle_flag == 0) 	//20201116added flag==2											
                    {
											if(gStorageData.StudyMode==0 && ValidTimeOkFlag == 1)
											{
                        PillowControl.SnoreOn = true;												
											}
											else
											{
												PillowControl.SnoreOn = 0;
											}
                    }
										if(SnoreFlag==1 && AppCtrlBag.enable==0 && ReadPos.enable==0 && AppTest==0) 	//20230714											
                    {
											SnoreTime_forAPP.year = sdatestructure.Year;
											SnoreTime_forAPP.month = sdatestructure.Month;
											SnoreTime_forAPP.day = sdatestructure.Date;
											SnoreTime_forAPP.hour = stimestructure.Hours;
											SnoreTime_forAPP.minute = stimestructure.Minutes;
											SnoreTime_forAPP.second = stimestructure.Seconds;
											SnoreTime_forAPP.valid = 1;
											
                    }
										
                }
                
                //--------------------
                if(PillowControl.SnoreOn == true)
                {
                    cnt = (15*1000/32);  //标志有效10秒 (10*1000/32);
                }

            }
            //-------------------------
            if(cnt1 > 0)
            {
                cnt1 --;
            }
            //-------------------------
            if(cnt > 0)
            {
                cnt--;
                if(cnt==0)
                {
                    PillowControl.SnoreOn = false;
                    cnt1 = (2*60*1000/32);   //没有鼾声后60秒内再来鼾声就启动
                }
            } 
						
					#ifdef AUTO_RUN	
						if(stimestructure.Hours>3&&(stimestructure.Minutes<40))
							PillowControl.SnoreOn = true;
					#endif
						

            
            //||||||||||||||||
        }
		osDelay(1);
        //------------------
	}

}



void SysTickIntHandler(void)
{    //static int a;
//	  HAL_IncTick();//added by zjp 14 20211029
    TimerTick++;
	  AirSensor_Personon_Timer++;
	  Led_OnOff_Timer++;
    if (TimerTick >= 60000)
    {
        TimerTick = 0;
    }
    
    if((TimerTick % 20) == 0)
    {
        Timer20msOn = 1;
			  LEDFlashTimer20msOn=1;
    }
		
		if((TimerTick % 100) == 0)
    {
        Timer10msOn = 1;
    }
    
    //a++;
    //   if(a==1000)
    //   	 PowerLed(1);
    //   else if(a==2000)
   	//{
    //   	PowerLed(0);
    //   	a=0;
   	//}



}
