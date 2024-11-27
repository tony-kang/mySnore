
#ifndef __POSITION_H__  
    #define __POSITION_H__
		
		
#include "stm32f4xx.h"
#include "config.h"

#define Date_Time_Save_Enable 0  //±£´æÊ±¼äÊÇ·ñÊ¹ÄÜ

//#define User_Keycode_Enable 1 //ÊÇ·ñÖ§³ÖÓÃ»§Éè¶¨ºÍÑéÖ¤ÊäÈëÃÜÂë
#define USERKEYCODE_ENABLE 0//1:ÓĞĞ§£¬0£ºÎŞĞ§
#define USERVALIDTIME_ENABLE 1
#define Update_Presskey3S_Enable 1 //ÊÇ·ñÖ§³ÖÉı¼¶Ê±°´¼ü3Ãë,¸úÓÃ»§ÃÜÂëÖ»ÄÜ¶şÑ¡Ò»

#define Seconds_for_savedateandtime  10 //µ¥Î»Ãë£¬¼ä¸ô¶à¾Ã±£´æÒ»´ÎÈÕÆÚºÍÊ±¼äµ½SPIÒÔ±¸°Î²åµçÔ´µ¼ÖÂRTC¸´Î»ÒÔ»Ö¸´Ê±¼ä


#define App_COMMAND_LOG 1
#define StudyDelayTime 120 //¼ì²âµ½ÓĞÈËµ½¿ªÊ¼Ñ§Ï°ÑÓ³ÙµÄÊ±¼ä£¬µ¥Î»Îª·ÖÖÓ 120
#define Checkposition_times_whennoperson 2 //Á¬Ğø¼ì²â¶àÉÙ´ÎÎŞÈË¾ÍÔİÍ£ÓĞ÷ıÉù¼ìÎ»ÖÃ
#define Checkposition_period_whennoperson 300 //µ¥Î»Ãë£¬03¼ìÎŞÈË¸ô¶à¾ÃÔÙ³¢ÊÔÓĞ÷ıÉùÔÙ´Î¼ì²â 300ÃëÎª5·ÖÖÓ


#define Recheck_when_person_without_snore_longtime_beyond_sleeptime_enable 0 // Ë¯ÃßÊ±¶ÎÖ®Íâ£¬ÔÚÓĞÈËµÄÇé¿öÏÂ£¬Èç¹û³¤Ê±¼äÎŞ÷ıÉùÔòÈ¡ÈËÒ»ÏÂÊÇ·ñÕæµÄÓĞÈË 0 or 1
#define Recheck_when_person_without_snore_longtime_beyond_sleeptime_time 60  //60·ÖÖÓ£¬ÉÏÃæ¼ì²éµÄÊ±¼ä²ÎÊı

#define UserSetSleepTime 1
#define UserSetTimeAutoONOFF 1
#define UserSleepTimeStartHour 25 //23
#define UserSleepTimeStartMin  60 //00
#define UserSleepTimeEndHour 25 //07
#define UserSleepTimeEndMin 60 //00




#define RECHECK_NO_DELAY_FLAG 1  //1:ÉèÖÃ¸´¼ì±êÖ¾ºó¾¡¿ì¸´¼ì 0£ºÉèÖÃ¸´¼ì±êÖ¾ºóÑÓ³ÙÒ»¶¨Ê±¼äºó¸´¼ì
//ÓÃ»§¹ºÂòºóÇ°24Ğ¡Ê±²»¹ÜsensorÁéÃô¶ÈÉèÖÃÎªºÎ£¬Ç¿ÖÆÊµ¼Ê²ÉÓÃÎª¸ßµÄÁéÃô¶È½øĞĞ÷ıÉùÊ¶±ğ
#define SENSORLEVEL_HIGH_FOR_24HOURS_ENABLED 0 //1:ÓĞĞ§ 0£ºÎŞĞ§

