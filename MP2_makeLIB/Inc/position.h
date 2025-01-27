
#ifndef __POSITION_H__  
    #define __POSITION_H__
		
		
#include "stm32f4xx.h"

#define UserSetSleepTime 1
#define UserSleepTimeStartHour 23
#define UserSleepTimeStartMin 00
#define UserSleepTimeEndHour 07
#define UserSleepTimeEndMin 00


#define RECHECK_NO_DELAY_FLAG 1  //1:设置复检标志后尽快复检 0：设置复检标志后延迟一定时间后复检
//用户购买后前2天不管sensor灵敏度设置为何，强制实际采用为高的灵敏度进行鼾声识别
#define SENSORLEVEL_HIGH_FOR_24HOURS_ENABLED 0 //1:有效 0：无效
#define  SENSORLEVEL_HIGH_TO_MED_FLAG   0xaa55aa55  //??ó?óúo?1ú°?D?1o?òó??§?aê?èyííêμ?ê2éó???￡?μ???è??a?D?￡
#define  SENSORLEVEL_HIGH_TO_MED_FINISHED_FLAG   0x55aa55aa  //??ó?óúo?1ú°?D?1o?òó??§?aê?èyííêμ?ê2éó???￡?μ???è??a?D?￡
//#define  SENSORLEVEL_HIGH_TO_MED_DAYS   3           //?¨ò??????a??μ?ììêy
//#define  CHECK_POSITION_AT_SensorLevelAutoSet 0     //?¨ò?×??ˉéè??sensor levelê±ê?·??D??óDè??ú′2
//#define  STARTTIME_AT_SensorLevelAutoSet 2  //??é?2μ?±?D??ú′2
//#define  ENDTIME_AT_SensorLevelAutoSet 5    //??é?5μ?±?D??ú′2
#define SENSORLEVEL_HIGH_TO_MED_Times 144 //以10分钟为单位，24个小时为4*6=24


//位置检测参数
//MP2产品
/*
#define AirSensor_threshold_person_on  135 //检测人压上枕头阈值  //125
#define AirSensor_threshold_person_on1 200 //检测人压上枕头阈值  //180
#define AirSensor_threshold_person_snore 135 //有鼾时有人阈值 //125
#define AirSensor_threshold_person1_on 200 //180 //检测人压上枕头2路之和阈值
#define AirSensor_threshold_person1_snore 200 //180 //有鼾时有人2路之和阈值 //120
*/

//头部位置检测高灵敏度
#define AirSensor_threshold_person_on_HIGH  125 //检测人压上枕头阈值  //125
#define AirSensor_threshold_person_on1_HIGH 200 //检测人压上枕头阈值  //180
#define AirSensor_threshold_person_snore_HIGH 125 //有鼾时有人阈值 //125
#define AirSensor_threshold_person1_on_HIGH 180 //180 //检测人压上枕头2路之和阈值
#define AirSensor_threshold_person1_snore_HIGH 180 //180 //有鼾时有人2路之和阈值 //120
//头部位置检测中灵敏度
#define AirSensor_threshold_person_on_MID  200 //检测人压上枕头阈值  //125
#define AirSensor_threshold_person_on1_MID 300 //检测人压上枕头阈值  //180
#define AirSensor_threshold_person_snore_MID 200 //有鼾时有人阈值 //125
#define AirSensor_threshold_person1_on_MID 300 //180 //检测人压上枕头2路之和阈值
#define AirSensor_threshold_person1_snore_MID 300 //180 //有鼾时有人2路之和阈值 //120
//头部位置检测低灵敏度
#define AirSensor_threshold_person_on_LOW  250 //检测人压上枕头阈值  //125
#define AirSensor_threshold_person_on1_LOW 350 //检测人压上枕头阈值  //180
#define AirSensor_threshold_person_snore_LOW 250 //有鼾时有人阈值 //125
#define AirSensor_threshold_person1_on_LOW 350 //180 //检测人压上枕头2路之和阈值
#define AirSensor_threshold_person1_snore_LOW 350 //180 //有鼾时有人2路之和阈值 //120


#define AirSensor_position_slowcheck_cycle 30000 //30000  //单位20ms,1分钟
#define AirSensor_position_fastcheck_cycle 1  //单位20ms,40ms

#define Airsensor_Max_Diff_airpressure_one_bag 2800
#define Airsensor_Max_Diff_airpressure_two_bag 5000



