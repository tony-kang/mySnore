/********************************************************************************/
/* Filename	: snore.c                                                           */
/*                                                                              */
/* Content	:                                                                   */
/* Note    	:                                                                   */
/*                                                                              */
/* Change History   :                                                           */
/*        V.0.00  2016/11/17      Linbo                                         */
/********************************************************************************/

#include "snore.h"
#include "arm_math.h"
#include "matdata.h"
#include "tuwan.h"
#include "math.h"
#include "position.h"

    static uint32_t lastfindtime=0,lastfindtime1=0,lastflag=0; //用作循环计数
    static uint32_t snoretimedelay = 0;
    
    static uint32_t Pauselastfindtime5 = 0;
    static uint32_t Pauselastfindtime6 = 0;
    static uint8_t  Pauselastfind5 = 0;
    static uint8_t  Pauselastfind6 = 0;
    static uint32_t snore1timeall = 0;
    static uint32_t snore1count = 0;
	static uint32_t FramCnt=0;
    static uint8_t  havesave = 0;
    static uint32_t DelayCountTime = 0;
		
		unsigned char study_hour_start,study_min_start,study_hour_end,study_min_end;


    int iniConfig(SnoreConfig *config);
    int Snoring(short int *WaveData, struct SnoreConfig *config, int time);
    float Autovol(struct SnoreConfig *config, float vol);
    float OTSU(float *buffer, int Num);
    void mfccFFTsub(short int *WaveData, SnoreConfig *config);
    int GetEnableStudy(void);

unsigned int ContinueErrorTime=0;

//=================================================
short int PCMVol;
uint8_t Snore_Time_Enable;
float   PVEvolBak,NoseLow,NoseHigh;
unsigned char   PVEvol8, Avol8,SnoreType,SnoreNose,SnoreNoseLow,SnoreNoseHigh,SnoreAver;
extern unsigned char SnoreDetectTime;
extern SemaphoreHandle_t xTimeMute;
extern unsigned char vectors_num,valid_vcetors_num;
extern uint16_t valid_vectors_weight[7];

uint8_t log_newfcc_flag=0,log_weight_add1_flag=0;

SnoreBuf_Def  SnoreBuf;
struct SnoreConfig Config;

SnoreTime_def SnoreTime_forAPP;

extern const arm_cfft_instance_f32 arm_cfft_sR_f32_len256;
extern uint8_t AirSensor_Position_Checking;
extern uint8_t PillowMaxOne_Willbesend,Body_Left_Right_Willbesend;
//extern uint8_t AirSensor_PersonOn;
extern uint8_t Snore_MaxDB_Detected;
extern uint8_t Snore_detected_type_flag;

extern unsigned int snore_times_for_antisnore;
extern unsigned int snore_times_for_snore;

short int   TestTempWaveData[24];
short int   TestOkWaveData[24];

//应韩国要求在V4.1.14版本中开始新增

#define StudyInSleepTimeWhenPersonOn 1 //需要有人在枕头才学习
#define DynamicValidVectorWeigthThreshold 1 //启用动逃行К特征向量权重的阀值，原来一直为100，现在要随最高权重值的提高而相应提高 0：关闭，就为原先的做法
uint16_t dynamic_weight=100,max_vectorweight_inLIB;
uint8_t min_snore_db=36;
uint8_t sleep_10minutes_mindb = 0;

//=================================================


//-------------------------------------------------------------------------
float OTSU(float *buffer, int Num)
{
    //ave为总平均值，ave0为阈值左侧平均值，ave1为阈值右侧平均值， variance为类间方差 
    //max为数据最大值，min为数据最小值，T为遍历阈值 ,maxT为方差最大阈值，maxV为最大方差 
    float ave, ave0, ave1, variance, max, min, T, maxT, maxV;
    int i, p0, p1;
    maxT = 0;
    maxV = 0;
    max = buffer[0];
    min = buffer[0];
    ave = buffer[0];

    //---------------------------------
    //求所有数据的最大值最小值及平均值 
    for (i = 1; i < Num; i++)
    {
        if (buffer[i] < min)
        {
            min = buffer[i];
        }
        if (buffer[i] > max)
        {
            max = buffer[i];
        }
        ave = ave + buffer[i];
    }
    
    if (ave == 0)
    {
        return 0;
    }

    //---------------------------------
    ave = ave / Num;
    T = min + (float)0.05;
    
    while (T < (max - (float)0.05))
    {
        ave0 = 0;
        ave1 = 0;
        p0 = 0;
        p1 = 0;
        for (i = 0; i < Num; i++)
        {
            if (buffer[i] <= T)
            {
                p0++;
                ave0 = ave0 + buffer[i];
            }
            else
            {
                p1++;
                ave1 = ave1 + buffer[i];
            }
        }
        ave0 = ave0 / p0;
        ave1 = ave1 / p1;
        
        variance = (float)p0 * pow((ave0 - ave), 2) / Num + (float)p1 *pow((ave1 - ave), 2) / Num;
        if (variance > maxV)
        {
            maxV = variance;
            maxT = T;
            NoseLow = ave0;
            NoseHigh = ave1;
        }
        T = T + (float)0.2;
    }
    //---------------------------------
    return maxT;
}

//-------------------------------------------------------------------------
float Autovol(struct SnoreConfig *config, float vol)
{
    static int enautovol = 0;

    config->VolIndex = config->VolIndex % VOLNUM;
    config->VolBuffer[config->VolIndex] = vol;
    config->VolIndex++;
    
    if(config->VolIndex == VOLNUM)
    {
        enautovol = 1;
    }

    if(enautovol == 0)
    {
        return 0;
    }
    else
    {
        return OTSU((float*)config->VolBuffer, VOLNUM);
    }    
}

//-------------------------------------------------------------------------

//初始化运算用各类缓存参数及运算用矩阵 
int iniConfig(SnoreConfig *config)
{
    int i;
    config->vol = 20; //初始化音量值
    config->prevol = 0;
    config->BufferNUM = 0;
    config->VolIndex = 0;
    for (i = 0; i < VOLNUM; i++)
    {
        config->VolBuffer[i] = 0;
    }
    config->autostudy = 1;
    config->TimeCount = 0;
    config->lastfindtime = 0;

    config->lastfindtimeEnd = 0;
    config->lastfindtimeStart = 0;
    
    return 1;
}


//初始化运算用各类缓存参数及运算用矩阵  added by zjp
int iniConfig1(SnoreConfig *config)
{
    int i;
//    config->vol = 20; //初始化音量值
//    config->prevol = 0;
    config->BufferNUM = 0;
//    config->VolIndex = 0;
//    for (i = 0; i < VOLNUM; i++)
//    {
//        config->VolBuffer[i] = 0;
//    }
    config->autostudy = 1;
    config->TimeCount = 0;
    config->lastfindtime = 0;

    config->lastfindtimeEnd = 0;
    config->lastfindtimeStart = 0;
    
    return 1;
}



//存储的mfcc矩阵减权并清理 
void mfcc_sub(float *data)
{
    int i, j, R_i, col;
    float temp = 0;
    float sum;
    float *AddressTemp;
    
    col = MELFILTER + 1;
    //-------------------------------------------
    for (i = 0; i < MELBuffer; i++)
    {
        sum = 0;
        AddressTemp = &data[i * col + 0];
        arm_power_f32(AddressTemp,MELFILTER,&temp);
        arm_sqrt_f32(temp,&sum);
        
        if (sum < (float)0.3)
        {
            data[i *col + MELWeight] = 0;
        }
    }
    //-------------------------------------------
    for (i = 0; i < MELBuffer; i++)
    {
        data[i *col + MELWeight] = data[i *col + MELWeight] - SUBWeight;
        //-------------------------------------------
        if (data[i *col + MELWeight] > MAXWeight)
        {
            data[i *col + MELWeight] = MAXWeight;
        }
        //-------------------------------------------
        if (data[i *col + MELWeight] < (float)0.5)
        {
            //----------------删除---------------------------
            for (j = 0; j < MELFILTER + 1; j++)
            {
                data[i *col + j] = 0;
            }
            //----------------前移---------------------------
            for (R_i = i + 1; R_i < MELBuffer; R_i++)
            {
                if (data[R_i *col + MELWeight] > 1)
                {
                    for (j = 0; j < MELFILTER + 1; j++)
                    {
                        data[i *col + j] = data[R_i *col + j];
                        data[R_i *col + j] = 0;
                    }
                    break;
                }
            }
            //-------------------------------------------
        }
        //-------------------------------------------
    }
}

//计算向量距离
float VectorDistance(float *V1, float *V2, int N)
{
    float mfcc[MELFILTER];
    float temp = 0;
    float distance = 0;
    arm_sub_f32(V1,V2,mfcc,N);
    arm_power_f32(mfcc,N,&temp);
    arm_sqrt_f32(temp,&distance);
    return distance;
}

