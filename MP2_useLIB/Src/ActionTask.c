#include "cmsis_os.h"
#include "tuwan.h"
#include  <string.h>
#include <pillow.h>
#include "position.h"

extern unsigned ShowModeOn;

Read_Pos_def  AppCtrlBag;
void AppControlAirBag(unsigned char w);
unsigned int PressureADC_ConvertedValue[6],InhaleADC_ConvertedValue[6],ExhaleADC_ConvertedValue[6],AutoStartTimer=0,ExhaleTime=0,InhaleTime=0;
unsigned char step=0,StartAllAction=0,ADok2=0,ADok3=0,ADok1=0,postionBak=0,ActionSteps=5,StartCountAntSnoreTime=0,NeedReturnToDefault=0,StartAction=0,needWork=0;
unsigned short step4time=0,step6time=0;
ProcessControl_def ProcessControl;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
extern unsigned char SnoreDetectTime,TestMode;
void ResetValuePump(void);
void SetAllValuePump(void);
extern RTC_TimeTypeDef stimestructure;
extern RTC_DateTypeDef sdatestructure;

SemaphoreHandle_t xTimeMute = NULL;
SemaphoreHandle_t xPositionMute = NULL;
SemaphoreHandle_t xLogoMute = NULL;

extern RTC_HandleTypeDef hrtc;
RTC_TimeTypeDef stimestructure_snore;
RTC_DateTypeDef sdatestructure_snore;
SnoreData_def detailed_snore;

uint8_t pump_operation_time[20][13];//20����ÿ����һ���ֽ�Ϊ0��������Ч��1��������Ч
uint8_t pump_operation_time_write_pointer=0,pump_operation_time_read_pointer=0;//��¼�ͱ���λ��ָ��

extern HeadPosition_def  position,LastHeadPosition;
extern xQueueHandle  ExhalAdcQHandle,InhalAdcQHandle;
extern unsigned char AppTest,tempAirBag,Side_Sleep,tempholdingtime;
extern uint8_t Body_Left_Right_Confirmed;
extern uint8_t AirSensor_Position_Changed;
unsigned char ErrorCode,RequestControlValve=0;
unsigned char    ACTION_TYPE=1;                 //1:���������в����������ֱ�ӳ䵽����     2.���������м������������ͣ��ֹͣ�����������������������  3.���������м������������ͣ����������  4.̧��ͷ��

extern uint8_t Position_State_Before_Recheck;
extern uint8_t rechecktime_minute,rechecktime_hour;
extern unsigned int Antisnore_count_timer;


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{  
	static unsigned char i=0,j=0,k=0;
	unsigned int ExhaleAD,InhaleAD,AirPressure;
	if(hadc==&hadc1)
  {  ExhaleAD = ADC1->DR;
		 if(ADok1==0)
		 {
		 ExhaleADC_ConvertedValue[i]=ExhaleAD; 
		 i++;
		 if(i==1)  //5
			{
			 ADok1=1;
				i=0;
			}
	   }
	}
	else if(hadc==&hadc2)
	{ InhaleAD = ADC2->DR;
		 if(ADok2==0)
		 {
		 InhaleADC_ConvertedValue[j]=InhaleAD; 
		 j++;
		 if(j==1)  //5
			{
			 ADok2=1;
			 j=0;
			}
	   }
	}
/*
  { InhaleAD = ADC2->DR;
		 if(ADok2==0)
		 {
		 InhaleADC_ConvertedValue[j]=InhaleAD; 
		 j++;
		 if(j==1)  //5
			{
			 ADok2=1;
				j=0;
			}
	   }
	}
*/
   else if(hadc==&hadc3)
  { 		AirPressure = ADC3->DR;
		 if(ADok3==0)
		 {
				PressureADC_ConvertedValue[k]=AirPressure; 
				k++;
			  #ifdef TUWAN_BLANKET
				if(k==5) //5
				{
					ADok3=1;
					k=0;
				}
				#else
				if(k==1) //5
				{
					ADok3=1;
					k=0;
				}
				#endif
	  }
   }
   
}
int ActionAndCheckNoSnore(int ms)
{
    int i;
   for(i=0;i<(ms/1000);i++)
	 {
	    if((PillowControl.SnoreOn==true)||(AppTest))
				return (ms/1000)-i;
	    osDelay(1000);
	 }
	 return 0;
}
int ActionAndCheckSnore(int ms)
{
    int i;
   for(i=0;i<(ms/1000);i++)
	 {
	    if((PillowControl.SnoreOn==false))
				return (ms/1000)-i;
	    osDelay(1000);
	    ExhaleTime++;
	 }
	 return 0;
}
void KeepAir(void)
{	
    SetAllValuePump();
}

void SetAllValuePump(void)
{
	ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
	osDelay(500);
	ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
	osDelay(500);
	//ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
	//osDelay(500);
 	//ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
	//osDelay(500);
	ValueDriver(BODY_R_VALUE,GPIO_PIN_SET);
	osDelay(500);
	ValueDriver(BODY_L_VALUE,GPIO_PIN_SET);
	osDelay(500);
	ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);

  	InhalePump(0);
	ExhalePump(0);



}
/*
 * �л�����Ӧ������λ�ó���
 * */
void SwitchInhale(unsigned char position)
{
  if(position==BODY_RIGHT)
  {
	  ValueDriver(BODY_L_VALUE,GPIO_PIN_SET);
	  osDelay(500);
	  ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);
	  osDelay(500);
	  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
	  osDelay(500);
	  ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);
	  osDelay(500);
		 ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
	    osDelay(500);
	    ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
	    osDelay(500);
	  InhalePump(1);

  }
  else if(position==BODY_LEFT)
  {
	  ValueDriver(BODY_R_VALUE,GPIO_PIN_SET);
	  osDelay(500);
	  ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);
	  osDelay(500);
	  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
	  osDelay(500);
	  ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);
	  osDelay(500);
		 ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
	    osDelay(500);
	    ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
	    osDelay(500);
	  InhalePump(1);

  }
  else
  {
	  	ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
	  	ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
	  	ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
	  	ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
	  	ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);
	  	ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);
	  	ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
	    InhalePump(0);
	  	ExhalePump(0);
  }
}


/*
 * �л�����Ӧ������λ�÷���
 * */
void SwitchExhale(unsigned char position)
{
  if(position==BODY_RIGHT)
  {
	  ValueDriver(BODY_L_VALUE,GPIO_PIN_SET);
	  osDelay(500);
	  ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);
	  osDelay(500);
	  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
	  osDelay(500);
	  ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
	  osDelay(500);
	  InhalePump(1);
  }
  else if(position==BODY_LEFT)
  {
	  ValueDriver(BODY_R_VALUE,GPIO_PIN_SET);
	  osDelay(500);
	  ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);
	  osDelay(500);
	  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
	  osDelay(500);
	  ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
	  osDelay(500);
	  InhalePump(1);
  }
  else
  {
	  	ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
	  	//ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
	  	//ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
	   	//ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
	  	ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);
	  	ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);
	  	ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
	    InhalePump(1);
  }
}

void KeepAirBody(unsigned char position)
{
	if(position==BODY_RIGHT)
	  {
		  ValueDriver(BODY_R_VALUE,GPIO_PIN_SET);
		  osDelay(500);
		  ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);
		  osDelay(500);
		  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
		  osDelay(500);
		  ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
		  osDelay(500);
		  InhalePump(0);
	  }
	  else if(position==BODY_LEFT)
	  {
		  ValueDriver(BODY_L_VALUE,GPIO_PIN_SET);
		  osDelay(500);
		  ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);
		  osDelay(500);
		  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
		  osDelay(500);
		  ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
		  osDelay(500);
		  InhalePump(0);
	  }
}



void ResetValuePump(void)
{
	ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
	
	ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
	
	ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
		
	ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
		
	ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);
	
	ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);
		
	ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
		
	InhalePump(0);
	ExhalePump(0);
}

void ExhaleAir(unsigned char pos)
{
	if(pos==1)
	{
		ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);

		ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);

		ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);

		ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);
	
		
	}
	else
	{
	ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);
	
	ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);

	ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
	
	ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
	}
	
	
	ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
	
	InhalePump(0);
	
	ExhalePump(1);
	osDelay(500);
}




void ExhaleAirHead(void)
{
	ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
	
	ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);

	ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
	
	ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);

	ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
	InhalePump(0);
	ExhalePump(1);
	osDelay(500);
}

void ExhaleAirBody(void)
{
	ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);

	ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);

	ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);

	InhalePump(0);

	ExhalePump(1);
	osDelay(500);
}

void StopAction(void)
{
	SwitchInhale(BODY_NO);
	osDelay(1000);
}

void Bubble_Sort(unsigned int *num, int n)
{
    int i, j;
    for(i = 0; i < n; i++)
    {
        for(j = 0; i + j < n - 1; j++)
        {
            if(num[j] > num[j + 1])
            {
                int temp = num[j];
                num[j] = num[j + 1];
                num[j + 1] = temp;
            }
           // Print(num, n);
        }
    }
    return;
}

void CalculateMaxMin(unsigned int *pt,unsigned int *max,unsigned int *min,int cnt)
{
		Bubble_Sort(pt, cnt);
	  
    *max=pt[cnt-1];
	  *min=pt[0]; 


}

unsigned int ExMax,ExMin,Exa[11],ExaTemp[11],InMax,InMin,Ina[11],InaTemp[11],baseADC=0;

unsigned char CalculateExhaleOverRun(bool CheckSnore)
{
	unsigned int j=0,total,x,n;
	osDelay(500);	
	ADok1=0;
	ExMax=0;
	ExMin=0;
	baseADC=0;
	while(1)
	{ 
		HAL_ADC_Start_IT(&hadc1);
		osDelay(10);
	  if(ADok1)
		{   //CalculateMaxMin(ExhaleADC_ConvertedValue,&ExMax,&ExMin,5);
			//  if((ExMax-ExhaleADC_ConvertedValue[1])<(ExhaleADC_ConvertedValue[1]/2))
				{				
							if(ExhaleADC_ConvertedValue[1]<0x10)
							{	 ADok1=0;
							
								 continue;
							}
							CalculateMaxMin(ExhaleADC_ConvertedValue,&x,&n,5);
								
					   // total=ExhaleADC_ConvertedValue[0]+ExhaleADC_ConvertedValue[1]+ExhaleADC_ConvertedValue[2]+
					   //       ExhaleADC_ConvertedValue[3]+ExhaleADC_ConvertedValue[4];
				      //total/=5;
							total=x;
							
							if(total>0x300)
								return 2;
							else if(total<0x10)
								return 2;
							if(baseADC==0)
								baseADC=ExhaleADC_ConvertedValue[3];
							if(ExMax<total)
							{	ExMax=total;
								
								ExMin=0;
								
							}
							else
							{
								ExMin++;
								if(ExMin>(100*7))
								{
									 if((ExMax-baseADC)>((baseADC/2)+(0x10)))
                   {
										 baseADC=ExMax-0x40;
										 return 2;
									 }		
										else			ExMin=0;							 
								
								}
								
							 }
					
					    //xQueueSend( ExhalAdcQHandle, ( void* )&total, 0 );  
				}
			//	else  
			//		total=0;
			 ADok1=0;
				/*
			if(uxQueueSpacesAvailable(ExhalAdcQHandle)==0)
			{
						xQueueReceive(ExhalAdcQHandle, &Exa[0], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[1], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[2], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[3], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[4], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[5], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[6], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[7], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[8], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[9], 10);
				
				    memcpy((unsigned char *)ExaTemp,(unsigned char *)Exa,40);
			
			      CalculateMaxMin(ExaTemp,&ExMax,&ExMin,10);
				    Exa[10]=ExaTemp[9]-ExaTemp[0];
				
						if((ExaTemp[9]-ExaTemp[0])>((ExaTemp[0]/3)+0x10))
						{  baseADC=ExaTemp[9];
							return 2;
						}
						else 
						{
							 xQueueSend( ExhalAdcQHandle, ( void* )&Exa[1], 0 );  
							 xQueueSend( ExhalAdcQHandle, ( void* )&Exa[2], 0 );  
							 xQueueSend( ExhalAdcQHandle, ( void* )&Exa[3], 0 );  
							 xQueueSend( ExhalAdcQHandle, ( void* )&Exa[4], 0 );  
							 xQueueSend( ExhalAdcQHandle, ( void* )&Exa[5], 0 );  
							 xQueueSend( ExhalAdcQHandle, ( void* )&Exa[6], 0 );  
							 xQueueSend( ExhalAdcQHandle, ( void* )&Exa[7], 0 );  
							 xQueueSend( ExhalAdcQHandle, ( void* )&Exa[8], 0 );  
							 xQueueSend( ExhalAdcQHandle, ( void* )&Exa[9], 0 );  
						}
			      
			}
			*/
		}
			
		j++;
		if(j>60*100*7) //10·???³¬?±
			break;
		
		
		
  }
	//uart_printf("EX OVER TIMER\r\n");
	 baseADC=ExMax;
	return 0;
}