//人体检测参数
#define AirSensor_LED_onoff_time 3 //单位为秒
#define AirSensor_recheck_person_onoff_time 2 //单位分钟
#define AirSensor_Pressure_fs 2//10  //气压值原始采样频率
#define AirSensor_PressureTest_fs  10      //采样频率 每秒10次
#define AirSensor_PressureTest_buffer_length 5*AirSensor_PressureTest_fs   //采样数据缓冲区长度,保存5秒时长

#define AirSensor_Avgvalue_Step  10// 20 //求当前气压平均值用n次滤波后的值求平均  20
#define AirSensor_Avgvalue_Step1 10 //求当前气压平均值用n次滤波后的值求平均  10

#define AirSensor_PressureTest_filter_step 5 //取n次采样值求平均
#define AirSensor_Avgvalue_Buffer_Size 10//20//判断是否平稳的Buffer大小  //30
#define AirSensor_Avgvalue_Buffer_forMaxMin_Size 200 //判断后期特性的Buffer大小
#define AirSensor_pressure_person_on_Max_threshold 20 //视为有人的气压上升幅度
#define stable_up_down_threshold 2 //相邻升高多少下降多少视为稳定
#define AirSensor_pressure_stable_threshold 3 //连续AirSensor_Avgvalue_Buffer_Size个值中视为稳定的变化范围(最大和最小值之绝对插值�

//修改调试
#define person_on_threshold  60 //60 //40//显示和判断为有人的上升门限
#define person_on_threshold1  60 //60 //指示等预先指示有人
#define main_down_period_threshold 50 //识别为主下段的下降值门限
#define stable_down_threshold 40 //-40 //-30 //稳定值之间下降多少开始进行是否离开的判别 //-40
#define stable_threshold_for_person_remain_on 70
#define sample_pressure_stepdown_threshold -30 //采样数据单步下降幅度


#define person_on_threshold_sampleavg  150

//原来MP2长气袋参数
/*
#define person_on_threshold 40 //显示和判断为有人的上升门限
#define person_on_threshold1 50 //指示等预先指示有人
#define main_down_period_threshold 50 //识别为主下段的下降值门限
#define stable_down_threshold -30 //稳定值之间下降多少开始进行是否离开的判别 //-40
#define stable_threshold_for_person_remain_on 70
*/


#define MIC_point_repeat_times 1
#define Airbag_To_Pump_PowerOn_Time  890 //910
#define Airbag_To_Pump_PowerOff_Time 210
#define Airbag_To_Air_PowerOn_Time   420 //430
#define Airbag_To_Air_PowerOff_Time  430 //460

/*
typedef	struct  //9bytes
{
	unsigned int    flag;       //0x55aa55aa
	uint16_t        days;       //默认为中，实际操作为高的天数
	uint8_t         checkposition;
	uint8_t         timestart;
	uint8_t         timeend;
  
}SensorLevelAutoSet_Def;
*/

typedef	struct  //9bytes
{
	unsigned int    flag;       //0x55aa55aa
	uint8_t         times;       //默认为中，实际操作为高小时数
}SensorLevelAutoSet_Def;


