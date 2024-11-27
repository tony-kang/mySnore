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

    static uint32_t lastfindtime=0,lastfindtime1=0,lastflag=0; //ÓÃ×÷Ñ­»·¼ÆÊý
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
SnoreBuf_Def  SnoreBuf;
struct SnoreConfig Config;
extern const arm_cfft_instance_f32 arm_cfft_sR_f32_len256;
extern uint8_t AirSensor_Position_Checking;
extern uint8_t PillowMaxOne_Willbesend,Body_Left_Right_Willbesend;
//extern uint8_t AirSensor_PersonOn;

short int   TestTempWaveData[24];
short int   TestOkWaveData[24];


//=================================================


//-------------------------------------------------------------------------
float OTSU(float *buffer, int Num)
{
    //aveÎª×ÜÆ½¾ùÖµ£¬ave0ÎªãÐÖµ×ó²àÆ½¾ùÖµ£¬ave1ÎªãÐÖµÓÒ²àÆ½¾ùÖµ£¬ varianceÎªÀà¼ä·½²î 
    //maxÎªÊý¾Ý×î´óÖµ£¬minÎªÊý¾Ý×îÐ¡Öµ£¬TÎª±éÀúãÐÖµ ,maxTÎª·½²î×î´óãÐÖµ£¬maxVÎª×î´ó·½²î 
    float ave, ave0, ave1, variance, max, min, T, maxT, maxV;
    int i, p0, p1;
    maxT = 0;
    maxV = 0;
    max = buffer[0];
    min = buffer[0];
    ave = buffer[0];

    //---------------------------------
    //ÇóËùÓÐÊý¾ÝµÄ×î´óÖµ×îÐ¡Öµ¼°Æ½¾ùÖµ 
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

//³õÊ¼»¯ÔËËãÓÃ¸÷Àà»º´æ²ÎÊý¼°ÔËËãÓÃ¾ØÕó 
int iniConfig(SnoreConfig *config)
{
    int i;
    config->vol = 20; //³õÊ¼»¯ÒôÁ¿Öµ
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


//³õÊ¼»¯ÔËËãÓÃ¸÷Àà»º´æ²ÎÊý¼°ÔËËãÓÃ¾ØÕó  added by zjp
int iniConfig1(SnoreConfig *config)
{
    int i;
//    config->vol = 20; //³õÊ¼»¯ÒôÁ¿Öµ
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



//´æ´¢µÄmfcc¾ØÕó¼õÈ¨²¢ÇåÀí 
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
            //----------------É¾³ý---------------------------
            for (j = 0; j < MELFILTER + 1; j++)
            {
                data[i *col + j] = 0;
            }
            //----------------Ç°ÒÆ---------------------------
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

//¼ÆËãÏòÁ¿¾àÀë
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

//·µ»Ømfcc¶Ô±ÈÖ¸±êÖµ 
//mfccÎªÊäÈëÏòÁ¿±í£¬configÎª´æ´¢ÔËËãÅäÖÃÖ¸Õë£¬Buffer¼ÇÂ¼»º´æÖ¡Êý£¬Mel¼ÇÂ¼µ¹Æ×½×Êý 
//MELBuffer, MELFILTER
//int MBuffer,int Mel
int mfccCount(SnoreConfig *config)
{
    int i, j, k, l, count, wcount, weight[MELBuffer];
    float distance,  *mfccBuffer;
    int r_index, lastnolike[CnWaveDataMfccNewMax];
    float *m;

    //------Ì«¶àÐÂÔöµÄ²»ÒªÌí¼ÓÖ±½ÓÈÏÎª²»Ïñ-----------
    if(config->WaveDataMfccNewMax >= CnWaveDataMfccNewMax)
    {
        return 0;
    }
    
    m = config->WaveDataMfcc;
    
    count = 0;
    wcount = 0;
    mfccBuffer = (float*)config->MfccMat;
    //³õÊ¼»¯¼ÓÈ¨¼ÇÂ¼Êý×é 
    for (i = 0; i < MELBuffer; i++)
    {
        weight[i] = 0;
    }

    //-------³õÊ¼»¯ÐÂÔöÎ»ÖÃ---------
    for(l=0;l<CnWaveDataMfccNewMax;l++)
    {
        lastnolike[l] = MELBuffer;
    }
    
    //-------------------------------
    for (i = 0; i < (config->BufferNUM -4); i++)
    {

        r_index = i + 2;


        j = m[r_index*(MELFILTER+1) + MELFILTER];

        //--------------¼ì²éÐÂÔö¿éÊÇ·ñÓÐÒ»ÑùµÄ------------------
        if(j == MELBuffer && config->Enstudy == 1)
        {
            //----------¼ì²éµ±Ç°¿éÐÂÔöÎ»ÖÃ-----------
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
                if (mfccBuffer[j *(MELFILTER + 1) + MELWeight] >= CONWeight)
                {
                    count++;
                }
            }    
            
            if (mfccBuffer[j *(MELFILTER + 1) + MELWeight] >(float) 0.9)
            {
                weight[j] = 1;
            }
        }



        
        //Èç¹ûÃ»ÓÐÔÚ´æ´¢µÄmfccÏòÁ¿±íÖÐ²éÕÒµ½£¬ÔòÏòÏòÁ¿±íÖÐÌí¼Óµ±Ç°mfccÏòÁ¿	
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
                    }

                    //-------------µ±Ç°¿éÐÂÔöÎ»ÖÃ----------------
                    for(l=0;l<CnWaveDataMfccNewMax;l++)
                    {
                        if(lastnolike[l] == MELBuffer)
                        {
                            lastnolike[l] = k;  //ÓÐ¿Õ´æ
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




//ÏÖÖ±½Ó¼ÆËãdbÖµ£¬×¢ÊÍ²¿·ÖÎªÄÜìØÖµ 
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

//ÅÐ¶ÏÊÇ·ñÎª ÷ýÉù ,²»ÊÇ·µ»Ø0£¬ÊÇ·µ»Ø1£¬ÁéÃô¶ÈÌ«¸ß·µ»Ø-1. 
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
	  PVEvol8=PVEvol;
    //²âÊÔ×Ô¶¯ÒôÁ¿
    Avol = Autovol(config, PVEvol);
	  PVEvolBak=PVEvol;
    if (Avol > 10 && Airsensor.AirSensor_PersonOn==1)
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
				ErrorCode|=(1<<ERROR_CODE_SNORE_ERROR);//÷ýÉùÒì³£
		}
		else ContinueErrorTime=0;
    //½áÊø²âÊÔ×Ô¶¯ÒôÁ¿				
    if (PVEvol > config->vol)
    {
        if (config->BufferNUM < BUFFER)
        //ÕýÔÚ»º³å 
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
        //³¬³ö»º³åÇø£¬»º³åÓïÒôÖ¡³¬³ö÷ýÉùÓ¦ÓÐ³¤¶È ¡£»º³å¼ÆÊý¼ÌÐøÔö¼Ó£¬µ«ÊÇ²»»º³åÓïÒôÖ¡ 
        {
            config->BufferNUM = config->BufferNUM + 1;
            return  - 1;
        }
    }
    else
    {
        if (config->BufferNUM < 10)
        // ½áÊø»º³å£¬ »º³åÓïÒôÖ¡Ð¡ÓÚ÷ýÉùÓ¦ÓÐ³¤¶È
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
        //	½áÊø»º³å£¬ »º³åÓïÒôÖ¡´óÓÚ÷ýÉùÓ¦ÓÐ³¤¶È	
        {
            config->BufferNUM = 0;
            config->lastfindtimeEnd = time;
            return  7;
           
        }
        else
        //½áÊø»º³å£¬Õý³£³¤¶È¡£¿ªÊ¼ÅÐ¶ÏÊÇ·ñÎª÷ýÉù 
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
            
            if((config->prevol * 2) < LOW_SNOREDB)
            {
                flag = 4;                
            }

            if((config->maxvol * 2) < LOW_SNOREDB)
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
                    //------ÅÐ¶ÏÁ¬Ðø-------
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





/**************************************************************************/
/*                                                                        */
/**************************************************************************/
void SnoreInit(void)
{
    int i,j;
    iniConfig(&Config);

    if(gStorageData.StudyMode)
    {
        Config.level =  HIGH_CHECK;
    }
    else
    {
        
			  //Config.level = gStorageData.SensorLevel;   //¼¶±ð 1:¸ß  2:ÖÐ  3:µÍ
			
			  //modified byz zjp for korea 3.1.29 ÔÚÒ»¶¨ÌìÊýÄÚÇ¿ÖÆsensor levelÎª¸ß
			  //start
			  if(SensorLevelAutoSetData.days==0)
           Config.level = gStorageData.SensorLevel;   //¼¶±ð 1:¸ß  2:ÖÐ  3:µÍ
				else
				{
					Config.level =  HIGH_CHECK;
				}
				//end
				//******************************************************
    }


    Config.SetContinuous = 3; //Á¬ÐøµÚÈý¸ö
    
    Config.SetLike = 8 * 0.1; //80%ÏàËÆ¶È


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


    for(i=0;i<MELBuffer+1;i++)
    {
        for(j=0;j<(MELFILTER+1);j++)
        {
            Config.MfccMat[i][j]=0;
        }
    }
    
	SnoreBuf.num=0;
	PillowControl.SnoreOn=false;
	SnoreDetectTime=0;

    //Config.level = MID_CHECK;   //¼¶±ð 1:¸ß  2:ÖÐ  3:µÍ
    //HIGH_CHECK   //MID_CHECK  //LOW_CHECK

    if(Config.level != HIGH_CHECK)
    {
        SnoreLibWR(&Config.MfccMat[0][0],25088/4,0);
        Config.CheckLeav = 0;       //ÑÏ¼ì
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

    if(gStorageData.StudyMode)
    {
        Config.level =  HIGH_CHECK;
    }
    else
    {
        //Config.level = gStorageData.SensorLevel;   //¼¶±ð 1:¸ß  2:ÖÐ  3:µÍ
			
			  //modified byz zjp for korea 3.1.29 ÔÚÒ»¶¨ÌìÊýÄÚÇ¿ÖÆsensor levelÎª¸ß
			  //start
			  if(SensorLevelAutoSetData.days==0)
           Config.level = gStorageData.SensorLevel;   //¼¶±ð 1:¸ß  2:ÖÐ  3:µÍ
				else
				{
					Config.level =  HIGH_CHECK;
				}
				//end
				//******************************************************
    }


    Config.SetContinuous = 3; //Á¬ÐøµÚÈý¸ö
    
    Config.SetLike = 8 * 0.1; //80%ÏàËÆ¶È


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


    for(i=0;i<MELBuffer+1;i++)
    {
        for(j=0;j<(MELFILTER+1);j++)
        {
            Config.MfccMat[i][j]=0;
        }
    }
    
	SnoreBuf.num=0;
	PillowControl.SnoreOn=false;
	SnoreDetectTime=0;

    //Config.level = MID_CHECK;   //¼¶±ð 1:¸ß  2:ÖÐ  3:µÍ
    //HIGH_CHECK   //MID_CHECK  //LOW_CHECK

    if(Config.level != HIGH_CHECK)
    {
        SnoreLibWR(&Config.MfccMat[0][0],25088/4,0);
        Config.CheckLeav = 0;       //ÑÏ¼ì
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
}


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
            if(Config.CheckLeav == 0)     //Ö®Ç°type=1Æô¶¯Ò»´Îºó1·ÖÖÓÄÚ²»¿¹¸ÉÈÅ
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
                if(Config.maxvol < (LOW_SNOREDB/2))
                {
                    flag = 0;
                }
            }
            else if(Config.level == MID_CHECK)
            {
                if(Config.maxvol < ((LOW_SNOREDB+0)/2)) 
                {
                    flag = 0;
                }
            }           
            else if(Config.level == LOW_CHECK)
            {
                if(Config.maxvol < ((LOW_SNOREDB+4)/2))
                {
                    flag = 0;
                }
            }  
        }    
        //--------------------------------
        highlowEn = 0;
        //ÒÔÏÂ´úÂë·¢ÐÐÇ°È¡Ïû,°×ÌìÃ»ÓÐÅÐ¶ÏÆµÓò
#ifdef SNORE_TESTMODE
        if(Config.level == HIGH_CHECK)
        {
            if((stimestructure.Hours >= 9) && (stimestructure.Hours < 21))
            {
                highlowEn = 1; 
            }   
        }
#endif
        //========================²âÊÔÄ£Ê½==========================
        //if((Config.level == TEST_CHECK)||(highlowEn == 1))
        if(Config.level == HIGH_CHECK)    
        {
            if(flag == 9)
            {
                flag = 3;
            }
            
            //========================================
            if ((flag > 0) && (flag < 4))   //  1ÏñºÍ3²»Ïñ
            {
                if(lastfindtime == 0)
                {
                    lastfindtime = FramCnt;
                    flag = 2;
                    lastflag = 1;
                }
                else if((FramCnt - lastfindtime) > (2200/32))     //2// 2.2s  24´Î
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
            
            //===============Á½·ÖÖÓÄÚ²ÉÓÃÊ±Óò===================
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
                if(lastfindtime == 0)
                {
                    lastfindtime = FramCnt;
                    flag = 2;
                }
                else if((FramCnt - lastfindtime) > (2200/32))     //2// 2.2s  24´Î
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
                        }
                    }
                    lastfindtime = FramCnt;
                }
                else
                {
                    flag = 2;
                    if(Config.level != HIGH_CHECK)
                    {
                        lastfindtime = FramCnt - (2200/32);
                        lastflag = 0;
                    }   
                    
                }

            }
    		//end-----ºôºÍÎü¼ÆÊýÎªÒ»¸ö-----
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
            //===============Á½·ÖÖÓÄÚ²ÉÓÃÊ±Óò===================
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

                }
               
                //----------------------------------------
            }
            //=================È·±£²»»áÌ«ÃÜ====================
            if((flag > 0) && (flag < 4))
            {
                if(lastfindtime1 == 0)
                {
                    lastfindtime1 = FramCnt;
                    flag = 0;
                }
                else if((FramCnt - lastfindtime1) > (2200/32))     //2// 2.2s  24´Î
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
    //-----------------¼Ó´°-----------------------------
    arm_mult_f32(config->Frame, (float *)Hamming, config->Frame, FRAME); 
    for (j = 0; j < FRAME; j++)
    {
        config->fft_inputbuf[2*j] = config->Frame[j];
        config->fft_inputbuf[2*j+1] = 0;
    }
    //-----------------fft----------------------------
    arm_cfft_f32(&arm_cfft_sR_f32_len256, config->fft_inputbuf, 0, 1);
    arm_cmplx_mag_f32(config->fft_inputbuf,config->Frame,FRAME);	

    //--------dctcoefÎªdctÏµÊý£¬bank¹éÒ»»¯melÂË²¨Æ÷×éÏµÊý----------
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
    //w1----------Îª¹éÒ»»¯µ¹Æ×ÌáÉý´°¿Ú-----------------
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


        //--------³õÊ¼»¯ÐÂÔö¿â---------
        if(config->BufferNUM == 4)
        {
            for (j = 0; j < ((MELFILTER + 1) * CnWaveDataMfccNewMax); j++)
            {
                config->WaveDataMfccNew[j] = 0;
            }
            config->WaveDataMfccNewMax = 0;
        }
        //--------²éÐÂÔö¿âÊÇ·ñÓÐÒ»Ñù-------------
        for (j = 0; j < CnWaveDataMfccNewMax; j++)
        {
            distance = VectorDistance(&m[r_index*(MELFILTER+1)], config->WaveDataMfccNew + j *(MELFILTER + 1), MELFILTER);
            if (distance < Threshold)
            {
                break;
            }
        }
        //---------²éÐÂÔö¿âÃ»ÓÐÕÒµ½Ò»ÑùµÄ£¬¼ÓÈë¿â--------
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


/**************************************************************************/
/*                                                                        */
/**************************************************************************/
int CheckSnoreSub(short int * snore,uint8_t *SnoreMaxVol)
{
    uint8_t flag = 0;
	static unsigned char k=0;
    uint8_t Ensave = 0;
    uint16_t  i,j;
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
		
		if(SensorLevelAutoSetData.days!=0)
		{
			  if((stimestructure.Hours == SensorLevelAutoSetData.timestart) && (stimestructure.Minutes <=15))
			  {
					if(SensorLevelAutoSetData.checkposition==1)  //ÐèÒªÈËÔÚ´²·ñ£
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
					if(SensorLevelAutoSetData.checkposition==1 && time_start_ok_flag==1)  //ÐèÒªÈËÔÚ´²·ñ£
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
		
		
		//end
		//*********************************************************************
		
		
    //==============================
    if( (FramCnt >= (1000/32*3600*0.5)) && GetEnableStudy() )      //  30 later 1:00-5:00
    //if( GetEnableStudy() )      //  30 later 1:00-5:00
    {
        Config.autostudy = 1;
        Snore_Time_Enable = 1;
    }
    else
    {
        Config.autostudy = 0;
        Snore_Time_Enable = 0;
    }

    if(Config.level == HIGH_CHECK)
    {
        Config.autostudy = 1;
    }   
    //----------------------------
    //starttime = HAL_GetTick();
    //vTaskSuspendAll();
    //--------------------------------------------------------
    flag = Snoring((short int *)snore, &Config, FramCnt);
    //xTaskResumeAll();
    //endtime = HAL_GetTick();
    flag = SnoreTime(FramCnt,flag,&Config);



    //------------------------------------
    if(Config.autostudy == 1)
    {
        //--------------------------------
        Ensave = 0;
        if(stimestructure.Minutes == 50)
        {
            if((stimestructure.Hours == 3) || (stimestructure.Hours == 4))
            //if(stimestructure.Hours == 4)
            {
                 Ensave = 1;
            }
        }
        //--------------------------------
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
                    }
                }    
            }
        }
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
    //out: flag  1:Á¬Ðø ÓÃÓÚÆô¶¯Ö¹÷ý   2:²»Á¬Ðø   5:ÓÐº¦ºôÎüÔÝÍ£  6ÎÞº¦ºôÎüÔÝÍ£
    //out: FramCnt
    //out: Config.maxvol
    //out: Config.BufferNUMCurr
    //out: Config.SnoreAverTime   //ms Ã¿¸ö÷ýÉùµÄ×ÜÊ±³¤(ÉùÒô+¿Õ°×)
    //-------------------------------------------------------------------------
    if (((flag == 5)) && (Config.HaveCheckSnore == 1))    //||(flag == 6)
    {
        //5:ÓÐº¦ºôÎüÔÝÍ£  µ±÷ýË¯±È´óÓÚ50%Ê±ÏÔÊ¾  
        //6:ÎÞº¦ºôÎüÔÝÍ£  µ±÷ýË¯±ÈÐ¡ÓÚ50%Ê±ÏÔÊ¾£¬×îºó×ÜÊý²»ÄÜ´óÓÚ30´Î
        //Config.OSASTime;   //·µ»ØÃëÊý
        if(gStorageData.StudyMode==1)
        {
//		    gStorageData.Snore.ApneaTimes++;
        }
        //------------------
        flag = 2;
    }


   
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
    	if(flag==1)
    	{
            Config.HaveCheckSnore = 1;
						gStorageData.Snore.SnoreAvilable=1;
    	}

      if(gStorageData.Snore.SnoreAvilable)
			{
				if((Config.maxvol*2)>62)
				{

					gStorageData.Snore.SSnoreTimes++;

				}
				else if((Config.maxvol*2)>=53)
				{

					gStorageData.Snore.MSnoreTimes++;

				}
				else gStorageData.Snore.LSnoreTimes++;
			
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


int GetEnableStudy(void)
{
    int retflag = 0;
    
    if((stimestructure.Hours >= 1) && (stimestructure.Hours <= 4))
    {
        retflag = 1;
    }

    return retflag;
}