unsigned char CalculateExhaleOverStart(bool CheckSnore,unsigned char type)
{
	unsigned int total;
	osDelay(200);	
	ADok1=0;
	while(1)
	{ 
		HAL_ADC_Start_IT(&hadc1);
		osDelay(10);
	  if(ADok1)
		{   CalculateMaxMin(ExhaleADC_ConvertedValue,&ExMax,&ExMin,5);
			 // if((ExMax-ExhaleADC_ConvertedValue[2])<(ExhaleADC_ConvertedValue[2]/2))
				{				
							total=ExhaleADC_ConvertedValue[1]+ExhaleADC_ConvertedValue[2]+
					          ExhaleADC_ConvertedValue[3]+ExhaleADC_ConvertedValue[4];
				      total/=4;
					    if(total>0x300)
							{
								ExhaleTime=0;
								return 2;
								
							}
					    xQueueSend( ExhalAdcQHandle, ( void* )&total, 0 );  
				}
			 ADok1=0;
			if(uxQueueSpacesAvailable(ExhalAdcQHandle)==0)
			{
						xQueueReceive(ExhalAdcQHandle, &Exa[0], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[1], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[2], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[3], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[4], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[5], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[6], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[7], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[8], 10);
						xQueueReceive(ExhalAdcQHandle, &Exa[9], 10);
				
				    memcpy((unsigned char *)ExaTemp,(unsigned char *)Exa,40);
			
			      CalculateMaxMin(ExaTemp,&ExMax,&ExMin,10);
				    //if((ExaTemp[0]>baseADC-0x30)&&(ExaTemp[1]>baseADC-0x30)&&(ExaTemp[2]>baseADC-0x30)&&(ExaTemp[3]>baseADC-0x30)&&(ExaTemp[4]>baseADC-0x30)
						//	 &&(ExaTemp[5]>baseADC-0x30)&&(ExaTemp[6]>baseADC-0x30)&&(ExaTemp[7]>baseADC-0x30)&&(ExaTemp[8]>baseADC-0x30))
						if((ExaTemp[6]>gStorageData.ExhaleOver-0x30)&&(ExaTemp[7]>gStorageData.ExhaleOver-0x30)&&(ExaTemp[8]>gStorageData.ExhaleOver-0x30)&&(gStorageData.ExhaleOver!=0))
				    {	 
							
							ExhaleTime=0;   
							return 2;
						} 
			}
			
		}
			
		ExhaleTime++;
		if(type==1)
		{
		if(ExhaleTime>60*100*6) //10·???³¬?±
			break;
	  }
		else 
		{
		if(ExhaleTime>60*100) //30?ë
			break;
	  }
		if(CheckSnore&&(TestMode==0))
		{
			 if(PillowControl.SnoreOn==false)
		      return 1;
		
		}
		
  }
	//uart_printf("EX OVER TIMER\r\n");
	/// baseADC=ExMax;
	ExhaleTime=0;
	gStorageData.ExhaleOver=ExMax;
	ErrorCode|=(1<<ERROR_CODE_EXHALE_OVER);//������ʱ
	BackupSysPara(&gStorageData);
	return 0;
}


unsigned char CalculatePressureOver(bool CheckSnore,unsigned char type)
{
	unsigned int total,time=0;
	osDelay(500);	
	ADok3=0;
	ExhaleTime=0;
	while(1)
	{ 
		HAL_ADC_Start_IT(&hadc3);
		osDelay(20);
	  if(ADok3)
		{  CalculateMaxMin(PressureADC_ConvertedValue,&ExMax,&ExMin,5);
			 ADok3=0;
			 if((PressureADC_ConvertedValue[0]<EXHALE_PRESURE_VALUE)&&(PressureADC_ConvertedValue[1]<EXHALE_PRESURE_VALUE)&&(PressureADC_ConvertedValue[2]<EXHALE_PRESURE_VALUE)&&(PressureADC_ConvertedValue[3]<EXHALE_PRESURE_VALUE))
			 {
				 time++;
				 if(time>20)
					return 1;
			 }
			 else time=0;
		}
		ExhaleTime++;
#if defined(TUWAN_PILLOW_3) || defined(TUWAN_MAT)
		if(ExhaleTime>60*50) //1���ӳ�ʱ
			break;
#else
		if(ExhaleTime>60*50*10) //6���ӳ�ʱ
			break;
#endif
		if(ExhaleTime==60)
			ExhalePump(1);
	
			
	}
	return 0;
}


unsigned char CalculateExhaleOverPressure(bool CheckSnore,unsigned char type)
{
	unsigned int total,time=0;
	osDelay(500);	
	ADok3=0;
	PressureADC_ConvertedValue[0]=0;
	PressureADC_ConvertedValue[1]=0;
	PressureADC_ConvertedValue[2]=0;
	PressureADC_ConvertedValue[3]=0;
	PressureADC_ConvertedValue[4]=0;
	while(1)
	{ 
		HAL_ADC_Start_IT(&hadc3);
		osDelay(200);
	  	if(ADok3)
		{   	total=(PressureADC_ConvertedValue[0]+PressureADC_ConvertedValue[1]+PressureADC_ConvertedValue[2]+PressureADC_ConvertedValue[3]+PressureADC_ConvertedValue[4]
				)/5;
			// xQueueSend( ExhalAdcQHandle, ( void* )&total, 0 ); 
			 ADok3=0;
			if(total<EXHALE_PRESURE_VALUE)
			{	
				if((type==1)&&(ExhaleTime>InhaleTime/2))
				{
					time++;
					osDelay(1000);
					if(time>11)
					return 2;
				}
				else if(type==0)
					return 2;
			}
			else time=0;
		}
		/*
		if(uxQueueSpacesAvailable(ExhalAdcQHandle)==0)
		{
			xQueueReceive(ExhalAdcQHandle, &Exa[0], 10);
			xQueueReceive(ExhalAdcQHandle, &Exa[1], 10);
			xQueueReceive(ExhalAdcQHandle, &Exa[2], 10);
			xQueueReceive(ExhalAdcQHandle, &Exa[3], 10);
			xQueueReceive(ExhalAdcQHandle, &Exa[4], 10);
			xQueueReceive(ExhalAdcQHandle, &Exa[5], 10);
			xQueueReceive(ExhalAdcQHandle, &Exa[6], 10);
			xQueueReceive(ExhalAdcQHandle, &Exa[7], 10);
			xQueueReceive(ExhalAdcQHandle, &Exa[8], 10);
			xQueueReceive(ExhalAdcQHandle, &Exa[9], 10);
				
			if((Exa[0]<9)&&(Exa[1]<9)&&(Exa[2]<9)&&(Exa[3]<9)&&(Exa[4]<9)&&(Exa[5]<9)&&(Exa[6]<9)&&(Exa[7]<9)&&(Exa[8]<9)&&(Exa[9]<9))
			{	
				if((type==1)&&(ExhaleTime>100*30))
				{
					time++;
					osDelay(1000);
					if(time>11)
					return 2;
				}
				else return 2;
			}
			else time=0;
		}
		*/
		ExhaleTime++;
		if(type==1)
		{
			if(ExhaleTime>60*5*10) //10·???³¬?±
			break;
	 	 }
		
		if(CheckSnore&&(TestMode==0))
		{
			 if(PillowControl.SnoreOn==false)
		     		 return 1;
		}
  }
	ExhaleTime=0;
	ErrorCode|=(1<<ERROR_CODE_EXHALE_OVER);//������ʱ
	return 0;
}




unsigned char CalculateInhaleOverPressure(bool CheckSnore,unsigned char type)
{
	unsigned int total,time=0;
	osDelay(500);	
	ADok3=0;
	PressureADC_ConvertedValue[0]=0;
	PressureADC_ConvertedValue[1]=0;
	PressureADC_ConvertedValue[2]=0;
	PressureADC_ConvertedValue[3]=0;
	PressureADC_ConvertedValue[4]=0;
	
	while(1)
	{ 
		HAL_ADC_Start_IT(&hadc3);
		osDelay(200);
	  	if(ADok3)
		{   
			total=(PressureADC_ConvertedValue[0]+PressureADC_ConvertedValue[1]+PressureADC_ConvertedValue[2]+PressureADC_ConvertedValue[3]+PressureADC_ConvertedValue[4])/5;
			 //xQueueSend( InhalAdcQHandle, ( void* )&total, 0 ); 			
			  ADok3=0;

			 if(((gStorageData.ActionLevel==0)&&((InhaleTime>5*60*4)&&(type==1)))||((gStorageData.ActionLevel==0)&&(type==0)))
			{    	if((PressureADC_ConvertedValue[0]>(INHALE_PRESURE_VALUE-0X40))&&(PressureADC_ConvertedValue[1]>(INHALE_PRESURE_VALUE-0X40))&&(PressureADC_ConvertedValue[2]>(INHALE_PRESURE_VALUE-0X40))&&(PressureADC_ConvertedValue[3]>(INHALE_PRESURE_VALUE-0X40))&&(PressureADC_ConvertedValue[4]>(INHALE_PRESURE_VALUE-0X40)))//µ?
				{		
					time++;
					if(time>2)
						return 2;
				}
				else time=0;
			}
			else 	if(((gStorageData.ActionLevel==1)&&((InhaleTime>5*60*4)&&(type==1)))||((gStorageData.ActionLevel==1)&&(type==0)))
			{    	if((PressureADC_ConvertedValue[0]>(INHALE_PRESURE_VALUE-0X30))&&(PressureADC_ConvertedValue[1]>(INHALE_PRESURE_VALUE-0X30))&&(PressureADC_ConvertedValue[2]>(INHALE_PRESURE_VALUE-0X30))&&(PressureADC_ConvertedValue[3]>(INHALE_PRESURE_VALUE-0X30))&&(PressureADC_ConvertedValue[4]>(INHALE_PRESURE_VALUE-0X30)))//??
				{		
					time++;
					if(time>2)
					return 2;
				}
				else time=0;
			}
			else	if(((gStorageData.ActionLevel==2)&&((InhaleTime>5*60*4)&&(type==1)))||((gStorageData.ActionLevel==2)&&(type==0)))
			{   		
				if((PressureADC_ConvertedValue[0]>INHALE_PRESURE_VALUE)&&(PressureADC_ConvertedValue[1]>INHALE_PRESURE_VALUE)&&(PressureADC_ConvertedValue[2]>INHALE_PRESURE_VALUE)&&(PressureADC_ConvertedValue[3]>INHALE_PRESURE_VALUE)&&(PressureADC_ConvertedValue[4]>INHALE_PRESURE_VALUE))//¸?
				{		
					time++;
					if(time>2)
						return 2;
				}
						else time=0;
			} 
			 
		}	
		/*
		if(uxQueueSpacesAvailable(InhalAdcQHandle)==0)
		{
					xQueueReceive(InhalAdcQHandle, &Ina[0], 10);
					xQueueReceive(InhalAdcQHandle, &Ina[1], 10);
					xQueueReceive(InhalAdcQHandle, &Ina[2], 10);
					xQueueReceive(InhalAdcQHandle, &Ina[3], 10);
					xQueueReceive(InhalAdcQHandle, &Ina[4], 10);
					xQueueReceive(InhalAdcQHandle, &Ina[5], 10);
					xQueueReceive(InhalAdcQHandle, &Ina[6], 10);
					xQueueReceive(InhalAdcQHandle, &Ina[7], 10);
					xQueueReceive(InhalAdcQHandle, &Ina[8], 10);
					xQueueReceive(InhalAdcQHandle, &Ina[9], 10);
				 
				    	if(((gStorageData.ActionLevel==0)&&((InhaleTime>100*60*4)&&(type==1)))||((gStorageData.ActionLevel==0)&&(type==0)))
					{    	if((Ina[0]>0x2d5)&&(Ina[1]>0x2d5)&&(Ina[2]>0x2d5)&&(Ina[3]>0x2d5)&&(Ina[4]>0x2d5)&&(Ina[5]>0x2d5))//µ?
						{		
							time++;
							if(time>2)
								return 2;
						}
						else time=0;
					}
					else 	if(((gStorageData.ActionLevel==1)&&((InhaleTime>100*60*4)&&(type==1)))||((gStorageData.ActionLevel==1)&&(type==0)))
					{    	if((Ina[0]>0x2e0)&&(Ina[1]>0x2e0)&&(Ina[2]>0x2e0)&&(Ina[3]>0x2e0)&&(Ina[4]>0x2e0)&&(Ina[5]>0x2e0))//??
						{		
							time++;
							if(time>2)
							return 2;
						}
						else time=0;
					}
					else	if(((gStorageData.ActionLevel==2)&&((InhaleTime>100*60*4)&&(type==1)))||((gStorageData.ActionLevel==2)&&(type==0)))
					{   		
						if((Ina[0]>0x310)&&(Ina[1]>0x310)&&(Ina[2]>0x310)&&(Ina[3]>0x310)&&(Ina[4]>0x310)&&(Ina[5]>0x310))//¸?
						{		
							time++;
							if(time>2)
								return 2;
						}
						else time=0;
					} 
		}
		*/
	  InhaleTime++;
	  if(InhaleTime>(60*5*8)) //7·???³¬?±
				break;
//	 if((type==1))
//	{
//			 if(InhaleTime==(5*100))
//			 {
				 // ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
				 // osDelay(300);
	//			  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
			// }
	//}

	if(CheckSnore&&(TestMode==0))
	{
			 if(PillowControl.SnoreOn==false)
		      return 1;
		
	}

	if(AirSensor_Position_Changed&&(AppTest==0))
	{
		NeedReturnToDefault=1;
		AirSensor_Position_Changed=0;
		return 2;

	}
		
 }
	ErrorCode|=(1<<ERROR_CODE_INHALE_OVER);//������ʱ
	return 0;
}