typedef	struct  //?bytes
{
	
	//位置检测变量
	uint16_t AirSensor_PressureValue[4][5];
	uint16_t AirSensor_PressureValueMin[4];
	uint16_t AirSensor_PressureValueMax[4];
	uint16_t AirSensor_PressureValueAvg[4];
	uint16_t MaxDiff;
	uint16_t MinDiff;
	uint16_t AvgDiff;
	uint16_t AvgDiff1;
	uint16_t AvgDiff_original;
	uint16_t AvgDiff1_original;
	uint16_t AvgDiff_Total;
	uint16_t AvgDiff_Total_original;
  uint16_t AirSensor_Pressure;
	uint16_t AirSensor_PressureValue_Diff[4];
	uint16_t AirSensor_PressureValue_Diff_filtered[4];
	uint16_t Airsensor_checkposition_valueavg_last[4];
	uint16_t AirSensor_PressureValue_Diff_filtered_last[4];
	uint16_t AirSensor_PressureValue_Diff_last_noperson[4];
	uint16_t AirSensor_PressureValue_Diff_filtered_new[4];
	uint8_t  AirSensor_Max_last_bag;
	
	
	
	//人体检测相关变量
	uint16_t AirSensor_PressureTest[AirSensor_PressureTest_buffer_length]; //采样数据缓冲区
  uint16_t AirSensor_PressureBuffer[AirSensor_Pressure_fs]; //最原始的采样数据，进行平均后放入Airsensor.AirSensor_PressureTest
  uint16_t AirSensor_PressureBuffer_WR_pointer;
  uint16_t AirSensor_PressureTest_WR_pointer;
  uint8_t AirSensor_PersonOn_Last; //0:no person 1:Person on
  uint8_t AirSensor_PersonOn; //0:no person 1:Person on
  uint8_t AirSensor_PersonOn1; //0:no person 1:Person on
  uint8_t AirSensor_PersonOn2; //0:no person 1:Person on
  uint8_t AirSensor_PersonOn3; //0:no person 1:Person on
  uint16_t AirSensor_Maxvalue; //最大气压值
  uint16_t AirSensor_Maxup; //气压上升的最大变化值（2个采样点之间，代表上升的陡度)
  uint16_t AirSensor_Minvalue; //最小气压值
  uint16_t AirSensor_Mindiff; //气压下降的最大变化值（2个采样点之间，代表下降的陡度)
  uint16_t AirSensor_Avgvalue; //当前气压平均值
  uint16_t AirSensor_Avgvalue1; //当前气压平均值

  uint8_t AirSensor_BufferRefresh; //当前buffer数据是否当前最新并全部有效 0：no  1:yes
  uint16_t AirSensor_PressureTest_filtered[AirSensor_PressureTest_buffer_length]; //采样滤波后的数据缓冲区

  uint16_t AirSensor_Avgvalue_Buffer_Pointer;
  uint16_t AirSensor_Avgvalue_Buffer[AirSensor_Avgvalue_Buffer_Size];
	
  uint16_t AirSensor_Avgvalue_Buffer_forMaxMin_Pointer;
  uint16_t AirSensor_Avgvalue_Buffer_forMaxMin[AirSensor_Avgvalue_Buffer_forMaxMin_Size];
  uint16_t AirSensor_Avgvalue_Buffer_forMaxMin1[AirSensor_Avgvalue_Buffer_forMaxMin_Size];//共用size和指针
	uint16_t AirSensor_Sample_Avgvalue_Buffer[AirSensor_Avgvalue_Buffer_forMaxMin_Size];//采样数据的缓冲区

  uint8_t Max_or_Min_detected; //最新检测到的是极大值还是极小值  1：极大值 0：极小值

  uint16_t AirSensor_pressure_sample_avg;
	uint16_t AirSensor_pressure_sample_avg_last;
	uint16_t AirSensor_pressure_sample_avg_diffsum;
  uint16_t AirSensor_pressure_sample;

  uint16_t AirSensor_pressure_stable_value;//这次的稳定值
  uint16_t AirSensor_pressure_stable_value_last;//上一次的稳定值

  uint8_t pressure_stable_value_last_is_valid; //上一次的稳定值是否有效标志 1：有效  0：无效
  uint8_t pressure_stable_value_valid_counter;

  int16_t pressure_stable_value_diff_between_now_before;

  float person_onoff_factor2;
  uint16_t AirSensor_Avgvalue_Last;
  uint16_t AirSensor_stepnum_between_twostablestate;

  uint8_t down_stepnum;
  uint8_t stable_stepnum;
  uint16_t down_total;
  uint16_t down_max;
  uint16_t down_MAX;
  uint16_t down_MIN;
  uint8_t onoff_result_reason;
  uint16_t last_diff_MaxMin;//最后阶段最大值减去最小值
  uint8_t min_stepnosition;//最低点位置

  uint8_t AirSensor_stable_min_detected_flag;
  uint8_t Led_Person_OnOff;//0:off 1:on 2:off
  uint8_t AirSensor_step_stable_flag;  //0:不稳定，1：稳定
	uint8_t AirSensor_step_last_stable_flag;  //0:不稳定，1：稳定
  uint8_t AirSensor_stable_noresult_flag;//需要判定人是否离开，但目前无法确定判断，留待后续继续判断
  uint16_t Body_Moved_Times; //体动次数
  int16_t Body_Pressure_Value; //大致的人体压力气压值,累计剩余值，如果人体的慢动导致升高或者无法统计在内，误差会比较大
  uint16_t AirSensor_Pressure_from_noperson_to_Personon;//由无人到有人之前的无人稳定气压值
  uint16_t Pressure_increase_several_times;//在无人状态下气压值分次增加的情况，每次可能低于阀值

  uint8_t AirSensor_position_checked_by_fastmode_flag;//位置是由02 fast mode检测的位置
  uint8_t personon_by_positioncheck_flag;//有人是通过位置检测判断出的标志
	uint8_t personon_by_positioncheck_flag_forpensononoff;//用于人体是否在床算法选择的标志
  uint8_t power_on_set_value_flag;//上电将读取到avg值赋予稳态初值
  uint8_t AirSensor_fast_check_position_finished_flag;
  uint8_t AirSensor_PersonOn_init_snore_flag;
  uint8_t AirSensor_fast_check_mode;
  uint8_t AirSensor_Checkposition_Mode;
	uint8_t AirSensor_Check_Mode;
	
	//新增标志
	uint8_t AirSensor_noperson_state_checked_istrue;
	uint8_t AirSensor_vale_exceed_upvalue_istrue; //气压值超越阀值
	uint8_t AirSensor_vale_exceed_upvalue_flag;
	
	uint8_t AirSensor_log_state;
  uint8_t AirSensor_recheck_times;	
	
	uint16_t headposition_sensitivity_person_on;
  uint16_t headposition_sensitivity_person_snore;
  uint16_t headposition_sensitivity_person1_on;
  uint16_t headposition_sensitivity_person1_snore;
	uint16_t headposition_sensitivity_person_on1;
	uint8_t Sleep_time_flag;
	uint8_t Sleep_rapidly_check_flag;
	uint8_t period_snore_checkposition_flag;
  uint8_t period_snore_checkposition_counter;
	uint8_t antisonre_timer_flag;
	unsigned int antisnore_timer_len;
	uint8_t Sleep_snore_study_flag;
	uint8_t Sleep_snore_study_control;
	uint8_t Tt_Is_Sleep_Time;
	uint8_t Tt_Is_Sleep_Time_last;
//	uint8_t onehour_snore_checkposition_flag;
	uint32_t onehour_snore_checkposition_counter;
	unsigned char   Body_detect_on_threshold;
	unsigned char   Body_detect_off_threshold;
	uint8_t It_Is_Standby_State;
	uint8_t LED_flash_state_flag;
	uint32_t Led_flash_counter;
	uint8_t Led_yellow_state;
	uint8_t Check_again_if_person_on;
	uint8_t Poweroffkey_savedata_flag;
	
}Airsensor_struct_Def;




