/********************************************************************************/
/* Filename	: snore.h                                                           */
/*                                                                              */
/* Content	:                                                                   */
/* Note    	:                                                                   */
/*                                                                              */
/* Change History   :                                                           */
/*        V.0.00  2016/11/17      Linbo                                         */
/********************************************************************************/
#ifndef __SNORE_H__  
    #define __SNORE_H__  
    #include "stm32f4xx.h"

	#define HIGH_CHECK      1    // 高
	#define MID_CHECK       2    // 中
	#define LOW_CHECK       3    // 低
	#define TEST_CHECK      4    // 测试

    #define LOW_SNOREDB     36      
    #define MAX_NOSEDB      34      
   
    #define BUFFER      200   //缓冲区数量 
    #define MELFILTER   24    //梅尔倒谱阶数 
    #define MELBuffer   250   //梅尔向量数量 
    #define CnWaveDataMfccNewMax 10
    #define MELWeight   24    //梅尔倒谱权值存储位置 
    #define MAXWeight   1000 //权值上限
    #define SUBWeight   1     //每次减权的权值
    #define ADDWeight   4     //每次加权的权值
    #define CONWeight   100  //置信权值，当向量权值到达此值时，为有效特征向量 150/3=50
    
    #define FRAME       256	  //没帧长度 
    #define Threshold   9     //梅尔向量距离阈值 
    #define VOLSTEP     256  //音量阶数 
    #define MELSTEP     256	  //梅尔倒谱计算时划窗速度 
    #define E           2.718281828459
    #define MINLength   0.32 //最小鼾声长度 
    #define VOLNUM      500  //音量缓存，500缓存16秒 

    //---------------------------------------------
    typedef struct SnoreConfig
    {
        float fft_inputbuf[FRAME*2];    //FFT输入数组
        float Frame[FRAME];             //FFT输出数组
        float WaveDataMfcc[(MELFILTER + 1) *BUFFER];    //声音数据mfcc缓存 
        float WaveDataMfccNew[(MELFILTER + 1) * CnWaveDataMfccNewMax];  //声音数据mfcc缓存 
        float FRAMEVol[BUFFER];         //每帧音量
        float MfccMat[MELBuffer+1][MELFILTER + 1];        //历史梅尔倒谱向量表，用于对比声音数据。 
        
        float VolBuffer[VOLNUM];        //音量值缓存区 ，用于自动判断音量阈值 
        float maxvol;
        float prevol;
        float vol;                      //能熵值 
        float PowerRatio;               //能量分布频段比例  
        float MfccRatio;                //检出的缓存特征向量与该语音段特征向量的比例

        float SetLike;
        float SetAver;
        float SetMax;
        float SetStudyAver;
        float SetStudyMax;

        long int TimeCount;
        long int lastfindtime;

        long int lastfindtimeEnd;
        long int lastfindtimeStart;

        int BufferNUMCurr;              //缓冲区数据帧数 
        int BufferNUM;                  //缓冲区数据帧数 
        int MfccIndex;
        int VolIndex;                   //音量值缓冲区索引 
        int autostudy;                  //是否开启自学习开关 
        int Enstudy;                    //是否开启学习开关 
        int WaveDataMfccNewMax;
        int SetContinuous;
        int AutoStudyEn;
        int level;                      //级别 1:高  2:中  3:低
        int i;
        int OSASTime;
        int SnoreAverTime;

        int HaveCheckSnore;

        int CheckLeav;                  // 0:严检  1:放松检
        int TestState;                  // 测试状态 0:等待  1:学习中  2:学习成功
        int Testvol;


        
    } SnoreConfig;
    //---------------------------------------------
    typedef struct Complex              //复数类型  
    {
        float real; //实部  
        float imag; //虚部  
    } Complex;
    //---------------------------------------------
    typedef struct
    {
		 short  SnoreBuffer[256];
		 short  num;
	  }SnoreBuf_Def;
    //---------------------------------------------

    //梅尔倒谱计算函数
    //WaveData指针指向原始声音数据 ，config指针指向一系列计算所需参数，详细见snore.h中的说明
    //返回为梅尔倒谱向量表指针，注意，申请了空间，返回指针需要释放。 
    extern void SnoreInit(void);
		extern void SnoreInit_personon(void);
    extern int CheckSnore(short int * snore);
    extern SnoreBuf_Def  SnoreBuf;
    int SnoreLibWR(float * data,unsigned int len,unsigned char wr);
		

    extern struct SnoreConfig Config;
    //==============================
    //#define SNORELEVEL  HIGH_CHECK            //HIGH_CHECK   //MID_CHECK  //LOW_CHECK
    //==============================



#endif
