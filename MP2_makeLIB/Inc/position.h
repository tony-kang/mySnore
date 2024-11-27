
#ifndef __POSITION_H__  
    #define __POSITION_H__
		
		
#include "stm32f4xx.h"

#define UserSetSleepTime 1
#define UserSleepTimeStartHour 23
#define UserSleepTimeStartMin 00
#define UserSleepTimeEndHour 07
#define UserSleepTimeEndMin 00


#define RECHECK_NO_DELAY_FLAG 1  //1:���ø����־�󾡿츴�� 0�����ø����־���ӳ�һ��ʱ��󸴼�
//�û������ǰ2�첻��sensor����������Ϊ�Σ�ǿ��ʵ�ʲ���Ϊ�ߵ������Ƚ�������ʶ��
#define SENSORLEVEL_HIGH_FOR_24HOURS_ENABLED 0 //1:��Ч 0����Ч
#define  SENSORLEVEL_HIGH_TO_MED_FLAG   0xaa55aa55  //??��?����o?1����?D?1o?����??��?a��?��y��������?��2����???��?��???��??a?D?��
#define  SENSORLEVEL_HIGH_TO_MED_FINISHED_FLAG   0x55aa55aa  //??��?����o?1����?D?1o?����??��?a��?��y��������?��2����???��?��???��??a?D?��
//#define  SENSORLEVEL_HIGH_TO_MED_DAYS   3           //?����??????a??��?������y
//#define  CHECK_POSITION_AT_SensorLevelAutoSet 0     //?����?��??������??sensor level������?��??D??��D��??����2
//#define  STARTTIME_AT_SensorLevelAutoSet 2  //??��?2��?��?D??����2
//#define  ENDTIME_AT_SensorLevelAutoSet 5    //??��?5��?��?D??����2
#define SENSORLEVEL_HIGH_TO_MED_Times 144 //��10����Ϊ��λ��24��СʱΪ4*6=24


//λ�ü�����
//MP2��Ʒ
/*
#define AirSensor_threshold_person_on  135 //�����ѹ����ͷ��ֵ  //125
#define AirSensor_threshold_person_on1 200 //�����ѹ����ͷ��ֵ  //180
#define AirSensor_threshold_person_snore 135 //����ʱ������ֵ //125
#define AirSensor_threshold_person1_on 200 //180 //�����ѹ����ͷ2·֮����ֵ
#define AirSensor_threshold_person1_snore 200 //180 //����ʱ����2·֮����ֵ //120
*/

//ͷ��λ�ü���������
#define AirSensor_threshold_person_on_HIGH  125 //�����ѹ����ͷ��ֵ  //125
#define AirSensor_threshold_person_on1_HIGH 200 //�����ѹ����ͷ��ֵ  //180
#define AirSensor_threshold_person_snore_HIGH 125 //����ʱ������ֵ //125
#define AirSensor_threshold_person1_on_HIGH 180 //180 //�����ѹ����ͷ2·֮����ֵ
#define AirSensor_threshold_person1_snore_HIGH 180 //180 //����ʱ����2·֮����ֵ //120
//ͷ��λ�ü����������
#define AirSensor_threshold_person_on_MID  200 //�����ѹ����ͷ��ֵ  //125
#define AirSensor_threshold_person_on1_MID 300 //�����ѹ����ͷ��ֵ  //180
#define AirSensor_threshold_person_snore_MID 200 //����ʱ������ֵ //125
#define AirSensor_threshold_person1_on_MID 300 //180 //�����ѹ����ͷ2·֮����ֵ
#define AirSensor_threshold_person1_snore_MID 300 //180 //����ʱ����2·֮����ֵ //120
//ͷ��λ�ü���������
#define AirSensor_threshold_person_on_LOW  250 //�����ѹ����ͷ��ֵ  //125
#define AirSensor_threshold_person_on1_LOW 350 //�����ѹ����ͷ��ֵ  //180
#define AirSensor_threshold_person_snore_LOW 250 //����ʱ������ֵ //125
#define AirSensor_threshold_person1_on_LOW 350 //180 //�����ѹ����ͷ2·֮����ֵ
#define AirSensor_threshold_person1_snore_LOW 350 //180 //����ʱ����2·֮����ֵ //120


#define AirSensor_position_slowcheck_cycle 30000 //30000  //��λ20ms,1����
#define AirSensor_position_fastcheck_cycle 1  //��λ20ms,40ms