#define  SENSORLEVEL_HIGH_TO_MED_FLAG   0xaa55aa55  //??¨®?¨®¨²o?1¨²¡ã?D?1o?¨°¨®??¡ì?a¨º?¨¨y¨ª¨ª¨º¦Ì?¨º2¨¦¨®???¡ê?¦Ì???¨¨??a?D?¡ê
#define  USERKEYCODE_FLAG   0x55aa55aa
#define  SENSORLEVEL_HIGH_TO_MED_FINISHED_FLAG   0x55aa55aa  //??¨®?¨®¨²o?1¨²¡ã?D?1o?¨°¨®??¡ì?a¨º?¨¨y¨ª¨ª¨º¦Ì?¨º2¨¦¨®???¡ê?¦Ì???¨¨??a?D?¡ê
//#define  SENSORLEVEL_HIGH_TO_MED_DAYS   3           //?¡§¨°??????a??¦Ì?¨¬¨¬¨ºy
//#define  CHECK_POSITION_AT_SensorLevelAutoSet 0     //?¡§¨°?¡Á??¡¥¨¦¨¨??sensor level¨º¡À¨º?¡¤??D??¨®D¨¨??¨²¡ä2
//#define  STARTTIME_AT_SensorLevelAutoSet 2  //??¨¦?2¦Ì?¡À?D??¨²¡ä2
//#define  ENDTIME_AT_SensorLevelAutoSet 5    //??¨¦?5¦Ì?¡À?D??¨²¡ä2
#define SENSORLEVEL_HIGH_TO_MED_Times 144 //ÒÔ10·ÖÖÓÎªµ¥Î»£¬24¸öĞ¡Ê±Îª4*6=24


//Î»ÖÃ¼ì²â²ÎÊı
//MP2²úÆ·
/*
#define AirSensor_threshold_person_on  135 //¼ì²âÈËÑ¹ÉÏÕíÍ·ãĞÖµ  //125
#define AirSensor_threshold_person_on1 200 //¼ì²âÈËÑ¹ÉÏÕíÍ·ãĞÖµ  //180
#define AirSensor_threshold_person_snore 135 //ÓĞ÷ıÊ±ÓĞÈËãĞÖµ //125
#define AirSensor_threshold_person1_on 200 //180 //¼ì²âÈËÑ¹ÉÏÕíÍ·2Â·Ö®ºÍãĞÖµ
#define AirSensor_threshold_person1_snore 200 //180 //ÓĞ÷ıÊ±ÓĞÈË2Â·Ö®ºÍãĞÖµ //120
*/

#ifdef MP1_PLUS
//MP1_PLUS²ÎÊı
//Í·²¿Î»ÖÃ¼ì²â¸ßÁéÃô¶È
#define AirSensor_threshold_person_on_HIGH  125//150 //¼ì²âÈËÑ¹ÉÏÕíÍ·ãĞÖµ  //125
#define AirSensor_threshold_person_on1_HIGH 180//240 //¼ì²âÈËÑ¹ÉÏÕíÍ·ãĞÖµ  //180
#define AirSensor_threshold_person_snore_HIGH 150 //ÓĞ÷ıÊ±ÓĞÈËãĞÖµ //125
#define AirSensor_threshold_person1_on_HIGH 180 //220 //180 //¼ì²âÈËÑ¹ÉÏÕíÍ·2Â·Ö®ºÍãĞÖµ
#define AirSensor_threshold_person1_snore_HIGH 220 //180 //ÓĞ÷ıÊ±ÓĞÈË2Â·Ö®ºÍãĞÖµ //120
//Í·²¿Î»ÖÃ¼ì²âÖĞÁéÃô¶È
#define AirSensor_threshold_person_on_MID  160 //¼ì²âÈËÑ¹ÉÏÕíÍ·ãĞÖµ  //125
#define AirSensor_threshold_person_on1_MID 240 //¼ì²âÈËÑ¹ÉÏÕíÍ·ãĞÖµ  //180
#define AirSensor_threshold_person_snore_MID 160 //ÓĞ÷ıÊ±ÓĞÈËãĞÖµ //125
#define AirSensor_threshold_person1_on_MID 240 //180 //¼ì²âÈËÑ¹ÉÏÕíÍ·2Â·Ö®ºÍãĞÖµ
#define AirSensor_threshold_person1_snore_MID 240 //180 //ÓĞ÷ıÊ±ÓĞÈË2Â·Ö®ºÍãĞÖµ //120
//Í·²¿Î»ÖÃ¼ì²âµÍÁéÃô¶È
#define AirSensor_threshold_person_on_LOW  200 //¼ì²âÈËÑ¹ÉÏÕíÍ·ãĞÖµ  //125
#define AirSensor_threshold_person_on1_LOW 300 //¼ì²âÈËÑ¹ÉÏÕíÍ·ãĞÖµ  //180
#define AirSensor_threshold_person_snore_LOW 200 //ÓĞ÷ıÊ±ÓĞÈËãĞÖµ //125
#define AirSensor_threshold_person1_on_LOW 300 //180 //¼ì²âÈËÑ¹ÉÏÕíÍ·2Â·Ö®ºÍãĞÖµ
#define AirSensor_threshold_person1_snore_LOW 300 //180 //ÓĞ÷ıÊ±ÓĞÈË2Â·Ö®ºÍãĞÖµ //120

