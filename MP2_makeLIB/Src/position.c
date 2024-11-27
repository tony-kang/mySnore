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


//Airsensor_struct_Def Airsensor;


#define debug_log 0

void AirSensor_position_process(Airsensor_struct_Def * air)
{
	uint8_t i,m,n,error,mm,nn,max_last;
	uint16_t j,k,max,min;
	
	uint8_t newm=0;
	uint16_t newmax=0;
	
	//选取5次采样的最小值
	for(i=0;i<4;i++)
	 {
		j=0xffff;
		for(n=0;n<5;n++)
     {
			if(air->AirSensor_PressureValue[i][n]<j)
			 {
        j=air->AirSensor_PressureValue[i][n];				 
			 }				 
		 }
    air->AirSensor_PressureValueMin[i]=j;		 
	 }
	//选取5次采样的最大值
	for(i=0;i<4;i++)
	 {
		j=0;
		for(n=0;n<5;n++)
     {
			if(air->AirSensor_PressureValue[i][n]>j)
			 {
        j=air->AirSensor_PressureValue[i][n];				 
			 }				 
		 }
    air->AirSensor_PressureValueMax[i]=j;		 
	 }
	//计算5次采样的平均值
	for(i=0;i<4;i++)
	 {
		j=0;
		for(n=0;n<5;n++)
     {
			
        j+=air->AirSensor_PressureValue[i][n];				 
			 				 
		 }
    air->AirSensor_PressureValueAvg[i]=j/5;		 
	 }
	//计算4路最小值的最大差异
	j=0;
	k=0xffff;
	for(i=0;i<4;i++)
	 {
	  if(air->AirSensor_PressureValueMin[i]>j)
		 {	
		  j=air->AirSensor_PressureValueMin[i];
			m=i;
		 }
		if(air->AirSensor_PressureValueMin[i]<k)
		 {	
		  k=air->AirSensor_PressureValueMin[i];
		 }
	 }
	air->MinDiff=j-k;
	//计算4路最大值的最大差异
	j=0;
	k=0xffff;
	for(i=0;i<4;i++)
	 {
	  if(air->AirSensor_PressureValueMax[i]>j)
		 {	
		  j=air->AirSensor_PressureValueMax[i];
			m=i;
		 }
		if(air->AirSensor_PressureValueMax[i]<k)
		 {	
		  k=air->AirSensor_PressureValueMax[i];
		 }
	 }
	air->MaxDiff=j-k;
	 
	 
	//计算4路平均值的最大差异
	j=0;
	k=0xffff;
	for(i=0;i<4;i++)
	 {
	  if(air->AirSensor_PressureValueAvg[i]>=j)
		 {	
		  j=air->AirSensor_PressureValueAvg[i];
			m=i;
		 }
		if(air->AirSensor_PressureValueAvg[i]<k)
		 {	
		  k=air->AirSensor_PressureValueAvg[i];
			n=i;
		 }
	 }
	air->AvgDiff=j-k;
	 
	//计算各路与最小值的差异
  for(i=0;i<4;i++)
   {
		air->AirSensor_PressureValue_Diff[i]=air->AirSensor_PressureValueAvg[i]-air->AirSensor_PressureValueAvg[n]; 
   }	

/*
  //新增处理算法2019年11月19日	

  //先判断当前各路相对差异值跟未检测到人时各路差异值哪路减少最多
	
  newmax=0;
  newm=0;	 
	if(air->AirSensor_noperson_state_checked_istrue == 1)
	{
    for(i=0;i<4;i++)		
		{
			if(air->AirSensor_PressureValue_Diff[i]<=air->AirSensor_PressureValue_Diff_last_noperson[i])
			{
				if((air->AirSensor_PressureValue_Diff_last_noperson[i]-air->AirSensor_PressureValue_Diff[i])>= newmax)
				{
					newmax=air->AirSensor_PressureValue_Diff_last_noperson[i]-air->AirSensor_PressureValue_Diff[i];
					newm=i;
				}
			}
		}
		
		//把变小的差异也加到其他各路，反应气路的变化
    for(i=0;i<4;i++)
	  {
			if(i!=newm)
			{
		    air->AirSensor_PressureValue_Diff_filtered_new[i]= air->AirSensor_PressureValue_Diff[i]+newmax;
				if(air->AirSensor_PressureValue_Diff_filtered_new[i]>=air->AirSensor_PressureValue_Diff_last_noperson[i])
				{
					air->AirSensor_PressureValue_Diff_filtered_new[i]= air->AirSensor_PressureValue_Diff_filtered_new[i]-air->AirSensor_PressureValue_Diff_last_noperson[i];
				}
				else
				  air->AirSensor_PressureValue_Diff_filtered_new[i]=0;	
			}
			
	  }		
    air->AirSensor_PressureValue_Diff_filtered_new[newm]=0;	
		air->AirSensor_PressureValue_Diff_filtered[0] = air->AirSensor_PressureValue_Diff_filtered_new[0];
		air->AirSensor_PressureValue_Diff_filtered[1] = air->AirSensor_PressureValue_Diff_filtered_new[1];
		air->AirSensor_PressureValue_Diff_filtered[2] = air->AirSensor_PressureValue_Diff_filtered_new[2];
		air->AirSensor_PressureValue_Diff_filtered[3] = air->AirSensor_PressureValue_Diff_filtered_new[3];
		
		//对数据进行排序
		
		j=0;
	  k=0xffff;
	  for(i=0;i<4;i++)
	   {
	    if(air->AirSensor_PressureValue_Diff_filtered[i]>=j)
		   {	
		    j=air->AirSensor_PressureValue_Diff_filtered[i];
			  m=i;
		   }
		  if(air->AirSensor_PressureValue_Diff_filtered[i]<k)
		   {	
		    k=air->AirSensor_PressureValue_Diff_filtered[i];
			  n=i;
		   }
	   }
	  air->AvgDiff=j-k;
		
		max=0;
	  min=0xffff;
	  for(i=0;i<4;i++)
    {	 
	    if(i!=m && i!=n)
		  {
			  if(air->AirSensor_PressureValue_Diff_filtered[i]>=max)
			  {
				  max=air->AirSensor_PressureValue_Diff_filtered[i];
				  mm=i;
			  }
			  if(air->AirSensor_PressureValue_Diff_filtered[i]<min)
			  {
				  min=air->AirSensor_PressureValue_Diff_filtered[i];
				  nn=i;
			  }
		  }
	  }
		
		
	}		 
*/
//	else
//	{		
  
  //对各路相比最小值的差异值进行滤波	 
	//找出第二大和第三大值的位置
	max=0;
	min=0xffff;
	for(i=0;i<4;i++)
  {	 
	  if(i!=m && i!=n)
		{
			if(air->AirSensor_PressureValue_Diff[i]>=max)
			{
				max=air->AirSensor_PressureValue_Diff[i];
				mm=i;
			}
			if(air->AirSensor_PressureValue_Diff[i]<min)
			{
				min=air->AirSensor_PressureValue_Diff[i];
				nn=i;
			}
		}
	}
	//diff从大到小顺序为m,mm,nn,n
	
	
	
	
	//对最高两路的值进行简单再滤波
	air->AirSensor_PressureValue_Diff_filtered[m]=air->AirSensor_PressureValue_Diff[m];
	air->AirSensor_PressureValue_Diff_filtered[mm]=air->AirSensor_PressureValue_Diff[mm];
	air->AirSensor_PressureValue_Diff_filtered[nn]=air->AirSensor_PressureValue_Diff[nn];
	air->AirSensor_PressureValue_Diff_filtered[n]=air->AirSensor_PressureValue_Diff[n];


#if 0
	min = air->AirSensor_PressureValue_Diff[nn]/2;
	
	air->AirSensor_PressureValue_Diff_filtered[m]=air->AirSensor_PressureValue_Diff[m]-min;	
	air->AirSensor_PressureValue_Diff_filtered[mm]=air->AirSensor_PressureValue_Diff[mm]-min;
	
	air->AirSensor_PressureValue_Diff_filtered[nn]=min;
#endif
	
	air->AvgDiff=air->AirSensor_PressureValue_Diff_filtered[m]; 
	air->AvgDiff_original=air->AirSensor_PressureValue_Diff[m];
	
	
// }
	
	
	
	/*
	
	if(n==0 || n==3)
  {
     air->AirSensor_PressureValue_Diff_filtered[m]=air->AirSensor_PressureValue_Diff[m]-air->AirSensor_PressureValue_Diff[nn];
		 air->AirSensor_PressureValue_Diff_filtered[mm]=air->AirSensor_PressureValue_Diff[mm]-air->AirSensor_PressureValue_Diff[nn];
     air->AirSensor_PressureValue_Diff_filtered[nn]=air->AirSensor_PressureValue_Diff[nn]-air->AirSensor_PressureValue_Diff[nn];
  }
  
  else if(n==1)
	{
		if(m!=0)
		{
      air->AirSensor_PressureValue_Diff_filtered[m]=air->AirSensor_PressureValue_Diff[m]-air->AirSensor_PressureValue_Diff[0];
			if(air->AirSensor_PressureValue_Diff[mm]>air->AirSensor_PressureValue_Diff[0])				
		    air->AirSensor_PressureValue_Diff_filtered[mm]=air->AirSensor_PressureValue_Diff[mm]-air->AirSensor_PressureValue_Diff[0];
			else
				air->AirSensor_PressureValue_Diff_filtered[mm]=0;
			if(air->AirSensor_PressureValue_Diff[nn]>air->AirSensor_PressureValue_Diff[0])				
		    air->AirSensor_PressureValue_Diff_filtered[nn]=air->AirSensor_PressureValue_Diff[nn]-air->AirSensor_PressureValue_Diff[0];
			else
				air->AirSensor_PressureValue_Diff_filtered[nn]=0;      
		}
		else 
		{
			air->AirSensor_PressureValue_Diff_filtered[m]=air->AirSensor_PressureValue_Diff[m]-air->AirSensor_PressureValue_Diff[mm];					
		  air->AirSensor_PressureValue_Diff_filtered[mm]=0;
			air->AirSensor_PressureValue_Diff_filtered[nn]=0;			
		}
	}
  else
	{
    if(m!=3)
		{
      air->AirSensor_PressureValue_Diff_filtered[m]=air->AirSensor_PressureValue_Diff[m]-air->AirSensor_PressureValue_Diff[3];
			if(air->AirSensor_PressureValue_Diff[mm]>air->AirSensor_PressureValue_Diff[3])				
		    air->AirSensor_PressureValue_Diff_filtered[mm]=air->AirSensor_PressureValue_Diff[mm]-air->AirSensor_PressureValue_Diff[3];
			else
				air->AirSensor_PressureValue_Diff_filtered[mm]=0;
			if(air->AirSensor_PressureValue_Diff[nn]>air->AirSensor_PressureValue_Diff[3])				
		    air->AirSensor_PressureValue_Diff_filtered[nn]=air->AirSensor_PressureValue_Diff[nn]-air->AirSensor_PressureValue_Diff[3];
			else
				air->AirSensor_PressureValue_Diff_filtered[nn]=0;      
		}
		else 
		{
			air->AirSensor_PressureValue_Diff_filtered[m]=air->AirSensor_PressureValue_Diff[m]-air->AirSensor_PressureValue_Diff[mm];					
		  air->AirSensor_PressureValue_Diff_filtered[mm]=0;
			air->AirSensor_PressureValue_Diff_filtered[nn]=0;			
		}
	}		
	air->AirSensor_PressureValue_Diff_filtered[n]=0;
  air->AvgDiff=air->AirSensor_PressureValue_Diff_filtered[m];
	air->AvgDiff_original=air->AirSensor_PressureValue_Diff[m];

	*/
	

	 
	//计算最大值路及旁边较大值之和（考虑睡两个枕头之间情况）
  error=0;
	if(m==0)
	 {
	  air->AvgDiff1=air->AirSensor_PressureValue_Diff_filtered[1];
		air->AvgDiff1_original=air->AirSensor_PressureValue_Diff[1]; 
		n=1;
		if(PillowMaxOne_Willbesend==0x10)			
		 {
		  if(air->AvgDiff1<air->AirSensor_PressureValue_Diff[2] && air->AvgDiff1<air->AirSensor_PressureValue_Diff[3])
		   error=1;
		 }
	 }
	else if(m==3)
	 {
	  air->AvgDiff1 = air->AirSensor_PressureValue_Diff_filtered[2];
		air->AvgDiff1_original = air->AirSensor_PressureValue_Diff[2];
		n=2;
		if(PillowMaxOne_Willbesend==0x10) 
		 {
		  if(air->AvgDiff1<air->AirSensor_PressureValue_Diff[1] && air->AvgDiff1<air->AirSensor_PressureValue_Diff[0])
 			 error=1;
		 }
	 }		 
	else if(m==1)
	 {
		if(air->AirSensor_PressureValue_Diff_filtered[0]>air->AirSensor_PressureValue_Diff_filtered[2])
		 { 
			 air->AvgDiff1=air->AirSensor_PressureValue_Diff_filtered[0];
			 air->AvgDiff1_original = air->AirSensor_PressureValue_Diff[0];
			 n=0;
			 if(PillowMaxOne_Willbesend==0x10)  
			 {
			  if(air->AvgDiff1<air->AirSensor_PressureValue_Diff[3])
		     error=1;
		   }
		 }
		else
		 {
			 air->AvgDiff1=air->AirSensor_PressureValue_Diff_filtered[2];
			 air->AvgDiff1_original = air->AirSensor_PressureValue_Diff[2];
			 n=2;
			 if(PillowMaxOne_Willbesend==0x10)  
			  {
			   if(air->AvgDiff1<air->AirSensor_PressureValue_Diff[3])
		      error=1;
		    }
			}
   }		
	else if(m==2)
	 {
		if(air->AirSensor_PressureValue_Diff_filtered[3]>air->AirSensor_PressureValue_Diff_filtered[1])
		 {
			 air->AvgDiff1=air->AirSensor_PressureValue_Diff_filtered[3];
			 air->AvgDiff1_original = air->AirSensor_PressureValue_Diff[3];
			 n=3;
			 if(PillowMaxOne_Willbesend==0x10)  
			  {
			   if(air->AvgDiff1<air->AirSensor_PressureValue_Diff[0])
		      error=1;
		    }
		 }
		else
		 {
			 air->AvgDiff1=air->AirSensor_PressureValue_Diff_filtered[1];
			 air->AvgDiff1_original = air->AirSensor_PressureValue_Diff[1];
			 n=1;
			 if(PillowMaxOne_Willbesend==0x10)  
			  {
			   if(air->AvgDiff1<air->AirSensor_PressureValue_Diff[0])
		      error=1;
		    }
		 }
   }	

  secondtest=n;//for debug only	
	 
	air->AvgDiff_Total= air->AvgDiff+air->AvgDiff1;
	air->AvgDiff_Total_original= air->AvgDiff_original+air->AvgDiff1_original;	 
	 
	max_last=air->AirSensor_Max_last_bag;
	air->AirSensor_Max_last_bag=m; 
	
	 
	if((air->AvgDiff >= Airsensor_Max_Diff_airpressure_one_bag) || (air->AvgDiff_Total >= Airsensor_Max_Diff_airpressure_two_bag))		
	{
		PillowMaxOne_Willbesend=0x10;
		Pillow2ndMaxOne_Willbesend=0x10;
		air->AirSensor_vale_exceed_upvalue_istrue=1;
		
		return;
	} 
	 
	air->AirSensor_vale_exceed_upvalue_istrue=0; 
	 
	 
  error=0; //如果error不强制设为0，则会做第二大值小于其他两个位置的错误检测
	 
	if(error==0)
	 {		
	  if(air->AvgDiff >= air->headposition_sensitivity_person_on || ((air->AvgDiff >= air->headposition_sensitivity_person_snore)&&(air->AirSensor_Checkposition_Mode==3))
		 ||((air->AvgDiff >= air->headposition_sensitivity_person_snore)&&(air->AirSensor_Checkposition_Mode==1)&&(PillowMaxOne_Willbesend!=0x10))
		 || air->AvgDiff_Total>=air->headposition_sensitivity_person1_on || ((air->AvgDiff_Total >= air->headposition_sensitivity_person1_snore)&&(air->AirSensor_Checkposition_Mode==3))
		 ||((air->AvgDiff_Total >= air->headposition_sensitivity_person1_snore)&&(air->AirSensor_Checkposition_Mode==1)&&(PillowMaxOne_Willbesend!=0x10)))
//		 ||((air->AvgDiff_original >= AirSensor_threshold_person_snore)&&(air->AirSensor_Checkposition_Mode==3))
//		 ||((air->AvgDiff_Total_original >= AirSensor_threshold_person1_snore)&&(air->AirSensor_Checkposition_Mode==3)))
		
	   {
			//再次检查是否其他路气压下降导致该路增大的误检
			if(air->AirSensor_position_checked_by_fastmode_flag==1)
			{
				
				if(PillowMaxOne_Willbesend!=m)
				{
					
				
				  //在头部位置测试模式下，处于连续检测模式，并且已经检测过至少一次，保留有上一次各路的平均值
				  if(air->AirSensor_PressureValueAvg[m]> air->Airsensor_checkposition_valueavg_last[m]+80 || PillowMaxOne_Willbesend==n 
//						|| (air->AirSensor_PressureValue_Diff_filtered[m] > 2*air->AirSensor_PressureValue_Diff_filtered[nn])
					  || ( Pillow2ndMaxOne_Willbesend==m) || (m==max_last)
						|| ((PillowMaxOne_Willbesend==0x10) && air->AirSensor_vale_exceed_upvalue_flag==1))					
				  {
					  PillowMaxOne_Willbesend=m;
		        Pillow2ndMaxOne_Willbesend=n;
				  }
				  else
				  {
					
					  PillowMaxOne_Willbesend=0x10;
		        Pillow2ndMaxOne_Willbesend=0x10;
				  }
			  }	
        else
				{
           PillowMaxOne_Willbesend=m;
		       Pillow2ndMaxOne_Willbesend=n;
				}					
			}
			else
			{				
	      PillowMaxOne_Willbesend=m;
		    Pillow2ndMaxOne_Willbesend=n;
			}
//			if(AirSensor_Checkposition_Mode==3)
//			{				
//			  personon_by_positioncheck_flag=1;
//				air->AirSensor_PersonOn=1;
//			}
			
//			if(air->AvgDiff <AirSensor_threshold_Sidesleep)
//			 Side_Sleep=1;
//			else
//			 Side_Sleep=0;
		}
		else
		{
			if(air->AirSensor_Checkposition_Mode==2)
			{
			  if(PillowMaxOne_Willbesend==m && air->AvgDiff >=air->headposition_sensitivity_person_on1 && air->AirSensor_position_checked_by_fastmode_flag==1)					
				{
					  PillowMaxOne_Willbesend=m;
		        Pillow2ndMaxOne_Willbesend=n;
				}
				else
				{
					
					  PillowMaxOne_Willbesend=0x10;
		        Pillow2ndMaxOne_Willbesend=0x10;
					
					  
					  if(air->AirSensor_PressureValue_Diff[0]>100 || air->AirSensor_PressureValue_Diff[1]>100
							  || air->AirSensor_PressureValue_Diff[0]>100 || air->AirSensor_PressureValue_Diff[0]>100)
						{
							
						}
						else
						{
							air->AirSensor_noperson_state_checked_istrue=1;
				      air->AirSensor_PressureValue_Diff_last_noperson[0]=air->AirSensor_PressureValue_Diff[0];
				      air->AirSensor_PressureValue_Diff_last_noperson[1]=air->AirSensor_PressureValue_Diff[1];
				      air->AirSensor_PressureValue_Diff_last_noperson[2]=air->AirSensor_PressureValue_Diff[2];
				      air->AirSensor_PressureValue_Diff_last_noperson[3]=air->AirSensor_PressureValue_Diff[3];	
            }							
				}
			
			}
      else
			{				
		    PillowMaxOne_Willbesend=0x10;
		    Pillow2ndMaxOne_Willbesend=0x10;
				
				//临时增加调试
				if(air->AirSensor_PressureValue_Diff[0]>100 || air->AirSensor_PressureValue_Diff[1]>100
							  || air->AirSensor_PressureValue_Diff[0]>100 || air->AirSensor_PressureValue_Diff[0]>100)
				{
							
				}
				
				else
				{
				  air->AirSensor_noperson_state_checked_istrue=1;
				
				  air->AirSensor_PressureValue_Diff_last_noperson[0]=air->AirSensor_PressureValue_Diff[0];
				  air->AirSensor_PressureValue_Diff_last_noperson[1]=air->AirSensor_PressureValue_Diff[1];
				  air->AirSensor_PressureValue_Diff_last_noperson[2]=air->AirSensor_PressureValue_Diff[2];
				  air->AirSensor_PressureValue_Diff_last_noperson[3]=air->AirSensor_PressureValue_Diff[3];
				}					
				
			}
//			if(AirSensor_Checkposition_Mode==3)
//			{	
//			  personon_by_positioncheck_flag=0;
//        air->AirSensor_PersonOn=0;	
//			}				
//			Side_Sleep=0;
	  }		
	}		 
	 
	
	
	
/*
		if(m==0)
		 Pillow2ndMaxOne_Willbesend=1;
		else if(m==3)
		 Pillow2ndMaxOne_Willbesend=2;
		else if(m==1)
		 {
			if(air->AirSensor_PressureValueAvg[0]>air->AirSensor_PressureValueAvg[2])
			 Pillow2ndMaxOne_Willbesend=0;
			else
			 Pillow2ndMaxOne_Willbesend=2;	
		 }
		else
		 {
			if(air->AirSensor_PressureValueAvg[3]>air->AirSensor_PressureValueAvg[1])
			 Pillow2ndMaxOne_Willbesend=3;
			else
			 Pillow2ndMaxOne_Willbesend=1;	
		 }
		 */
	 
	else
	 {
		PillowMaxOne_Willbesend=0x10;
		Pillow2ndMaxOne_Willbesend=0x10;
	 }		 
}