//返回mfcc对比指标值 
//mfcc为输入向量表，config为存储运算配置指针，Buffer记录缓存帧数，Mel记录倒谱阶数 
//MELBuffer, MELFILTER
//int MBuffer,int Mel
int mfccCount(SnoreConfig *config)
{
    int i, j, k, l, count, wcount, weight[MELBuffer];
    float distance,  *mfccBuffer;
    int r_index, lastnolike[CnWaveDataMfccNewMax];
    float *m;

    //------太多新增的不要添加直接认为不像-----------
    if(config->WaveDataMfccNewMax >= CnWaveDataMfccNewMax)
    {
        return 0;
    }
    
    m = config->WaveDataMfcc;
    
    count = 0;
    wcount = 0;
    mfccBuffer = (float*)config->MfccMat;
    //初始化加权记录数组 
    for (i = 0; i < MELBuffer; i++)
    {
        weight[i] = 0;
    }

    //-------初始化新增位置---------
    for(l=0;l<CnWaveDataMfccNewMax;l++)
    {
        lastnolike[l] = MELBuffer;
    }
    
    //-------------------------------
    for (i = 0; i < (config->BufferNUM -4); i++)
    {

        r_index = i + 2;


        j = m[r_index*(MELFILTER+1) + MELFILTER];

        //--------------检查新增块是否有一样的------------------
        if(j == MELBuffer && config->Enstudy == 1)
        {
            //----------检查当前块新增位置-----------
            for(l=0;l<CnWaveDataMfccNewMax;l++)
            {
                if(lastnolike[l] != MELBuffer)
                {
                    distance = VectorDistance(&m[r_index*(MELFILTER+1)], mfccBuffer + lastnolike[l] *(MELFILTER + 1), MELFILTER);
                    if (distance < Threshold)
                    {
                        j = lastnolike[l];
                        break;
                    }
                }
            }

        }

        //--------------------------------
        if(j < MELBuffer)
        {

            if(config->level == HIGH_CHECK)
            {
                if (mfccBuffer[j *(MELFILTER + 1) + MELWeight] >= 15)
                {
                    count++;
                }
            }
            else
            {
//                if (mfccBuffer[j *(MELFILTER + 1) + MELWeight] >= CONWeight)
								if (mfccBuffer[j *(MELFILTER + 1) + MELWeight] >= dynamic_weight)	//新调整
                {
                    count++;
                }
            }    
            
            if (mfccBuffer[j *(MELFILTER + 1) + MELWeight] >(float) 0.9)
            {
                weight[j] = 1;
            }
        }



        
        //如果没有在存储的mfcc向量表中查找到，则向向量表中添加当前mfcc向量	
        if (j == MELBuffer && config->Enstudy == 1)
        {
            for (k = 0; k < MELBuffer; k++)
            {
                if (mfccBuffer[k *(MELFILTER + 1) + MELWeight] < (float)0.5)
                {
                    mfccBuffer[k *(MELFILTER + 1) + MELWeight] = 1;
                    for (l = 0; l < MELFILTER; l++)
                    {
                        mfccBuffer[k *(MELFILTER + 1) + l] = m[r_index*(MELFILTER+1) + l];
											  log_newfcc_flag=1;
                    }

                    //-------------当前块新增位置----------------
                    for(l=0;l<CnWaveDataMfccNewMax;l++)
                    {
                        if(lastnolike[l] == MELBuffer)
                        {
                            lastnolike[l] = k;  //有空存
                            break;
                        }
                    }
                    //-----------------------------
                    
                    break;

                    
                }
            }
        }
    }
    
    //-------------------------------
    if (config->Enstudy == 1)
    {
        for (i = 0; i < MELBuffer; i++)
        {
            if (mfccBuffer[i *(MELFILTER + 1) + MELWeight] <= MAXWeight && weight[i] > 0.9)
            {
                mfccBuffer[i *(MELFILTER + 1) + MELWeight] = mfccBuffer[i *(MELFILTER + 1) + MELWeight] + ADDWeight;
                //printf("MEL[%d]=%.0f\n",i,mfccBuffer[i *(MELFILTER + 1) + MELWeight]);
							  log_weight_add1_flag=1;
            }
        }
    }
    //-------------------------------
    for (i = 0; i < MELBuffer; i++)
    {
        if (weight[i] > 0.9)
        {
            wcount++;
        }
    }
    config->MfccRatio = ((float)wcount / (float)(config->BufferNUM-4));
    return count;
}




//现直接计算db值，注释部分为能熵值 
float PVE(short int *WaveData)
{
    int i;
    float min, max, vol;
    min = WaveData[0];
    max = WaveData[0];
    for (i = 0; i < FRAME; i++)
    {
        min = (min < WaveData[i]) ? min : WaveData[i];
        max = (max > WaveData[i]) ? max : WaveData[i];
    }
    vol = (max - min) / 2;
    PCMVol = vol; 

    if (vol == 0)
    {
        return vol;
    }
    vol = log10(vol);
    vol = 10 * vol;
    return vol;
}

//判断是否为 鼾声 ,不是返回0，是返回1，灵敏度太高返回-1. 
int Snoring(short int *WaveData, struct SnoreConfig *config, int time)
{
    int i,j, flag, mCount;
    int mCountTemp;
    float PVEvol,Avol;
    float addvol;
    int addVolcount;
    
    config->TimeCount++;
    flag = 0;
    PVEvol = PVE(WaveData);
	
	  //2023年7月14日新增，为app提供10分钟睡眠中的声音最小值
	  if(gStorageData.Snore.HeadPosion > PVEvol &&  Airsensor.Tt_Is_Sleep_Time==1)
			gStorageData.Snore.HeadPosion = PVEvol*2;
	
		//2023年7月14日新增结束
	
	  PVEvol8=PVEvol;
    //测试自动音量
    Avol = Autovol(config, PVEvol);
	  PVEvolBak=PVEvol;
	  //临时加入晚上12：00以后检测到鼾声也检测位置
	
	  if (Avol > 10 && Airsensor.Tt_Is_Sleep_Time==1)
	
//    if (Avol > 10 && Airsensor.AirSensor_PersonOn==1)
//		if(Avol > 10 && Airsensor.AirSensor_PersonOn==1)
    {
			  
        config->vol = Avol + (float)0.5;
    } 
    else
    {
        return 0;
    }
		
		
		if(((PVEvolBak*2)>75)||((PVEvolBak*2)<10))
		{	ContinueErrorTime++;
			if(ContinueErrorTime>=3200)
				ErrorCode|=(1<<ERROR_CODE_SNORE_ERROR);//鼾声异常
		}
		else ContinueErrorTime=0;
    //结束测试自动音量				
    if (PVEvol > config->vol)
    {
        if (config->BufferNUM < BUFFER)
        //正在缓冲 
        {
            if (config->BufferNUM == 12)
            {
                for(j=0;j<24;j++)
                {
                    TestTempWaveData[j] = WaveData[128+j];
                }
            }
            mfccFFTsub(WaveData, config);

            //----------------------------------
            if(config->BufferNUM == 0)
            {
                config->maxvol = 0;
            }

            if (PVEvol > config->maxvol)
            {
                config->maxvol = PVEvol;
            }
            //----------------------------------

            config->FRAMEVol[config->BufferNUM] = PVEvol;
            config->BufferNUM = config->BufferNUM + 1;

            if (config->BufferNUM == 1)
            {
                config->lastfindtimeStart = time;
            }
           
            return 0;
        }
        else
        //超出缓冲区，缓冲语音帧超出鼾声应有长度 。缓冲计数继续增加，但是不缓冲语音帧 
        {
            config->BufferNUM = config->BufferNUM + 1;
            return  - 1;
        }
    }
    else
    {
        if (config->BufferNUM < 10)
        // 结束缓冲， 缓冲语音帧小于鼾声应有长度
        {
            flag = 4;
            if (config->BufferNUM >= 2)
            {
                if(config->lastfindtimeStart-config->lastfindtimeEnd < (1000 / 32))
                {
                    flag = 7;
                }
                config->lastfindtimeEnd = time;
            }
            config->BufferNUM = 0;
            return flag;
        }
        else if (config->BufferNUM > BUFFER)
        //	结束缓冲， 缓冲语音帧大于鼾声应有长度	
        {
            config->BufferNUM = 0;
            config->lastfindtimeEnd = time;
            return  7;
           
        }
        else
        //结束缓冲，正常长度。开始判断是否为鼾声 
        {
            //-------------
            addvol = 0;
            addVolcount = 0;
            for (i = 0; i < config->BufferNUM; i++)
            {
                if ((i > 3) && (i < (config->BufferNUM - 3)))
                {
                    addvol = addvol + config->FRAMEVol[i];
                    addVolcount++;
                }
            }
            config->prevol = addvol / addVolcount;
            //-------------

            config->Enstudy = 0;


            //===================================================
/*            
            if(config->level == HIGH_CHECK)
            {
                config->SetMax  = 3 * 0.5; // 6db
                config->SetAver = config->SetMax * 0.5; // 4db
                config->SetStudyMax  = 4 * 0.5; // 8db
                config->SetStudyAver = config->SetStudyMax * 0.5; // 6db 
            }
            else
            {
                if((config->vol * 2) >= MAX_NOSEDB)
                {
                    config->SetMax  = 3 * 0.5; // 6db
                    config->SetAver = config->SetMax * 0.5; // 4db
                    config->SetStudyMax  = 4 * 0.5; // 8db
                    config->SetStudyAver = config->SetStudyMax * 0.5; // 6db 
                }
                else
                {
                    config->SetMax  = 6 * 0.5; // 6db
                    config->SetAver = config->SetMax * 0.5; // 4db
                    config->SetStudyMax  = 8 * 0.5; // 8db
                    config->SetStudyAver = config->SetStudyMax * 0.5; // 6db 
                }
            }   
*/            

            //--------------------------------
            if((NoseHigh - config->vol)*2 < 1)
            {
                flag = 4;
            }
            
//            if((config->prevol * 2) < LOW_SNOREDB)
						if((config->prevol * 2) < min_snore_db)
						
            {
                flag = 4;                
            }

//            if((config->maxvol * 2) < LOW_SNOREDB)
            if((config->maxvol * 2) < min_snore_db)
							
            {
                flag = 4;                
            }


            if(config->lastfindtimeStart-config->lastfindtimeEnd < (200 / 32))
            {
                flag = 4;                
            }
            
            //===================================================
            if (((config->prevol - config->vol) > config->SetAver) && ((config->maxvol - config->vol) > config->SetMax) && (flag == 0) )
            {
                if (((config->prevol - config->vol) > config->SetStudyAver) && ((config->maxvol - config->vol) > config->SetStudyMax))
                {
                    //------判断连续-------
                    if (config->lastfindtime == 0)
                    {
                        config->lastfindtime = config->TimeCount;
                    }
                    else if ((config->TimeCount - config->lastfindtime) > (2200 / 32))
                    {
                        if ((config->TimeCount - config->lastfindtime) > (6000 / 32)){}
                        else
                        {
                            if (config->autostudy == 1)
                            {
                                if((ActionSteps == 1) || (ActionSteps == 5) || (ActionSteps == 8) || (ActionSteps == 12))
                                {
                                    config->Enstudy = 1;
                                }
                                else    
                                {
                                    config->Enstudy = 1;
                                }   
                            }
                        }
                        config->lastfindtime = config->TimeCount;
                    }
                    else
                    {
                    }
                    //---------------------------------------------
                }
                
                mCount = mfccCount(config);
                //mCount = 0;
                mCountTemp = (int)((float)(config->BufferNUM - 4) * config->SetLike);
                if (mCount >= mCountTemp)
                {
                    flag = 1;
                    config->TestState = 2;
                }
                else
                {
                    flag = 3;
                    config->TestState = 1;
                }
                
                config->Testvol = config->maxvol;
                for(j=0;j<24;j++)
                {
                   TestOkWaveData[j]  = TestTempWaveData[j];
                }
               
            }
            else
            {
                flag = 4;
            }
            //--------------------------------
            if (config->Enstudy == 1)
            {
                mfcc_sub((float*)config->MfccMat);
            }

            //--------------------------------
            
            if((config->lastfindtimeStart-config->lastfindtimeEnd < (500 / 32)) && (config->level != HIGH_CHECK))
            {
                if(flag == 1) 
                {
                    flag = 8;
                }
                else if(flag == 3)    
                {
                    flag = 9;
                }
                else
                {
                    flag = 7;
                }
                
            }
            
            config->lastfindtimeEnd = time;
            //--------------------------------
        }
    }

    config->BufferNUMCurr = config->BufferNUM;
    config->BufferNUM = 0;

    if(config->BufferNUMCurr >= (4000 / 32))
    {
        flag = 7;
    }
    
    return flag;
}