#else

//MP2²ÎÊı
//Í·²¿Î»ÖÃ¼ì²â¸ßÁéÃô¶È
#define AirSensor_threshold_person_on_HIGH  125 //¼ì²âÈËÑ¹ÉÏÕíÍ·ãĞÖµ  //125
#define AirSensor_threshold_person_on1_HIGH 200 //¼ì²âÈËÑ¹ÉÏÕíÍ·ãĞÖµ  //180
#define AirSensor_threshold_person_snore_HIGH 125 //ÓĞ÷ıÊ±ÓĞÈËãĞÖµ //125
#define AirSensor_threshold_person1_on_HIGH 180 //180 //¼ì²âÈËÑ¹ÉÏÕíÍ·2Â·Ö®ºÍãĞÖµ
#define AirSensor_threshold_person1_snore_HIGH 180 //180 //ÓĞ÷ıÊ±ÓĞÈË2Â·Ö®ºÍãĞÖµ //120
//Í·²¿Î»ÖÃ¼ì²âÖĞÁéÃô¶È
#define AirSensor_threshold_person_on_MID  160 //¼ì²âÈËÑ¹ÉÏÕíÍ·ãĞÖµ  //125
#define AirSensor_threshold_person_on1_MID 240 //¼ì²âÈËÑ¹ÉÏÕíÍ·ãĞÖµ  //180
#define AirSensor_threshold_person_snore_MID 160 //ÓĞ÷ıÊ±ÓĞÈËãĞÖµ //125
#define AirSensor_threshold_person1_on_MID 240 //180 //¼ì²âÈËÑ¹ÉÏÕíÍ·2Â·Ö®ºÍãĞÖµ
#define AirSensor_threshold_person1_snore_MID 240 //180 //ÓĞ÷ıÊ±ÓĞÈË2Â·Ö®ºÍãĞÖµ //120
//Í·²¿Î»ÖÃ¼ì²âµÍÁéÃô¶È
#define AirSensor_threshold_person_on_LOW  200 //¼ì²âÈËÑ¹ÉÏÕíÍ·ãĞÖµ  //125
#define AirSensor_threshold_person_on1_LOW 300 //¼ì²âÈËÑ¹ÉÏÕíÍ·ãĞÖµ  //180
#define AirSensor_threshold_person_snore_LOW 200 //ÓĞ÷ıÊ±ÓĞÈËãĞÖµ //125
#define AirSensor_threshold_person1_on_LOW 300 //180 //¼ì²âÈËÑ¹ÉÏÕíÍ·2Â·Ö®ºÍãĞÖµ
#define AirSensor_threshold_person1_snore_LOW 300 //180 //ÓĞ÷ıÊ±ÓĞÈË2Â·Ö®ºÍãĞÖµ //120