#if 1
void AirSensor_monitor_Pressure_AvgValue1(Airsensor_struct_Def * air)
{
	  uint8_t i,j;
	  uint16_t sum=0;
	  uint16_t pointer=0;
    if(air->AirSensor_BufferRefresh==0)
		{
			if(air->AirSensor_PressureTest_WR_pointer<AirSensor_Avgvalue_Step1)
			{
				for(i=0;i<air->AirSensor_PressureTest_WR_pointer;i++)
				{
					sum=sum+air->AirSensor_PressureTest[i];
				}
				air->AirSensor_Avgvalue1=sum/i;        				
			}
			else
			{
				for(i=0;i<AirSensor_Avgvalue_Step1;i++)
				{
					sum=sum+air->AirSensor_PressureTest[air->AirSensor_PressureTest_WR_pointer-i-1];
				}
				air->AirSensor_Avgvalue1=sum/AirSensor_Avgvalue_Step1;
			}
		}
		else
		{
			if(air->AirSensor_PressureTest_WR_pointer==0)
			{
				pointer=AirSensor_PressureTest_buffer_length;
			}
			else
			{
				pointer=air->AirSensor_PressureTest_WR_pointer;
			}
			if(pointer<AirSensor_Avgvalue_Step1)
			{
				for(i=0;i<pointer;i++)
				{
					sum=sum+air->AirSensor_PressureTest[i];
				}
				for(j=0;j<AirSensor_Avgvalue_Step1-i;j++)
				{
					sum=sum+air->AirSensor_PressureTest[AirSensor_PressureTest_buffer_length-j-1];
				}
				air->AirSensor_Avgvalue1=sum/AirSensor_Avgvalue_Step1;
				
			}
			else
			{
				for(i=0;i<AirSensor_Avgvalue_Step1;i++)
				{
					sum=sum+air->AirSensor_PressureTest[pointer-i-1];
				}
				air->AirSensor_Avgvalue1=sum/AirSensor_Avgvalue_Step1;
			}
		}
		air->AirSensor_Avgvalue_Buffer_forMaxMin1[air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer]=air->AirSensor_Avgvalue1;    		
}