void SetValidVectorWeightThrehold(void)
{
	 int i,j;
	 float max_temp_weight=0;
	 if(DynamicValidVectorWeigthThreshold==1)
	 {
		 for(i=0;i<MELBuffer+1;i++)
     {
       if(Config.MfccMat[i][MELFILTER]>max_temp_weight)
			 {
				 max_temp_weight=Config.MfccMat[i][MELFILTER];      			
		   }			 
     }
		 max_vectorweight_inLIB=max_temp_weight;
		 if(max_temp_weight > 650000)
		 {
			 dynamic_weight=650;
		 }
		 else
		 {
       dynamic_weight= (int)(max_temp_weight/1000.0);
		 }
     if(dynamic_weight==0)
        dynamic_weight=CONWeight;
		 else
			  dynamic_weight=dynamic_weight*100; 
     if(((int)max_vectorweight_inLIB)>1000 && ((int)max_vectorweight_inLIB)<3000)
		 {
			 dynamic_weight=dynamic_weight+(int)(((int)max_temp_weight-dynamic_weight*10)/100)*10;
		 }			 
		 
	 }
	 else
		 dynamic_weight=CONWeight;
	
}

void snore_mindb_Init(void)
{
	   min_snore_db = gStorageData.MinSnoredB;	
}

/**************************************************************************/
/*                                                                        */
/**************************************************************************/
void SnoreInit(void)
{
    int i,j;
    iniConfig(&Config);
		Config.level = gStorageData.SensorLevel; //临时添加测试 20220313
    if(gStorageData.StudyMode)
    {
//        Config.level =  HIGH_CHECK; //临时添加测试 20220313
    }
    else
    {   
			  if(SENSORLEVEL_HIGH_FOR_24HOURS_ENABLED==0)
				{        
			    Config.level = gStorageData.SensorLevel;   //级别 1:高  2:中  3:低
				}
			  else
				{
			    //modified byz zjp for korea 3.1.29 在一定天数内强制sensor level为高
			    //start
			    if(SensorLevelAutoSetData.times==0)
            Config.level = gStorageData.SensorLevel;   //级别 1:高  2:中  3:低
				  else
				  {
					  Config.level =  HIGH_CHECK;
				  }
				  //end
				}
				//******************************************************
    }


    Config.SetContinuous = 3; //连续第三个
    
    Config.SetLike = 8 * 0.1; //80%相似度


    Config.SetMax  = 3 * 0.5; // 6db
    Config.SetAver = 2 * 0.5; // 4db
    
    Config.SetStudyMax  = 5 * 0.5; // 8db
    Config.SetStudyAver = 3 * 0.5; // 6db 


/*
    if(Config.level == HIGH_CHECK)
    {
        Config.SetMax  = 3 * 0.5; // 6db
        Config.SetAver = Config.SetMax * 0.5; // 4db
        Config.SetStudyMax  = 4 * 0.5; // 8db
        Config.SetStudyAver = Config.SetStudyMax * 0.5; // 6db 
    }
    else
    {
        Config.SetMax  = 6 * 0.5; // 6db
        Config.SetAver = Config.SetMax * 0.5; // 4db
        Config.SetStudyMax  = 8 * 0.5; // 8db
        Config.SetStudyAver = Config.SetStudyMax * 0.5; // 6db 
    }        
*/

  SnoreLibWR(&Config.MfccMat[0][0],25088/4,0);//读取鼾声特征值库到内存
	
	//统计特征值数量及有效特征值数量供app读取
	
	SetValidVectorWeightThrehold();
	
	vectors_num=0;
	valid_vcetors_num=0;
	for(i=0;i<7;i++)
	  valid_vectors_weight[i]=0;
	
	for(i=0;i<MELBuffer+1;i++)
  {
    if(Config.MfccMat[i][MELFILTER]!=0)
      vectors_num++;
     		
    if(Config.MfccMat[i][MELFILTER]>=dynamic_weight)
		{
			if(valid_vcetors_num<=6)
			{
			  valid_vectors_weight[valid_vcetors_num]=Config.MfccMat[i][MELFILTER];
			}
      valid_vcetors_num++;
      			
		}			
  }
	
	
	//鼾声灵敏度为高将内存中的特征值清0
	if(Config.level == HIGH_CHECK)
	{
    for(i=0;i<MELBuffer+1;i++)
    {
        for(j=0;j<(MELFILTER+1);j++)
        {
            Config.MfccMat[i][j]=0;
        }
    }
  }  
	SnoreBuf.num=0;
	PillowControl.SnoreOn=false;
	SnoreDetectTime=0;

    //Config.level = MID_CHECK;   //级别 1:高  2:中  3:低
    //HIGH_CHECK   //MID_CHECK  //LOW_CHECK

    if(Config.level != HIGH_CHECK)
    {
//        SnoreLibWR(&Config.MfccMat[0][0],25088/4,0);
        Config.CheckLeav = 0;       //严检
    }
    else
    {
        Config.CheckLeav = 1;   
    }
    
    Config.SnoreAverTime = 4000; 
    FramCnt=0;
    havesave = 0;
    
    Config.HaveCheckSnore = 0; 




    lastfindtime=0;
    lastfindtime1=0;
    lastflag=0;
    snoretimedelay = 0;
    Pauselastfindtime5 = 0;
    Pauselastfindtime6 = 0;
    Pauselastfind5 = 0;
    Pauselastfind6 = 0;
    snore1timeall = 0;
    snore1count = 0;
    DelayCountTime = 0;

    //-----------------------
    for(j=0;j<24;j++)
    {
       TestOkWaveData[j]  = 0;
    }
    Config.TestState = 0;
    //-----------------------
}