#endif

#define AirSensor_position_slowcheck_cycle 30000 //30000  //µ¥Î»20ms,1·ÖÖÓ
#define AirSensor_position_fastcheck_cycle 1  //µ¥Î»20ms,40ms

#define Airsensor_Max_Diff_airpressure_one_bag 0xaf0 //0x0480
#define Airsensor_Max_Diff_airpressure_two_bag 0xfa0 //0x0680



//ÈËÌå¼ì²â²ÎÊı
#define AirSensor_LED_onoff_time 3 //µ¥Î»ÎªÃë
#define AirSensor_recheck_person_onoff_time 2 //µ¥Î»·ÖÖÓ
#define AirSensor_Pressure_fs 2//10  //ÆøÑ¹ÖµÔ­Ê¼²ÉÑùÆµÂÊ
#define AirSensor_PressureTest_fs  10      //²ÉÑùÆµÂÊ Ã¿Ãë10´Î
#define AirSensor_PressureTest_buffer_length 5*AirSensor_PressureTest_fs   //²ÉÑùÊı¾İ»º³åÇø³¤¶È,±£´æ5ÃëÊ±³¤

#define AirSensor_Avgvalue_Step 10//20 //Çóµ±Ç°ÆøÑ¹Æ½¾ùÖµÓÃn´ÎÂË²¨ºóµÄÖµÇóÆ½¾ù  20
#define AirSensor_Avgvalue_Step1 10 //Çóµ±Ç°ÆøÑ¹Æ½¾ùÖµÓÃn´ÎÂË²¨ºóµÄÖµÇóÆ½¾ù  10

#define AirSensor_PressureTest_filter_step 5 //È¡n´Î²ÉÑùÖµÇóÆ½¾ù
#define AirSensor_Avgvalue_Buffer_Size 10 //20//ÅĞ¶ÏÊÇ·ñÆ½ÎÈµÄBuffer´óĞ¡  //30
#define AirSensor_Avgvalue_Buffer_forMaxMin_Size 200 //ÅĞ¶ÏºóÆÚÌØĞÔµÄBuffer´óĞ¡
#define AirSensor_pressure_person_on_Max_threshold 20 //ÊÓÎªÓĞÈËµÄÆøÑ¹ÉÏÉı·ù¶È
#define MIC_data_for_Walve_debug_length 1*10*8
#define AirSensor_pressure_stable_threshold 3 //Á¬ĞøAirSensor_Avgvalue_Buffer_Size¸öÖµÖĞÊÓÎªÎÈ¶¨µÄ±ä»¯·¶Î§(×î´óºÍ×îĞ¡ÖµÖ®¾ø¶Ô²åÖµ£
#define person_on_threshold 60 //ÏÔÊ¾ºÍÅĞ¶ÏÎªÓĞÈËµÄÉÏÉıÃÅÏŞ
#define person_on_threshold1 50 //Ö¸Ê¾µÈÔ¤ÏÈÖ¸Ê¾ÓĞÈË

#define stable_down_threshold 40 //-40 //ÎÈ¶¨ÖµÖ®¼äÏÂ½µ¶àÉÙ¿ªÊ¼½øĞĞÊÇ·ñÀë¿ªµÄÅĞ±ğ
#define stable_threshold_for_person_remain_on 70


#define MIC_point_repeat_times 1
#define Airbag_To_Pump_PowerOn_Time  910
#define Airbag_To_Pump_PowerOff_Time 210
#define Airbag_To_Air_PowerOn_Time   430
#define Airbag_To_Air_PowerOff_Time  460