#define Airsensor_Max_Diff_airpressure_one_bag 2800
#define Airsensor_Max_Diff_airpressure_two_bag 5000



//���������
#define AirSensor_LED_onoff_time 3 //��λΪ��
#define AirSensor_recheck_person_onoff_time 2 //��λ����
#define AirSensor_Pressure_fs 2//10  //��ѹֵԭʼ����Ƶ��
#define AirSensor_PressureTest_fs  10      //����Ƶ�� ÿ��10��
#define AirSensor_PressureTest_buffer_length 5*AirSensor_PressureTest_fs   //�������ݻ���������,����5��ʱ��

#define AirSensor_Avgvalue_Step  10// 20 //��ǰ��ѹƽ��ֵ��n���˲����ֵ��ƽ��  20
#define AirSensor_Avgvalue_Step1 10 //��ǰ��ѹƽ��ֵ��n���˲����ֵ��ƽ��  10

#define AirSensor_PressureTest_filter_step 5 //ȡn�β���ֵ��ƽ��
#define AirSensor_Avgvalue_Buffer_Size 10//20//�ж��Ƿ�ƽ�ȵ�Buffer��С  //30
#define AirSensor_Avgvalue_Buffer_forMaxMin_Size 200 //�жϺ������Ե�Buffer��С
#define AirSensor_pressure_person_on_Max_threshold 20 //��Ϊ���˵���ѹ��������
#define stable_up_down_threshold 2 //�������߶����½�������Ϊ�ȶ�
#define AirSensor_pressure_stable_threshold 3 //����AirSensor_Avgvalue_Buffer_Size��ֵ����Ϊ�ȶ��ı仯��Χ(������Сֵ֮���Բ�ֵ�

//�޸ĵ���
#define person_on_threshold  60 //60 //40//��ʾ���ж�Ϊ���˵���������
#define person_on_threshold1  60 //60 //ָʾ��Ԥ��ָʾ����
#define main_down_period_threshold 50 //ʶ��Ϊ���¶ε��½�ֵ����
#define stable_down_threshold 40 //-40 //-30 //�ȶ�ֵ֮���½����ٿ�ʼ�����Ƿ��뿪���б� //-40
#define stable_threshold_for_person_remain_on 70
#define sample_pressure_stepdown_threshold -30 //�������ݵ����½�����


#define person_on_threshold_sampleavg  150

//ԭ��MP2����������
/*
#define person_on_threshold 40 //��ʾ���ж�Ϊ���˵���������
#define person_on_threshold1 50 //ָʾ��Ԥ��ָʾ����
#define main_down_period_threshold 50 //ʶ��Ϊ���¶ε��½�ֵ����
#define stable_down_threshold -30 //�ȶ�ֵ֮���½����ٿ�ʼ�����Ƿ��뿪���б� //-40
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
	uint16_t        days;       //Ĭ��Ϊ�У�ʵ�ʲ���Ϊ�ߵ�����
	uint8_t         checkposition;
	uint8_t         timestart;
	uint8_t         timeend;
  
}SensorLevelAutoSet_Def;
*/

typedef	struct  //9bytes
{
	unsigned int    flag;       //0x55aa55aa
	uint8_t         times;       //Ĭ��Ϊ�У�ʵ�ʲ���Ϊ��Сʱ��
}SensorLevelAutoSet_Def;


