
#ifndef __FORTH_TEMINAL_H_
#define __FORTH_TEMINAL_H_

#include   <stdlib.h>

#define CMDTotal     40
#define StackSize    32
#define CMDType      0
#define NumType      1
#define NoInput      2
#define CommandLen   30
#define PadTotal	 10
#define BufferCommandTotal  10


#define CELL            4

//////////////////constant and var define
#define ConstantAndVarBase 0xf0100000
#define NameLength         20
#define VarSize            100
#define Const              1
#define Vartype            2
#define ArrayType		   3
#define CArrayType         4
#define CommandType        5
#define ArrayBufferSize   1024

/////////////////////////////////////////


typedef struct __CommandBuffer{

	char Buffer[BufferCommandTotal][CMDTotal];
	int  Index;
	int  NowGot;
}CommandBufferTypedef;


typedef struct __VARIABLE__{
	unsigned int Var[VarSize];
	char VarName[VarSize][NameLength];
	unsigned int  UsedVar;
	unsigned char attribute[VarSize];
	unsigned int Size;//array using only
	char Help[VarSize][NameLength+10];
	int Ctotal;
	
}VarDef;

typedef struct __ArrayBuffer__{
	unsigned int Buffer[ArrayBufferSize];
	unsigned int Base;
}ArrayTypedef;


typedef char (*function)(void);
void FconsoleInit(void);
void FT_Kernel(void);

#endif 


