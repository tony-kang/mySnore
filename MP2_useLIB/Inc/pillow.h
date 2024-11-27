#include "stdbool.h"
#ifndef HEAD_POS
#define HEAD_POS
#define  STX_HEAD   0XFF
#define  ETX_HEAD   0XAA
#define  BODY_RIGHT     1
#define  BODY_MIDDLE    2
#define  BODY_LEFT      3
#define  BODY_NO        0


#define  HEAD_RIGHT_0  0
#define  HEAD_RIGHT_1  1
#define  HEAD_RIGHT_2  2
#define  HEAD_RIGHT_3  3
#define  HEAD_RIGHT_4  4
#define  HEAD_RIGHT_5  5
#define  HEAD_NO       16

#define HEAD_R_0_VALUE    1
#define HEAD_R_1_VALUE    2
#define HEAD_R_2_VALUE    3
#define HEAD_R_3_VALUE    4
#define BODY_R_VALUE      6
#define BODY_L_VALUE      5
#define EXHALE_VALUE      7



typedef struct{
unsigned char body;
unsigned char head1;
unsigned char head2;
unsigned char sleepOneSide;
unsigned char workon;
unsigned char res[3];
}HeadPosition_def;

typedef struct{
	bool SnoreOn;
	bool InhalePump;
	bool ExhalePump;
	bool PillowAction;
}PillowControl_def;

#endif



