
#include "position.h"

#ifndef SPI_FLASH_FILE_DEF
#define SPI_FLASH_FILE_DEF 1
#define  SYS_PARA_SIZE 64
#define  SYS_PARA_CYCLE_TIMES 128
#define  SNORE_DATA_ADDRESS 0x2000  //8K
#define  SNORE_DATA_CNT     1440    //350     //16K 1440����Լ240��Сʱ�� 240*60/10=1440��ÿ10����Ϊһ�����ݣ�ÿ������Ϊ32�ֽ�
#define  SNORE_DATA_SIZE    32
#define  SYS_PARA_FLAG   0x55aa55aa


#define PRIVATE_SNORE_LIB_OFFSET_32K    0x18000 //0x8000  //32k
#define PRIVATE_SNORE_LIB_MAX_SIZE_64K  0x10000  //64K

#define DATE_TIME_OFFSET      PRIVATE_SNORE_LIB_OFFSET_32K+PRIVATE_SNORE_LIB_MAX_SIZE_64K
#define DATE_TIME_SIZE_12K 0x3000

#define POSITION_DATA_OFFSET    DATE_TIME_OFFSET+DATE_TIME_SIZE_12K    


typedef struct{   //32 bytes
	  unsigned int   UsedRecord;
		unsigned char  StudyMode;
		unsigned char  StartTime[5];
	  unsigned char  SSnoreTimes;    //�ضȴ�������
	  unsigned char  MSnoreTimes;    //�жȴ�������
	  unsigned char  LSnoreTimes;    //��ȴ���ʱ��
		unsigned char  Airbag1Times;   //����1��������
	  unsigned char  Airbag2Times;   //����2��������
	  unsigned char  Airbag3Times;   //����3��������
	  unsigned char  Airbag4Times;   //����4��������
	  unsigned char  HeadPosion;     //ͷ����λ��
	  unsigned char  MaxDB;          //��������ֱ�
	  unsigned short  AVGSnoreTime;           //ƽ������ʱ�� 
//	  unsigned char ApneaTimes;
	  unsigned char HeadMovingTimes;
	  unsigned char SnoreAvilable;
	
	  //Ϊ�����ϴ�/�뿪ʱ���app��ʱ���
	  unsigned char person_on_minute;
    unsigned char person_on_second;
    unsigned char person_off_minute;
    unsigned char person_off_second;//total 23byte
	 
}SnoreData_def;

typedef	struct  //64bytes
{
	unsigned int    flag;       //0x55aa55aa
  unsigned short  SPIFlashAddress;
	unsigned short  SPIFlashOutAddress;
	unsigned short  ActionLevel;   
	unsigned char   SensorLevel; 
  unsigned char   HeadpositionLevel; 
	unsigned char   WorkDelay;   
	unsigned short  StudyMode;  //StudyMode 0:����ģʽ 1��ѧϰģʽ 2����ʾģʽ
	unsigned short  Standby;
	//unsigned short  InhaleOver;
	unsigned short  ExhaleOver;
	//unsigned char   workTimeOn;
	//unsigned char   workTime[2][4];
	#ifdef UserSetSleepTime
	unsigned char   UserSleepTime[4];//UserSleepTime[0]:��ʼʱ UserSleepTime[1]:��ʼ�� UserSleepTime[2]:����ʱ UserSleepTime[3]:������
	#endif
	unsigned short  PositionValue_Onebag;
	unsigned short  PositionValue_Twobag;
	unsigned char   MinSnoredB;  //total 25bytes
	unsigned char   Body_detect_on_threshold;
	unsigned char   Body_detect_off_threshold;
	unsigned char   One_hour_auto_detect_onoff;
	unsigned char   auto_detect_timecycle;//total 32bytes
	int   					Modifystudymodestamptime;//��ѧϰģʽ�Զ�תΪֹ��ģʽ��ʱ���
	//unsigned char   keycodebyte1;
	//unsigned char		keycodebyte2;	
	//unsigned char		keycodebyte3;
	//unsigned char		keycodebyte4;//total 40bytes
	SnoreData_def   Snore; //total 23byte
}stStorageData_Def;
/*
typedef struct
{     unsigned  int    SaveSystemDate;  //???????
      unsigned  int  SaveSystemTime;  //?????
      unsigned  short 	UsedRecord; 
	    unsigned  short   StudyMode;
    	unsigned  char	SnoreTimes;      //????
    	unsigned  char 	InterfereTimes;    //????
    	unsigned  char 	Volume;          //????
      unsigned  char   SleepPositon;     //?? 1:??? ;2:???;3:???;5:???; 6:???  ; 7:??
      unsigned  int    res[4];  //??
}PillowData_Def;
*/
#endif
extern stStorageData_Def gStorageData;

unsigned char WriteSnoreData(SnoreData_def *p);
unsigned char ReadSnoreData(SnoreData_def *p);
unsigned char BackupSysPara(stStorageData_Def *s);
unsigned char ReadSysPara(stStorageData_Def *s);
unsigned short GetSnoreDataCnt(void);
int SnoreLibWR(float * data,unsigned int len,unsigned char wr);
unsigned char Backup_RTC(RTC_DateTypeDef *date,RTC_TimeTypeDef *time);
void spitest(void);
unsigned char Read_Backup_RTC(RTC_DateTypeDef *date,RTC_TimeTypeDef *time);
unsigned char Read_Backup_RTC_test(void);