#endif



#if 1  //没有了之前的filter，直接从air->AirSensor_PressureTest中取值
void AirSensor_monitor_Pressure_AvgValue(Airsensor_struct_Def * air)
{
	  uint8_t i,j;
	  uint32_t sum=0;
	  uint16_t pointer=0;
	  
    if(air->AirSensor_BufferRefresh==0)
		{
			
			if(air->AirSensor_PressureTest_WR_pointer<AirSensor_Avgvalue_Step)
			{
				for(i=0;i<air->AirSensor_PressureTest_WR_pointer;i++)
				{
					sum=sum+air->AirSensor_PressureTest[i];
				}
				air->AirSensor_Avgvalue=sum/i;	        				
			}
			else
			{
				for(i=0;i<AirSensor_Avgvalue_Step;i++)
				{
					sum=sum+air->AirSensor_PressureTest[air->AirSensor_PressureTest_WR_pointer-i-1];
				}
				air->AirSensor_Avgvalue=sum/AirSensor_Avgvalue_Step;
			}
			
			if(air->power_on_set_value_flag==0)
			{
        air->AirSensor_Pressure_from_noperson_to_Personon=	air->AirSensor_Avgvalue;
        air->AirSensor_pressure_stable_value_last=	air->AirSensor_Avgvalue;
				air->power_on_set_value_flag=1;
			}
			
		}
		else
		{
			
			if(air->AirSensor_PressureTest_WR_pointer==0)
			{
				pointer=AirSensor_PressureTest_buffer_length;
			}
			else
			{
				pointer=air->AirSensor_PressureTest_WR_pointer;
			}
			if(pointer<AirSensor_Avgvalue_Step)
			{
				for(i=0;i<pointer;i++)
				{
					sum=sum+air->AirSensor_PressureTest[i];
				}
				for(j=0;j<AirSensor_Avgvalue_Step-i;j++)
				{
					sum=sum+air->AirSensor_PressureTest[AirSensor_PressureTest_buffer_length-j-1];
				}
				air->AirSensor_Avgvalue=sum/AirSensor_Avgvalue_Step;
			}
			else
			{
				for(i=0;i<AirSensor_Avgvalue_Step;i++)
				{
					sum=sum+air->AirSensor_PressureTest[pointer-i-1];
				}
				air->AirSensor_Avgvalue=sum/AirSensor_Avgvalue_Step;
			}
		}
     
		for(i=0;i<AirSensor_Avgvalue_Buffer_Size-1;i++)
		{
			air->AirSensor_Avgvalue_Buffer[i]=air->AirSensor_Avgvalue_Buffer[i+1];
		}
		air->AirSensor_Avgvalue_Buffer[AirSensor_Avgvalue_Buffer_Size-1]=air->AirSensor_Avgvalue;

//		air->AirSensor_Avgvalue_Buffer_forMaxMin[air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer]=air->AirSensor_Avgvalue;
		air->AirSensor_stepnum_between_twostablestate++;
    if(air->AirSensor_Avgvalue>=air->AirSensor_Maxvalue)
		{
			air->AirSensor_Maxvalue=air->AirSensor_Avgvalue;			
			air->Max_or_Min_detected=1;
					
			if(air->AirSensor_PersonOn==0)
			{
			  if((air->AirSensor_Maxvalue-air->AirSensor_Minvalue)>person_on_threshold1)
				{
					if(air->AirSensor_stable_min_detected_flag==0)
					{
						air->AirSensor_stable_min_detected_flag=1;

//						Led_OnOff_Timer=0;
//						air->Led_Person_OnOff=1;
//						WrokingLed(RED_LED,1);
						
//						Led_OnOff_Timer=0;
//						air->Led_Person_OnOff=1;
// 					  WrokingLed(BLUE_LED_HIGH,2);
						
					}
			  }
		  }					
    }
				 
	  if(air->AirSensor_Avgvalue<air->AirSensor_Minvalue)
		{
			air->AirSensor_Minvalue=air->AirSensor_Avgvalue;
			air->Max_or_Min_detected=0;
		}		
		
		//调用另外一种滤波参数
		AirSensor_monitor_Pressure_AvgValue1(air);

		//保存采样原始数据
		air->AirSensor_Sample_Avgvalue_Buffer[air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer]=air->AirSensor_pressure_sample_avg;
		air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer++;
		if(air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer>=AirSensor_Avgvalue_Buffer_forMaxMin_Size)
      air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer=0;

    		
    air->AirSensor_Avgvalue_Last=air->AirSensor_Avgvalue;	
    		
}

#endif