unsigned char CalculateExhaleOver(bool CheckSnore,unsigned char type)
{
	#ifdef PRESSURE_CONTROL
	 return CalculateExhaleOverPressure(CheckSnore,type);
	#endif
	osDelay(200);	
	ADok1=0;
	while(1)
	{   osDelay(10);
		ExhaleTime++;
		if(gStorageData.ActionLevel==0)
		{
			if(ExhaleTime>15*100) //15?ë
				break;
		}
		else if(gStorageData.ActionLevel==1)
		{
			if(ExhaleTime>30*100) //30?ë
				break;
		}
		else if(gStorageData.ActionLevel==2)
		{
			if(ExhaleTime>50*100) //30?ë
				break;
		}
		if(CheckSnore)
		{
			 if(PillowControl.SnoreOn==false)
		      return 1;
		}
  }
	ExhaleTime=0;
	return 0;
}




unsigned char CalculateInhaleOver(bool CheckSnore,unsigned char type)
{
  	#ifdef PRESSURE_CONTROL
	 return CalculateInhaleOverPressure(CheckSnore,type);
	#endif
	osDelay(900);	
	ADok2=0;
	while(1)
	{ 
		osDelay(10);
		InhaleTime++;
		if(gStorageData.ActionLevel==0)
		{
			if(InhaleTime>15*100) //15?ë³¬?±
				break;
		}
		else if(gStorageData.ActionLevel==1)
		{
			if(InhaleTime>25*100) //25?ë³¬?±
				break;
		}
		else if(gStorageData.ActionLevel==2)
		{
			if(InhaleTime>50*100) //50?ë³¬?±
				break;
		}
		if(CheckSnore)
		{
			 if(PillowControl.SnoreOn==false)
		      return 1;
		}
  }
	InhaleTime=0;
	return 0;
}


void InhaleAtFirst(void)
{

	ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);
	osDelay(500);
	InhalePump(1);
	osDelay(5000);
	InhalePump(0);
	
}

void ExhaleBodyHead(void)
{
	ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);

	ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
	
	ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);

	ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);

	ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);

	ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);

	ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);

	InhalePump(0);
	#ifndef PRESSURE_CONTROL
	ExhalePump(1);
	#endif
		osDelay(100);
	//osDelay(500);



}


void TestPressurePosition()
{
	SwitchInhale(BODY_NO);
	osDelay(10000);

	/*
	ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);
	 osDelay(500);
	 ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);
	 osDelay(500);
	 ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
	 osDelay(500);
	 ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
	 osDelay(500);
	InhalePump(1);
	osDelay(2000);
	osDelay(2000);
	osDelay(2000);
	osDelay(2000);
	osDelay(2000);
	osDelay(2000);
	osDelay(2000);
	osDelay(2000);
	osDelay(2000);
	*/
	//SwitchInhale(BODY_RIGHT);
	ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);
	osDelay(500);
	ValueDriver(BODY_R_VALUE,GPIO_PIN_SET);
	osDelay(500);
	ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
	osDelay(500);
	ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);
	osDelay(500);
	InhalePump(1);



	 //osDelay(2500);
	 osDelay(5000);
	 SwitchInhale(BODY_NO);
	 ValueDriver(BODY_R_VALUE,GPIO_PIN_SET);
	 ValueDriver(BODY_L_VALUE,GPIO_PIN_SET);
	 ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
	 //////////////////////////////////////////////////

	    ValueDriver(BODY_L_VALUE,GPIO_PIN_SET);
	 	osDelay(500);
	 	ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);
	 	osDelay(500);
	 	ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
	 	osDelay(500);
	 	ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);
	 	osDelay(500);
	 	InhalePump(1);



	 	 //osDelay(2500);
	 	 osDelay(5000);
	 	 SwitchInhale(BODY_NO);
	 	 ValueDriver(BODY_R_VALUE,GPIO_PIN_SET);
	 	 ValueDriver(BODY_L_VALUE,GPIO_PIN_SET);
	 	 ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);

	 while(1)
	 {
	 		HAL_ADC_Start_IT(&hadc3);
	 		osDelay(20);
	 		if(ADok3)
	 		{  CalculateMaxMin(PressureADC_ConvertedValue,&ExMax,&ExMin,5);
	 			 ADok3=0;

	 		}


	 }




}

#ifdef TUWAN_BLANKET