void SnoreInit_personon(void)
{
    int i,j;
    iniConfig1(&Config);

		Config.level = gStorageData.SensorLevel; //临时添加测试 20220313
    if(gStorageData.StudyMode)
    {
//        Config.level =  HIGH_CHECK; //临时关闭 20220313
    }
    else
    {
			  if(SENSORLEVEL_HIGH_FOR_24HOURS_ENABLED==0)
				{
          Config.level = gStorageData.SensorLevel;   //级别 1:高  2:中  3:低
				}
				else
				{
			
			    //modified byz zjp for korea 3.1.29 在一定天数内强制sensor level为高
			    //start
			    if(SensorLevelAutoSetData.times==0)
            Config.level = gStorageData.SensorLevel;   //级别 1:高  2:中  3:低
				  else
				  {
					  Config.level =  HIGH_CHECK;
				  }
				  //end
				}
				//******************************************************
    }


    Config.SetContinuous = 3; //连续第三个
    
    Config.SetLike = 8 * 0.1; //80%相似度


    Config.SetMax  = 3 * 0.5; // 6db
    Config.SetAver = 2 * 0.5; // 4db
    
    Config.SetStudyMax  = 5 * 0.5; // 8db
    Config.SetStudyAver = 3 * 0.5; // 6db 


/*
    if(Config.level == HIGH_CHECK)
    {
        Config.SetMax  = 3 * 0.5; // 6db
        Config.SetAver = Config.SetMax * 0.5; // 4db
        Config.SetStudyMax  = 4 * 0.5; // 8db
        Config.SetStudyAver = Config.SetStudyMax * 0.5; // 6db 
    }
    else
    {
        Config.SetMax  = 6 * 0.5; // 6db
        Config.SetAver = Config.SetMax * 0.5; // 4db
        Config.SetStudyMax  = 8 * 0.5; // 8db
        Config.SetStudyAver = Config.SetStudyMax * 0.5; // 6db 
    }        
*/

  SnoreLibWR(&Config.MfccMat[0][0],25088/4,0); //读取鼾声特征库到内存 

  //统计特征值数量及有效特征值数量供app读取
	SetValidVectorWeightThrehold();
	vectors_num=0;
	valid_vcetors_num=0;
	for(i=0;i<7;i++)
	  valid_vectors_weight[i]=0;
		
	for(i=0;i<MELBuffer+1;i++)
  {
    if(Config.MfccMat[i][MELFILTER]!=0)
      vectors_num++;
     		
    if(Config.MfccMat[i][MELFILTER]>=dynamic_weight)
		{
			if(valid_vcetors_num<=6)
			{
			  valid_vectors_weight[valid_vcetors_num]=Config.MfccMat[i][MELFILTER];
			}
      valid_vcetors_num++;
		}			
  }


	if(Config.level == HIGH_CHECK)
  {
    for(i=0;i<MELBuffer+1;i++)
    {
        for(j=0;j<(MELFILTER+1);j++)
        {
            Config.MfccMat[i][j]=0;
        }
    }
  }  
	SnoreBuf.num=0;
	PillowControl.SnoreOn=false;
	SnoreDetectTime=0;

    //Config.level = MID_CHECK;   //级别 1:高  2:中  3:低
    //HIGH_CHECK   //MID_CHECK  //LOW_CHECK

    if(Config.level != HIGH_CHECK)
    {
//       SnoreLibWR(&Config.MfccMat[0][0],25088/4,0);
        Config.CheckLeav = 0;       //严检
    }
    else
    {
        Config.CheckLeav = 1;   
    }
    
    Config.SnoreAverTime = 4000; 
    FramCnt=0;
    havesave = 0;
    
    Config.HaveCheckSnore = 0; 




    lastfindtime=0;
    lastfindtime1=0;
    lastflag=0;
    snoretimedelay = 0;
    Pauselastfindtime5 = 0;
    Pauselastfindtime6 = 0;
    Pauselastfind5 = 0;
    Pauselastfind6 = 0;
    snore1timeall = 0;
    snore1count = 0;
    DelayCountTime = 0;

    //-----------------------
    for(j=0;j<24;j++)
    {
       TestOkWaveData[j]  = 0;
    }
    Config.TestState = 0;
    //-----------------------
}


/**************************************************************************/
/*                                                                        */
/**************************************************************************/
void firstSnoreInit(void)
{
    int i,j;
    for(i=0;i<MELBuffer+1;i++)
    {
        for(j=0;j<(MELFILTER+1);j++)
        {
            Config.MfccMat[i][j]=0;
        }
    }

    //MELBuffer * (MELFILTER + 1)   25000=>25088 /256  
    SnoreLibWR(&Config.MfccMat[0][0],25088/4,1);
		//统计特征值数量及有效特征值数量供app读取
	  SetValidVectorWeightThrehold();
	  vectors_num=0;
	  valid_vcetors_num=0;
	  for(i=0;i<7;i++)
	  valid_vectors_weight[i]=0;
		
	  for(i=0;i<MELBuffer+1;i++)
    {
      if(Config.MfccMat[i][MELFILTER]!=0)
        vectors_num++;
     		
      if(Config.MfccMat[i][MELFILTER]>=dynamic_weight)
			{
				if(valid_vcetors_num<=6)
			  {
			    valid_vectors_weight[valid_vcetors_num]=Config.MfccMat[i][MELFILTER];
			  }
        valid_vcetors_num++;	
			}				
    }
	
}
uint8_t test_flag3 = 0;