/* old version
typedef	struct  //9bytes
{
	unsigned int    flag;       //0x55aa55aa
	uint16_t        days;       //Ä¬ÈÏÎªÖĞ£¬Êµ¼Ê²Ù×÷Îª¸ßµÄÌìÊı
	uint8_t         checkposition;
	uint8_t         timestart;
	uint8_t         timeend;
  
}SensorLevelAutoSet_Def;
*/

typedef	struct  //9bytes
{
	unsigned int    flag;       //0x55aa55aa
	uint8_t         times;       //Ä¬ÈÏÎªÖĞ£¬Êµ¼Ê²Ù×÷Îª¸ßĞ¡Ê±Êı
}SensorLevelAutoSet_Def;

typedef	struct  //7bytes
{
	//unsigned int    flag;       //0x55aa55aa
	//uint8_t         userkeycode1;       //Ä¬ÈÏÎªÖĞ£¬Êµ¼Ê²Ù×÷Îª¸ßĞ¡Ê±Êı
	//uint8_t         userkeycode2;
	//uint8_t         userkeycode3;
	//uint8_t         userkeycode4;
	//uint8_t         userkeytype; //00:È±Ê¡ 01:ÓÃ»§ÉèÖÃ
	unsigned int    validtimeflag;       //0x55aa55aa
	uint8_t         validtime_year;
	uint8_t         validtime_month;
	uint8_t         validtime_day;	
}UserkeycodeAutoSet_Def;