//针对原始采样数据
//判断单次采样数据之间下降幅度是否超过阀值
uint8_t AirSensor_getupaction_filter1(Airsensor_struct_Def * air)
{
    uint16_t i,j,stable_position;
	  uint16_t stable_pressure,last_pressure,max_pressure,min_pressure,temp;
	  uint16_t max_position,min_position;
	  if((air->AirSensor_stepnum_between_twostablestate+1)>AirSensor_Avgvalue_Buffer_forMaxMin_Size)
		{
			return 0;
		}
	
	  i = air->AirSensor_stepnum_between_twostablestate;//两次稳定数据之间的步数	  
	  j = air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer;//最后数据+1的位置
	  if(j==0)
			j = AirSensor_Avgvalue_Buffer_forMaxMin_Size-1;
		else
			j--;
		stable_position=j;
	  stable_pressure = air->AirSensor_Sample_Avgvalue_Buffer[j];//取最后一个值
		min_pressure=stable_pressure;
		max_pressure=stable_pressure;
		max_position=j;
		min_position=j;
		temp=0;
    while(i!=0)
		{
		  if(j==0)
			  j = AirSensor_Avgvalue_Buffer_forMaxMin_Size-1;
		  else
			  j--;
			last_pressure = air->AirSensor_Sample_Avgvalue_Buffer[j];//取前一个值
			i--;	
			if((max_pressure-last_pressure)<sample_pressure_stepdown_threshold)
			{
				temp=1;
				break;
			}
      max_pressure=last_pressure;			
    }
		if(temp==0)
		  return 0;
		else
			return 1;
    		
}



uint8_t AirSensor_getupaction_filter(Airsensor_struct_Def * air)
{
    uint16_t i,j,stable_position;
	  uint16_t stable_pressure,last_pressure,max_pressure,min_pressure,temp;
	  uint16_t max_position,min_position;
	  i = air->AirSensor_stepnum_between_twostablestate;//两次稳定数据之间的步数	  
	  j = air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer;//最后数据+1的位置
	  if(j==0)
			j = AirSensor_Avgvalue_Buffer_forMaxMin_Size-1;
		else
			j--;
		stable_position=j;
	  stable_pressure = air->AirSensor_Avgvalue_Buffer_forMaxMin1[j];//取最后一个值
		min_pressure=stable_pressure;
		max_pressure=stable_pressure;
		max_position=j;
		min_position=j;
		temp=0;
    while(i!=0)
		{
		  if(j==0)
			  j = AirSensor_Avgvalue_Buffer_forMaxMin_Size-1;
		  else
			  j--;
			last_pressure = air->AirSensor_Avgvalue_Buffer_forMaxMin1[j];//取前一个值
			i--;	
			if(last_pressure<max_pressure)
			{
				temp=1;
				break;
			}
      max_pressure=last_pressure;			
    }
		if(temp==0)
		  return 0;
		else
			return 1;
    		
}