/**************************************************************************/
/*                                                                        */
/**************************************************************************/
uint8_t SnoreTime(uint32_t FramCnt,uint8_t flag,struct SnoreConfig *config)
{
        uint32_t snore1time=0;
        uint8_t  highlowEn=0;
   
        #if 0
            if ((flag > 0) && (flag < 4))
            {
                //printf("A");
                printf("%d T=%d Max:%.0f Nose:%.0f\r\n",flag,endtime-starttime,Config.maxvol*2, Config.vol*2);
                //sprintf((char *)pHeaderBuff,"%d T=%d Max:%.0f Nose:%.0f\r\n",flag,endtime-starttime,Config.maxvol*2, Config.vol*2);
                //printf("t=%f  Max:%.2f Nose:%.2f\n\r", time*0.032, Config.maxvol*2, Config.vol*2);
                //return 1;
            }
            else if(endtime-starttime>10)
            printf("%d\r\n",endtime-starttime);
          
        #endif

        if(FramCnt == 1)
        {
            lastfindtime=0;
            lastfindtime1=0;
            lastflag=0;
            snoretimedelay = 0;
            Pauselastfindtime5 = 0;
            Pauselastfindtime6 = 0;
            Pauselastfind5 = 0;
            Pauselastfind6 = 0;
            snore1timeall = 0;
            snore1count = 0;
            DelayCountTime = 0;
        }


        //----------------------------------------------
        if(Config.level != HIGH_CHECK)
        {
            if(Config.CheckLeav == 0)     //之前type=1启动一次后1分钟内不抗干扰
            {
                if((flag == 7)||(flag == 8)||(flag == 9))
                {
                    DelayCountTime = (5000 / 32);
                }
            
                if(DelayCountTime > (5000 / 32))
                {
                    DelayCountTime = 0;
                }

                if(DelayCountTime > 0)
                {
                    DelayCountTime--;

                    if((flag == 1)||(flag == 8))
                    {
                        flag = 2;
											  test_flag3 = 6;
                    }
                }
            }
            else
            {
                if(flag == 8)
                {
                    flag = 1;
                }
                //----------------------
                if(flag == 9)
                {
                    flag = 3;
                }
            }
        }    
        //----------------------------------------------

        if(flag > 0)
        {
            if(Config.level == HIGH_CHECK)
            {
//                if(Config.maxvol < (LOW_SNOREDB/2))
								if(Config.maxvol < (min_snore_db/2))	
                {
                    flag = 0;
                }
            }
            else if(Config.level == MID_CHECK)
            {
 //               if(Config.maxvol < ((LOW_SNOREDB+0)/2)) 
								if(Config.maxvol < ((min_snore_db+0)/2))	
                {
                    flag = 0;
                }
            }           
            else if(Config.level == LOW_CHECK)
            {
//                if(Config.maxvol < ((LOW_SNOREDB+4)/2))
								if(Config.maxvol < ((min_snore_db+4)/2))	
                {
                    flag = 0;
                }
            }  
        }    
        //--------------------------------
        highlowEn = 0;
        //以下代码发行前取消,白天没有判断频域
#ifdef SNORE_TESTMODE
        if(Config.level == HIGH_CHECK)
        {
            if((stimestructure.Hours >= 9) && (stimestructure.Hours < 21))
            {
                highlowEn = 1; 
            }   
        }
#endif
        //========================测试模式==========================
        //if((Config.level == TEST_CHECK)||(highlowEn == 1))
        if(Config.level == HIGH_CHECK)    
        {
            if(flag == 9)
            {
                flag = 3;
            }
            
            //========================================
            if ((flag > 0) && (flag < 4))   //  1像和3不像
            {
                if(lastfindtime == 0)
                {
                    lastfindtime = FramCnt;
                    flag = 2;
                    lastflag = 1;
                }
                else if((FramCnt - lastfindtime) > (2200/32))     //2// 2.2s  24次
                {
    				if((FramCnt - lastfindtime) > (6000/32))
    				{
    					lastflag = 1;
                        if(Config.SetContinuous == 1)
                        {
                            snore1time = FramCnt - lastfindtime;
                            flag = 1;
                        }
                        else
                        {
                            flag = 2;
                        }
    				}
                    else
                    {   
                        lastflag++;
                        if(lastflag>=Config.SetContinuous)
                        {
                            snore1time = FramCnt - lastfindtime;
                            flag = 1;
                        }
                        else
                        {
                            flag = 2;
                        }
                    }
                    lastfindtime = FramCnt;
                }
                else
                {
                    flag = 0;
                }

            }
            
            //===============两分钟内采用时域===================
/*
            if(flag == 1)
            {
                snoretimedelay = 4000;      // 128s
            }

            if(snoretimedelay > 4000)
            {
                snoretimedelay = 4000;      // 128s
            }

            if(snoretimedelay > 0)
            {
                snoretimedelay--;
            }
            
            if((snoretimedelay > 0) && (flag == 2))
            {
                flag = 1;
            }
*/
            //========================================
        }
        else
        {
            if(flag == 1)
            {
							  test_flag3 = 0;
                if(lastfindtime == 0)
                {
                    lastfindtime = FramCnt;
                    flag = 2;
									  test_flag3 = 1;
                }
                else if((FramCnt - lastfindtime) > (2200/32))     //2// 2.2s  24次
                {
    				       if((FramCnt - lastfindtime) > (6000/32))
    				       {
    					          lastflag = 0;
                        if(Config.SetContinuous == 1)
                        {
                            snore1time = FramCnt - lastfindtime;
                            //flag = 1;
                        }
                        else
                        {
                            flag = 2;
													  test_flag3 = 2;
                        }
    				       }
                   else
                   {   
                        lastflag++;
                        if(lastflag>=(Config.SetContinuous-1))
                        {
                            snore1time = FramCnt - lastfindtime;
                            //flag = 1;
                        }
                        else
                        {
                            flag = 2;
													  test_flag3 = 3;
                        }
                    }
                    lastfindtime = FramCnt;
                }
                else
                {
                    flag = 2;
									  test_flag3 = 4;
                    if(Config.level != HIGH_CHECK)
                    {
                        lastfindtime = FramCnt - (2200/32);
                        lastflag = 0;
                    }   
                    
                }

            }
    		//end-----呼和吸计数为一个-----
/*
            if(flag == 3)
            {
                if(Config.level == LOW_CHECK)
                {
                    if(Config.maxvol < (60/2))
                    {
                        flag = 0;
                    }
                }
                else if(Config.level == MID_CHECK)
                {
                    if(Config.maxvol < (40/2))
                    {
                        flag = 0;
                    }
                }
            }
*/
            //===============两分钟内采用时域===================
            if(flag == 1)
            {
                if(Config.level == MID_CHECK)
                {
                    snoretimedelay = 200;      // 128s 4000
                }
                else
                {
                    snoretimedelay = 200;      // 6s
                }
            }

            if(snoretimedelay > 200)
            {
                snoretimedelay = 200;      // 128s
            }

            if(snoretimedelay > 0)
            {
                snoretimedelay--;
                if(flag == 3)
                {
                    flag = 2;
									  test_flag3 = 5;
                }
            }
            
            //---------------------------------------------------
            if(flag == 1)
            {
                if(AirSensor_Position_Checking == 1)
                {
                    flag = 2;
                }
                //----------------------------------------
                if(Config.level == MID_CHECK)
                {
                    if(Config.CheckLeav == 0)
                    {
                        Config.CheckLeav = 1;
                        Config.SetContinuous = 2;
                    }
                   
								  #if 1	//diabled	by zjp at 20220811 无效这段代码在为中时，在非晚上时段及检测无人的情况下止鼾鼾声检测的次数会更多，更为灵敏
                    //---------------------------
                    if((stimestructure.Hours >= 6) && (stimestructure.Hours < 21))
                    {
                        if(Config.CheckLeav == 1)
                        {
                            Config.CheckLeav = 0;
                            Config.SetContinuous = 3;
                        }
                    }
                    else
                    {
#ifdef TUWAN_BLANKET
                        if(Body_Left_Right_Willbesend == 0x00)
#else
                        if(PillowMaxOne_Willbesend    == 0x10)
#endif
                        {
                            if(Config.CheckLeav == 1)
                            {
                                Config.CheckLeav = 0;
                                Config.SetContinuous = 3;
                            }
                        }
                    }
									#endif

                }
               
                //----------------------------------------
            }
            //=================确保不会太密====================
            if((flag > 0) && (flag < 4))
            {
                if(lastfindtime1 == 0)
                {
                    lastfindtime1 = FramCnt;
                    flag = 0;
                }
                else if((FramCnt - lastfindtime1) > (2200/32))     //2// 2.2s  24次
                {

                    lastfindtime1 = FramCnt;
                }
                else
                {
                    flag = 0;
                    if(Config.level != HIGH_CHECK)
                    {
                        lastfindtime1 = FramCnt - (2200/32);
                    }    
                }
            }
            //=================================================
           
        }


        if(flag == 1)
        {
            snore1timeall = snore1timeall + snore1time;
            snore1count++;
            if(snore1count >=3)
            {
                Config.SnoreAverTime = snore1timeall/snore1count*32; 
                if(Config.SnoreAverTime > 5000)
                {
                    Config.SnoreAverTime = 5000;
                }
            }
            else    
            {
                Config.SnoreAverTime = 4000; 
            }
        }


        //-------------------pause low----------
        if((FramCnt >= (1000/32*3600*0.5)) || (Config.level == HIGH_CHECK))
        {
            if((flag > 0) && (flag < 5))
            {
                if((flag == 1) || (flag == 2))
                {
                    if(Pauselastfind6 == 1)
                    {
                        if( ((FramCnt - Pauselastfindtime6 - Config.BufferNUMCurr) > (10 * 1000 /32))
                            && ((FramCnt - Pauselastfindtime6 - Config.BufferNUMCurr) < (20 * 1000 /32)) )
                        {
                            Config.OSASTime = (FramCnt - Pauselastfindtime6 - Config.BufferNUMCurr) * 0.032;
                            flag = 6;
                        }
                    }
                    
                    if(flag == 1)
                    {
                        Pauselastfind6 = 1;
                    }
                    else
                    {
                        Pauselastfind6 = 0;
                    }
                    Pauselastfindtime6 = FramCnt;
                    
                } 
                else
                {
                    Pauselastfindtime6 = FramCnt;
                    Pauselastfind6 = 0;
                }
            }
        }

        //-----------------pause high------------------
        if((FramCnt >= (1000/32*3600*0.5)) || (Config.level == HIGH_CHECK))
        {
            if((flag > 0) && (flag < 4))
            {
                if(Pauselastfind5 == 1)
                {
                
                    if( ((FramCnt - Pauselastfindtime5 - Config.BufferNUMCurr) > (20 * 1000 /32))
                        && ((FramCnt - Pauselastfindtime5 - Config.BufferNUMCurr) < (120 * 1000 /32)) )
                    {
                        Config.OSASTime = (FramCnt - Pauselastfindtime5 - Config.BufferNUMCurr) * 0.032;
                        flag = 5;
                    }
                    Pauselastfindtime5 = FramCnt;
                }

                if((flag == 1) || (flag == 2))
                {
                    Pauselastfind5 = 1;
                    Pauselastfindtime5 = FramCnt;
                }
                else
                {
                    Pauselastfind5 = 0;
                    
                }
            }
        }
        //-------------------------------------------
        return flag;
}

