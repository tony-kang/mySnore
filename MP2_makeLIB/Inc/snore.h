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

	#define HIGH_CHECK      1    // ��
	#define MID_CHECK       2    // ��
	#define LOW_CHECK       3    // ��
	#define TEST_CHECK      4    // ����

    #define LOW_SNOREDB     36      
    #define MAX_NOSEDB      34      
   
    #define BUFFER      200   //���������� 
    #define MELFILTER   24    //÷�����׽��� 
    #define MELBuffer   250   //÷���������� 
    #define CnWaveDataMfccNewMax 10
    #define MELWeight   24    //÷������Ȩֵ�洢λ�� 
    #define MAXWeight   1000 //Ȩֵ����
    #define SUBWeight   1     //ÿ�μ�Ȩ��Ȩֵ
    #define ADDWeight   4     //ÿ�μ�Ȩ��Ȩֵ
    #define CONWeight   100  //����Ȩֵ��������Ȩֵ�����ֵʱ��Ϊ��Ч�������� 150/3=50
    
    #define FRAME       256	  //û֡���� 
    #define Threshold   9     //÷������������ֵ 
    #define VOLSTEP     256  //�������� 
    #define MELSTEP     256	  //÷�����׼���ʱ�����ٶ� 
    #define E           2.718281828459
    #define MINLength   0.32 //��С�������� 
    #define VOLNUM      500  //�������棬500����16�� 

    //---------------------------------------------
    typedef struct SnoreConfig
    {
        float fft_inputbuf[FRAME*2];    //FFT��������
        float Frame[FRAME];             //FFT�������
        float WaveDataMfcc[(MELFILTER + 1) *BUFFER];    //��������mfcc���� 
        float WaveDataMfccNew[(MELFILTER + 1) * CnWaveDataMfccNewMax];  //��������mfcc���� 
        float FRAMEVol[BUFFER];         //ÿ֡����
        float MfccMat[MELBuffer+1][MELFILTER + 1];        //��ʷ÷���������������ڶԱ��������ݡ� 
        
        float VolBuffer[VOLNUM];        //����ֵ������ �������Զ��ж�������ֵ 
        float maxvol;
        float prevol;
        float vol;                      //����ֵ 
        float PowerRatio;               //�����ֲ�Ƶ�α���  
        float MfccRatio;                //����Ļ�����������������������������ı���

        float SetLike;
        float SetAver;
        float SetMax;
        float SetStudyAver;
        float SetStudyMax;

        long int TimeCount;
        long int lastfindtime;

        long int lastfindtimeEnd;
        long int lastfindtimeStart;

        int BufferNUMCurr;              //����������֡�� 
        int BufferNUM;                  //����������֡�� 
        int MfccIndex;
        int VolIndex;                   //����ֵ���������� 
        int autostudy;                  //�Ƿ�����ѧϰ���� 
        int Enstudy;                    //�Ƿ���ѧϰ���� 
        int WaveDataMfccNewMax;
        int SetContinuous;
        int AutoStudyEn;
        int level;                      //���� 1:��  2:��  3:��
        int i;
        int OSASTime;
        int SnoreAverTime;

        int HaveCheckSnore;

        int CheckLeav;                  // 0:�ϼ�  1:���ɼ�
        int TestState;                  // ����״̬ 0:�ȴ�  1:ѧϰ��  2:ѧϰ�ɹ�
        int Testvol;


        
    } SnoreConfig;
    //---------------------------------------------
    typedef struct Complex              //��������  
    {
        float real; //ʵ��  
        float imag; //�鲿  
    } Complex;
    //---------------------------------------------
    typedef struct
    {
		 short  SnoreBuffer[256];
		 short  num;
	  }SnoreBuf_Def;
    //---------------------------------------------

    //÷�����׼��㺯��
    //WaveDataָ��ָ��ԭʼ�������� ��configָ��ָ��һϵ�м��������������ϸ��snore.h�е�˵��
    //����Ϊ÷������������ָ�룬ע�⣬�����˿ռ䣬����ָ����Ҫ�ͷš� 
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