void AirSensor_find_valid_MaxMin(Airsensor_struct_Def * air)
{
	
	  uint8_t i,j,nn,mm,up_start_flag=0,down_counter,up_counter,stable_position,temp1,MAX_position,len;
	  uint16_t current_pressure,last_pressure,max_pressure,min_pressure,temp;
	  uint8_t stepnum_between_downMIN_and_mostMIN,mostMIN_position;
	  uint16_t mostMIN;
	  uint8_t find_special_up_period_flag=0,stepnum_between_mostMIN_and_startUP;
	  uint8_t error_up_again_flag=0,find_agin_special_down_period_flag=0;
	  uint16_t up_MAX=0,up_down_again_MIN=0;
	  uint8_t temp2;
	  int offset=0;
	  

	  
//		osDelay(20);
	
	
		air->AirSensor_stable_noresult_flag=0;
		if((air->AirSensor_stepnum_between_twostablestate+1)>AirSensor_Avgvalue_Buffer_forMaxMin_Size)
		{
			air->AirSensor_PersonOn1=1;
			return;
		}
		
		//寻找持续较大下降的那一段，即特征下降阶段
		//*************************************************************************
		
	  i = air->AirSensor_stepnum_between_twostablestate;//两次稳定数据之间的步数	  
	  j = air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer;//最后数据+1的位置
	  if(j==0)
			j = AirSensor_Avgvalue_Buffer_forMaxMin_Size-1;
		else
			j--;
		stable_position=j;
	  current_pressure = air->AirSensor_Avgvalue_Buffer_forMaxMin1[j];//取最后一个值
		min_pressure=current_pressure;
		max_pressure=current_pressure;
    air->down_max=0;
		air->down_stepnum=0;
		
		while(i!=0)
		{

#if 0			
			offset=0;
			offset += sprintf(SendData+offset,"| ");
 	 	  offset += sprintf(SendData+offset,"%4d ",air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer);
			offset += sprintf(SendData+offset,"%4d ",j); 
      offset += sprintf(SendData+offset,"%4d ",i); 
			offset += sprintf(SendData+offset,"%4d ",current_pressure);
      offset += sprintf(SendData+offset,"| ");
			offset += sprintf(SendData+offset,"* ");
      offset += sprintf(SendData+offset,"%4d ",last_pressure);
			offset += sprintf(SendData+offset,"%4d ",current_pressure);
			offset += sprintf(SendData+offset,"* ");
			offset += sprintf(SendData+offset,"# ");
			
			offset += sprintf(SendData+offset,"%4d ",max_pressure);
			offset += sprintf(SendData+offset,"%4d ",min_pressure);
			offset += sprintf(SendData+offset,"%4d ",air->down_stepnum);
			offset += sprintf(SendData+offset,"%4d ",air->down_max);
			offset += sprintf(SendData+offset,"%4d ",air->min_stepnosition);			
			offset += sprintf(SendData+offset,"#\n");
		 
   
	    uart_send(SendData, strlen(SendData));
		 
			osDelay(60);
#endif			
			
			
			if(j==0)
			  j = AirSensor_Avgvalue_Buffer_forMaxMin_Size-1;
		  else
			  j--;
			last_pressure = air->AirSensor_Avgvalue_Buffer_forMaxMin1[j];//取前一个值
			i--;
			if(last_pressure<current_pressure+stable_up_down_threshold)
			{
				if(max_pressure>min_pressure+main_down_period_threshold)
				{
//					air->down_total=max_pressure-min_pressure;
					break;
				}
				else
				{
					min_pressure=last_pressure;
		      max_pressure=last_pressure;
					air->down_stepnum=0;
					air->down_max=0;
					current_pressure=last_pressure;
					air->min_stepnosition=j;
				}
			}
			else
			{
				if((last_pressure-current_pressure)>air->down_max)
					air->down_max=last_pressure-current_pressure;
				max_pressure=last_pressure;
			  air->down_stepnum++;
				current_pressure=last_pressure;				
			}
			
		}
		air->down_total=max_pressure-min_pressure;
//		down_avg=(float)air->down_total/(float)air->down_stepnum;
//		down_acc=down_avg/(float)air->down_stepnum;
	  air->down_MAX=max_pressure;
		air->down_MIN=min_pressure;
		len=i;
		
		MAX_position=j;
		if(i!=0)
		{
		  MAX_position++;
			i++;
		  if(MAX_position>=AirSensor_Avgvalue_Buffer_forMaxMin_Size)
			  MAX_position=0;
		}
		air->stable_stepnum=air->AirSensor_stepnum_between_twostablestate-i;
		air->down_total=air->down_MAX-air->down_MIN;
#if debug_log		
			offset=0;
			offset += sprintf(SendData+offset,"| ");
 	 	  offset += sprintf(SendData+offset,"%4d ",air->down_MAX);//特征下降开始的最大值
			offset += sprintf(SendData+offset,"%4d ",air->down_MIN); //特征下降结束时的最小值
		  offset += sprintf(SendData+offset,"%4d ",air->down_total);//特征下降阶段的下降值
      offset += sprintf(SendData+offset,"%4d ",air->down_stepnum);//特征下降阶段的步数 
			offset += sprintf(SendData+offset,"%4d ",air->down_max);//特征下降期间的单步最大下降值
		  offset += sprintf(SendData+offset,"%4d ",i);//开始特征下降点到这次不稳定开始的步数
		  offset += sprintf(SendData+offset,"%4d ",air->stable_stepnum);//特征下降开始到稳定的步数
      offset += sprintf(SendData+offset,"|\n ");
			   
	    uart_send(SendData, strlen(SendData));
		 
			osDelay(60);
#endif		
		
#if 0		
		if(air->stable_stepnum>70 || air->down_stepnum>28)
		{
  		air->AirSensor_PersonOn1=1;
			air->onoff_result_reason=1;
			return;	
		}
		
#endif
		
		//进一步往当前寻找比特征下降阶段最低值更低的最小值，遇见升高即停止寻找
		//**************************************************************************
		
		i=air->min_stepnosition;
		i++;
		if(i>=AirSensor_Avgvalue_Buffer_forMaxMin_Size)
			i=0;
		while(i!=air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer)
		{
#if 0
			offset=0;			
			offset += sprintf(SendData+offset,"| ");
 	 	  offset += sprintf(SendData+offset,"%4d ",air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer);
			offset += sprintf(SendData+offset,"%4d ",i); 
			offset += sprintf(SendData+offset,"%4d ",air->down_MAX);
      offset += sprintf(SendData+offset,"%4d ",air->AirSensor_Avgvalue_Buffer_forMaxMin[i]);
			offset += sprintf(SendData+offset,"%4d ",air->down_MIN);
			offset += sprintf(SendData+offset,"%4d ",air->down_stepnum);
			offset += sprintf(SendData+offset,"%4d ",air->min_stepnosition);	
      offset += sprintf(SendData+offset,"|\n ");   
	    uart_send(SendData, strlen(SendData));		 
			osDelay(60);
#endif			
			
			if(air->AirSensor_Avgvalue_Buffer_forMaxMin1[i]<=min_pressure)
			{
				min_pressure=air->AirSensor_Avgvalue_Buffer_forMaxMin1[i];
//				air->down_stepnum=air->down_stepnum+i-air->min_stepnosition;
//				air->min_stepnosition=i;
				
			}
			else if(air->AirSensor_Avgvalue_Buffer_forMaxMin1[i]>min_pressure)
			{
				break;				
			}
			i++;
			if(i>=AirSensor_Avgvalue_Buffer_forMaxMin_Size)
				i=0;			
		}	
//		air->down_total=air->down_MAX-air->down_MIN;
		
		mostMIN=min_pressure;
		stepnum_between_downMIN_and_mostMIN=i-air->min_stepnosition-1;
		mostMIN_position=i-1;
#if debug_log	
			offset=0;
			offset += sprintf(SendData+offset,"| ");
		offset += sprintf(SendData+offset,"特征下降最小值点:%4d ",mostMIN);//特征下降结束后的更小值
			offset += sprintf(SendData+offset,"%4d ",stepnum_between_downMIN_and_mostMIN); //特征下降结束时的最小值跟更小值之间的步数		  
      offset += sprintf(SendData+offset,"|\n ");
			   
	    uart_send(SendData, strlen(SendData));
		 
			osDelay(60);
#endif
		
		
		//寻找是否有回升特征阶段开始点
		//*********************************************************************************************
		i=mostMIN_position;
	  last_pressure=mostMIN;
    stepnum_between_mostMIN_and_startUP=0;		
		i++;
		if(i>=AirSensor_Avgvalue_Buffer_forMaxMin_Size)
			i=0;
		
		while(i!=air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer)
		{
			current_pressure=air->AirSensor_Avgvalue_Buffer_forMaxMin1[i];
			
		  if(current_pressure>last_pressure+2)
			{
				break;							
			}
      last_pressure=current_pressure;
      stepnum_between_mostMIN_and_startUP++;			
      i++;
			if(i>=AirSensor_Avgvalue_Buffer_forMaxMin_Size)
				i=0;					
		}
		
		if(i==air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer)
		{
			//没找到特征回升阶段
			find_special_up_period_flag=0;
#if debug_log		
			offset=0;
			offset += sprintf(SendData+offset,"| ");
 	 	  offset += sprintf(SendData+offset,"Not Found Special Up Start Point!",air->AirSensor_Avgvalue_Buffer_forMaxMin1[i-1]);//			  
      offset += sprintf(SendData+offset,"|\n ");
			   
	    uart_send(SendData, strlen(SendData));
		 
			osDelay(60);
#endif
			
			//没有找到特征回升阶段，看看是否能找到异常下降阶段
			
			i=mostMIN_position;
	    last_pressure=mostMIN;
      stepnum_between_mostMIN_and_startUP=0;		
		  i++;
		  if(i>=AirSensor_Avgvalue_Buffer_forMaxMin_Size)
			  i=0;
			while(i!=air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer)
		  {
			  current_pressure=air->AirSensor_Avgvalue_Buffer_forMaxMin1[i];
			
		    if(current_pressure+2<last_pressure)
			  {
				  break;							
			  }				
				
#if 0		
			offset=0;
			offset += sprintf(SendData+offset,"| ");
 	 	  offset += sprintf(SendData+offset,"%4d ",i);
			offset += sprintf(SendData+offset,"%4d ",last_pressure);//
			offset += sprintf(SendData+offset,"%4d ",current_pressure);//	
      offset += sprintf(SendData+offset,"|\n ");
			   
	    uart_send(SendData, strlen(SendData));
		 
			osDelay(60);
#endif					
        last_pressure=current_pressure;
        i++;
			  if(i>=AirSensor_Avgvalue_Buffer_forMaxMin_Size)
				  i=0;				
		  }	
      if(i==air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer)
			{
				//未找到特征回升阶段
				//未找到异常下降阶段
				
				//看看是否一直下降比较多
				if(air->down_MIN>air->AirSensor_Avgvalue_Buffer_forMaxMin1[stable_position])
				{
					//看看是否从平稳态开始特征下降的
					
					if(air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer>MAX_position)
					{
						temp2=air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer-MAX_position;						
					}
					else
					{
            temp2=air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer+AirSensor_Avgvalue_Buffer_forMaxMin_Size-MAX_position;	
					}						
					if(air->AirSensor_stepnum_between_twostablestate<temp2+2)
          {						
					  air->AirSensor_PersonOn1=0;
			      air->onoff_result_reason=13;
			      return;
					}
					else
					{
						air->AirSensor_PersonOn1=1;
			      air->onoff_result_reason=17;
			      return;
					}
				}
				else
				{
				  air->AirSensor_PersonOn1=0;
			    air->onoff_result_reason=2;
			    return;
				}					
			}
			else
			{
				//未找到特征回升阶段
				//找到异常下降阶段
				if(i==0)
			    i = AirSensor_Avgvalue_Buffer_forMaxMin_Size-1;
		    else
			    i--;
			  if(i==0)
			    i = AirSensor_Avgvalue_Buffer_forMaxMin_Size-1;
		    else
			    i--;
				if(air->AirSensor_Avgvalue_Buffer_forMaxMin1[i]+1==last_pressure)
				{
					air->AirSensor_PersonOn1=0;
			    air->onoff_result_reason=3;
			    return;	
				}
			  else
				{
					air->AirSensor_PersonOn1=1;
			    air->onoff_result_reason=4;
			    return;	
				}				
			}			
		}
		else
		{
			find_special_up_period_flag=1;			
#if 0		
			offset=0;
			offset += sprintf(SendData+offset,"| ");
 	 	  offset += sprintf(SendData+offset,"回升开始点:%4d ",air->AirSensor_Avgvalue_Buffer_forMaxMin1[i-1]);//特征回升开始点的值
			offset += sprintf(SendData+offset,"%4d ",i-1); //特征回升开始点位置
			offset += sprintf(SendData+offset,"%4d ",stepnum_between_mostMIN_and_startUP); //最小值点到特征回升开始点位置的步数
      offset += sprintf(SendData+offset,"|\n ");
			   
	    uart_send(SendData, strlen(SendData));
		 
			osDelay(60);
#endif
			
//			if(stepnum_between_mostMIN_and_startUP+stepnum_between_downMIN_and_mostMIN>2)
//			{
//				air->AirSensor_PersonOn1=1;
//			  air->onoff_result_reason=12;
//			  return;	
//			}
			
			//找寻回升阶段的结束点，找到不再上升或者下降为止
			//*************************************************************************
			
	    //变量数接着上面继续用
		  i++;
		  if(i>=AirSensor_Avgvalue_Buffer_forMaxMin_Size)
			  i=0;
		  last_pressure=current_pressure;
		  while(i!=air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer)
		  {
			  current_pressure=air->AirSensor_Avgvalue_Buffer_forMaxMin1[i];
			
		    if(current_pressure<=last_pressure)
			  {
				  break;							
			  }
        last_pressure=current_pressure;
        i++;
			  if(i>=AirSensor_Avgvalue_Buffer_forMaxMin_Size)
				  i=0;					
		  }			
			up_MAX=air->AirSensor_Avgvalue_Buffer_forMaxMin1[i-1];
#if debug_log		
			offset=0;
			offset += sprintf(SendData+offset,"| ");
 	 	  offset += sprintf(SendData+offset,"回升结束点:%4d ",air->AirSensor_Avgvalue_Buffer_forMaxMin1[i-1]);//特征回升结束点的值（不再上升或者开始下降）
			offset += sprintf(SendData+offset,"%4d ",i-1); //特征回升开始点位置
      offset += sprintf(SendData+offset,"|\n ");
			   
	    uart_send(SendData, strlen(SendData));
		 
			osDelay(60);
#endif	

			
			
			//往后继续找寻是否有特征回升阶段之后的特征回降阶段或者异常的再次特征上升
			//变量数接着上面继续用

		  while(i!=air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer)
		  {
			  current_pressure=air->AirSensor_Avgvalue_Buffer_forMaxMin1[i];
			
		    if(current_pressure+2<last_pressure)
			  {
				  break;							
			  }
				
				else if(current_pressure>last_pressure+3)
				{
					error_up_again_flag=1;
				}
				
#if 0		
			offset=0;
			offset += sprintf(SendData+offset,"| ");
 	 	  offset += sprintf(SendData+offset,"%4d ",i);
			offset += sprintf(SendData+offset,"%4d ",last_pressure);//
			offset += sprintf(SendData+offset,"%4d ",current_pressure);//	
      offset += sprintf(SendData+offset,"|\n ");
			   
	    uart_send(SendData, strlen(SendData));
		 
			osDelay(60);
#endif					
        last_pressure=current_pressure;
        i++;
			  if(i>=AirSensor_Avgvalue_Buffer_forMaxMin_Size)
				  i=0;	

				
		  }	

      
      if(i==air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer)
      {
        //无特征回降阶段
				find_agin_special_down_period_flag=0;
#if debug_log		
			offset=0;
			offset += sprintf(SendData+offset,"| ");
 	 	  offset += sprintf(SendData+offset,"Not Found Agin Special Down Start Point!");//			  
      offset += sprintf(SendData+offset,"|\n ");
			   
	    uart_send(SendData, strlen(SendData));
		 
			osDelay(60);
#endif				 
				if(error_up_again_flag==1)
				{
					 if(last_pressure+4==current_pressure)
					 {
					   if(i==0)
			         i = AirSensor_Avgvalue_Buffer_forMaxMin_Size-1;
		         else
			         i--;
					   if(i==0)
			         i = AirSensor_Avgvalue_Buffer_forMaxMin_Size-1;
		         else
			         i--;
					   if(air->AirSensor_Avgvalue_Buffer_forMaxMin1[i]+1==last_pressure)
					   {
						   air->AirSensor_PersonOn1=0;
			         air->onoff_result_reason=5;
			         return;	
					   }
					   else
					   {
						   air->AirSensor_PersonOn1=1;
			         air->onoff_result_reason=6;
			         return;	
					   }
					 }
					 else
					 {
						 air->AirSensor_PersonOn1=1;
			       air->onoff_result_reason=7;
			       return;	
					 }
					
				}
				else
				{
					//有特征上升，无回降阶段
					
					//看看是否上升幅度较大，
					
					if(up_MAX>air->down_MIN+60 || (up_MAX>air->down_MIN+35 && air->down_MIN+120>air->down_MAX))
					{
						//看看是否还是一直上升
						if(air->AirSensor_Avgvalue_Buffer_forMaxMin1[stable_position]>up_MAX)
						{
							air->AirSensor_PersonOn1=0;
			        air->onoff_result_reason=14;
			        return;
						}
						else
						{
							air->AirSensor_PersonOn1=1;
			        air->onoff_result_reason=16;
			        return;	
						}
					}
					
					else
					{
					
					  air->AirSensor_PersonOn1=0;
			      air->onoff_result_reason=8;
			      return;	
					}
				}
			}	
			
      else
      {		

        //找到回升下降阶段				
			  find_agin_special_down_period_flag=1;
#if debug_log		
			offset=0;
			offset += sprintf(SendData+offset,"| ");
				offset += sprintf(SendData+offset,"回升下降开始点:%4d ",air->AirSensor_Avgvalue_Buffer_forMaxMin1[i-1]);//特征回升下降开始点的值
			offset += sprintf(SendData+offset,"%4d ",i-1); //特征回上陆谍开始点位置
      offset += sprintf(SendData+offset,"|\n ");
			   
	    uart_send(SendData, strlen(SendData));
		 
			osDelay(60);
#endif				
			
				//找寻下降结束位置点
				
			  while(i!=air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer)
		    {
			    current_pressure=air->AirSensor_Avgvalue_Buffer_forMaxMin1[i];
			
		      if(current_pressure>=last_pressure)
			    {
				    break;							
			    }
				
				  				
#if 0		
			offset=0;
			offset += sprintf(SendData+offset,"| ");
 	 	  offset += sprintf(SendData+offset,"%4d ",i);
			offset += sprintf(SendData+offset,"%4d ",last_pressure);//
			offset += sprintf(SendData+offset,"%4d ",current_pressure);//	
      offset += sprintf(SendData+offset,"|\n ");
			   
	    uart_send(SendData, strlen(SendData));
		 
			osDelay(60);
#endif					
          last_pressure=current_pressure;
          i++;
			    if(i>=AirSensor_Avgvalue_Buffer_forMaxMin_Size)
				    i=0;
				
		    }
        if(i==air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer)
				{
          //未找到回升下降结束点
					air->AirSensor_PersonOn1=0;
			    air->onoff_result_reason=9;
			    return;	
					
				}
				else
				{
					//找到回升下降结束点
					up_down_again_MIN=air->AirSensor_Avgvalue_Buffer_forMaxMin1[i-1];
					
					
#if debug_log		
			offset=0;
			offset += sprintf(SendData+offset,"| ");
					offset += sprintf(SendData+offset,"回升下降结束点:%4d ",air->AirSensor_Avgvalue_Buffer_forMaxMin1[i-1]);//特征回升下降结束点的值（不再下降或者开始上升）
			offset += sprintf(SendData+offset,"%4d ",i-1); //特征回升下降结束位置
      offset += sprintf(SendData+offset,"|\n ");
			   
	    uart_send(SendData, strlen(SendData));
		 
			osDelay(60);
#endif					
					
					//找寻回升下降结束后是否还有异常上升和结束情况
					
					while(i!=air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer)
		      {
			      current_pressure=air->AirSensor_Avgvalue_Buffer_forMaxMin1[i];
			
		        if(current_pressure>last_pressure+2)
			      {
				      break;							
			      }
						else if(current_pressure+2<last_pressure)
				    {
							break;
						}
				  				
#if 0		
			offset=0;
			offset += sprintf(SendData+offset,"| ");
 	 	  offset += sprintf(SendData+offset,"%4d ",i);
			offset += sprintf(SendData+offset,"%4d ",last_pressure);//
			offset += sprintf(SendData+offset,"%4d ",current_pressure);//	
      offset += sprintf(SendData+offset,"|\n ");
			   
	    uart_send(SendData, strlen(SendData));
		 
			osDelay(60);
#endif					
            last_pressure=current_pressure;
            i++;
			      if(i>=AirSensor_Avgvalue_Buffer_forMaxMin_Size)
				      i=0;
				
		      }	
          
					if(i==air->AirSensor_Avgvalue_Buffer_forMaxMin_Pointer)
					{
						//未发现异常						
						//看看回升下降阶段下降了多少
						
#if debug_log		
			offset=0;
			offset += sprintf(SendData+offset,"| ");
					offset += sprintf(SendData+offset,"回升下降值:%4d ",up_MAX-up_down_again_MIN);//特征回升下降结束点的值（不再下降或者开始上升）
			
      offset += sprintf(SendData+offset,"|\n ");
			   
	    uart_send(SendData, strlen(SendData));
		 
			osDelay(60);
#endif							
						
						if(up_MAX>up_down_again_MIN+15)
						{
							air->AirSensor_PersonOn1=1;
			        air->onoff_result_reason=12;
			        return;	
						}
						else
						{
							
							//如果稳态值比回升下降结束还低
							if(air->AirSensor_Avgvalue_Buffer_forMaxMin1[stable_position]<up_down_again_MIN)
							{
								air->AirSensor_PersonOn1=1;
			          air->onoff_result_reason=15;
			          return;	
							}
							else
							{
						    air->AirSensor_PersonOn1=0;
			          air->onoff_result_reason=10;
			          return;
							}								
						}
					}
					else
					{
						//有异常
						air->AirSensor_PersonOn1=1;
			      air->onoff_result_reason=11;
			      return;	
					}
					
				}				
			}
		}
}