typedef	struct  //?bytes
{
  
  uint16_t TIM7_operation_counter_time[10];
  int8_t TIM7_operation_code[10];
  unsigned char  Valve_Number_Operate;

}Valve_struct_Def;




extern SensorLevelAutoSet_Def SensorLevelAutoSetData;
extern Airsensor_struct_Def Airsensor;
extern Valve_struct_Def Valve_Control;

extern uint8_t PillowMaxOne_Willbesend;
extern uint8_t Pillow2ndMaxOne_Willbesend;
extern uint8_t secondtest;
extern volatile uint32_t Led_OnOff_Timer;

extern uint8_t time_start_ok_flag;
extern uint8_t SensorLevelAutoSetData_timeout_flag;

//extern uint8_t AirSensor_PersonOn;
//extern uint8_t personon_by_positioncheck_flag;
//extern uint8_t Body_Moved_Times;


extern void check_sensorlevel_autoset(void);
extern void Build_SensorLevelAutoSet_Flag(void);
extern void Delete_SensorLevelAutoSet_Flag(void);
extern void update_sensorlevel_autoset(void);
extern void Airsensor_SendlogtoPC(void);

extern void AirSensor_position_process(Airsensor_struct_Def * air);
extern void AirSensor_monitor_Pressure_AvgValue(Airsensor_struct_Def * air);
extern void AirSensor_monitor_Pressure_AvgValue1(Airsensor_struct_Def * air);
//extern void AirSensor_person_detect_init(void);
extern void AirSensor_find_valid_MaxMin(Airsensor_struct_Def * air);
extern void AirSensor_detect_person_onoff(Airsensor_struct_Def * air);

extern void AirSensor_fast_check_person_poweron(void);

extern void Switch_Valve_Do_Airbag_To_Air_Sub(Valve_struct_Def * valve_struct,unsigned char valve);
extern void Switch_Valve_Do_Airbag_To_Pump_Sub(Valve_struct_Def * valve_struct,unsigned char valve);

#endif

