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
extern HeadPosition_def  position,LastHeadPosition;
extern xQueueHandle  ExhalAdcQHandle,InhalAdcQHandle;
extern unsigned char AppTest,tempAirBag,Side_Sleep;
extern uint8_t Body_Left_Right_Confirmed;
extern uint8_t AirSensor_Position_Changed;
unsigned char ErrorCode,RequestControlValve=0;
unsigned char    ACTION_TYPE=4;                 //1:动作过程中不检测鼾声，直接充到饱。     2.动作过程中检测鼾声，鼾声停则停止充气，鼾声起则继续充气。  3.动作过程中检测鼾声，鼾声停则立即放气  4.抬高头部
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
 * 切换到相应的身体位置充气
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
 * 切换到相应的身体位置放气
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
		if(j>60*100*7) //10路???鲁卢?卤
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
		if(ExhaleTime>60*100*6) //10路???鲁卢?卤
			break;
	  }
		else 
		{
		if(ExhaleTime>60*100) //30?毛
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
	ErrorCode|=(1<<ERROR_CODE_EXHALE_OVER);//抽气超时
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
		if(ExhaleTime>60*50) //1分钟超时
			break;
#else
		if(ExhaleTime>60*50*10) //6分钟超时
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
			if(ExhaleTime>60*5*10) //10路???鲁卢?卤
			break;
	 	 }
		
		if(CheckSnore&&(TestMode==0))
		{
			 if(PillowControl.SnoreOn==false)
		     		 return 1;
		}
  }
	ExhaleTime=0;
	ErrorCode|=(1<<ERROR_CODE_EXHALE_OVER);//抽气超时
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
			{    	if((PressureADC_ConvertedValue[0]>(INHALE_PRESURE_VALUE-0X40))&&(PressureADC_ConvertedValue[1]>(INHALE_PRESURE_VALUE-0X40))&&(PressureADC_ConvertedValue[2]>(INHALE_PRESURE_VALUE-0X40))&&(PressureADC_ConvertedValue[3]>(INHALE_PRESURE_VALUE-0X40))&&(PressureADC_ConvertedValue[4]>(INHALE_PRESURE_VALUE-0X40)))//碌?
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
				if((PressureADC_ConvertedValue[0]>INHALE_PRESURE_VALUE)&&(PressureADC_ConvertedValue[1]>INHALE_PRESURE_VALUE)&&(PressureADC_ConvertedValue[2]>INHALE_PRESURE_VALUE)&&(PressureADC_ConvertedValue[3]>INHALE_PRESURE_VALUE)&&(PressureADC_ConvertedValue[4]>INHALE_PRESURE_VALUE))//赂?
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
					{    	if((Ina[0]>0x2d5)&&(Ina[1]>0x2d5)&&(Ina[2]>0x2d5)&&(Ina[3]>0x2d5)&&(Ina[4]>0x2d5)&&(Ina[5]>0x2d5))//碌?
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
						if((Ina[0]>0x310)&&(Ina[1]>0x310)&&(Ina[2]>0x310)&&(Ina[3]>0x310)&&(Ina[4]>0x310)&&(Ina[5]>0x310))//赂?
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
	  if(InhaleTime>(60*5*8)) //7路???鲁卢?卤
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
	ErrorCode|=(1<<ERROR_CODE_INHALE_OVER);//充气超时
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
			if(ExhaleTime>15*100) //15?毛
				break;
		}
		else if(gStorageData.ActionLevel==1)
		{
			if(ExhaleTime>30*100) //30?毛
				break;
		}
		else if(gStorageData.ActionLevel==2)
		{
			if(ExhaleTime>50*100) //30?毛
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
			if(InhaleTime>15*100) //15?毛鲁卢?卤
				break;
		}
		else if(gStorageData.ActionLevel==1)
		{
			if(InhaleTime>25*100) //25?毛鲁卢?卤
				break;
		}
		else if(gStorageData.ActionLevel==2)
		{
			if(InhaleTime>50*100) //50?毛鲁卢?卤
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
			ti++;	//确定人已经离开，则要恢复气袋为无气状态
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
	{        //人已经离开，且放气完毕后，则保存本次的睡眠数据，并切换工作状态为休眠
				t2++;
		if(t2>40)
		{
				t2=0;
				if( gStorageData.Standby==0)  //正常工作状态,增加对数据的存储部份
				{

					 WriteSnoreData(&gStorageData.Snore); //保存上次
					 gStorageData.Standby=1;
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
				gStorageData.Snore.HeadMovingTimes=0;
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
				gStorageData.Standby=0;
				
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
									//StopAction();  //录矛虏芒碌陆梅媒?霉
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
				PillowControl.PillowAction=false; //????录矛虏芒?路虏驴潞??铆?氓碌??禄??
				osDelay(5000);
				PillowControl.PillowAction=true;
				InhaleTime=0;
				ProcessControl.on=false;
				ProcessControl.tick=0;
				//	StopAction();
							  
			}
			if(step==4)    //身体和头部同时动作
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


#else    //三代止鼾枕动作程序

void request_position_stop_work(void)
{	
		ActionSteps=1;
		RequestControlValve=1;
		while(1)
		{

		    	 if(RequestControlValve==2)
		    		 break;
		    	 osDelay(10);
		 }	
}

void ActionTaskFunction(void const * argument)
{
  /* USER CODE BEGIN ActionTaskFunction */
	unsigned char mode=0,headRunTime=0;
  	unsigned int t2=0,ti=0,temp,aTime=0;
  /* Infinite loop */
	//StartAllAction=0;
	ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
	ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
	ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
	ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
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
        
        if(gStorageData.StudyMode==2) //演示模式
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

        

  	if((step==0)&&(PillowControl.SnoreOn==true)&&(AppTest==0)&&(ReadPos.enable==0))  //有鼾声，也肯定有人
  	{
			if((gStorageData.Standby==0)&&(gStorageData.StudyMode))   //StudyMode 0:工作模式 1：学习模式 2：演示模式
				{
				
				}
				else
				{
					//if((AutoStartTimer>=(gStorageData.WorkDelay*10*60))||(gStorageData.Standby==0))
					//{	
					
					  //启动头部位置检测
						AirSensor_Checkposition_Mode=ONCE_TIME;
						while(AirSensor_Checkposition_Mode==ONCE_TIME)
						osDelay(100);
						osDelay(350);
					//}
				}
  	}
		
#if 0		//disabled by zjp at 20190505 不可能出现这种情况 
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
	
#if 0 //disabled by zjp at 20190505 没人时不再保存鼾声数据和检测位置
	if((gStorageData.Standby==0)&&(PillowControl.SnoreOn==false)&&(AppTest==0))//&&(stimestructure.Hours>5))
	{
			t2++;
			if((t2>10*60*30))//&&((stimestructure.Hours>=8)&&(stimestructure.Hours<22)))  //30分钟内无鼾声，设置为待机状态
			{
				 t2=0;
				 if( gStorageData.Standby==0)  //正常工作状态,增加对数据的存储部份
				 {
		  			gStorageData.Standby=1;
				 }
			}
	}
	else t2=0;	
#endif 

	//最近一次的判断为有人是由位置检测判断的，由于有死区，在10分钟内如果无鼾声，需要重新采用03位置检测再次确认一下是否有人在床
	
  if((gStorageData.Standby==0)&&(PillowControl.SnoreOn==false)&&(Airsensor.personon_by_positioncheck_flag==1)&&(AppTest==0))//&&(stimestructure.Hours>5))
	{
			t2++;
			if((t2>10*60*10))//&&((stimestructure.Hours>=8)&&(stimestructure.Hours<22)))  //10分钟内无鼾
			{
				 t2=0;
				 //进行03位置检测，再次确认是否有人
				 AirSensor_Checkposition_Mode=ONCE_TIME;
				 while(AirSensor_Checkposition_Mode==ONCE_TIME)
				 osDelay(100);				
			}
	}
	else t2=0;	
	
	//added by zjp at 20190505 
	//***********************************************************************************************************
	if((gStorageData.Standby==0)&&(Airsensor.AirSensor_PersonOn==0)&&(AppTest==0))//&&(stimestructure.Hours>5))
	{
			
				 if( gStorageData.Standby==0)  //正常工作状态,增加对数据的存储部份
				 {
		  			gStorageData.Standby=1;
				 }
			
	}	
	//************************************************************************************************************
	
	if((AppTest==0)&&(AppCtrlBag.enable==0))
    gStorageData.Snore.HeadPosion=HeadPosition.head1;
	
	if((AppTest==0)&&(AppCtrlBag.enable==0)&&(ReadPos.enable==0)&&(Airsensor.AirSensor_PersonOn==1)&&(gStorageData.Standby==1))		
  {	
		      //有人后切换为工作状态
		      InhaleTime=0;
					ExhaleTime=0;
					memset(&gStorageData.Snore,0,32);
					gStorageData.Snore.UsedRecord=0x55aa;
					gStorageData.Snore.StudyMode=gStorageData.StudyMode;
					gStorageData.Standby=0;
					ProcessControl.tick=0;
					ProcessControl.on=false;
					step=0;		
	}
	
  if((AppTest==0)&&(AppCtrlBag.enable==0)&&(ReadPos.enable==0)&&(PillowControl.SnoreOn==true))
  {			
			//if(gStorageData.workTimeOn==0)
			//{
		
		 #if 0 //disabled by zjp at 20190505 移到上面进行处理
				if(gStorageData.Standby)
				{
					//if(AutoStartTimer<(gStorageData.WorkDelay*10*60))
					//{
					//	AutoStartTimer++;
					//	continue;
				//}
					InhaleTime=0;
					ExhaleTime=0;
					memset(&gStorageData.Snore,0,32);
					gStorageData.Snore.UsedRecord=0x55aa;
					gStorageData.Snore.StudyMode=gStorageData.StudyMode;
					gStorageData.Standby=0;
					ProcessControl.tick=0;
					ProcessControl.on=false;
					step=0;
				}
			#endif 	
				
				//else 	AutoStartTimer=0;
			//}
			 if(gStorageData.Standby)
				continue;
			 
			 if(HeadPosition.head1==HEAD_NO)
						continue;
			
			if(PillowControl.SnoreOn==false)
				continue;
			if(gStorageData.StudyMode==1)
				continue;
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
						while(AirSensor_Checkposition_Mode==ONCE_TIME)
							osDelay(100);
						if(gStorageData.Standby)
						{	
							StopAction();
							continue;
						}
						ExhaleTime=0;
						ActionSteps=1;  //head air bag inhale
						osDelay(1000);
						
						
						if(((LastHeadPosition.head1==HeadPosition.head1)&&(aTime==0))&&((HeadPosition.head1==HEAD_RIGHT_1)||(HeadPosition.head1==HEAD_RIGHT_2)))
            {
									aTime=1;
						      HeadPosition.head1=HeadPosition.head2;
									LastHeadPosition.head1=0x10;
						
						
						}
						else if(aTime)
							aTime=0;
							
						switch(HeadPosition.head1)
						{
								case HEAD_RIGHT_0:
									ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
									gStorageData.Snore.Airbag1Times++;
									osDelay(500);
									break;
								case HEAD_RIGHT_1:
									ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
								gStorageData.Snore.Airbag2Times++;
									osDelay(500);
									break;
								case HEAD_RIGHT_2:
									ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
								gStorageData.Snore.Airbag3Times++;
									osDelay(500);
									break;
								case HEAD_RIGHT_3:
									ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
								gStorageData.Snore.Airbag4Times++;
									osDelay(500);
									break;
								default:
									PillowControl.PillowAction=false;
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
							if(gStorageData.ActionLevel==0)
								osDelay(PILLOW_AIRTIME_LOW);
							else if(gStorageData.ActionLevel==1)
								osDelay(PILLOW_AIRTIME_MID);
							else
							{
								osDelay(PILLOW_AIRTIME_HIGH);
							}
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
						if(gStorageData.Standby)
						{	
							StopAction();
							continue;
						}
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
				        	ActionAndCheckNoSnore(60000);  //一分钟无鼾声保压
				        else if(ACTION_TYPE==3)
				        {
				        	ActionAndCheckNoSnore(60000);  //一分钟无鼾声保压
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
							StopAction();
							continue;
						}
					step++;
					ADok3=0;
					ADCstart3();
					osDelay(500);
					if(PressureADC_ConvertedValue[0]<10)
	    					ErrorCode|=0x01;

					
					ActionSteps=5;  //exhale air bag
//					ExhaleAirHead();
					ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
					ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
					ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
					ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
					ValueDriver(EXHALE_VALUE,GPIO_PIN_RESET);
					InhalePump(0);
					if(ACTION_TYPE==3)
					{
						osDelay(ExhaleTime*1000+3000);
					}
					else if(ACTION_TYPE==4)
						osDelay(PILLOW_AIROFFTIME*2);
					else
					osDelay(PILLOW_AIROFFTIME);

					ExhaleTime=0;
		}
		if(step==3)                          ////KEEP AIR 20 seconds
		{
			InhaleTime = 0;
			if (gStorageData.Standby)
			{	
							StopAction();
							continue;
			}
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
			ActionSteps = 0;
			step = 0;
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
		    		AirSensor_Checkposition_Mode=ONCE_TIME;
		    		 while(AirSensor_Checkposition_Mode==ONCE_TIME)
		    		  osDelay(100);
		    }
		     while(1)
		     {

		    	 if(RequestControlValve==2)
		    		 break;
		    	 osDelay(100);
		     }
		     if(Masaji)
		     {
		    	 MasajiControlAirBag(5);
					 Masaji=0;
		    	// MasajiControlAirBag(position.head2+1);

		     }
		     else
		     {
		 	 AppControlAirBag(tempAirBag);
			 AppTest=0;
		     }
		     RequestControlValve=0;
		     osDelay(3000);

	 }
	 else if((HeadPosition.head1!=HEAD_NO)&&(AppCtrlBag.enable==0)&&(PillowControl.SnoreOn))
	 {
		

	 }
		//osDelay(500);
  }
  /* USER CODE END ActionTaskFunction */
}




void AppControlAirBag(unsigned char w)
{
	  int  air_time_len,airoff_time_len;
	  if(gStorageData.StudyMode==2)
			{
				air_time_len=PILLOW_AIRTIME_SHOWMODE;
				airoff_time_len=PILLOW_AIROFFTIME_SHOWMODE;
			}
		else
			{
				air_time_len=PILLOW_AIRTIME_TESTMODE;
				airoff_time_len=PILLOW_AIROFFTIME_TESTMODE;
				
			}			
		switch(w)
		{
			
			case 1:
				ActionSteps=1;
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
				InhalePump(1);
				//ExhalePump(0);
				InhaleTime=0;
				osDelay(air_time_len);
				ActionSteps=5;
			  	InhalePump(0);
			//	ExhalePump(0);
//			  	osDelay(3000);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
//				osDelay(500);

				ExhaleTime=0;
//				osDelay(500);
				osDelay(airoff_time_len);
				ActionSteps=0;
				break;
			case 2:
				ActionSteps=1;
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
				InhalePump(1);
				ExhalePump(0);
				InhaleTime=0;
//				osDelay(2000);
					osDelay(air_time_len);
				InhalePump(0);
				ExhalePump(0);
//				osDelay(5000);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
//				osDelay(500);
				ActionSteps=5;
				ExhaleTime=0;
//				osDelay(500);
				osDelay(airoff_time_len);
				ActionSteps=0;
				break;
			case 3:
				ActionSteps=1;
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
				InhalePump(1);
				ExhalePump(0);
				InhaleTime=0;
//				osDelay(2000);
					osDelay(air_time_len);
				InhalePump(0);
				ExhalePump(0);
//				osDelay(5000);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
//				osDelay(500);
				ActionSteps=5;
				ExhaleTime=0;
//				osDelay(500);
					osDelay(airoff_time_len);
				ActionSteps=0;
				break;
			case 4:
				ActionSteps=1;
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_SET);
				osDelay(500);
				ValueDriver(HEAD_R_1_VALUE,GPIO_PIN_RESET);
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
				InhalePump(1);
				ExhalePump(0);
				InhaleTime=0;
//				osDelay(2000);
				osDelay(air_time_len);
				InhalePump(0);
				ExhalePump(0);
//				osDelay(5000);
				ValueDriver(HEAD_R_3_VALUE,GPIO_PIN_RESET);
//				osDelay(500);
				ActionSteps=5;
				ExhaleTime=0;
//				osDelay(500);
				osDelay(airoff_time_len);
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
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
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
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
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
				ValueDriver(HEAD_R_0_VALUE,GPIO_PIN_RESET);
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
				ValueDriver(HEAD_R_2_VALUE,GPIO_PIN_RESET);
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