typedef	struct  //?bytes
{
	
	//Î»ÖÃ¼ì²â±äÁ¿
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
	
	
	
	//ÈËÌå¼ì²âÏà¹Ø±äÁ¿
	uint16_t AirSensor_PressureTest[AirSensor_PressureTest_buffer_length]; //²ÉÑùÊı¾İ»º³åÇø
  uint16_t AirSensor_PressureBuffer[AirSensor_Pressure_fs]; //×îÔ­Ê¼µÄ²ÉÑùÊı¾İ£¬½øĞĞÆ½¾ùºó·ÅÈëAirsensor.AirSensor_PressureTest
  uint16_t AirSensor_PressureBuffer_WR_pointer;
  uint16_t AirSensor_PressureTest_WR_pointer;
  uint8_t AirSensor_PersonOn_Last; //0:no person 1:Person on
  uint8_t AirSensor_PersonOn; //0:no person 1:Person on
  uint8_t AirSensor_PersonOn1; //0:no person 1:Person on
  uint8_t AirSensor_PersonOn2; //0:no person 1:Person on
  uint8_t AirSensor_PersonOn3; //0:no person 1:Person on
  uint16_t AirSensor_Maxvalue; //×î´óÆøÑ¹Öµ
  uint16_t AirSensor_Maxup; //ÆøÑ¹ÉÏÉıµÄ×î´ó±ä»¯Öµ£¨2¸ö²ÉÑùµãÖ®¼ä£¬´ú±íÉÏÉıµÄ¶¸¶È)
  uint16_t AirSensor_Minvalue; //×îĞ¡ÆøÑ¹Öµ
  uint16_t AirSensor_Mindiff; //ÆøÑ¹ÏÂ½µµÄ×î´ó±ä»¯Öµ£¨2¸ö²ÉÑùµãÖ®¼ä£¬´ú±íÏÂ½µµÄ¶¸¶È)
  uint16_t AirSensor_Avgvalue; //µ±Ç°ÆøÑ¹Æ½¾ùÖµ
  uint16_t AirSensor_Avgvalue1; //µ±Ç°ÆøÑ¹Æ½¾ùÖµ

  uint8_t AirSensor_BufferRefresh; //µ±Ç°bufferÊı¾İÊÇ·ñµ±Ç°×îĞÂ²¢È«²¿ÓĞĞ§ 0£ºno  1:yes
  uint16_t AirSensor_PressureTest_filtered[AirSensor_PressureTest_buffer_length]; //²ÉÑùÂË²¨ºóµÄÊı¾İ»º³åÇø

  uint16_t AirSensor_Avgvalue_Buffer_Pointer;
  uint16_t AirSensor_Avgvalue_Buffer[AirSensor_Avgvalue_Buffer_Size];


  uint16_t AirSensor_Avgvalue_Buffer_forMaxMin_Pointer;
  uint16_t AirSensor_Avgvalue_Buffer_forMaxMin[AirSensor_Avgvalue_Buffer_forMaxMin_Size];
  uint16_t AirSensor_Avgvalue_Buffer_forMaxMin1[AirSensor_Avgvalue_Buffer_forMaxMin_Size];//¹²ÓÃsizeºÍÖ¸Õ
	uint16_t AirSensor_Sample_Avgvalue_Buffer[AirSensor_Avgvalue_Buffer_forMaxMin_Size];//²ÉÑùÊı¾İµÄ»º³åÇø

  uint8_t Max_or_Min_detected; //×îĞÂ¼ì²âµ½µÄÊÇ¼«´óÖµ»¹ÊÇ¼«Ğ¡Öµ  1£º¼«´óÖµ 0£º¼«Ğ¡Öµ

  uint16_t AirSensor_pressure_sample_avg;
	uint16_t AirSensor_pressure_sample_avg_last;
	uint16_t AirSensor_pressure_sample_avg_diffsum;
  uint16_t AirSensor_pressure_sample;

  uint16_t AirSensor_pressure_stable_value;//Õâ´ÎµÄÎÈ¶¨Öµ
  uint16_t AirSensor_pressure_stable_value_last;//ÉÏÒ»´ÎµÄÎÈ¶¨Öµ

  uint8_t pressure_stable_value_last_is_valid; //ÉÏÒ»´ÎµÄÎÈ¶¨ÖµÊÇ·ñÓĞĞ§±êÖ¾ 1£ºÓĞĞ§  0£ºÎŞĞ§
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
  uint16_t last_diff_MaxMin;//×îºó½×¶Î×î´óÖµ¼õÈ¥×îĞ¡Öµ
  uint8_t min_stepnosition;//×îµÍµãÎ»ÖÃ

  uint8_t AirSensor_stable_min_detected_flag;
  uint8_t Led_Person_OnOff;//0:off 1:on 2:off
  uint8_t AirSensor_step_stable_flag;  //0:²»ÎÈ¶¨£¬1£ºÎÈ¶¨
	uint8_t AirSensor_step_last_stable_flag;  //0:²»ÎÈ¶¨£¬1£ºÎÈ¶¨
  uint8_t AirSensor_stable_noresult_flag;//ĞèÒªÅĞ¶¨ÈËÊÇ·ñÀë¿ª£¬µ«Ä¿Ç°ÎŞ·¨È·¶¨ÅĞ¶Ï£¬Áô´ıºóĞø¼ÌĞøÅĞ¶Ï
  uint16_t Body_Moved_Times; //Ìå¶¯´ÎÊı
  int16_t Body_Pressure_Value; //´óÖÂµÄÈËÌåÑ¹Á¦ÆøÑ¹Öµ,ÀÛ¼ÆÊ£ÓàÖµ£¬Èç¹ûÈËÌåµÄÂı¶¯µ¼ÖÂÉı¸ß»òÕßÎŞ·¨Í³¼ÆÔÚÄÚ£¬Îó²î»á±È½Ï´ó
  uint16_t AirSensor_Pressure_from_noperson_to_Personon;//ÓÉÎŞÈËµ½ÓĞÈËÖ®Ç°µÄÎŞÈËÎÈ¶¨ÆøÑ¹Öµ
  uint16_t Pressure_increase_several_times;//ÔÚÎŞÈË×´Ì¬ÏÂÆøÑ¹Öµ·Ö´ÎÔö¼ÓµÄÇé¿ö£¬Ã¿´Î¿ÉÄÜµÍÓÚ·§Öµ

  uint8_t AirSensor_position_checked_by_fastmode_flag;//Î»ÖÃÊÇÓÉ02 fast mode¼ì²âµÄÎ»ÖÃ
  uint8_t personon_by_positioncheck_flag;//ÓĞÈËÊÇÍ¨¹ıÎ»ÖÃ¼ì²âÅĞ¶Ï³öµÄ±êÖ¾
	uint8_t personon_by_positioncheck_flag_forpensononoff;//ÓÃÓÚÈËÌåÊÇ·ñÔÚ´²Ëã·¨Ñ¡ÔñµÄ±êÖ¾
  uint8_t power_on_set_value_flag;//ÉÏµç½«¶ÁÈ¡µ½avgÖµ¸³ÓèÎÈÌ¬³õÖµ
  uint8_t AirSensor_fast_check_position_finished_flag;
  uint8_t AirSensor_PersonOn_init_snore_flag;
  uint8_t AirSensor_fast_check_mode;
  uint8_t AirSensor_Checkposition_Mode;
	uint8_t AirSensor_Check_Mode;
	
	//ĞÂÔö±êÖ¾
	uint8_t AirSensor_noperson_state_checked_istrue;
	uint8_t AirSensor_vale_exceed_upvalue_istrue; //ÆøÑ¹Öµ³¬Ô½·§Öµ
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


typedef	struct  //7bytes
{
	int SnoreDataItemNumber;
	unsigned char   Year;       
	unsigned char   Month; 
	unsigned char   Day; 
	unsigned char   Hour; 
	unsigned char   Minute; 
	unsigned char   Second; 
	unsigned char   dB_or_POS;
  
}DetailedSnoreData_Def;

extern SensorLevelAutoSet_Def SensorLevelAutoSetData;
extern UserkeycodeAutoSet_Def UserkeycodeAutoSetData;

extern Airsensor_struct_Def Airsensor;
extern Valve_struct_Def Valve_Control;

extern uint8_t PillowMaxOne_Willbesend;
extern uint8_t Pillow2ndMaxOne_Willbesend;
extern uint8_t secondtest;

extern uint8_t Valve_State[4];

extern uint8_t Tt_Is_Sleep_Time;

extern volatile uint32_t Led_OnOff_Timer;

extern uint8_t time_start_ok_flag;
extern uint8_t SensorLevelAutoSetData_timeout_flag;

//extern uint8_t AirSensor_PersonOn;
//extern uint8_t personon_by_positioncheck_flag;
//extern uint8_t Body_Moved_Times;

extern uint8_t MIC_data_stop_detect_flag;
extern void check_sensorlevel_autoset(void);
extern void check_userkeycode_autoset(void);
extern void check_validtime_autoset(void);
extern void Build_SensorLevelAutoSet_Flag(void);
extern void Build_UserkeycodeAutoSet_Flag(void);
extern void Build_validtimeAutoSet_Flag(void);
extern void Delete_SensorLevelAutoSet_Flag(void);
extern void update_sensorlevel_autoset(void);
extern void update_userkeycode_autoset(void);
extern void update_validtime_autoset(void);
extern void Airsensor_SendlogtoPC(void);
extern void claer_validtimeflag(void);

extern void AirSensor_position_process(Airsensor_struct_Def * air);
extern void AirSensor_monitor_Pressure_AvgValue(Airsensor_struct_Def * air);
extern void AirSensor_monitor_Pressure_AvgValue1(Airsensor_struct_Def * air);
//extern void AirSensor_person_detect_init(void);
extern void Switch_Valve_Do_Airbag_To_Air(unsigned char valve);
extern void AirSensor_detect_person_onoff(Airsensor_struct_Def * air);

extern void Switch_Valve_Do_Airbag_To_Pump(unsigned char valve);

extern void Switch_Valve_Do_Airbag_To_Air_Sub(Valve_struct_Def * valve_struct,unsigned char valve);
extern void Switch_Valve_Do_Airbag_To_Pump_Sub(Valve_struct_Def * valve_struct,unsigned char valve);
extern void LOGO_system_error(void);
#endif 