/**************************************************************************/
/*                                                                        */
/**************************************************************************/
void mfccFFTsub(short int *WaveData, SnoreConfig *config)
{
    
    const int half = MELFILTER / 2;
    int j,k,l, r_index, c_index;
    float t, c_log[MELFILTER], c1[MELFILTER / 2];
    float *AddressTemp;
    float *m, dtm;
    float distance,  *mfccBuffer;
    m = config->WaveDataMfcc;
    mfccBuffer = (float*)config->MfccMat;
    //-----------------------------------------
    for (j = 0; j < FRAME; j++)
    {
        config->Frame[j] = (float)WaveData[j] / 32768;
    }
    //-----------------加窗-----------------------------
    arm_mult_f32(config->Frame, (float *)Hamming, config->Frame, FRAME); 
    for (j = 0; j < FRAME; j++)
    {
        config->fft_inputbuf[2*j] = config->Frame[j];
        config->fft_inputbuf[2*j+1] = 0;
    }
    //-----------------fft----------------------------
    arm_cfft_f32(&arm_cfft_sR_f32_len256, config->fft_inputbuf, 0, 1);
    arm_cmplx_mag_f32(config->fft_inputbuf,config->Frame,FRAME);	

    //--------dctcoef为dct系数，bank归一化mel滤波器组系数----------
    for (r_index = 0; r_index < MELFILTER; r_index++)
    {
        t = 0;
        AddressTemp =(float *) &Bank[r_index][0];
        arm_dot_prod_f32(AddressTemp, config->Frame, (FRAME / 2+1), &t);
        c_log[r_index] = log(t);
    }
    //-------------------------------------------------
    for (r_index = 0; r_index < MELFILTER / 2; r_index++)
    {
        t = 0;
        AddressTemp =(float *) &Dctcoef[r_index][0];
        arm_dot_prod_f32(AddressTemp, c_log, MELFILTER, &t);
        c1[r_index] = t;
    }
    //w1----------为归一化倒谱提升窗口-----------------
    AddressTemp = &config->WaveDataMfcc[config->BufferNUM *(MELFILTER+1) + 0];
    arm_mult_f32(c1, (float *)MFCCW, AddressTemp, half); 
    //-------------------------------------------------
    if(config->BufferNUM >= 4)
    {
        r_index = config->BufferNUM - 2;
        for (c_index = 0; c_index < half; c_index++)
        {
            dtm =  
                - 2 * m[(r_index - 2) *(MELFILTER+1) + c_index] 
                -     m[(r_index - 1) *(MELFILTER+1)          ] 
                +     m[(r_index + 1) *(MELFILTER+1)          ] 
                + 2 * m[(r_index + 2) *(MELFILTER+1) + c_index];
            m[r_index*(MELFILTER+1) + half + c_index] = dtm / 3;
        }
      
        //--------------------------------
        for (j = 0; j < MELBuffer; j++)
        {
            distance = VectorDistance(&m[r_index*(MELFILTER+1)], mfccBuffer + j *(MELFILTER + 1), MELFILTER);
            if (distance < Threshold)
            {
                break;
            }
        }
        //--------------------------------
        m[r_index*(MELFILTER+1) + MELFILTER] = j;
        //--------------------------------


        //--------初始化新增库---------
        if(config->BufferNUM == 4)
        {
            for (j = 0; j < ((MELFILTER + 1) * CnWaveDataMfccNewMax); j++)
            {
                config->WaveDataMfccNew[j] = 0;
            }
            config->WaveDataMfccNewMax = 0;
        }
        //--------查新增库是否有一样-------------
        for (j = 0; j < CnWaveDataMfccNewMax; j++)
        {
            distance = VectorDistance(&m[r_index*(MELFILTER+1)], config->WaveDataMfccNew + j *(MELFILTER + 1), MELFILTER);
            if (distance < Threshold)
            {
                break;
            }
        }
        //---------查新增库没有找到一样的，加入库--------
        if(j == CnWaveDataMfccNewMax)
        {
            for (k = 0; k < CnWaveDataMfccNewMax; k++)
            {
                if (config->WaveDataMfccNew[k *(MELFILTER + 1) + MELWeight] < (float)0.5)
                {
                    config->WaveDataMfccNew[k *(MELFILTER + 1) + MELWeight] = 1;
                    for (l = 0; l < MELFILTER; l++)
                    {
                        config->WaveDataMfccNew[k *(MELFILTER + 1) + l] = m[r_index*(MELFILTER+1) + l];
                    }
                    config->WaveDataMfccNewMax++;
                    break;
                }   
            }
        }

        //--------------------------------
    }
    
}

uint8_t test_flag1 = 0,test_flag2 = 0;