void AirSensor_detect_person_onoff(Airsensor_struct_Def * air)
{
	  uint8_t i,tt=0;
	  uint16_t Maxvalue=0,Minvalue=0xfff;
	
	  
	
	  //判断稳定性计算1
	  for(i=0;i<AirSensor_Avgvalue_Buffer_Size;i++)
	  {
			if(air->AirSensor_Avgvalue_Buffer[i]>Maxvalue)
			{
			  Maxvalue=air->AirSensor_Avgvalue_Buffer[i];
			}
			if(air->AirSensor_Avgvalue_Buffer[i]<Minvalue)
			{
			  Minvalue=air->AirSensor_Avgvalue_Buffer[i];
			}
		}
		//判断稳定性计算2
		for(i=0;i<AirSensor_Avgvalue_Buffer_Size-1;i++)
	  {
		    if(air->AirSensor_Avgvalue_Buffer[i]>air->AirSensor_Avgvalue_Buffer[i+1])
			{
				//吸收掉慢速下降的
				if(air->AirSensor_Avgvalue_Buffer[i]>(air->AirSensor_Avgvalue_Buffer[i+1]+stable_up_down_threshold))
				{
					tt=1;
					break;
				}
			}
			else
			{
				//吸收掉慢速上升的
				if((air->AirSensor_Avgvalue_Buffer[i]+stable_up_down_threshold)<air->AirSensor_Avgvalue_Buffer[i+1])
				{
					tt=1;
					break;
				}
			}
		}
		air->AirSensor_step_last_stable_flag=air->AirSensor_step_stable_flag;
	  if(abs(Maxvalue-Minvalue)<AirSensor_pressure_stable_threshold || tt==0)
		{
			air->AirSensor_step_stable_flag=1;
			air->AirSensor_pressure_stable_value=air->AirSensor_Avgvalue_Buffer[AirSensor_Avgvalue_Buffer_Size-1];
			air->pressure_stable_value_diff_between_now_before=air->AirSensor_pressure_stable_value-air->AirSensor_pressure_stable_value_last;
			
/*			
			if(air->pressure_stable_value_last_is_valid==0)
			{
				air->pressure_stable_value_valid_counter++;
				if(air->pressure_stable_value_valid_counter>=2)
					air->pressure_stable_value_last_is_valid=1;
			}
			if(air->pressure_stable_value_last_is_valid==1)
			{

			  air->pressure_stable_value_diff_between_now_before=AirSensor_pressure_stable_value-air->AirSensor_pressure_stable_value_last;


		  }
*/
			
			//判断处理逻辑
			
			//判断上床
			if(air->AirSensor_PersonOn==0 && air->It_Is_Standby_State==0)
			{
				if(air->AirSensor_stable_min_detected_flag==1)
				{
				  if(air->pressure_stable_value_diff_between_now_before>=air->Body_detect_on_threshold)
					{
						air->personon_by_positioncheck_flag=0;
						air->AirSensor_log_state=1;
						air->personon_by_positioncheck_flag_forpensononoff=0;
						
						air->AirSensor_PersonOn=1; //灯早已指示过有上床
						air->Tt_Is_Sleep_Time=1;
						air->Sleep_snore_study_flag=1;
						air->AirSensor_Pressure_from_noperson_to_Personon=air->AirSensor_pressure_stable_value_last;
						air->Pressure_increase_several_times=0;
						air->period_snore_checkposition_counter=0;
					}
					else
					{
						//指示人离开
//						Led_OnOff_Timer=0;
//				 	  air->Led_Person_OnOff=2;
//						WrokingLed(BLUE_LED_HIGH,2);
						  
//						  WrokingLed(RED_LED,1);
						  air->Led_Person_OnOff=2;
						  Led_OnOff_Timer=0;
						air->Pressure_increase_several_times=air->Pressure_increase_several_times+air->pressure_stable_value_diff_between_now_before;
						
					}
					
					//大致估算人体压力
					air->Body_Pressure_Value=air->pressure_stable_value_diff_between_now_before;
					
				}	
				
				
				
				else if(air->pressure_stable_value_diff_between_now_before<=-10)

				{
					 //指示人离开
//						Led_OnOff_Timer=0;
//						air->Led_Person_OnOff=2;
//						WrokingLed(BLUE_LED_HIGH,2);
					
//					  WrokingLed(RED_LED,1);
						air->Led_Person_OnOff=2;  
					  Led_OnOff_Timer=0;
					
					  air->Body_Moved_Times=0;
				}
			  
				
			}
			//判断离开
			else if(air->It_Is_Standby_State==0)
			{
			 if(air->Check_again_if_person_on==1)
			 {
				 if(air->pressure_stable_value_diff_between_now_before>=air->Body_detect_on_threshold)
				 {
					  air->AirSensor_Pressure_from_noperson_to_Personon=air->AirSensor_pressure_stable_value_last;
						air->Pressure_increase_several_times=0;
						air->period_snore_checkposition_counter=0;
				 }else
				 {
					 air->AirSensor_PersonOn=1;  //0 modified by zjp at 20220813 关闭人体检测判断有人无人
				   air->Tt_Is_Sleep_Time=1;
				   air->Sleep_snore_study_flag=1;
					 
				 }
				 air->Check_again_if_person_on=0;
			 }
			 else
			 {
				if(air->pressure_stable_value_diff_between_now_before>1 || air->pressure_stable_value_diff_between_now_before<-1)
				  air->Body_Pressure_Value=air->Body_Pressure_Value+air->pressure_stable_value_diff_between_now_before;

				
#if 0	//20210312关闭			
				//临时调试增加
				if(air->pressure_stable_value_diff_between_now_before<=-1*(int)air->Body_detect_off_threshold && AirSensor_getupaction_filter1(air)==1 ) //-15.。。-20？
				{
					 air->personon_by_positioncheck_flag=1;
				}
				
#endif				

#if 0	//临时调试关闭	20210117
        if(air->pressure_stable_value_diff_between_now_before<=-1*(int)air->Body_detect_off_threshold) //-15.。。-20？				
//				if(air->pressure_stable_value_diff_between_now_before<=-1*stable_down_threshold) //-15.。。-20？
				{
					//时序判别法，仅根据气压波形变化及时间特性来判断是否为离开
					AirSensor_find_valid_MaxMin(air);
					
					//气压值判别法，气袋内气较多时压上，再加上长时间压上可能出现误判，主要是气袋内气多时，获取的相当于人体的压力会比较大，需要补偿
					
//					if(air->Body_Pressure_Value<abs((int)air->Body_detect_off_threshold))
//						air->AirSensor_PersonOn2=0;
//					else
//						air->AirSensor_PersonOn2=1;
					
//					air->AirSensor_PersonOn=air->AirSensor_PersonOn1*air->AirSensor_PersonOn2;
					
					
					if(air->Body_Pressure_Value<abs(-1*(int)air->Body_detect_off_threshold))
						   air->AirSensor_PersonOn2=0;
					else
						   air->AirSensor_PersonOn2=1;
					
					
					if(air->AirSensor_pressure_stable_value<air->AirSensor_Pressure_from_noperson_to_Personon+abs(stable_threshold_for_person_remain_on))
//					if(air->AirSensor_pressure_stable_value<air->AirSensor_Pressure_from_noperson_to_Personon+abs(-1*stable_down_threshold))
							 air->AirSensor_PersonOn3=0;
					else
							 air->AirSensor_PersonOn3=1;
					
					
          if((air->AirSensor_PersonOn2==0 || air->AirSensor_PersonOn1==0) && air->personon_by_positioncheck_flag_forpensononoff==0)
					{
//						     if(RECHECK_NO_DELAY_FLAG==0)
						
     						 air->AirSensor_PersonOn=0;	
						     air->AirSensor_log_state=2;
						     air->personon_by_positioncheck_flag=1;// 如果为离开需要设置复检标志
						
						     //新增上升幅度到一定程度判为未离开
						     
							   
					}
					else if((air->AirSensor_PersonOn3==1 && air->personon_by_positioncheck_flag_forpensononoff==0) || air->AirSensor_PersonOn1==1)
					{							 
					      air->AirSensor_PersonOn=1;	
                air->personon_by_positioncheck_flag=1;	
                air->AirSensor_log_state=3;						
					      
				  }
					else
					{
						air->AirSensor_PersonOn=0;
					}
/*					
					else if(air->AirSensor_PersonOn1==0)
					{
						   if(RECHECK_NO_DELAY_FLAG==0)
//							    air->AirSensor_PersonOn=0;
						   air->personon_by_positioncheck_flag=1;// 如果为离开需要设置复检标志
						   air->AirSensor_recheck_times=1;
							 
					}
					
					
					else if(air->AirSensor_PersonOn1==1)
					{
						   air->personon_by_positioncheck_flag=1;// 如果为离开需要设置复检标志
						   air->AirSensor_recheck_times=1;
					}
					
*/					
					if(air->AirSensor_stable_noresult_flag==0)
					{
					
//					if(pressure_value_up_stepmax==0)
//						pressure_value_up_stepmax=1;
//				  person_onoff_factor=(float)abs(air->pressure_stable_value_diff_between_now_before)*pressure_value_down_stepavg/(float)pressure_value_up_stepmax;
//					person_onoff_factor1= (float)pressure_value_air->down_stepnum_bak/(float)pressure_value_down_stepmax_bak;
					  if(air->AirSensor_PersonOn==0)
					  {
               
                 
//					       Led_OnOff_Timer=0;
//					       air->Led_Person_OnOff=2;
//					       WrokingLed(BLUE_LED_HIGH,2);
							
//							   WrokingLed(RED_LED,1);
						     air->Led_Person_OnOff=2;
							   Led_OnOff_Timer=0;
							
						
						     air->Body_Moved_Times=0;
//                 air->personon_by_positioncheck_flag=1;// 如果为离开需要设置复检标志

				    }
					  else
					  {
                 
						     air->Body_Moved_Times++;
//							   gStorageData.Snore.HeadMovingTimes++;
							 
					  }
						
				  }

			  }

#endif	

#if 1	//临时调试关闭	20210117
        if(air->pressure_stable_value_diff_between_now_before<=-1*(int)air->Body_detect_off_threshold) //-15.。。-20？				
//				if(air->pressure_stable_value_diff_between_now_before<=-1*stable_down_threshold) //-15.。。-20？
				{
					//时序判别法，仅根据气压波形变化及时间特性来判断是否为离开
//					AirSensor_find_valid_MaxMin(air);
					
					//气压值判别法，气袋内气较多时压上，再加上长时间压上可能出现误判，主要是气袋内气多时，获取的相当于人体的压力会比较大，需要补偿
					
//					if(air->Body_Pressure_Value<abs((int)air->Body_detect_off_threshold))
//						air->AirSensor_PersonOn2=0;
//					else
//						air->AirSensor_PersonOn2=1;
					
//					air->AirSensor_PersonOn=air->AirSensor_PersonOn1*air->AirSensor_PersonOn2;
					
					
					if(air->Body_Pressure_Value<abs(-1*(int)air->Body_detect_off_threshold))
						   air->AirSensor_PersonOn2=0;
					else
						   air->AirSensor_PersonOn2=1;
					
					
					if(air->AirSensor_pressure_stable_value<air->AirSensor_Pressure_from_noperson_to_Personon+abs(stable_threshold_for_person_remain_on))
//					if(air->AirSensor_pressure_stable_value<air->AirSensor_Pressure_from_noperson_to_Personon+abs(-1*stable_down_threshold))
							 air->AirSensor_PersonOn3=0;
					else
							 air->AirSensor_PersonOn3=1;
					
					
          if((air->AirSensor_PersonOn2==0 ) && air->personon_by_positioncheck_flag_forpensononoff==0)
					{
//						     if(RECHECK_NO_DELAY_FLAG==0)
						
 //    						 air->AirSensor_PersonOn=0;	//disabled by zjp at 20220813
						     air->AirSensor_log_state=2;
						     air->personon_by_positioncheck_flag=1;// 如果为离开需要设置复检标志
						
						     //新增上升幅度到一定程度判为未离开
						     
							   
					}
					else if((air->AirSensor_PersonOn3==1 && air->personon_by_positioncheck_flag_forpensononoff==0) )
					{							 
//					      air->AirSensor_PersonOn=1;	//disabled by zjp at 20220813
                air->personon_by_positioncheck_flag=1;	
                air->AirSensor_log_state=3;						
					      
				  }
					
					else
					{
					//	air->AirSensor_PersonOn=0; //diabled by zjp at 20220813
					}
/*					
					else if(air->AirSensor_PersonOn1==0)
					{
						   if(RECHECK_NO_DELAY_FLAG==0)
//							    air->AirSensor_PersonOn=0;
						   air->personon_by_positioncheck_flag=1;// 如果为离开需要设置复检标志
						   air->AirSensor_recheck_times=1;
							 
					}
					
					
					else if(air->AirSensor_PersonOn1==1)
					{
						   air->personon_by_positioncheck_flag=1;// 如果为离开需要设置复检标志
						   air->AirSensor_recheck_times=1;
					}
					
*/					
					if(air->AirSensor_stable_noresult_flag==0)
					{
					
//					if(pressure_value_up_stepmax==0)
//						pressure_value_up_stepmax=1;
//				  person_onoff_factor=(float)abs(air->pressure_stable_value_diff_between_now_before)*pressure_value_down_stepavg/(float)pressure_value_up_stepmax;
//					person_onoff_factor1= (float)pressure_value_air->down_stepnum_bak/(float)pressure_value_down_stepmax_bak;
					  if(air->AirSensor_PersonOn==0)
					  {
               
                 
//					       Led_OnOff_Timer=0;
//					       air->Led_Person_OnOff=2;
//					       WrokingLed(BLUE_LED_HIGH,2);
							
//							   WrokingLed(RED_LED,1);
						     air->Led_Person_OnOff=2;
							   Led_OnOff_Timer=0;
							
						
						     air->Body_Moved_Times=0;
//                 air->personon_by_positioncheck_flag=1;// 如果为离开需要设置复检标志

				    }
					  else
					  {
                 
						     air->Body_Moved_Times++;
//							   gStorageData.Snore.HeadMovingTimes++;
							 
					  }
						
				  }

			  }

#endif	
				
			#if 0	//20200617修改，全部复检
				
				else if(air->pressure_stable_value_diff_between_now_before>5)
				{
					air->Body_Moved_Times++;	
//					gStorageData.Snore.HeadMovingTimes++;
				}
				else if(air->pressure_stable_value_diff_between_now_before<-5)
				{
					//减小变动值小于门限值，但有可能是用户多次变动导致压力逐渐减小，已经离开
          
             if(air->Body_Pressure_Value<abs((int)air->Body_detect_off_threshold))
						   air->AirSensor_PersonOn2=0;
					   else
						   air->AirSensor_PersonOn2=1;
						 if(air->AirSensor_pressure_stable_value<air->AirSensor_Pressure_from_noperson_to_Personon+abs((int)air->Body_detect_off_threshold))
							 air->AirSensor_PersonOn3=0;
						 else
							 air->AirSensor_PersonOn3=1;
						 
             if(air->AirSensor_PersonOn2==0 && air->personon_by_positioncheck_flag_forpensononoff==0)
						 {
							   
//							   Led_OnOff_Timer=0;
//					       air->Led_Person_OnOff=2;
//					       WrokingLed(BLUE_LED_HIGH,2);
							 
							   WrokingLed(RED_LED,1);
						     air->Led_Person_OnOff=2;
						     Led_OnOff_Timer=0;
							 
						     air->Body_Moved_Times=0;
							   if(RECHECK_NO_DELAY_FLAG==0)									 
							      air->AirSensor_PersonOn=0;
							   air->personon_by_positioncheck_flag=1;// 如果为离开需要设置复检标志
                 air->AirSensor_recheck_times=1;							 
						 }
						 else if(air->AirSensor_PersonOn3==1 && air->personon_by_positioncheck_flag_forpensononoff==0)
						 {							 
					      air->AirSensor_PersonOn=1;	
							  air->personon_by_positioncheck_flag=0;
							  air->AirSensor_log_state=5;
							  air->AirSensor_log_state=4;
					      air->Body_Moved_Times++;
//                gStorageData.Snore.HeadMovingTimes++;							 
				     }
						 else 
						 {
							 air->AirSensor_PersonOn=1;
							 air->personon_by_positioncheck_flag=0;
							 air->AirSensor_log_state=6;
							 air->Body_Moved_Times++;
//							 gStorageData.Snore.HeadMovingTimes++;
//							 Led_OnOff_Timer=0;
//					     air->Led_Person_OnOff=2;
//					     WrokingLed(BLUE_LED_HIGH,2);
							 
//							 WrokingLed(RED_LED,1);
//						   air->Led_Person_OnOff=2;
//							 Led_OnOff_Timer=0;
							 
							 
						 }
						 
						 
						 
				}
				else if(air->AirSensor_Maxvalue>(air->AirSensor_Minvalue+20)  || (air->AirSensor_Maxvalue+20)<air->AirSensor_Minvalue)
				{
					air->Body_Moved_Times++;	
//					gStorageData.Snore.HeadMovingTimes++;
				}
				
			#endif
			}				
				
			}		

//			if(air->AirSensor_PersonOn==0)
//			{
//				air->AirSensor_pressure_stable_value_noperson_last=air->AirSensor_pressure_stable_value;
//				if(air->AirSensor_pressure_stable_value_noperson_last>air->AirSensor_pressure_stable_value_noperson_MAX)
//					air->AirSensor_pressure_stable_value_noperson_MAX=air->AirSensor_pressure_stable_value_noperson_last;
//				else if(air->AirSensor_pressure_stable_value_noperson_last<air->AirSensor_pressure_stable_value_noperson_MIN)
//					air->AirSensor_pressure_stable_value_noperson_MIN=air->AirSensor_pressure_stable_value_noperson_last;
//			}
			
      if(air->AirSensor_stable_noresult_flag==0)
			{
			  air->AirSensor_Maxvalue=air->AirSensor_Avgvalue_Buffer[AirSensor_Avgvalue_Buffer_Size-1];
  		  air->AirSensor_Minvalue=air->AirSensor_Avgvalue_Buffer[AirSensor_Avgvalue_Buffer_Size-1];

			  air->Max_or_Min_detected=0;
			  air->AirSensor_stable_min_detected_flag=0;	
        air->AirSensor_stepnum_between_twostablestate=0;	
			
				if(air->It_Is_Standby_State==0)
			    Airsensor_SendlogtoPC(); //临时关闭
				
			
			  air->AirSensor_pressure_stable_value_last=air->AirSensor_pressure_stable_value;
			}
			else
			{
				if(air->It_Is_Standby_State==0)
				  Airsensor_SendlogtoPC(); //临时关闭
			}
			
			
			
			
		}
		else
		{
			air->AirSensor_step_stable_flag=0;
			if(air->It_Is_Standby_State==0)
			  Airsensor_SendlogtoPC(); //临时关闭
		}
		
		
		
		air->AirSensor_pressure_stable_value=0;
		air->pressure_stable_value_diff_between_now_before=0;
		air->down_stepnum=0;
		air->down_max=0;
		air->down_total=0;
		air->person_onoff_factor2=0;
		air->down_MAX=0;
		air->down_MIN=0;
		air->last_diff_MaxMin=0;
		air->onoff_result_reason=0;

#if 1		
		//MP1 PLUS临时检测压力值连续上升一定程度为检测到有人睡上来
		
		if(air->AirSensor_pressure_sample_avg >= air->AirSensor_pressure_sample_avg_last && air->AirSensor_pressure_sample_avg_last !=0 && air->Sleep_rapidly_check_flag==1)
		{
			air->AirSensor_pressure_sample_avg_diffsum=air->AirSensor_pressure_sample_avg_diffsum+air->AirSensor_pressure_sample_avg-air->AirSensor_pressure_sample_avg_last;
			if(air->AirSensor_pressure_sample_avg_diffsum>=person_on_threshold_sampleavg)
			{
				air->AirSensor_PersonOn=1;
				air->Tt_Is_Sleep_Time=1;
				air->Sleep_snore_study_flag=1;
				air->Sleep_rapidly_check_flag=0;
				air->Check_again_if_person_on=1;
			}
		}
		else
		{
			air->AirSensor_pressure_sample_avg_diffsum=0;
		}
		
#endif
		
}