void ActionTaskFunction(void const * argument)
{
  /* USER CODE BEGIN ActionTaskFunction */
	unsigned char mode=0,headRunTime=0,ti=0;
  	unsigned short t2=0;
	
	
  /* Infinite loop */
	StartAllAction=0;
 	while(StartAllAction==0)
		 osDelay(10);

	ActionSteps=5;
	
	
	//TestPressurePosition();

    #ifdef PRESSURE_CONTROL
		StartAction=1;
		ExhaleBodyHead();
		SwitchExhale(BODY_NO);
		ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
		osDelay(500);
		ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
		osDelay(500);
	    CalculatePressureOver(0,1);
	    ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
	    osDelay(500);
	    ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
	    osDelay(500);
  	#else 
		//CalculateInhaleOver(false);
		if((gStorageData.ExhaleOver==0))
		{
			ResetValuePump();
			InhaleAtFirst();	
			ExhaleBodyHead();
			CalculateExhaleOverRun(false);
			gStorageData.ExhaleOver=baseADC;
			BackupSysPara(&gStorageData);
		//gStorageData.InhaleOver=baseADC;
		}
		else
		{    
		  	StartAction=1;
				ExhaleBodyHead();
	    	CalculateExhaleOverStart(false,1);
		  	StartAction=0;
		}
	#endif
	
	InhalePump(0);
	ExhalePump(0);
	
	ActionSteps=0;
	StartCountAntSnoreTime=0;
	InhaleTime=0;
	ExhaleTime=0;
	PillowControl.PillowAction=false;
	ProcessControl.on=false;
  for(;;)
  { 		
  	osDelay(100);
		if(gStorageData.StudyMode==2)
		{
  	if(AppTest)
  	{
  		AppControlAirBag(tempAirBag);
  		AppTest=0;
  		osDelay(3000);
  	}

  	if(ShowModeOn)
  	{
  		AppControlAirBag(3);
  		//AppControlAirBag(1);
				ShowModeOn=0;

  	}
  	continue;
	}
	 if((step!=0)&&(gStorageData.Standby==0)&&(PillowControl.SnoreOn==false)&&(Body_Left_Right_Confirmed)&&(TestMode==0)) 
	 {
			ti++;	//ȷ�����Ѿ��뿪����Ҫ�ָ�����Ϊ����״̬
       		if(ti>60)
			 NeedReturnToDefault=1;
	}
	else ti=0;
		 
	if(NeedReturnToDefault)
	{
			HeadPosition=position;
			SwitchExhale(BODY_NO);
			ActionSteps=12;  //head & body air bag exhale
			ProcessControl.on=true;
			ProcessControl.tick=0;
			ExhaleTime=0;
			CalculateExhaleOver(false,1);
			PillowControl.PillowAction=false;
			NeedReturnToDefault=0;
			PillowControl.SnoreOn=false;
			AutoStartTimer=0;
			ActionSteps=0;
			step=0;
			ProcessControl.on=false;
			ProcessControl.tick=0;
			InhaleTime=0;
			osDelay(2000);
	}

	if((PillowControl.SnoreOn==false)&&(gStorageData.Standby==0)&&(HeadPosition.body==BODY_NO)&&(TestMode==0)&&(AppTest==0)&&(Body_Left_Right_Confirmed)&&(stimestructure.Hours>5))
	{        //���Ѿ��뿪���ҷ�����Ϻ��򱣴汾�ε�˯�����ݣ����л�����״̬Ϊ����
				t2++;
		if(t2>40)
		{
				t2=0;
				if( gStorageData.Standby==0)  //��������״̬,���Ӷ����ݵĴ洢����
				{
//           gStorageData.Snore.HeadMovingTimes= Airsensor.AirSensor_PersonOn;
					 WriteSnoreData(&gStorageData.Snore); //�����ϴ�
//					 gStorageData.Standby=1;
	 			}
		}



	}
	else t2=0;

	  if(((HeadPosition.body!=BODY_NO)&&(TestMode==0)&&(AppTest==0))||(TestMode&&(AppTest==0)&&(HeadPosition.body!=BODY_NO)))
   	{
		if(gStorageData.Standby&&(TestMode==0)&&(AppCtrlBag.enable==0))
		{
				if(AutoStartTimer<300)
				{  
					AutoStartTimer++;
					continue;
				}
				InhaleTime=0;	
				ExhaleTime=0;
				gStorageData.Snore.UsedRecord=0x55aa;
				gStorageData.Snore.InterfereTimes=0;
//				gStorageData.Snore.ApneaTimes=0;
//				gStorageData.Snore.HeadMovingTimes=0;
				gStorageData.Snore.StudyMode=gStorageData.StudyMode;
				
				gStorageData.Snore.LSnoreTimes=0;
				gStorageData.Snore.SSnoreTimes=0;
				gStorageData.Snore.MSnoreTimes=0;
				gStorageData.Snore.ActionNoPosition=0;
				gStorageData.Snore.PositionTime=0;
				gStorageData.Snore.ActionRunTime=0;


				gStorageData.Snore.StartTime[0]=sdatestructure.Year;
				gStorageData.Snore.StartTime[1]=sdatestructure.Month;
				gStorageData.Snore.StartTime[2]=sdatestructure.Date;
				gStorageData.Snore.StartTime[3]=stimestructure.Hours;
				gStorageData.Snore.StartTime[4]=stimestructure.Minutes;
				
				gStorageData.Snore.EndTime[0]=0;
				gStorageData.Snore.EndTime[1]=0;
				gStorageData.Snore.EndTime[2]=0;
				gStorageData.Snore.EndTime[3]=0;
				gStorageData.Snore.EndTime[4]=0;

				ProcessControl.tick=0;
				ProcessControl.on=false;
//				gStorageData.Standby=0;
				
				step=0;
				
				
		}	
		else 		AutoStartTimer=0;		

		if((PillowControl.SnoreOn==false)||(gStorageData.Standby))
			continue;
		if(gStorageData.StudyMode==1)
					continue;
    	PillowControl.PillowAction=true;
		gStorageData.Snore.InterfereTimes++;
		if(step==0)
		{   				
						ProcessControl.on=true;
						if((gStorageData.Standby)&&(TestMode==0))
							continue;
						step=4;
						ProcessControl.tick=0;
						continue;
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

						if(TestMode)
						{
							step=4;
							ProcessControl.tick=0;
							continue;
						}
						ExhaleTime=0;
						ActionSteps=1;  //head air bag inhale
						osDelay(1000);
						
						
						switch(HeadPosition.body)
						{
								case BODY_RIGHT:
									ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);
									osDelay(500);
									ValueDriver(BODY_L_VALUE,GPIO_PIN_SET);
									 osDelay(500);
									ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);
									  osDelay(500);
									 ExhalePump(0);
									InhalePump(1);									  
									break;	
								case BODY_LEFT:
									ValueDriver(BODY_R_VALUE,GPIO_PIN_SET);
									osDelay(500);
									ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);
									osDelay(500);
									ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);
									osDelay(500);
									ExhalePump(0);
									InhalePump(1);	
										break;
								default:
									PillowControl.PillowAction=false;
									continue;
							}
							osDelay(300);
							  StartCountAntSnoreTime=1;
						#ifdef ACTION_CHECK
							if(CalculateInhaleOver(true,0)==1)
						    //if(ActionAndCheckSnore(1000*25)!=0)  //inhale 25 seconds
							{
									//StopAction();  //¼ì²âµ½÷ý?ù
									//SetAllValuePump();
								InhalePump(0);
								ActionSteps=2;  //head air bag inhale process,but no snore
								StartCountAntSnoreTime=1;
								continue;
							}
						#else
							CalculateInhaleOver(false,0);
						#endif
						step++;
						ProcessControl.on=false;
						ProcessControl.tick=0;
								
		}
		if(step==1)  
		{ 	
						if(gStorageData.Standby)
							continue;
						
						ActionSteps=3;  //head air bag inhale completed
						ProcessControl.on=true;
						  //KeepAir();
						InhalePump(0);
						ExhalePump(0);
						if(HeadPosition.body==BODY_RIGHT)
						{
							ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
							ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);
							ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);
							ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);	
						}
						else
						{
							//ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
							//ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
							//ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
							//ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
							ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);
							ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);
							ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
						  }	
						  StartCountAntSnoreTime=1;
						  if(ActionAndCheckSnore(1000*20)!=0)  //KEEP AIR 20 seconds
						  {
								  InhalePump(0);
								  ExhalePump(0);
								  ActionSteps=4;  //head air bag inhale completed,but no snore

								 continue;
						 }
						step++;
						ProcessControl.on=false;
						ProcessControl.tick=0;
							
			}
						
			if(step==2)                           //exhale air 25 seconds
			{ 
					if(gStorageData.Standby)
						continue;
					step++;
					ActionSteps=5;  //exhale air bag 
					ExhaleAirHead();
					osDelay(500);
					ExhaleTime=0;
					CalculateExhaleOver(false,0);
						  
			}
						
			if(step==3)                          ////KEEP AIR 20 seconds
			{ 
				InhaleTime=0;
				if(gStorageData.Standby)
					continue;
				StopAction();
				ActionSteps=6;  //exhale air bag completed
				step4time=0;
				// te=HeadPosition.head2;
				PillowControl.PillowAction=false; 
				if(ActionAndCheckSnore(1000*20)!=0)  
				{   
					PillowControl.PillowAction=false; 
					StopAction();
					StartCountAntSnoreTime=1;
					//  ActionSteps=7;  //exhale air bag completed,but no snore
					continue;
				}
				step++;
				osDelay(1000);
				mode=0;
				PillowControl.PillowAction=false; //????¼ì²â?·²¿º??í?åµ??»??
				osDelay(5000);
				PillowControl.PillowAction=true;
				InhaleTime=0;
				ProcessControl.on=false;
				ProcessControl.tick=0;
				//	StopAction();
							  
			}
			if(step==4)    //�����ͷ��ͬʱ����
			{  
				if(gStorageData.Standby)
					continue;	
				StartCountAntSnoreTime=1;
				ExhaleTime=0;
				ActionSteps=8;  //head & body air bag inhale
				ProcessControl.on=true;
				osDelay(1000);
				switch(HeadPosition.body)
				{
					case BODY_RIGHT:
						SwitchInhale(BODY_RIGHT);
						postionBak=1;
						break;
					case BODY_LEFT:
						SwitchInhale(BODY_LEFT);
						postionBak=2;
						break;
					default:
						break;
				}
				headRunTime=0;
				#ifdef ACTION_CHECK
					if(CalculateInhaleOver(true,1)==1)
					{
						StartCountAntSnoreTime=1;
						if(HeadPosition.body==BODY_RIGHT)
						{
							KeepAirBody(BODY_RIGHT);
						}
						else if(HeadPosition.body==BODY_LEFT)
						{
							KeepAirBody(BODY_LEFT);
						}
						InhalePump(0);
						ExhalePump(0);
						ActionSteps=9;  //head & body air bag inhale,but no snore
						continue;
					}
				#else
					CalculateInhaleOver(false,1);
				#endif

				step++;
				ProcessControl.on=false;
				ProcessControl.tick=0;	
				if(NeedReturnToDefault)
					continue;
			}
			if(step==5)  
			{ 
				if(gStorageData.Standby&&(TestMode==0))
					continue;
				if(HeadPosition.body==BODY_RIGHT)
					KeepAirBody(BODY_RIGHT);
				else KeepAirBody(BODY_LEFT);
				step6time=0;
#ifdef SKIP_HEAD_BODY_KEEP_AIR
				continue;
#endif
				ActionSteps=10;  //head & body air bag inhale completed
				ProcessControl.on=true;

				StartCountAntSnoreTime=1;
				if(ActionAndCheckSnore(1000*1*40)!=0)  //KEEP AIR 40 seconds
				{
					if(HeadPosition.body==BODY_RIGHT)
						KeepAirBody(BODY_RIGHT);
					else KeepAirBody(BODY_LEFT);
						ActionSteps=11;  //head & body air bag inhale completed,but no snore
					continue;
				}

				step++;	
				ExhaleTime=0;
				ProcessControl.on=false;
				ProcessControl.tick=0;	
			}
						
			if(step==6)  
			{
				if(gStorageData.Standby&&(TestMode==0))
					continue;
				SwitchExhale(BODY_NO);
				ActionSteps=12;  //head & body air bag exhale
				ProcessControl.on=true;
				#ifdef ACTION_CHECK
					if(CalculateExhaleOver(true,1)==1)
					{   StartCountAntSnoreTime=1;
						StopAction();
						ActionSteps=13;  //head & body air bag is exhaleing ,but no snore
						continue;
					}
				#else
					CalculateExhaleOver(false,1);
				#endif
				step++;
				ProcessControl.on=false;
				ProcessControl.tick=0;	
			}
						
			if(step==7)  
			{	
				if(gStorageData.Standby&&(TestMode==0))
					continue;
				StopAction();
				step=0;
				ActionSteps=14;  //head & body air bag exhale complete
				InhaleTime=0;
				if(ActionAndCheckSnore(1000*40)!=0)  //KEEP AIR 40 seconds
				{
					StartCountAntSnoreTime=1;
					StopAction();
					step=4;
					PillowControl.PillowAction=false;
					continue;
				}
				SnoreDetectTime=0;
				PillowControl.PillowAction=false;
				StopAction();
				osDelay(1000*3);
				step=0;
				ActionSteps=0;  //no work,waiting 
				continue;
			}
			if(step>7)
           	{
				if(gStorageData.Standby)
					continue;
				PillowControl.PillowAction=false;
				ResetValuePump();
				ActionSteps=0;  //no work,waiting 
				ProcessControl.on=false;
				ProcessControl.tick=0;	
				continue;
			}							
		}
	 /*
		else if(TestMode&&(AppTest==0))
		{
			HeadPosition=position;
			ActionSteps=0; 
			gStorageData.ActionLevel=2;
			if(HeadPosition.body==1)
			{
				ActionSteps=1;
				SetAllValuePump();
				osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);
				osDelay(500);
				InhalePump(1);
				ExhalePump(0);
				InhaleTime=0;
				osDelay(2000);
				CalculateInhaleOver(false,0);
				InhalePump(0);
				osDelay(200);
				ExhalePump(0);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
				osDelay(200);					
				ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);
				InhalePump(1);
				InhaleTime=0;
				osDelay(2000);
				CalculateInhaleOver(false,1);
				InhalePump(0);
				ExhalePump(0);					
				 ActionSteps=3; 
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
				osDelay(5000);
				ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ExhalePump(1);
				ActionSteps=5; 
				osDelay(500);
				ExhaleTime=0;
				CalculateExhaleOver(false,1);
				ExhalePump(0);
				ResetValuePump();
				osDelay(2000);
				ActionSteps=0; 
						  
			}
						
			else if(HeadPosition.body==3)
			{				
				ActionSteps=1; 
				SetAllValuePump();
				osDelay(500);			
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);
				osDelay(500);
				InhalePump(1);
				ExhalePump(0);
				InhaleTime=0;
				osDelay(2000);
				CalculateInhaleOver(false,0);
							
				InhalePump(0);
				ExhalePump(0);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
				osDelay(500);
							      //if(HeadPosition.body==1)
										//	ValueDriver(BODY_R_VALUE,GPIO_PIN_RESET);
										//else if(HeadPosition.body==3)
				ValueDriver(BODY_L_VALUE,GPIO_PIN_RESET);
										
										
				InhaleTime=0;
				InhalePump(1);
				osDelay(2000);
				CalculateInhaleOver(false,1);
				InhalePump(0);
				ExhalePump(0);
										
				ActionSteps=3; 
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
				osDelay(5000);
				ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ExhalePump(1);
				ActionSteps=5; 
				ExhaleTime=0;
				osDelay(500);
				CalculateExhaleOver(false,1);
				ExhalePump(0);
				ResetValuePump();
				osDelay(2000);
				ActionSteps=0; 
						  
			}
						
		
		
		
		
		}
		*/
		else if(AppTest)
		{

			RequestControlValve=1;
			while(1)
			{

				if(RequestControlValve==2)
					   break;
				osDelay(100);
			}

			AppControlAirBag(tempAirBag);
			 AppTest=0;
			 RequestControlValve=0;
			 osDelay(3000);

		}
		osDelay(500);
		 
  }
  /* USER CODE END ActionTaskFunction */
}



void AppControlAirBag(unsigned char w)
{
		switch(w)
		{
			case 1:
				ActionSteps=1;
				SetAllValuePump();
				osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);
				osDelay(500);
				InhalePump(1);
				ExhalePump(0);
				InhaleTime=0;
				osDelay(2000);
				CalculateInhaleOver(false,0);
				InhalePump(0);
				ExhalePump(0);
				osDelay(5000);
				ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ExhalePump(1);
				ActionSteps=5;
				ExhaleTime=0;
				osDelay(500);
				CalculateExhaleOver(false,1);
				ExhalePump(0);
				ResetValuePump();
				osDelay(2000);
				ActionSteps=0;
				break;
			case 2:
				ActionSteps=1;
				SetAllValuePump();
				osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(EXHALE_VALUE,GPIO_PIN_SET);
				osDelay(500);
				InhalePump(1);
				ExhalePump(0);
				InhaleTime=0;
				osDelay(2000);
				CalculateInhaleOver(false,0);
				InhalePump(0);
				ExhalePump(0);
				osDelay(5000);
				ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ExhalePump(1);
				ActionSteps=5;
				ExhaleTime=0;
				osDelay(500);
				CalculateExhaleOver(false,1);
				ExhalePump(0);
				ResetValuePump();
				osDelay(2000);
				ActionSteps=0;
				break;
			case 3:
				ActionSteps=1;
				SwitchInhale(BODY_LEFT);
				osDelay(2000);
				InhaleTime=0;
				CalculateInhaleOver(false,0);
				KeepAirBody(BODY_LEFT);
				ActionSteps=3;
				osDelay(5000);
				ActionSteps=5;
				SwitchExhale(BODY_LEFT);
				ExhaleTime=0;
				CalculateExhaleOver(false,1);
				SwitchInhale(BODY_NO);
				ActionSteps=0;
				osDelay(5000);
				break;
			case 4:
				ActionSteps=1;
				SwitchInhale(BODY_RIGHT);
				osDelay(2000);
				InhaleTime=0;
				CalculateInhaleOver(false,0);
				KeepAirBody(BODY_RIGHT);
				ActionSteps=3;
				osDelay(5000);
				ActionSteps=5;
				SwitchExhale(BODY_RIGHT);
				ExhaleTime=0;
				CalculateExhaleOver(false,1);
				SwitchInhale(BODY_NO);
				ActionSteps=0;
				osDelay(5000);
				break;
			default:
				break;
		}






}


#else    //����ֹ����������

void request_position_stop_work(void)
{	
		ActionSteps=1;
		RequestControlValve=1;
		while(1)
		{

		    	 if(RequestControlValve==2 || RequestControlValve==3)
		    		 break;
		    	 osDelay(10);
		 }	
}