/**************************************************************************/
/*                                                                        */
/**************************************************************************/
int CheckSnoreSub(short int * snore,uint8_t *SnoreMaxVol)
{
    uint8_t flag = 0;
	static unsigned char k=0;
    uint8_t Ensave = 0;
    uint16_t  i,j;
	  unsigned char endhour,timehour;
    //char temp[40];
    
    //uint32_t  starttime=0,endtime=0,timetemp = 0;
    //==============================
    FramCnt++;

    if(Config.vol == 20)
    {
      //  RunLed(k); 
	    //k=!k;
        Config.TestState = 0;
    }
    else
    {
        if(Config.TestState == 0)
        {
            Config.TestState = 1;
        }
    }
    
    //added by zjp for korea 3.1.29
		//start
//		if(SensorLevelAutoSetData.times!=0)
//		{
		if(SensorLevelAutoSetData_timeout_flag==1)
		{
		  if(SensorLevelAutoSetData_timeout_flag==1)
	    {
			  SnoreInit();
			  SensorLevelAutoSetData_timeout_flag=0;
		  }		
		}			
		
//		}
/*		
		if(SensorLevelAutoSetData.days!=0)
		{
			  if((stimestructure.Hours == SensorLevelAutoSetData.timestart) && (stimestructure.Minutes <=15))
			  {
					if(SensorLevelAutoSetData.checkposition==1)  
					{
						 if(PillowMaxOne_Willbesend!=0x10)
						 {
							 time_start_ok_flag=1;
						 }
						 					 
					}
				  else
					{
						time_start_ok_flag=1;
					}
					
				}
				else if((stimestructure.Hours == SensorLevelAutoSetData.timeend) && (stimestructure.Minutes <= 15))
				{
					if(SensorLevelAutoSetData.checkposition==1 && time_start_ok_flag==1)  
					{
						 if(PillowMaxOne_Willbesend!=0x10)
						 {
							 time_start_ok_flag=0;
							 SensorLevelAutoSetData.days--;
							 if(SensorLevelAutoSetData.days!=0)
							 {
							   update_sensorlevel_autoset();
							 }
							 else
							 {
								 Delete_SensorLevelAutoSet_Flag();
							   SnoreInit();
							 }
						 }
						  
					}
				  else if(time_start_ok_flag==1)
					{
						time_start_ok_flag=0;
						SensorLevelAutoSetData.days--;
						
						if(SensorLevelAutoSetData.days!=0)
						{
							update_sensorlevel_autoset();
						}
						else
						{
							Delete_SensorLevelAutoSet_Flag();
							SnoreInit();
						}					
					}
					
				}
				else if((stimestructure.Hours == SensorLevelAutoSetData.timeend) && (stimestructure.Minutes >15))
				{
					time_start_ok_flag=0;					
				}
		}
		
*/		
		//end
		//*********************************************************************
		
		
    //==============================
//    if( (FramCnt >= (1000/32*3600*0.5)) && GetEnableStudy() )      //  30 later 1:00-5:00
    if( GetEnableStudy() )      //  30 later 1:00-5:00
    {
			  //新添加在学习时间有人才学习
			  if(StudyInSleepTimeWhenPersonOn==1 && Airsensor.AirSensor_PersonOn==0)
				{
					Config.autostudy = 0;
          Snore_Time_Enable = 0;
				}
				else
				{
          Config.autostudy = 1;
          Snore_Time_Enable = 1;
				}
    }
    else
    {
        Config.autostudy = 0;
        Snore_Time_Enable = 0;
    }

    if(Config.level == HIGH_CHECK && Airsensor.AirSensor_PersonOn==1)
    {
        Config.autostudy = 1;
    }   
    //----------------------------
    //starttime = HAL_GetTick();
    //vTaskSuspendAll();
    //--------------------------------------------------------
    flag = Snoring((short int *)snore, &Config, FramCnt);
		test_flag1 = flag;
		
    //xTaskResumeAll();
    //endtime = HAL_GetTick();
    flag = SnoreTime(FramCnt,flag,&Config);
		test_flag2 = flag;

    if(test_flag1 ==  1 && test_flag2 == 2)
		{
       test_flag1 =1 ;
		}			
    //------------------------------------
//    if(Config.autostudy == 1)  //在学习时段内的存时间点还必须有人
		if(Airsensor.Tt_Is_Sleep_Time==1)	//在学习时段内保存时间点就行
    {
        //--------------------------------
        Ensave = 0;
			  while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
	      {
           osDelay(1);
		    } 
			  #ifdef UserSetSleepTime
				  //3个小时学习时间，每学一个小时存储一次
				  if(study_hour_start <= study_hour_end)
			    {
			      if(((stimestructure.Hours >= (study_hour_start + 1)) && (stimestructure.Hours <= study_hour_end)) && (stimestructure.Minutes == study_min_start))
			      {
					     Ensave = 1;				       
				    }
			    }
			    else
			    {
				    endhour = study_hour_end + 24;
				    if(stimestructure.Hours < study_hour_start)
					    timehour = stimestructure.Hours+24;
				    else
					    timehour = stimestructure.Hours;
						
						if(((timehour >= (study_hour_start + 1)) && (timehour <= endhour)) && (stimestructure.Minutes == study_min_start))
						{
							Ensave = 1;
						}
					}
				
/*
				  if(gStorageData.UserSleepTime[0]<=gStorageData.UserSleepTime[2])
			    {
			      if(((stimestructure.Hours>=(gStorageData.UserSleepTime[0]+2)) && (stimestructure.Hours<=(gStorageData.UserSleepTime[2]-2))) && (stimestructure.Minutes==gStorageData.UserSleepTime[3]))
			      {
					     Ensave = 1;				       
				    }
			    }
			    else
			    {
				    endhour=gStorageData.UserSleepTime[2]+24;
				    if(stimestructure.Hours<gStorageData.UserSleepTime[0])
					    timehour=stimestructure.Hours+24;
				    else
					    timehour=stimestructure.Hours;
						
						if(((timehour>=(gStorageData.UserSleepTime[0]+2)) && (timehour<=(endhour-2))) && (stimestructure.Minutes==gStorageData.UserSleepTime[3]))
						{
							Ensave = 1;
						}
					}
*/
			  #else
          if(stimestructure.Minutes == 50)
          {
            if((stimestructure.Hours == 3) || (stimestructure.Hours == 4))
            //if(stimestructure.Hours == 4)
            {
                 Ensave = 1;
            }
          }
				#endif
        //--------------------------------
				#ifdef UserSetSleepTime
					if(Config.level == HIGH_CHECK)
          {
						if((stimestructure.Hours == study_hour_start) && (stimestructure.Minutes==study_min_start))
						{						
                if(stimestructure.Seconds < 5)
                {
                    for(i=0;i<MELBuffer+1;i++)
                    {
                        for(j=0;j<(MELFILTER+1);j++)
                        {
                            Config.MfccMat[i][j]=0;
                        }
                    }
                }    
                Ensave = 1;
            }
          }		
/*					
        	if(Config.level == HIGH_CHECK)
          {
						if(stimestructure.Hours<gStorageData.UserSleepTime[0])
					    timehour=stimestructure.Hours+24;
				    else
					    timehour=stimestructure.Hours;
						if((timehour==(gStorageData.UserSleepTime[0]+1)) && (stimestructure.Minutes==gStorageData.UserSleepTime[1]))
						{						
                if(stimestructure.Seconds < 5)
                {
                    for(i=0;i<MELBuffer+1;i++)
                    {
                        for(j=0;j<(MELFILTER+1);j++)
                        {
                            Config.MfccMat[i][j]=0;
                        }
                    }
                }    
                Ensave = 1;
            }
          }					
*/					
				#else	
          if(Config.level == HIGH_CHECK)
          {
            if((stimestructure.Hours == 1) && (stimestructure.Minutes == 00))
            {
                if(stimestructure.Seconds < 5)
                {
                    for(i=0;i<MELBuffer+1;i++)
                    {
                        for(j=0;j<(MELFILTER+1);j++)
                        {
                            Config.MfccMat[i][j]=0;
                        }
                    }
                }    
                Ensave = 1;
            }
          }	
        #endif	
					
				xSemaphoreGive( xTimeMute );
				
        //--------------------------------
        if(Ensave == 0)
        {
            havesave = 0;
        }
        else
        {
            //----------2:50  4:50----------
            if(havesave == 0)
            {
                //MELBuffer * (MELFILTER + 1)   25000=>25088 /256  
                //if(Config.level != HIGH_CHECK)
                {
                    if(SnoreLibWR(&Config.MfccMat[0][0],25088/4,1) == 1)
                    {
                        havesave = 1;
											
											  //统计特征值数量及有效特征值数量供app读取
	
	                      vectors_num=0;
	                      valid_vcetors_num=0;
	                      for(i=0;i<7;i++)
	                        valid_vectors_weight[i]=0;
	                      for(i=0;i<MELBuffer+1;i++)
                        {
                           if(Config.MfccMat[i][MELFILTER]!=0)
                              vectors_num++;
     		
                           if(Config.MfccMat[i][MELFILTER]>=dynamic_weight)
													 {
														  if(valid_vcetors_num<=6)
															{
														    valid_vectors_weight[valid_vcetors_num]=Config.MfccMat[i][MELFILTER];
															}
                              valid_vcetors_num++;
													 }														 
                         }
											
                    }
                }    
            }
        }
				//在学习有效时，动态检查特征向量达到的最高权值，以动态调整有效特征向量的权值门限
				SetValidVectorWeightThrehold();	
				
				
    }
    //------------------------------------
/*
    timetemp = endtime - starttime;
    if(timetemp > 32)
    {
        
         Config.autostudy = 1;
    }
*/
  	//sprintf(temp,"t=%f  Max:%.2f Nose:%.2f\n\r", FramCnt*0.032, Config.maxvol*2, Config.vol*2);
/*
    if((flag > 0) && (flag < 4))
    {
  	    sprintf(temp,"%d : %.2f\n", flag, Config.maxvol*2);
        uart_puts((unsigned char*)temp);
    }
    else if((flag == 5) || (flag == 6))  
    {
  	    sprintf(temp,"%d : %d\n", flag, Config.OSASTime);
        uart_puts((unsigned char*)temp);
    }
  */  

    //--------------------------------------------------------
    //out: flag  1:连续 用于启动止鼾   2:不连续   5:有害呼吸暂停  6无害呼吸暂停
    //out: FramCnt
    //out: Config.maxvol
    //out: Config.BufferNUMCurr
    //out: Config.SnoreAverTime   //ms 每个鼾声的总时长(声音+空白)
    //-------------------------------------------------------------------------
    if (((flag == 5)) && (Config.HaveCheckSnore == 1))    //||(flag == 6)
    {
        //5:有害呼吸暂停  当鼾睡比大于50%时显示  
        //6:无害呼吸暂停  当鼾睡比小于50%时显示，最后总数不能大于30次
        //Config.OSASTime;   //返回秒数
        if(gStorageData.StudyMode==1)
        {
//		    gStorageData.Snore.ApneaTimes++;
        }
        //------------------
        flag = 2;
    }


    Snore_detected_type_flag=flag;
    //--------------------------------------
    if ((flag == 1)||(flag == 2))
    {   
        Avol8 = (unsigned char)Config.maxvol;
				SnoreNose = (unsigned char)Config.vol;
        SnoreAver = (unsigned char)Config.prevol;
        SnoreType = flag;
        SnoreNoseLow  = NoseLow;
        SnoreNoseHigh = NoseHigh;
        
       // RunLed(k); 
    	//  k=!k;
        
        //------------------------------
    	if(flag==1 || flag==2) //连续鼾声和零散鼾声都统计在内
//			if(flag==1) //只统计连续鼾声
    	{
            Config.HaveCheckSnore = 1;
						gStorageData.Snore.SnoreAvilable=1;
    	}
			if(gStorageData.Snore.SnoreAvilable && (flag == 1)) //|| flag == 2))
			{
				
				snore_times_for_antisnore++;//用于测试
			}
      if(gStorageData.Snore.SnoreAvilable && (flag == 1 || flag == 2))
//      if(gStorageData.Snore.SnoreAvilable && flag==1)
			{
				snore_times_for_snore++;
				if((Config.maxvol*2)>62)
				{

					gStorageData.Snore.SSnoreTimes++;

				}
				else if((Config.maxvol*2)>=53)
				{

					gStorageData.Snore.MSnoreTimes++;

				}
				else gStorageData.Snore.LSnoreTimes++;
				
			  Snore_MaxDB_Detected=(uint8_t)(Config.maxvol*2);
			}
      *SnoreMaxVol = (uint8_t)(Config.maxvol*2);
			
			
			if(gStorageData.Snore.MaxDB<*SnoreMaxVol)
				gStorageData.Snore.MaxDB=*SnoreMaxVol;
    	return flag;
        //------------------------------
    }
    else if((flag == 3))  //||(flag == 4)||(flag == 7))
    {
        Avol8 = (unsigned char)Config.maxvol;
        SnoreNose = (unsigned char)Config.vol;
        SnoreAver = (unsigned char)Config.prevol;
        SnoreType = flag;
        SnoreNoseLow  = NoseLow;
        SnoreNoseHigh = NoseHigh;
    }
    return 0;
}