typedef	struct  //?bytes
{
	
	//λ�ü�����
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
	
	
	
	//��������ر���
	uint16_t AirSensor_PressureTest[AirSensor_PressureTest_buffer_length]; //�������ݻ�����
  uint16_t AirSensor_PressureBuffer[AirSensor_Pressure_fs]; //��ԭʼ�Ĳ������ݣ�����ƽ�������Airsensor.AirSensor_PressureTest
  uint16_t AirSensor_PressureBuffer_WR_pointer;
  uint16_t AirSensor_PressureTest_WR_pointer;
  uint8_t AirSensor_PersonOn_Last; //0:no person 1:Person on
  uint8_t AirSensor_PersonOn; //0:no person 1:Person on
  uint8_t AirSensor_PersonOn1; //0:no person 1:Person on
  uint8_t AirSensor_PersonOn2; //0:no person 1:Person on
  uint8_t AirSensor_PersonOn3; //0:no person 1:Person on
  uint16_t AirSensor_Maxvalue; //�����ѹֵ
  uint16_t AirSensor_Maxup; //��ѹ���������仯ֵ��2��������֮�䣬���������Ķ���)
  uint16_t AirSensor_Minvalue; //��С��ѹֵ
  uint16_t AirSensor_Mindiff; //��ѹ�½������仯ֵ��2��������֮�䣬�����½��Ķ���)
  uint16_t AirSensor_Avgvalue; //��ǰ��ѹƽ��ֵ
  uint16_t AirSensor_Avgvalue1; //��ǰ��ѹƽ��ֵ

  uint8_t AirSensor_BufferRefresh; //��ǰbuffer�����Ƿ�ǰ���²�ȫ����Ч 0��no  1:yes
  uint16_t AirSensor_PressureTest_filtered[AirSensor_PressureTest_buffer_length]; //�����˲�������ݻ�����

  uint16_t AirSensor_Avgvalue_Buffer_Pointer;
  uint16_t AirSensor_Avgvalue_Buffer[AirSensor_Avgvalue_Buffer_Size];
	
  uint16_t AirSensor_Avgvalue_Buffer_forMaxMin_Pointer;
  uint16_t AirSensor_Avgvalue_Buffer_forMaxMin[AirSensor_Avgvalue_Buffer_forMaxMin_Size];
  uint16_t AirSensor_Avgvalue_Buffer_forMaxMin1[AirSensor_Avgvalue_Buffer_forMaxMin_Size];//����size��ָ��
	uint16_t AirSensor_Sample_Avgvalue_Buffer[AirSensor_Avgvalue_Buffer_forMaxMin_Size];//�������ݵĻ�����

  uint8_t Max_or_Min_detected; //���¼�⵽���Ǽ���ֵ���Ǽ�Сֵ  1������ֵ 0����Сֵ

  uint16_t AirSensor_pressure_sample_avg;
	uint16_t AirSensor_pressure_sample_avg_last;
	uint16_t AirSensor_pressure_sample_avg_diffsum;
  uint16_t AirSensor_pressure_sample;

  uint16_t AirSensor_pressure_stable_value;//��ε��ȶ�ֵ
  uint16_t AirSensor_pressure_stable_value_last;//��һ�ε��ȶ�ֵ

  uint8_t pressure_stable_value_last_is_valid; //��һ�ε��ȶ�ֵ�Ƿ���Ч��־ 1����Ч  0����Ч
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
  uint16_t last_diff_MaxMin;//���׶����ֵ��ȥ��Сֵ
  uint8_t min_stepnosition;//��͵�λ��

  uint8_t AirSensor_stable_min_detected_flag;
  uint8_t Led_Person_OnOff;//0:off 1:on 2:off
  uint8_t AirSensor_step_stable_flag;  //0:���ȶ���1���ȶ�
	uint8_t AirSensor_step_last_stable_flag;  //0:���ȶ���1���ȶ�
  uint8_t AirSensor_stable_noresult_flag;//��Ҫ�ж����Ƿ��뿪����Ŀǰ�޷�ȷ���жϣ��������������ж�
  uint16_t Body_Moved_Times; //�嶯����
  int16_t Body_Pressure_Value; //���µ�����ѹ����ѹֵ,�ۼ�ʣ��ֵ���������������������߻����޷�ͳ�����ڣ�����Ƚϴ�
  uint16_t AirSensor_Pressure_from_noperson_to_Personon;//�����˵�����֮ǰ�������ȶ���ѹֵ
  uint16_t Pressure_increase_several_times;//������״̬����ѹֵ�ִ����ӵ������ÿ�ο��ܵ��ڷ�ֵ

  uint8_t AirSensor_position_checked_by_fastmode_flag;//λ������02 fast mode����λ��
  uint8_t personon_by_positioncheck_flag;//������ͨ��λ�ü���жϳ��ı�־
	uint8_t personon_by_positioncheck_flag_forpensononoff;//���������Ƿ��ڴ��㷨ѡ��ı�־
  uint8_t power_on_set_value_flag;//�ϵ罫��ȡ��avgֵ������̬��ֵ
  uint8_t AirSensor_fast_check_position_finished_flag;
  uint8_t AirSensor_PersonOn_init_snore_flag;
  uint8_t AirSensor_fast_check_mode;
  uint8_t AirSensor_Checkposition_Mode;
	uint8_t AirSensor_Check_Mode;
	
	//������־
	uint8_t AirSensor_noperson_state_checked_istrue;
	uint8_t AirSensor_vale_exceed_upvalue_istrue; //��ѹֵ��Խ��ֵ
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