extern uint8_t SendData[60];
void Airsensor_Print_RTC_Time_Actiontask(void)
{
	      int offset=0;
 	 
	 
		    offset += sprintf(SendData+offset,"%2d ",sdatestructure_snore.Year);
		    offset += sprintf(SendData+offset,"%2d ",sdatestructure_snore.Month);	
		    offset += sprintf(SendData+offset,"%2d ",sdatestructure_snore.Date);
	      offset += sprintf(SendData+offset,"%2d ",stimestructure_snore.Hours);
		    offset += sprintf(SendData+offset,"%2d ",stimestructure_snore.Minutes);	
		    offset += sprintf(SendData+offset,"%2d ",stimestructure_snore.Seconds);		    	
	      offset += sprintf(SendData+offset,"\n");		 
	      uart_send(SendData, strlen(SendData));
	
}

void Airsensor_Print_RTC_Time_Actiontask1(void)
{
	      int offset=0;		    	    	
	      offset += sprintf(SendData+offset,"%2d ",sdatestructure_snore.Year);
		    offset += sprintf(SendData+offset,"%2d ",sdatestructure_snore.Month);	
		    offset += sprintf(SendData+offset,"%2d ",sdatestructure_snore.Date);
	      offset += sprintf(SendData+offset,"%2d ",stimestructure_snore.Hours);
		    offset += sprintf(SendData+offset,"%2d ",stimestructure_snore.Minutes);	
		    offset += sprintf(SendData+offset,"%2d | ",stimestructure_snore.Seconds);	
	      if(pump_operation_time_write_pointer==0)
				{
					offset += sprintf(SendData+offset,"%2d ",19);	
          offset += sprintf(SendData+offset,"%2d ",pump_operation_time[19][0]); 
				}
				else
				{
	       offset += sprintf(SendData+offset,"%2d ",pump_operation_time_write_pointer-1);	
         offset += sprintf(SendData+offset,"%2d ",pump_operation_time[pump_operation_time_write_pointer-1][0]); 
				}					
	      offset += sprintf(SendData+offset,"\n\n");		 
	      uart_send(SendData, strlen(SendData));
					
	
}