void Switch_Valve_Do_Airbag_To_Air_Sub(Valve_struct_Def * valve_struct,unsigned char valve)
{
	//操作气阀，将气袋接通到空气泄气
	
	      valve_struct->Valve_Number_Operate = valve;
	
	      valve_struct->TIM7_operation_counter_time[0]=1;
			  valve_struct->TIM7_operation_code[0]=2;
			  valve_struct->TIM7_operation_counter_time[1]=Airbag_To_Air_PowerOff_Time;//461
			
			  valve_struct->TIM7_operation_code[1]=1;
	
			  valve_struct->TIM7_operation_counter_time[2]=Airbag_To_Air_PowerOn_Time;
			  valve_struct->TIM7_operation_code[2]=2;
			  valve_struct->TIM7_operation_counter_time[3]=4001;
			  valve_struct->TIM7_operation_code[3]=2;
			  valve_struct->TIM7_operation_code[4]=0;	
	
}

void Switch_Valve_Do_Airbag_To_Pump_Sub(Valve_struct_Def * valve_struct,unsigned char valve)
{
	//操作气阀，将气袋接通到气阀，可进行保气或者充气
	
	      valve_struct->Valve_Number_Operate = valve;
	
	      valve_struct->TIM7_operation_counter_time[0]=1;
			  valve_struct->TIM7_operation_code[0]=1;
			  valve_struct->TIM7_operation_counter_time[1]=Airbag_To_Pump_PowerOn_Time;//461
		
			
			  valve_struct->TIM7_operation_code[1]=2;		
			
			
			  valve_struct->TIM7_operation_counter_time[2]=Airbag_To_Pump_PowerOff_Time;
			  valve_struct->TIM7_operation_code[2]=1;
			  valve_struct->TIM7_operation_counter_time[3]=4001;
			  valve_struct->TIM7_operation_code[3]=1;
			  valve_struct->TIM7_operation_code[4]=0;
	
}

