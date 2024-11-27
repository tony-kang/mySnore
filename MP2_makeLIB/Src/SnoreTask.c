#include "tuwan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"
#include "I2s.h"
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
volatile uint8_t  Timer10msOn   = 0;

void SnoreTaskFunction(void const * argument)
{
	static int cnt=0;
    static int cnt1=0;
    int SnoreFlag;
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
           SnoreFlag = CheckSnoreSub(buf_snore,&SnoreMaxVol);


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

            if((SnoreFlag==1)||(SnoreFlag==2))
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
                    if(SnoreFlag==1) 
                    {
                        PillowControl.SnoreOn = true;
                    }
                }
                
                //--------------------
                if(PillowControl.SnoreOn == true)
                {
                    cnt = (10*1000/32);
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
    }
		
		if((TimerTick % 10) == 0)
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