void ActionTaskFunction(void const * argument)
{
  /* USER CODE BEGIN ActionTaskFunction */
	unsigned char mode=0,headRunTime=0;
  	unsigned int t2=0,ti=0,temp,aTime=0;
	uint8_t action_head_position = 0;
	
	RTC_TimeTypeDef stimestructure_temp;	
  RTC_DateTypeDef sdatestructure_temp;
	
  /* Infinite loop */
	//StartAllAction=0;
	ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
	osDelay(500);
	ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
	osDelay(500);
	ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
	osDelay(500);
	ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
	osDelay(500);
	InhalePump(0);
	ExhalePump(0);
	

 	while(StartAllAction==0)
		 osDelay(10);
	
	

	if(gStorageData.StudyMode==2)
	{
		
		request_position_stop_work();
/*		
		ActionSteps=1;
		osDelay(3000);
		RequestControlValve=1;
		
		while(1)
		{

		    	 if(RequestControlValve==2)
		    		 break;
		    	 osDelay(100);
		 }
		
*/
	}
	else
		ActionSteps=0;


	osDelay(1000);
	StartCountAntSnoreTime=0;
	InhaleTime=0;
	ExhaleTime=0;
	PillowControl.PillowAction=false;
	ProcessControl.on=false;

  for(;;)
  {
        osDelay(100);
        
        if(gStorageData.StudyMode==2) //��ʾģʽ
        {
					  request_position_stop_work();
            ActionSteps=1;
            AppControlAirBag(1);
            ActionSteps=0;
        }

        if(gStorageData.StudyMode==2)
        {
					  request_position_stop_work();
            ActionSteps=1;
            AppControlAirBag(2);
            ActionSteps=0;
        }

        
        if(gStorageData.StudyMode==2)
        {
					  request_position_stop_work();
            ActionSteps=1;
            AppControlAirBag(3);
            ActionSteps=0;
        }


        if(gStorageData.StudyMode==2)
        {
					  request_position_stop_work();
            ActionSteps=1;
            AppControlAirBag(4);
            ActionSteps=0;
            continue;
        }

        
#if 0
  	if((step==0)&&(PillowControl.SnoreOn==true)&&(AppTest==0)&&(ReadPos.enable==0))  //��������Ҳ�϶�����
  	{
			if((gStorageData.Standby==0)&&(gStorageData.StudyMode))   //StudyMode 0:����ģʽ 1��ѧϰģʽ 2����ʾģʽ
				{
				
				}
				else
				{
					//if((AutoStartTimer>=(gStorageData.WorkDelay*10*60))||(gStorageData.Standby==0))
					//{	
					
					  //����ͷ��λ�ü��
						Airsensor.AirSensor_Checkposition_Mode=ONCE_TIME;
						while(Airsensor.AirSensor_Checkposition_Mode==ONCE_TIME)
						  osDelay(100);
						osDelay(350);
					//}
				}
  	}
#endif		
#if 0		//disabled by zjp at 20190505 �����ܳ���������� 
	 if((step!=0)&&(gStorageData.Standby==0)&&(PillowControl.SnoreOn==false)&&(HeadPosition.head1==HEAD_NO)&&(TestMode==0))
	 {
		 ti++;
		 if((ti>10*60*31))
			 NeedReturnToDefault=1;
	}
	else ti=0;

	if(NeedReturnToDefault)
	{
			ActionSteps=5;
			ResetValuePump();
			ExhaleTime=0;
			osDelay(PILLOW_AIROFFTIME);
			PillowControl.PillowAction=false;
			NeedReturnToDefault=0;
			PillowControl.SnoreOn=false;
			AutoStartTimer=0;
			ActionSteps=0;
			step=0;
			ProcessControl.on=false;
			ProcessControl.tick=0;
			InhaleTime=0;
			osDelay(2000);
	}

#endif 
	
#if 0 //disabled by zjp at 20190505 û��ʱ���ٱ����������ݺͼ��λ��
	if((gStorageData.Standby==0)&&(PillowControl.SnoreOn==false)&&(AppTest==0))//&&(stimestructure.Hours>5))
	{
			t2++;
			if((t2>10*60*30))//&&((stimestructure.Hours>=8)&&(stimestructure.Hours<22)))  //30������������������Ϊ����״̬
			{
				 t2=0;
				 if( gStorageData.Standby==0)  //��������״̬,���Ӷ����ݵĴ洢����
				 {
//		  			gStorageData.Standby=1;
				 }
			}
	}
	else t2=0;	
#endif 

	
#if 0	//�رո��� 20210312
	if(RECHECK_NO_DELAY_FLAG==1)
	{
		//����ʱ��������
		if((PillowControl.SnoreOn==false)&&(Airsensor.personon_by_positioncheck_flag==1)&&(AppTest==0))//&&(stimestructure.Hours>5))			
	  {
		  if(Airsensor.AirSensor_step_stable_flag==1 && Airsensor.AirSensor_step_last_stable_flag==1)
			{
					 while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				   {
              osDelay(1);
				   }
				   Airsensor.AirSensor_Checkposition_Mode=0;
           xSemaphoreGive( xPositionMute );
					 
					 while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
					 {
              osDelay(1);
					 }	
					 rechecktime_minute=stimestructure.Minutes;
					 rechecktime_hour=stimestructure.Hours;										
					 xSemaphoreGive( xTimeMute );
					 
					 
				   Airsensor.AirSensor_Check_Mode=5;				
				
 			     Position_State_Before_Recheck=Airsensor.AirSensor_PersonOn;//���渴��ǰ���Ƿ�����״̬�����������Ƚ�
					 
           if(RECHECK_NO_DELAY_FLAG==1)
				     Airsensor.personon_by_positioncheck_flag=0;
			}
			
		}
	}
	else
	{
	
	  //���һ�ε��ж�Ϊ��������03λ�ü���жϵģ���������������10�������������������Ҫ���²���03λ�ü���ٴ�ȷ��һ���Ƿ������ڴ�
	
    //  if((gStorageData.Standby==1)&&(PillowControl.SnoreOn==false)&&(Airsensor.personon_by_positioncheck_flag==1)&&(AppTest==0))//&&(stimestructure.Hours>5))	
    if((PillowControl.SnoreOn==false)&&(Airsensor.personon_by_positioncheck_flag==1)&&(AppTest==0))//&&(stimestructure.Hours>5))			
	  {
			t2++;
			if((t2>AirSensor_recheck_person_onoff_time*60*10))  //��־��Чһ��ʱ����ؼ�ȷ��        //&&((stimestructure.Hours>=8)&&(stimestructure.Hours<22)))  //10����������				
			{
				 t2=0;
				 //����λ�ü�⣬�ٴ�ȷ���Ƿ�����
				 if(Airsensor.AirSensor_step_stable_flag==1)
				 {
					 while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				   {
              osDelay(1);
				   }
				   Airsensor.AirSensor_Checkposition_Mode=0;
           xSemaphoreGive( xPositionMute );
					 
					 while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
					 {
              osDelay(1);
					 }	
					 rechecktime_minute=stimestructure.Minutes;
					 rechecktime_hour=stimestructure.Hours;										
					 xSemaphoreGive( xTimeMute );
					 
					 
				   Airsensor.AirSensor_Check_Mode=5;				
				
 			     Position_State_Before_Recheck=Airsensor.AirSensor_PersonOn;//���渴��ǰ���Ƿ�����״̬�����������Ƚ�

//				   Airsensor.personon_by_positioncheck_flag=0;
				 }
				
				
/*				
				 ActionSteps = 0x05;//��ס���ý���������ģʽ
         AirSensor_fast_check_person_poweron();					
				 Airsensor.AirSensor_Checkposition_Mode=0;
				
				
				 Airsensor.AirSensor_Check_Mode=5;
				 ActionSteps = 0x00;
				 Airsensor.personon_by_positioncheck_flag=0;
				
*/				
				
//				 Airsensor.AirSensor_Checkposition_Mode=ONCE_TIME;
//				 while(Airsensor.AirSensor_Checkposition_Mode==ONCE_TIME)
//				 osDelay(100);				
			}
	  }
	  else t2=0;	
	}
	
#endif	
	
	
#if 0	
	//added by zjp at 20190505 
	//***********************************************************************************************************
	if((gStorageData.Standby==0)&&(Airsensor.AirSensor_PersonOn==0)&&(AppTest==0))//&&(stimestructure.Hours>5))
	{
			
				 if( gStorageData.Standby==0)  //��������״̬,���Ӷ����ݵĴ洢����
				 {
					  //��ȡʱ�䣬���浱ǰ����
					 
					  
					  
					 
//					  WriteSnoreData(&gStorageData.Snore); //�����ϴ�
//		  			gStorageData.Standby=1;
//					  gStorageData.Standby=0; 
				 }
			
	}	
	
	//��ʱ���ӣ�MP2 gStorageData.Standbyʼ��Ϊ0������״̬
	if(gStorageData.Standby==1)
//		gStorageData.Standby=0;

#endif
	
	//************************************************************************************************************
	
	if((AppTest==0)&&(AppCtrlBag.enable==0))
    gStorageData.Snore.HeadPosion=HeadPosition.head1;
	
#if 0	
	
	if((AppTest==0)&&(AppCtrlBag.enable==0)&&(ReadPos.enable==0)&&(Airsensor.AirSensor_PersonOn==1)&&(gStorageData.Standby==1))		
  {	
		      //���˺��л�Ϊ����״̬
//		      InhaleTime=0;
//					ExhaleTime=0;
//					memset(&gStorageData.Snore,0,sizeof(SnoreData_def)); //Ϊ����������Ҳ���������ʱ�ر�
//					gStorageData.Snore.UsedRecord=0x55aa;
//					gStorageData.Snore.StudyMode=gStorageData.StudyMode;
//					gStorageData.Standby=0;
//					ProcessControl.tick=0;
//					ProcessControl.on=false;
//					step=0;		
	}
#endif
	
  if((AppTest==0)&&(AppCtrlBag.enable==0)&&(ReadPos.enable==0)&&(PillowControl.SnoreOn==true) && Airsensor.period_snore_checkposition_flag==0 && Airsensor.Tt_Is_Sleep_Time==1)
//  if((AppTest==0)&&(AppCtrlBag.enable==0)&&(ReadPos.enable==0)&&(PillowControl.SnoreOn==true) && Airsensor.period_snore_checkposition_flag==0 && Airsensor.AirSensor_PersonOn==1)
  {	
		
			//if(gStorageData.workTimeOn==0)
			//{
		
//		 if((gStorageData.Standby==0)&&(gStorageData.StudyMode))   //StudyMode 0:����ģʽ 1��ѧϰģʽ 2����ʾģʽ
//		 {
//				continue;
//		 }
		 
		 //�޸���2020.06.08
		 if(gStorageData.StudyMode==1 || gStorageData.StudyMode==2)   //StudyMode 0:����ģʽ 1��ѧϰģʽ 2����ʾģʽ
		 {
				continue;
		 }
		
		 //Ϊ������12��00������Ҳ���λ����ʱ�򿪣�ԭ���رյ�
		
		 #if 1 //disabled by zjp at 20190505 �Ƶ�������д���
				if(gStorageData.Standby)
				{
					//if(AutoStartTimer<(gStorageData.WorkDelay*10*60))
					//{
					//	AutoStartTimer++;
					//	continue;
				//}
					InhaleTime=0;
					ExhaleTime=0;
//					memset(&gStorageData.Snore,0,sizeof(SnoreData_def)); //Ϊ����������Ҳ���������ʱ�ر�
					gStorageData.Snore.UsedRecord=0x55aa;
					gStorageData.Snore.StudyMode=gStorageData.StudyMode;
//					gStorageData.Standby=0;
					ProcessControl.tick=0;
					ProcessControl.on=false;
					step=0;
				}
			#endif 	
				
				//else 	AutoStartTimer=0;
			//}
				//������20200608
//			 if(gStorageData.Standby)
//				continue;
#if 0			 
			 if(HeadPosition.head1==HEAD_NO)
						continue;
#endif	

#if 0			 
			if(PillowControl.SnoreOn==false)
				continue;
#endif
			
//			if(gStorageData.StudyMode==1)
//				continue;
			PillowControl.PillowAction=true;
		
			
			if(ActionSteps==0)
			{
				if(Side_Sleep)
				{
					ACTION_TYPE=4;

				}
				else ACTION_TYPE=1;
			}

			if(step==0)
			{
						ProcessControl.on=true;
				    
				    while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				    {
              osDelay(1);
				    }
				
				    Airsensor.AirSensor_Checkposition_Mode=ONCE_TIME;
						
						xSemaphoreGive( xPositionMute );
						
						while(Airsensor.AirSensor_Checkposition_Mode==ONCE_TIME)
							osDelay(100);
						if(RequestControlValve!=3) //����ֹ������ʹ��������
							osDelay(50);
						
						while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				    {
              osDelay(1);
				    }
						Airsensor.AirSensor_Checkposition_Mode=SLOW_MODE;
						xSemaphoreGive( xPositionMute );
#if 1
						if(gStorageData.Standby)
						{	
							InhaleTime = 0;
				      ActionSteps = 0;
				      RequestControlValve=0;//����������ȵ�ռ��
				      PillowControl.PillowAction = false;
				      ProcessControl.on = false;
			        ProcessControl.tick = 0;							
							StopAction();
							continue;
						}
#endif
						ExhaleTime=0;
						ActionSteps=1;  //head air bag inhale
//						osDelay(1000);
						
/* disabled by zjp,�Ա�ʼ���ƶ����λ�õ㣬��������˴����λ�������ϴε�λ�ã��������ڶ����λ��	
//------------------------------------------------------------------------------------------------------------						
						if(((LastHeadPosition.head1==HeadPosition.head1)&&(aTime==0))&&((HeadPosition.head1==HEAD_RIGHT_1)||(HeadPosition.head1==HEAD_RIGHT_2)))
            {
									aTime=1;
						      HeadPosition.head1=HeadPosition.head2;
									LastHeadPosition.head1=0x10;
						
						
						}
						else if(aTime)
							aTime=0;
//-------------------------------------------------------------------------------------------------------------------
*/					
            
						
						switch(HeadPosition.head1)
						{
								case HEAD_RIGHT_0:
									MIC_data_stop_detect_flag = 1;
									ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
									//gStorageData.Snore.Airbag1Times++;
								  action_head_position = 1;
									osDelay(500);
									MIC_data_stop_detect_flag = 0;
									break;
								case HEAD_RIGHT_1:
									MIC_data_stop_detect_flag = 1;
									ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
									//gStorageData.Snore.Airbag2Times++;
								  action_head_position = 2;
									osDelay(500);
									MIC_data_stop_detect_flag = 0;
									break;
								case HEAD_RIGHT_2:
									MIC_data_stop_detect_flag = 1;
									ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
								  //gStorageData.Snore.Airbag3Times++;
								  action_head_position = 3;
									osDelay(500);
									MIC_data_stop_detect_flag = 0;
									break;
								case HEAD_RIGHT_3:
									MIC_data_stop_detect_flag = 1;
									ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
								  //gStorageData.Snore.Airbag4Times++;
								  action_head_position = 4;
									osDelay(500);
									MIC_data_stop_detect_flag = 0;
									break;
								default:
									PillowControl.PillowAction=false;
								  action_head_position = 0;
								  RequestControlValve=0;//��λ�ã��ָ�״̬
								  ActionSteps=0;
								  {
										//����20201224  ������ֻ��һ��λ�ã�Ȼ���������ģʽ
										PillowControl.SnoreOn=false;
										Airsensor.AirSensor_fast_check_position_finished_flag=1;										
									}
								
									continue;
						}
						
						
						if(ACTION_TYPE==4)
						{
							switch(HeadPosition.head2)
							{
									case HEAD_RIGHT_0:
										ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
										gStorageData.Snore.Airbag4Times++;
										osDelay(500);
										break;
									case HEAD_RIGHT_1:
										ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
										gStorageData.Snore.Airbag3Times++;
										osDelay(500);
										break;
									case HEAD_RIGHT_2:
										ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
										gStorageData.Snore.Airbag2Times++;
										osDelay(500);
										break;
									case HEAD_RIGHT_3:
										ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
										gStorageData.Snore.Airbag1Times++;
										osDelay(500);
										break;
									default:
										PillowControl.PillowAction=false;
										continue;
							}
						}
						ExhalePump(0);
						InhalePump(1);
						
						while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
						{
              osDelay(1);
						}	

//            HAL_RTC_GetTime(&hrtc, &stimestructure_snore, RTC_FORMAT_BIN);
//			      HAL_RTC_GetDate(&hrtc, &sdatestructure_snore, RTC_FORMAT_BIN);
						
						while(1)
						{
			        HAL_RTC_GetTime(&hrtc, &stimestructure_snore, RTC_FORMAT_BIN);
			        HAL_RTC_GetDate(&hrtc, &sdatestructure_snore, RTC_FORMAT_BIN);
			        HAL_RTC_GetTime(&hrtc, &stimestructure_temp, RTC_FORMAT_BIN);
			        HAL_RTC_GetDate(&hrtc, &sdatestructure_temp, RTC_FORMAT_BIN);
				      if(stimestructure_snore.Hours == stimestructure_temp.Hours && stimestructure_snore.Minutes == stimestructure_temp.Minutes && stimestructure_snore.Seconds == stimestructure_temp.Seconds)
				      {
					      if(sdatestructure_snore.Year == sdatestructure_temp.Year && sdatestructure_snore.Month == sdatestructure_temp.Month && sdatestructure_snore.Date == sdatestructure_temp.Date)
					      {
						      if(stimestructure_snore.Hours <= 23 && stimestructure_snore.Minutes <=59 && stimestructure_snore.Seconds <= 59)
						      {
							      if(sdatestructure_snore.Year <=99 && sdatestructure_snore.Month <=12 && sdatestructure_snore.Date <=31)
						          break;
						      }
					      }
				      }
			      }	
						
						pump_operation_time[pump_operation_time_write_pointer][1]=sdatestructure_snore.Year;
						pump_operation_time[pump_operation_time_write_pointer][2]=sdatestructure_snore.Month;
						pump_operation_time[pump_operation_time_write_pointer][3]=sdatestructure_snore.Date;
						pump_operation_time[pump_operation_time_write_pointer][4]=stimestructure_snore.Hours;
						pump_operation_time[pump_operation_time_write_pointer][5]=stimestructure_snore.Minutes;
						pump_operation_time[pump_operation_time_write_pointer][6]=stimestructure_snore.Seconds;
											
#if 0             
						pump_operation_time[pump_operation_time_write_pointer][1]=sdatestructure.Year;
						pump_operation_time[pump_operation_time_write_pointer][2]=sdatestructure.Month;
						pump_operation_time[pump_operation_time_write_pointer][3]=sdatestructure.Date;
						pump_operation_time[pump_operation_time_write_pointer][4]=stimestructure.Hours;
						pump_operation_time[pump_operation_time_write_pointer][5]=stimestructure.Minutes;
						pump_operation_time[pump_operation_time_write_pointer][6]=stimestructure.Seconds;
#endif						
						
						pump_operation_time[pump_operation_time_write_pointer][0]=0x0;
							
						xSemaphoreGive( xTimeMute );
//            Airsensor_Print_RTC_Time_Actiontask();							
						
						osDelay(300);
					    StartCountAntSnoreTime = 1;
						if(ACTION_TYPE==2)
						{
							if (ActionAndCheckSnore(1000 * 60) != 0) {
								PillowControl.PillowAction = false;
								StopAction();
								StartCountAntSnoreTime = 1;
								ActionSteps = 2;  //exhale air bag completed,but no snore
								continue;
							}
						}
						else if(ACTION_TYPE==1)
						{
								uint8_t temp_flag=0;
								Antisnore_count_timer=0;
								Airsensor.antisnore_timer_len=gStorageData.ActionLevel;
								Airsensor.antisonre_timer_flag=1;
								while(Airsensor.antisonre_timer_flag==1)
								{
									if(gStorageData.Standby)
						      {	
							      InhalePump(0);
										InhaleTime = 0;
//				            ActionSteps = 0;
//				            RequestControlValve=0;//����������ȵ�ռ��
//				            PillowControl.PillowAction = false;
//				            ProcessControl.on = false;
//			              ProcessControl.tick = 0;
//							      StopAction();
//										temp_flag=1;
//							      break;
										Airsensor.antisonre_timer_flag=0;
										Airsensor.LED_flash_state_flag=1;
						      }
									osDelay(10);
								}
//								if(temp_flag==1)
//									continue;
							/*
							if(gStorageData.ActionLevel==0)
							{
								uint8_t temp_flag=0;
								Antisnore_count_timer=0;
								Airsensor.antisnore_timer_len=PILLOW_AIRTIME_LOW/1000;
								Airsensor.antisonre_timer_flag=1;
								while(Airsensor.antisonre_timer_flag==1)
								{
									if(gStorageData.Standby)
						      {	
							      InhalePump(0);
										InhaleTime = 0;
//				            ActionSteps = 0;
//				            RequestControlValve=0;//����������ȵ�ռ��
//				            PillowControl.PillowAction = false;
//				            ProcessControl.on = false;
//			              ProcessControl.tick = 0;
//							      StopAction();
//										temp_flag=1;
//							      break;
										Airsensor.antisonre_timer_flag=0;
										Airsensor.LED_flash_state_flag=1;
						      }
									osDelay(10);
								}
//								if(temp_flag==1)
//									continue;
								
								//ֱ����ʱ����
//								osDelay(PILLOW_AIRTIME_LOW);
							}
							
							else if(gStorageData.ActionLevel==1)
							{
								uint8_t temp_flag=0;
								Antisnore_count_timer=0;
								Airsensor.antisnore_timer_len=PILLOW_AIRTIME_LOWER/1000;
								Airsensor.antisonre_timer_flag=1;
								while(Airsensor.antisonre_timer_flag==1)
								{
									if(gStorageData.Standby)
						      {	
							      InhalePump(0);
										InhaleTime = 0;
//				            ActionSteps = 0;
//				            RequestControlValve=0;//����������ȵ�ռ��
//				            PillowControl.PillowAction = false;
//				            ProcessControl.on = false;
//			              ProcessControl.tick = 0;
//							      StopAction();
//										temp_flag=1;
//							      break;
										Airsensor.antisonre_timer_flag=0;
										Airsensor.LED_flash_state_flag=1;
						      }
									osDelay(10);
								}
//								if(temp_flag==1)
//									continue;
								
								//ֱ����ʱ����
//								osDelay(PILLOW_AIRTIME_LOW);
							}
							else if(gStorageData.ActionLevel==2)
							{
								uint8_t temp_flag=0;
								Antisnore_count_timer=0;
								Airsensor.antisnore_timer_len=PILLOW_AIRTIME_MID/1000;
								Airsensor.antisonre_timer_flag=1;
								while(Airsensor.antisonre_timer_flag==1)
								{
									if(gStorageData.Standby)
						      {	
							      InhalePump(0);
										InhaleTime = 0;
//				            ActionSteps = 0;
//				            RequestControlValve=0;//����������ȵ�ռ��
//				            PillowControl.PillowAction = false;
//				            ProcessControl.on = false;
//			              ProcessControl.tick = 0;
//							      StopAction();
//										temp_flag=1;
//							      break;
										Airsensor.antisonre_timer_flag=0;
										Airsensor.LED_flash_state_flag=1;
						      }
									osDelay(10);
								}
//								if(temp_flag==1)
//									continue;
								
//								osDelay(PILLOW_AIRTIME_MID);
							}
							else if(gStorageData.ActionLevel==3)
							{
								uint8_t temp_flag=0;
								Antisnore_count_timer=0;
								Airsensor.antisnore_timer_len=PILLOW_AIRTIME_HIGHER/1000;
								Airsensor.antisonre_timer_flag=1;
								while(Airsensor.antisonre_timer_flag==1)
								{
									if(gStorageData.Standby)
						      {	
							      InhalePump(0);
										InhaleTime = 0;
//				            ActionSteps = 0;
//				            RequestControlValve=0;//����������ȵ�ռ��
//				            PillowControl.PillowAction = false;
//				            ProcessControl.on = false;
//			              ProcessControl.tick = 0;
//							      StopAction();
//										temp_flag=1;
//							      break;
										Airsensor.antisonre_timer_flag=0;
										Airsensor.LED_flash_state_flag=1;
						      }
									osDelay(10);
								}
//								if(temp_flag==1)
//									continue;
								
//								osDelay(PILLOW_AIRTIME_MID);
							}
							else
							{
								uint8_t temp_flag=0;
								Antisnore_count_timer=0;
								Airsensor.antisnore_timer_len=PILLOW_AIRTIME_HIGH/1000;
								Airsensor.antisonre_timer_flag=1;
								while(Airsensor.antisonre_timer_flag==1)
								{
									if(gStorageData.Standby)
						      {	
							      InhalePump(0);
										InhaleTime = 0;
//				            ActionSteps = 0;
//				            RequestControlValve=0;//����������ȵ�ռ��
//				            PillowControl.PillowAction = false;
//				            ProcessControl.on = false;
//			              ProcessControl.tick = 0;
//							      StopAction();
//										temp_flag=1;
//							      break;
										Airsensor.antisonre_timer_flag=0;
										Airsensor.LED_flash_state_flag=1;
						      }
									osDelay(10);
								}
//								if(temp_flag==1)
//									continue;
//								osDelay(PILLOW_AIRTIME_HIGH);
							}
							*/
						}
						else if(ACTION_TYPE==3)
						{
							if(gStorageData.ActionLevel==0)
									temp=PILLOW_AIRTIME_LOW;
							else if(gStorageData.ActionLevel==1)
									temp=PILLOW_AIRTIME_MID;
							else
							{
									temp=PILLOW_AIRTIME_HIGH;
							}
							ExhaleTime=0;
							ActionAndCheckSnore(temp);
						}
						else if(ACTION_TYPE==4)
						{
							if(gStorageData.ActionLevel==0)
									temp=PILLOW_AIRTIME_LOW*2;
							else if(gStorageData.ActionLevel==1)
									temp=PILLOW_AIRTIME_MID*2;
							else
							{
								 temp=PILLOW_AIRTIME_HIGH*2;
							}

							ExhaleTime=0;
							osDelay(temp);
						}
						step++;
						ProcessControl.on=false;
						ProcessControl.tick=0;
		}
		if(step==1)
		{
			#if 0
						if(gStorageData.Standby)
						{	
							InhalePump(0);
							InhaleTime = 0;
				      ActionSteps = 0;
				      RequestControlValve=0;//����������ȵ�ռ��
				      PillowControl.PillowAction = false;
				      ProcessControl.on = false;
			        ProcessControl.tick = 0;
							StopAction();
							continue;
						}
			#endif
						ActionSteps=3;  //head air bag inhale completed
						ProcessControl.on=true;
						InhalePump(0);
						ExhalePump(0);
				        StartCountAntSnoreTime = 1;
				        if(ACTION_TYPE==2)
				        {
								if (ActionAndCheckSnore(1000 * 20) != 0)  //KEEP AIR 20 seconds
								{
									InhalePump(0);
									ExhalePump(0);
									ActionSteps = 4;  //head air bag inhale completed,but no snore
									StartCountAntSnoreTime = 1;
									continue;
								}
				        }
				        else if(ACTION_TYPE==1)
								{
									uint8_t temp_flag=0;
								  Antisnore_count_timer=0;
								  Airsensor.antisnore_timer_len=60;//�����������������ʱ60��
								  Airsensor.antisonre_timer_flag=1;
								  while(Airsensor.antisonre_timer_flag==1 && PillowControl.SnoreOn==false)
								  {
									  if(gStorageData.Standby )
						        {	
//							        InhalePump(0);
											
//										  InhaleTime = 0;
//				              ActionSteps = 0;
//				              RequestControlValve=0;//����������ȵ�ռ��
//				              PillowControl.PillowAction = false;
//				              ProcessControl.on = false;
//			                ProcessControl.tick = 0;
//							        StopAction();
//										  temp_flag=1;
//							        break;
											Airsensor.antisonre_timer_flag=0;
											Airsensor.LED_flash_state_flag=1;
						        }
									  osDelay(10);
								  }
//								  if(temp_flag==1)
//									  continue;
									
//				        	ActionAndCheckNoSnore(60000);  //һ������������ѹ
								}
				        else if(ACTION_TYPE==3)
				        {
				        	ActionAndCheckNoSnore(60000);  //һ������������ѹ
				        }
				        else if(ACTION_TYPE==4)
				       	{
				        	InhalePump(0);
				        	ExhalePump(0);
				       	}
						step++;
						ProcessControl.on=false;
						ProcessControl.tick=0;
		}

		if(step==2)                           //exhale air 25 seconds
		{
			
					if(gStorageData.Standby)
					{	
//						  InhaleTime = 0;
//				      ActionSteps = 0;
//				      RequestControlValve=0;//����������ȵ�ռ��
//				      PillowControl.PillowAction = false;
//				      ProcessControl.on = false;
//			        ProcessControl.tick = 0;
//							StopAction();
//							continue;
						  Airsensor.LED_flash_state_flag=1;
						}
			
					step++;
//					ADok3=0;
//					ADCstart3();
//					osDelay(500);
//					if(PressureADC_ConvertedValue[0]<10)
//	    					ErrorCode|=0x01;

					
					ActionSteps=5;  //exhale air bag
//					ExhaleAirHead();
					
#if 0					
					ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
					ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
					ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
					ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
					ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
					InhalePump(0);
					
#endif						
					

					
					
					while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
					{
              osDelay(1);
					}	
					
#if 0					
					pump_operation_time[pump_operation_time_write_pointer][7]=sdatestructure.Year;
					pump_operation_time[pump_operation_time_write_pointer][8]=sdatestructure.Month;
					pump_operation_time[pump_operation_time_write_pointer][9]=sdatestructure.Date;
					pump_operation_time[pump_operation_time_write_pointer][10]=stimestructure.Hours;
					pump_operation_time[pump_operation_time_write_pointer][11]=stimestructure.Minutes;
					pump_operation_time[pump_operation_time_write_pointer][12]=stimestructure.Seconds;
#endif

//					HAL_RTC_GetTime(&hrtc, &stimestructure_snore, RTC_FORMAT_BIN);
//			    HAL_RTC_GetDate(&hrtc, &sdatestructure_snore, RTC_FORMAT_BIN);
					
					while(1)
					{
			        HAL_RTC_GetTime(&hrtc, &stimestructure_snore, RTC_FORMAT_BIN);
			        HAL_RTC_GetDate(&hrtc, &sdatestructure_snore, RTC_FORMAT_BIN);
			        HAL_RTC_GetTime(&hrtc, &stimestructure_temp, RTC_FORMAT_BIN);
			        HAL_RTC_GetDate(&hrtc, &sdatestructure_temp, RTC_FORMAT_BIN);
				      if(stimestructure_snore.Hours == stimestructure_temp.Hours && stimestructure_snore.Minutes == stimestructure_temp.Minutes && stimestructure_snore.Seconds == stimestructure_temp.Seconds)
				      {
					      if(sdatestructure_snore.Year == sdatestructure_temp.Year && sdatestructure_snore.Month == sdatestructure_temp.Month && sdatestructure_snore.Date == sdatestructure_temp.Date)
					      {
						      if(stimestructure_snore.Hours <= 23 && stimestructure_snore.Minutes <=59 && stimestructure_snore.Seconds <= 59)
						      {
							      if(sdatestructure_snore.Year <=99 && sdatestructure_snore.Month <=12 && sdatestructure_snore.Date <=31)
						          break;
						      }
					      }
				      }
			    }	
					
					pump_operation_time[pump_operation_time_write_pointer][7]=sdatestructure_snore.Year;
					pump_operation_time[pump_operation_time_write_pointer][8]=sdatestructure_snore.Month;
					pump_operation_time[pump_operation_time_write_pointer][9]=sdatestructure_snore.Date;
					pump_operation_time[pump_operation_time_write_pointer][10]=stimestructure_snore.Hours;
					pump_operation_time[pump_operation_time_write_pointer][11]=stimestructure_snore.Minutes;
					pump_operation_time[pump_operation_time_write_pointer][12]=stimestructure_snore.Seconds;
					
					pump_operation_time[pump_operation_time_write_pointer][0]=0x1;	//��Ч�ô�ʱ���¼					
					
          pump_operation_time_write_pointer++;
					if(pump_operation_time_write_pointer>=20)
						pump_operation_time_write_pointer=0;

					xSemaphoreGive( xTimeMute );
          if(action_head_position == 1)
						gStorageData.Snore.Airbag1Times++;
					else if(action_head_position == 2)
						gStorageData.Snore.Airbag2Times++;
					else if(action_head_position == 3)
						gStorageData.Snore.Airbag3Times++;
					else if(action_head_position == 4)
						gStorageData.Snore.Airbag4Times++;
					
//					Airsensor_Print_RTC_Time_Actiontask1();				
					
#if 1			
					MIC_data_stop_detect_flag = 1;
					osDelay(200);
					ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
					osDelay(200);
					ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
					osDelay(200);
					ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
					osDelay(200);
					ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
					osDelay(200);
					MIC_data_stop_detect_flag = 0;				
					//osDelay(10000);
					uint8_t temp_flag=0;
					Antisnore_count_timer=0;
				  Airsensor.antisnore_timer_len=10;//�����������������ʱ60��
					Airsensor.antisonre_timer_flag=1;
					while(Airsensor.antisonre_timer_flag==1)
					{
					
						if(gStorageData.Standby )
						{	

//							InhaleTime = 0;
//				      ActionSteps = 0;
//				      RequestControlValve=0;//����������ȵ�ռ��
//				      PillowControl.PillowAction = false;
//				      ProcessControl.on = false;
//			        ProcessControl.tick = 0;
//							StopAction();
	//						temp_flag=1;
//						  break;
							Airsensor.LED_flash_state_flag=1;
						}
					
						osDelay(10);
				  }
//				  if(temp_flag==1)
//					  continue;	
					MIC_data_stop_detect_flag = 1;
					osDelay(200);
					ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
					osDelay(200);
					ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
					osDelay(200);
					ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
					osDelay(200);
					ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
					osDelay(200);
					MIC_data_stop_detect_flag = 0;
#endif
					
					
					if(ACTION_TYPE==3)
					{
						osDelay(ExhaleTime*1000+3000);
					}
					else if(ACTION_TYPE==4)
					{
						osDelay(PILLOW_AIROFFTIME*2);
					}
					else
					{
					  //osDelay(PILLOW_AIROFFTIME-11000);
						temp_flag=0;
					  Antisnore_count_timer=0;
				    Airsensor.antisnore_timer_len=(PILLOW_AIROFFTIME-11000)/1000;//�����������������ʱ60��
					  Airsensor.antisonre_timer_flag=1;
					  while(Airsensor.antisonre_timer_flag==1)
					  {
						
						  if(gStorageData.Standby )
						  {	

//							  InhaleTime = 0;
//				        ActionSteps = 0;
//				        RequestControlValve=0;//����������ȵ�ռ��
//				        PillowControl.PillowAction = false;
//				        ProcessControl.on = false;
//			          ProcessControl.tick = 0;
//							  StopAction();
//							  temp_flag=1;
//						    break;
								Airsensor.LED_flash_state_flag=1;
						  }
							
						  osDelay(10);
				    }
//				    if(temp_flag==1)
//					    continue;			
						
					}

					ExhaleTime=0;
		}
		
		
		if(step==3)                          ////KEEP AIR 20 seconds
		{
			InhaleTime = 0;
	#if 0
			if (gStorageData.Standby)
			{	
				      InhaleTime = 0;
				      ActionSteps = 0;
				      RequestControlValve=0;//����������ȵ�ռ��
				      PillowControl.PillowAction = false;
				      ProcessControl.on = false;
			        ProcessControl.tick = 0;
							StopAction();
							continue;
			}
	#endif
			StopAction();
			ActionSteps = 6;  //exhale air bag completed
			step4time = 0;
			PillowControl.PillowAction = false;
			 if(ACTION_TYPE==2)
			 {
					if (ActionAndCheckSnore(1000 * 20) != 0) {
						PillowControl.PillowAction = false;
						StopAction();
						StartCountAntSnoreTime = 1;
						continue;
					}
			 }
			step++;
			PillowControl.PillowAction = false;
			ProcessControl.on = false;
			ProcessControl.tick = 0;
			LastHeadPosition=HeadPosition;
//			osDelay(15000);
			InhaleTime = 0;
			
			step = 0;
			RequestControlValve=0;//����������ȵ�ռ��
			if(gStorageData.Standby==1 || Airsensor.LED_flash_state_flag==1)
			{
			  Airsensor.LED_flash_state_flag=0;
        WrokingLed(RED_LED,1);
		    Airsensor.Led_yellow_state=1;
			}				
			ActionSteps = 0; 
		}

	}
	 else if(AppTest)
	 {
		 	//AirSensor_Checkposition_Mode=ONCE_TIME;
		 	//while(AirSensor_Checkposition_Mode==ONCE_TIME)
		 	//	osDelay(100);
		     RequestControlValve=1;
		     if(Masaji)
		    {
//					  RequestControlValve=0; 
					  while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				    {
              osDelay(1);
				    }
		    		Airsensor.AirSensor_Checkposition_Mode=ONCE_TIME;
					  xSemaphoreGive( xPositionMute );
						
//		    		 while(Airsensor.AirSensor_Checkposition_Mode==ONCE_TIME)
//		    		  osDelay(100);
						 
						
						while(Airsensor.AirSensor_Checkposition_Mode==ONCE_TIME)
							osDelay(100);
						if(RequestControlValve!=3) //����ֹ������ʹ��������
							osDelay(50);
						while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				    {
              osDelay(1);
				    }
						Airsensor.AirSensor_Checkposition_Mode=SLOW_MODE;
						xSemaphoreGive( xPositionMute );
						
//						RequestControlValve=2��
		     }
		     while(1)
		     {

		    	 if(RequestControlValve==2 || RequestControlValve==3 )
		    		 break;
		    	 osDelay(100);
		     }
		     if(Masaji)
		     {
		    	 MasajiControlAirBag(5);
					 Masaji=0;
					 RequestControlValve=0;
		    	// MasajiControlAirBag(position.head2+1);

		     }
		     else
		     {
					 if(RequestControlValve==2)
					 {
					   PillowControl.SnoreOn==false;
		 	       AppControlAirBag(tempAirBag);
			       AppTest=0;
						 RequestControlValve=0;
					 }
					 else
					 {
						 AppTest=0;
					 }
		     }
//		     RequestControlValve=0;
//		     osDelay(3000);

	 }
	 else if((HeadPosition.head1!=HEAD_NO)&&(AppCtrlBag.enable==0)&&(PillowControl.SnoreOn))
	 {
		

	 }
	 else if(ReadPos.enable==1)
	 {
		  if(ActionSteps==0 && Airsensor.AirSensor_Checkposition_Mode!=ONCE_TIME && gStorageData.Standby==1)
			{
				while(xSemaphoreTake( xPositionMute, portMAX_DELAY ) != pdTRUE)
				{
           osDelay(1);
				} 
        Airsensor.AirSensor_Checkposition_Mode=FAST_MODE;
			  xSemaphoreGive( xPositionMute );
				PillowControl.SnoreOn==false;
			}
	 }
		//osDelay(500);
  }
  /* USER CODE END ActionTaskFunction */
}