#if 0
void User_set_studytime_process(void)
{
	uint16_t time_temp;
	//预处理学习时间，中间三小时为学习时间
		  if(gStorageData.UserSleepTime[0]<=gStorageData.UserSleepTime[2])
			{
				//设置时间没横跨晚上0点
				if(gStorageData.UserSleepTime[1]<=gStorageData.UserSleepTime[3])
				{
					time_temp = gStorageData.UserSleepTime[3]- gStorageData.UserSleepTime[1];
					time_temp = time_temp + (gStorageData.UserSleepTime[2] - gStorageData.UserSleepTime[0])*60;
					time_temp = (time_temp - 3*60)/2;
					study_min_start = gStorageData.UserSleepTime[1] + time_temp%60;
					if(study_min_start>=60)
					{
						study_min_start = study_min_start - 60;
						study_hour_start = gStorageData.UserSleepTime[0] + time_temp/60 + 1;
					}
					else
					{
						study_hour_start = gStorageData.UserSleepTime[0] + time_temp/60;
					}
					if(gStorageData.UserSleepTime[3] >= time_temp%60)
					{					
						study_min_end = gStorageData.UserSleepTime[3] - time_temp%60;
						study_hour_end = gStorageData.UserSleepTime[2] - time_temp/60;
					}
					else
					{
						study_min_end = 60 + gStorageData.UserSleepTime[3] - time_temp%60;
						study_hour_end = gStorageData.UserSleepTime[2] - time_temp/60 - 1;
					}					
				}
				
				else
				{					
					time_temp = 60 + gStorageData.UserSleepTime[3]- gStorageData.UserSleepTime[1];
					time_temp = time_temp + (gStorageData.UserSleepTime[2] - gStorageData.UserSleepTime[0]-1)*60;
					time_temp = (time_temp - 3*60)/2;
					study_min_start = gStorageData.UserSleepTime[1] + time_temp%60;
					if(study_min_start>=60)
					{
						study_min_start = study_min_start - 60;
						study_hour_start = gStorageData.UserSleepTime[0] + time_temp/60 + 1;
					}
					else
					{
						study_hour_start = gStorageData.UserSleepTime[0] + time_temp/60;
					}
					if(gStorageData.UserSleepTime[3] >= time_temp%60)
					{					
						study_min_end = gStorageData.UserSleepTime[3] - time_temp%60;
						study_hour_end = gStorageData.UserSleepTime[2] - time_temp/60;
					}
					else
					{
						study_min_end = 60 + gStorageData.UserSleepTime[3] - time_temp%60;
						study_hour_end = gStorageData.UserSleepTime[2] - time_temp/60 - 1;
					}					
				}
			}
			
			else
			{
				//设置时间横跨晚上0点
				
				if(gStorageData.UserSleepTime[1]<=gStorageData.UserSleepTime[3])
				{
					time_temp = gStorageData.UserSleepTime[3]- gStorageData.UserSleepTime[1];
					time_temp = time_temp + (gStorageData.UserSleepTime[2] + 24 - gStorageData.UserSleepTime[0])*60;
					time_temp = (time_temp - 3*60)/2;
					study_min_start = gStorageData.UserSleepTime[1] + time_temp%60;
					if(study_min_start>=60)
					{
						study_min_start = study_min_start - 60;
						study_hour_start = gStorageData.UserSleepTime[0] + time_temp/60 + 1;
						if(study_hour_start >= 24)
							study_hour_start = study_hour_start - 24;
					}
					else
					{
						study_hour_start = gStorageData.UserSleepTime[0] + time_temp/60;
						if(study_hour_start >= 24)
							study_hour_start = study_hour_start - 24;
					}
					if(gStorageData.UserSleepTime[3] >= time_temp%60)
					{					
						study_min_end = gStorageData.UserSleepTime[3] - time_temp%60;
						if(gStorageData.UserSleepTime[2] < time_temp/60)							
						  study_hour_end = 24 + gStorageData.UserSleepTime[2] - time_temp/60;
						else
							study_hour_end = gStorageData.UserSleepTime[2] - time_temp/60;
					}
					else
					{
						study_min_end = 60 + gStorageData.UserSleepTime[3] - time_temp%60;
						if(gStorageData.UserSleepTime[2] < (time_temp/60+1))
						  study_hour_end = 24 + gStorageData.UserSleepTime[2] - time_temp/60 - 1;
						else
							study_hour_end = gStorageData.UserSleepTime[2] - time_temp/60 - 1;
					}					
				}
				
				else
				{					
					time_temp = 60 + gStorageData.UserSleepTime[3]- gStorageData.UserSleepTime[1];
					time_temp = time_temp + (gStorageData.UserSleepTime[2] - gStorageData.UserSleepTime[0]-1)*60;
					time_temp = (time_temp - 3*60)/2;
					study_min_start = gStorageData.UserSleepTime[1] + time_temp%60;
					if(study_min_start>=60)
					{
						study_min_start = study_min_start - 60;
						study_hour_start = gStorageData.UserSleepTime[0] + time_temp/60 + 1;
						if(study_hour_start >= 24)
							study_hour_start = study_hour_start - 24;
					}
					else
					{
						study_hour_start = gStorageData.UserSleepTime[0] + time_temp/60;
						if(study_hour_start >= 24)
							study_hour_start = study_hour_start - 24;
					}
					if(gStorageData.UserSleepTime[3] >= time_temp%60)
					{					
						study_min_end = gStorageData.UserSleepTime[3] - time_temp%60;
						if(gStorageData.UserSleepTime[2] < time_temp/60)							
						  study_hour_end = 24 + gStorageData.UserSleepTime[2] - time_temp/60;
						else
							study_hour_end = gStorageData.UserSleepTime[2] - time_temp/60;
					}
					else
					{
						study_min_end = 60 + gStorageData.UserSleepTime[3] - time_temp%60;
						if(gStorageData.UserSleepTime[2] < (time_temp/60+1))
						  study_hour_end = 24 + gStorageData.UserSleepTime[2] - time_temp/60 - 1;
						else
							study_hour_end = gStorageData.UserSleepTime[2] - time_temp/60 - 1;
					}					
				}			
			}
}
#endif
int GetEnableStudy(void)
{
    int retflag = 0;
	  unsigned char endhour,timehour;
	  
	
//  测试代码
/*
	  gStorageData.UserSleepTime[0]=23;
	  gStorageData.UserSleepTime[1]=00;
	  gStorageData.UserSleepTime[2]=06;
	  gStorageData.UserSleepTime[3]=00;
	  stimestructure.Hours=4;
	  stimestructure.Minutes=0;
*/	
	  //一直没按起矗第二天也不学习
//	  if(Airsensor.Sleep_snore_study_flag==0)
//		{
//			return retflag;
//		}			
	
	  while(xSemaphoreTake( xTimeMute, portMAX_DELAY ) != pdTRUE)
	  {
        osDelay(1);
		}  
    #ifdef UserSetSleepTime	    
			
			//判断是否处于学习时间段
			if(study_hour_start <= study_hour_end)
			{
			   if(((stimestructure.Hours > study_hour_start) || ((stimestructure.Minutes >= study_min_start)  && (stimestructure.Hours==study_hour_start)))			 
				    && ((stimestructure.Hours < study_hour_end) || ((stimestructure.Minutes <= study_min_end)  && (stimestructure.Hours==study_hour_end))))
			   {
					 retflag = 1;
					 Airsensor.Sleep_snore_study_control=1;
				 }
				 
				 
			 }
			 else
			 {
				 endhour = study_hour_end + 24;
				 if(stimestructure.Hours < study_hour_start)
					 timehour = stimestructure.Hours + 24;
				 else
					 timehour = stimestructure.Hours;
				 
				 if(((timehour > study_hour_start) || ((stimestructure.Minutes >= study_min_start)  && (timehour == study_hour_start)))			 
				    && ((timehour < endhour)  || ((stimestructure.Minutes <= study_min_end)  && (timehour == endhour))))
			   {
				   
				   retflag = 1;
					 Airsensor.Sleep_snore_study_control=1;
				 }
				  
			 }
			
/*			
			if(gStorageData.UserSleepTime[0]<=gStorageData.UserSleepTime[2])
			{
			   if(((stimestructure.Hours>(gStorageData.UserSleepTime[0])+1) || ((stimestructure.Minutes>=gStorageData.UserSleepTime[1])  && (stimestructure.Hours==(gStorageData.UserSleepTime[0]+1))))			 
				    && ((stimestructure.Hours<(gStorageData.UserSleepTime[2]-1)) || ((stimestructure.Minutes<gStorageData.UserSleepTime[3])  && (stimestructure.Hours==(gStorageData.UserSleepTime[2]-1)))))
			   {
					 //前后各减一个小时内为学习时间
				   retflag = 1;
				 }
			 }
			 else
			 {
				 endhour=gStorageData.UserSleepTime[2]+24;
				 if(stimestructure.Hours<gStorageData.UserSleepTime[0])
					 timehour=stimestructure.Hours+24;
				 else
					 timehour=stimestructure.Hours;
				 
				 if(((timehour>(gStorageData.UserSleepTime[0]+1)) || ((stimestructure.Minutes>=gStorageData.UserSleepTime[1])  && (timehour==(gStorageData.UserSleepTime[0]+1))))			 
				    && ((timehour<(endhour-1))  || ((stimestructure.Minutes<gStorageData.UserSleepTime[3])  && (timehour==(endhour-1)))))
			   {
				   //前后各减一个小时内为学习时间
				   retflag = 1;
				 }
				  
			 }
*/	
	  #else	
      if((stimestructure.Hours >= 1) && (stimestructure.Hours <= 4))
      {
        retflag = 1;
      }
    #endif
		xSemaphoreGive( xTimeMute );
		//如果不按键起床，只学习一天
		if(Airsensor.Sleep_snore_study_control==1 && retflag==0)
			Airsensor.Sleep_snore_study_flag=0;
    return retflag;
}