void AppControlAirBag(unsigned char w)
{
	  int  air_time_len,airoff_time_len;
	  ReadPos.enable=0;
	  Airsensor.AirSensor_Checkposition_Mode=SLOW_MODE;
	  Airsensor.LED_flash_state_flag=1;
	  Airsensor.Led_flash_counter=0;
	  Airsensor.Led_yellow_state=1;//�Ƶ�����
	  if(gStorageData.StudyMode==2)
			{
				air_time_len=PILLOW_AIRTIME_SHOWMODE;
				airoff_time_len=PILLOW_AIROFFTIME_SHOWMODE;
			}
		else
			{
				air_time_len=gStorageData.ActionLevel*1000;
				airoff_time_len=PILLOW_AIROFFTIME;
				
//				air_time_len=PILLOW_AIRTIME_TESTMODE;
//				airoff_time_len=PILLOW_AIROFFTIME_TESTMODE;
/*				
				if(gStorageData.ActionLevel==0)
				{
				  air_time_len=PILLOW_AIRTIME_LOW;
				  airoff_time_len=PILLOW_AIROFFTIME;
				}
				else if(gStorageData.ActionLevel==1)
				{
				  air_time_len=PILLOW_AIRTIME_LOWER;
				  airoff_time_len=PILLOW_AIROFFTIME;
				}
				else if(gStorageData.ActionLevel==2)
				{
				  air_time_len=PILLOW_AIRTIME_MID;
				  airoff_time_len=PILLOW_AIROFFTIME;
				}
				else if(gStorageData.ActionLevel==3)
				{
					air_time_len=PILLOW_AIRTIME_HIGHER;
				  airoff_time_len=PILLOW_AIROFFTIME;
				}
				else if(gStorageData.ActionLevel==4)
				{
					air_time_len=PILLOW_AIRTIME_HIGH;
				  airoff_time_len=PILLOW_AIROFFTIME;
				}
*/					
				
			}			
		switch(w)
		{
			
			case 1:
				ActionSteps=1;
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				InhalePump(1);
				//ExhalePump(0);
				InhaleTime=0;
#if 0
        //ֱ�����������淽ʽ
			  osDelay(air_time_len);
				ActionSteps=5;
			  	InhalePump(0);
			//	ExhalePump(0);
//			  	osDelay(3000);
			
			  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
				ExhaleTime=0;
//				osDelay(500);
				osDelay(airoff_time_len); 

#endif			
			
			
#if 1		
        //����4������������ʽ			
				osDelay(air_time_len-2000);
				ActionSteps=5;
			  InhalePump(0);
				if(tempholdingtime != 0)
				{
					osDelay(tempholdingtime*1000);
				}
					
					
			//	ExhalePump(0);
//			  	osDelay(3000);
			  ActionSteps=5;
			  ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
			  osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
			  osDelay(10000);
			
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
//				osDelay(500);
				
//				osDelay(500);

				ExhaleTime=0;
//				osDelay(500);
				osDelay(airoff_time_len-12500);
				
#endif				
				
				
				ActionSteps=0;
				break;
			case 2:
				ActionSteps=1;
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				InhalePump(1);
				ExhalePump(0);
				InhaleTime=0;
//				osDelay(2000);
			
#if 0
        //ֱ�����������淽ʽ
			  osDelay(air_time_len);
				ActionSteps=5;
			  InhalePump(0);		
			
			//	ExhalePump(0);
//			  	osDelay(3000);
			
			  ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
				ExhaleTime=0;
//				osDelay(500);
				osDelay(airoff_time_len); 

#endif			
			
			
#if 1			
			
				osDelay(air_time_len-2000);
				InhalePump(0);
				if(tempholdingtime != 0)
				{
					osDelay(tempholdingtime*1000);
				}				
				
				ExhalePump(0);
//				osDelay(5000);
			
			  ActionSteps=5;
			  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
			  osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
			  osDelay(10000);
			
			  ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
//				osDelay(500);
				ActionSteps=5;
				ExhaleTime=0;
//				osDelay(500);
				osDelay(airoff_time_len-12500);
				
#endif
				ActionSteps=0;
				break;
			case 3:
				ActionSteps=1;
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				InhalePump(1);
				ExhalePump(0);
				InhaleTime=0;
			
#if 0
        //ֱ�����������淽ʽ
			  osDelay(air_time_len);
				ActionSteps=5;
			  InhalePump(0);	
			
			//	ExhalePump(0);
//			  	osDelay(3000);
			
			  ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
				ExhaleTime=0;
//				osDelay(500);
				osDelay(airoff_time_len); 

#endif				
			
#if 1			
			
//				osDelay(2000);
				osDelay(air_time_len-2000);
				InhalePump(0);
				if(tempholdingtime != 0)
				{
					osDelay(tempholdingtime*1000);
				}
				
				
				
				ExhalePump(0);
//				osDelay(10000);
        ActionSteps=5;
			  ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
			  osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
			  osDelay(500);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
			  osDelay(10000);
			
			  ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
				osDelay(500); 
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
//				osDelay(500);
				ActionSteps=5;
				ExhaleTime=0;
//				osDelay(500);
				osDelay(airoff_time_len-12500);		

#endif
				ActionSteps=0;
				break;
			case 4:
				ActionSteps=1;
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				InhalePump(1);
				ExhalePump(0);			
				InhaleTime=0;
			
#if 0
        //ֱ�����������淽ʽ
			  osDelay(air_time_len);
				ActionSteps=5;
			  	InhalePump(0);
			//	ExhalePump(0);
//			  	osDelay(3000);
			
			  ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
				ExhaleTime=0;
//				osDelay(500);
				osDelay(airoff_time_len); 

#endif				
			
#if 1			
//				osDelay(2000);
				osDelay(air_time_len-1500);
				InhalePump(0);
				
				if(tempholdingtime != 0)
				{
					osDelay(tempholdingtime*1000);
				}			
				
				ExhalePump(0);
//				osDelay(5000);
			  ActionSteps=5;
			  ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
			  osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
			  osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
			  osDelay(10000);
			  
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
				osDelay(500); 
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
				osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
//				osDelay(500);
				ActionSteps=5;
				ExhaleTime=0;
//				osDelay(500);
				osDelay(airoff_time_len-12500);
#endif

				ActionSteps=0;
				break;
			case 5:
				ActionSteps=1;
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
				InhalePump(1);
				ExhalePump(0);
				InhaleTime=0;
//				osDelay(2000);
				osDelay(air_time_len);
				InhalePump(0);
				ExhalePump(0);
//				osDelay(5000);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
//				osDelay(500);
				ActionSteps=5;
				ExhaleTime=0;
//				osDelay(500);
				osDelay(airoff_time_len);
				ActionSteps=0;
				break;
			default:
				break;
		}
		Airsensor.LED_flash_state_flag=0;
		WrokingLed(RED_LED,1);
		Airsensor.Led_yellow_state=1;
		
}
void MasajiControlAirBag(unsigned char w)
{
		switch(w)
		{
			case 1:
				ActionSteps=1;
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
			  osDelay(500); 
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
				InhalePump(1);
				//ExhalePump(0);
				InhaleTime=0;
				osDelay(40000);
				ActionSteps=5;
			  	InhalePump(0);
			//	ExhalePump(0);
//			  	osDelay(3000);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
//				osDelay(500);

				ExhaleTime=0;
//				osDelay(500);
				osDelay(20000);
				ActionSteps=0;
				break;
			case 2:
				ActionSteps=1;
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
				InhalePump(1);
				ExhalePump(0);
				InhaleTime=0;
//				osDelay(2000);
					osDelay(40000);
				InhalePump(0);
				ExhalePump(0);
//				osDelay(5000);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
//				osDelay(500);
				ActionSteps=5;
				ExhaleTime=0;
//				osDelay(500);
				osDelay(20000);
				ActionSteps=0;
				break;
			case 3:
				ActionSteps=1;
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
				InhalePump(1);
				ExhalePump(0);
				InhaleTime=0;
//				osDelay(2000);
					osDelay(40000);
				InhalePump(0);
				ExhalePump(0);
//				osDelay(5000);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
//				osDelay(500);
				ActionSteps=5;
				ExhaleTime=0;
//				osDelay(500);
					osDelay(20000);
				ActionSteps=0;
				break;
			case 4:
				ActionSteps=1;
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
			  osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
				InhalePump(1);
				ExhalePump(0);
				InhaleTime=0;
//				osDelay(2000);
				osDelay(40000);
				InhalePump(0);
				ExhalePump(0);
//				osDelay(5000);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
//				osDelay(500);
				ActionSteps=5;
				ExhaleTime=0;
//				osDelay(500);
				osDelay(20000);
				ActionSteps=0;
				break;
			case 5:
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
				osDelay(500);
				InhalePump(1);
				osDelay(2800);
				InhalePump(0);
			  osDelay(20000);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
				
				
				break;
			default:
				break;
		}
}
#endif

