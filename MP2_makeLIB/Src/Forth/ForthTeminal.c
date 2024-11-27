#include "ForthTeminal.h"
#include <string.h>
#include <stdio.h>
#include "ForthWords.h"
#include "ForthDecode.h"

#ifdef ARM
//#include "lowlevel.h"
//#include "ucos_ii.h"
//#include "usart.h"
#endif

ArrayTypedef ArrayBuffer;

//TableDef Table;

 unsigned int Stack[StackSize],*Sp,HEX=0;

struct __Stack__{
	unsigned int U;
	unsigned int B;
}StackPattern;

char Data[200];

CommandBufferTypedef CommandBuffer;
VarDef Var;

int Ten(int i)
{
	int Total=1;
	while(--i)
	{
		Total*=10;
	}
	return Total;
}

int Sixteen(int i)
{
	
	int Total=1;
	while(--i)
	{
	
	  Total*=16;
	
	
	}
	
	
	return Total;
	

}
int StrToInt(char *p)
{
	int Size=0,data=0,temp;
	Size=strlen(p);
	if(Size==0)
	return 0;
	
	if(*p!='-')
	{
		if(HEX==0)
		{
	
			while(*p)
			{
				data+=((*p)-48)*Ten(Size);
				Size--;
				p++;
			}
			return data;
		}
		else
		{
	
			while(*p)
			{   	switch(*p)
				{
					case '0':
					temp=((*p)-48);
					break;
					case '1':
					temp=((*p)-48);
					break;
					case '2':
					temp=((*p)-48);
					break;
					case '3':
					temp=((*p)-48);
					break;
					case '4':
					temp=((*p)-48);
					break;
					case '5':
					temp=((*p)-48);
					break;
					case '6':
					temp=((*p)-48);
					break;
					case '7':
					temp=((*p)-48);
					break;
					case '8':
					temp=((*p)-48);
					break;
					case '9':
					temp=((*p)-48);
					break;
					case 'a':
					temp=10;
					break;
					case 'b':
					temp=11;
					break;
					case 'c':
					temp=12;
					break;
					case 'd':
					temp=13;
					break;
					case 'e':
					temp=14;
					break;
					case 'f':
					temp=15;
					break;
					default:
					break;
				
				}
				data+=temp*Sixteen(Size);
				Size--;
				p++;
			}
			return data;
	
	
	
		}
	}
	else
	{
	    p++;
	    Size--;
	
		if(HEX==0)
		{
	
			while(*p)
			{
				data+=((*p)-48)*Ten(Size);
				Size--;
				p++;
			}
			
			data=~data+1;
			
			
			return data;
		}
		else
		{
	
			while(*p)
			{   	switch(*p)
				{
					case '0':
					temp=((*p)-48);
					break;
					case '1':
					temp=((*p)-48);
					break;
					case '2':
					temp=((*p)-48);
					break;
					case '3':
					temp=((*p)-48);
					break;
					case '4':
					temp=((*p)-48);
					break;
					case '5':
					temp=((*p)-48);
					break;
					case '6':
					temp=((*p)-48);
					break;
					case '7':
					temp=((*p)-48);
					break;
					case '8':
					temp=((*p)-48);
					break;
					case '9':
					temp=((*p)-48);
					break;
					case 'a':
					temp=10;
					break;
					case 'b':
					temp=11;
					break;
					case 'c':
					temp=12;
					break;
					case 'd':
					temp=13;
					break;
					case 'e':
					temp=14;
					break;
					case 'f':
					temp=15;
					break;
					default:
					break;
				
				}
				data+=temp*Sixteen(Size);
				Size--;
				p++;
			}
			
			data=~data+1;
			return data;
	
	
	
		}
	
	
	
	
	
	}
	
	
	

}

int StrCheck(char *p,char *t)
{
	   while(*p)
	   {
	   		if(*p!=*t)
	   		return 0xff;
	   		else
	   		{
	   		
	   			p++;
	   			t++;
	   		
	   		}
	   
	   }
	   
	   
	   if(*t!=0)
	   return 0xff;
	   else return 0;
}


int DetermindType(char *p)
{
  
	if(*p)
	{
		if((*p=='-'))
		{    p++;
			if(HEX==0)
 	 		{
				if((*p==0)||(*p==' '))
				return CMDType;
				
				
				while(*p)
				{
					if((*p<48)||(*p>57))
					return CMDType;
		
					p++;
		
	
				}
				return NumType;
			}
			else
			{
				if((*p==0)||(*p==' '))
				return CMDType;
				
				
				
				while(*p)
				{
					if((*p<48)||((*p>57)&&(*p<97))||(*p>102))
					return CMDType;
		
					p++;
		
	
				}
				return NumType;
		
		
		
			}
		}
		else
		{
		
			if(HEX==0)
 	 		{
				while(*p)
				{
					if((*p<48)||(*p>57))
					return CMDType;
		
					p++;
		
	
				}
				return NumType;
			}
			else
			{
			
				while(*p)
				{
					if((*p<48)||((*p>57)&&(*p<97))||(*p>102))
					return CMDType;
		
					p++;
		
	
				}
				return NumType;
		
		
		
			}
		
		}
	}
	return NoInput;

}


char RecoderAdd(char *cmd,unsigned int address,char *help)
{
	static int i=0;
	strcpy(Var.VarName[i],cmd);
	Var.Var[i]=(unsigned int )address;
	strcpy(Var.Help[i],help);
	Var.attribute[i]=CommandType;
	i++;
	Var.UsedVar=i;
	Var.Ctotal=i;

	return 1;
}




void AddWords(void)
{   
	RecoderAdd("+",(unsigned int )Add,"( N1 N2 - N3 )");
	RecoderAdd(".s",(unsigned int )ViewStack,"( - )");
	RecoderAdd(".",(unsigned int )Pop,"( - N1)");
	RecoderAdd("words",(unsigned int )Words,"( - )");
	RecoderAdd("cls",(unsigned int )ClearScreen,"( - )");
	RecoderAdd("-",(unsigned int )dec,"( N1 N2 - N3 )");
	RecoderAdd("*",(unsigned int )Mul,"( N1 N2 - N3 )");
	RecoderAdd("/",(unsigned int )Division,"( N1 N2 - N3 )");
	RecoderAdd("clrs",(unsigned int )ClrStack,"(  -  )");
	RecoderAdd("@",(unsigned int )Fetch,"( ADDR - N1 )");	
	RecoderAdd("w@",(unsigned int )WFetch,"( ADDR - N1 )");
	RecoderAdd("c@",(unsigned int )CFetch,"( ADDR - N1 )");
	RecoderAdd("c!",(unsigned int )CStore,"( C ADDR - )");
	RecoderAdd("w!",(unsigned int )WStore,"( w ADDR - )");
	RecoderAdd("!",(unsigned int )Store,"( N1 ADDR - )");
	RecoderAdd("hex",(unsigned int )Hex,"( - )");
	RecoderAdd("decimal",(unsigned int )Decimal,"( - )");
	RecoderAdd("dump",(unsigned int )Dump,"( ADDR LEN - )");
	RecoderAdd("dup",(unsigned int )dup,"( N1 - N1 N1 )");
	RecoderAdd("swap",(unsigned int )Swap,"( N1 N2 - N2 N1 )  ");
	RecoderAdd("over",(unsigned int )Over,"( N1 N2 - N1 N2 N1 )");
	RecoderAdd("drop",(unsigned int )Drop,"(  - )");
	RecoderAdd("rot",(unsigned int )Rot,"( N1 N2 N3 - N2 N3 N1 )");
	RecoderAdd("-rot",(unsigned int )RotRot,"( N1 N2 N3 - N3 N1 N2 )");
	RecoderAdd("2drop",(unsigned int )Drop2,"( N1 N2 - ) ");
	RecoderAdd("2dup",(unsigned int )Dup2,"( N1 N2 - N1 N2 N1 N2) ");
	RecoderAdd("mod",(unsigned int )Mod,"( N1 N2 - N3 ) ");
	RecoderAdd("pad",(unsigned int )Pad,"( - ADDR ) RESERVE 1 K BYTES BUFFER  ");
	RecoderAdd("fill",(unsigned int )Fill,"( ADDR LEN BYTE - )");
	RecoderAdd("erase",(unsigned int )Erase,"( ADDR LEN -  )");
	RecoderAdd("cmove",(unsigned int )Cmove,"( SRC DES BYTES - )");
	RecoderAdd("c+!",(unsigned int )CAddStore,"( n addr-- )");
	RecoderAdd("+!",(unsigned int )AddStore,"( n addr-- )");
	RecoderAdd("=",(unsigned int )Equal,"(N1 N2 - F)");
	RecoderAdd("0<",(unsigned int )LessThanZero,"(N1 - F)");
	RecoderAdd("0>",(unsigned int )MoreThanZero,"(N1 - F)");
	RecoderAdd("invert",(unsigned int )Invert,"(N1 - N2)");
	RecoderAdd("ms",(unsigned int )Ms,"(N1 - )");
	RecoderAdd("u.",(unsigned int )UPop,"(- U)");
	RecoderAdd("cr",(unsigned int )Cr,"(- )");
	RecoderAdd("blank",(unsigned int )Blank,"(- )");
	RecoderAdd("blanks",(unsigned int )BlankN,"(N1- )");
	RecoderAdd(".r",(unsigned int )RPop,"(N DIGIT - )");
	RecoderAdd("key",(unsigned int )Key,"(- ASCII)");
	RecoderAdd("emit",(unsigned int )Emit,"( ASCII-)");
	RecoderAdd("type",(unsigned int )Type,"( ADDR LEN - )");
      //	RecoderAdd("home",(unsigned int )HOME,"(  - )");
	RecoderAdd("constant",(unsigned int )Constant,"(N constant NAME - )");
	RecoderAdd("variable",(unsigned int )Variable,"(variable NAME - )");
	RecoderAdd("equ",(unsigned int )Equ,"(N equ NAME -)");
	RecoderAdd("cell",(unsigned int )Cell,"(- 4)");
	RecoderAdd("cells",(unsigned int )Cells,"(N - N*CELL)");
	RecoderAdd("array",(unsigned int )Array,"10 ARRAY VAR1");
	RecoderAdd("carray",(unsigned int )CArray,"10 CARRAY VAR1");
	RecoderAdd("help",(unsigned int )Help,"help words");
	RecoderAdd("?",(unsigned int )GetValue,"(N1 - )");
	RecoderAdd("forget",(unsigned int )Forget,"( - )");
  RecoderAdd("at",(unsigned int )At,"( N1 N2- )");
  RecoderAdd("0=",(unsigned int )Not,"( N1 - N1)");
  RecoderAdd("bounds",(unsigned int )Bounds,"( N1 N2 - N1+N2 N1)");
	RecoderAdd("log",(unsigned int )LogSwitch,"( N1 - )");
//	char LogoSwitch(void)

	//	WordO_AddWords();
}

void FconsoleInit(void)
{   
	//*VIC1VectAddr2=0;
	//*VIC1IntEnable&=~(1<<25);
	//*VIC1VectCntl2=0;

	Sp=(unsigned int *)&Stack[StackSize];
	 
	AddWords();
	StackPattern.U=(unsigned int )&Stack[StackSize-1];
	StackPattern.B=(unsigned int )Stack;
	/*
	ClrScreen;
	uart_printf("\n\r");
	uart_printf("********************\n\r");
	uart_printf("*      FOC v1.3    *\n\r");
	uart_printf("* author:SuRongBin *\n\r");
	uart_printf("********************\n\r");
   */
	ArrayBuffer.Base=0;
	CommandBuffer.Index=-1;
	CommandBuffer.NowGot=0;	
	ArrayBuffer.Base=0;
}

void FT_Kernel(void)
{	
	char Type,Cr[20],Total,*p,exit=0;
	function Exec;
	unsigned int i,k,temp;
	FconsoleInit();
	

	while(1)
	{
		uart_scanf(Data);
		Type=DetermindType(Data);
		p=(char *)Data;
		Total=0;
		switch(Type)
		{
			case CMDType:
				while(1)
				{   
				    exit=0;
					if((*p!=' ')&&(*p!=0))
					{
						Cr[Total]=*p;
						p++;
						Total++;
					}
					else
					{    
						
						if(*p==0)
						{
							Cr[Total]=0;
							Total=0;
							p++;
							Type=DetermindType(Cr);
							switch(Type)
							{
								case NumType:
								  	  
								 	Sp--;
								 	*Sp=StrToInt(Cr);
								 	Enter;
								  	exit=1;
							     	break;
								case CMDType:
								
								
									for(i=Var.UsedVar;i>0;i--)
								 	{
								 	   		if(strcmp(Var.VarName[i-1],Cr)==0)
								 	
								 				break;
								 	 }
								
								   if(i!=0)
								   {
								 		
								 		switch(Var.attribute[i-1])
								 		{
								 			case CommandType:
								 				Exec=(function)Var.Var[i-1];
								 	    		if(Exec()==1)
								 	    		{
								 	    			Enter;
								 	    
								 	    		}
								 	    		strcpy(CommandBuffer.Buffer[ CommandBuffer.NowGot],Data);
								 	    
								 	    		CommandBuffer.NowGot++;
								 	   
								 	    		if(CommandBuffer.NowGot>BufferCommandTotal)
								 	   		 	CommandBuffer.NowGot=0;
								 	    
								 	    
								 	    		if(CommandBuffer.NowGot!=0)
								 	   	 		{
								 	    
								 	    			CommandBuffer.Index=CommandBuffer.NowGot-1;
								 	    
								 	    		}
								 	    		exit=1;
								 	    		break;
								 	    	case Const:
								 	    		
								 	   			Sp--;
								 				*Sp=Var.Var[i-1];
								 				Enter;
								  				exit=1;
							     				break;
							     			case Vartype:
							     				
								 	   			Sp--;
								 				*Sp=(unsigned int)&Var.Var[i-1];
								 				Enter;
								  				exit=1;
							     				break;
								 	    	case ArrayType:
								 	    		temp=0;
								 	    	    PStack((int *)&temp);
								 	    	    temp=Var.Var[i-1]+CELL*temp;
								 	    	    SStack(temp);
								 	    	    exit=1;
							     				break;
							     			case CArrayType:
								 	    		temp=0;
								 	    	    PStack((int *)&temp);
								 	    	    temp=Var.Var[i-1]+temp;
								 	    	    SStack(temp);
								 	    	    exit=1;
							     				break;
								 	    	default:
								 	    		EnterError
								 	   	 		Sp=	(unsigned int *)StackPattern.U+1;
								 				uart_printf("Input word is undefine !\n\r");
								 				break;
								 		}
								 		
								 	}
								 	else
								 	{
								 	    Sp=(unsigned int *)StackPattern.U+1;
								 	    EnterError;
								 		uart_printf("Input word is undefine !\n\r");
								 		exit=1;
								 	   
								 	}
								 	exit=1;;
									break;
								default: 
								 Enter;
								 exit=1;;
								 break;
								
						
						
						
							}
						
							if(exit)
							break;
						
						
						}
						else
						{
							Cr[Total]=0;
							Total=0;
							p++;
							Type=DetermindType(Cr);
							switch(Type)
							{
								case NumType:
								 Sp--;
								 *Sp=StrToInt(Cr);
							     break;
								case CMDType:
									for(i=Var.UsedVar;i>0;i--)
								 	{
								 	   		if(strcmp(Var.VarName[i-1],Cr)==0)
								 	
								 				break;
								 	 }
								
								   if(i!=0)
								   {
								 		
								 		switch(Var.attribute[i-1])
								 		{
								 			case CommandType:
								 				Exec=(function)Var.Var[i-1];
								 				k=Exec();
								 	    		if(k>=0x80)
								 	    		{
								 	    			p+=k&(~0x80);
								 	    
								 	    		}
								 	    		strcpy(CommandBuffer.Buffer[ CommandBuffer.NowGot],Data);
								 	    
								 	    		CommandBuffer.NowGot++;
								 	   
								 	    		if(CommandBuffer.NowGot>BufferCommandTotal)
								 	   		 	CommandBuffer.NowGot=0;
								 	    
								 	    
								 	    		if(CommandBuffer.NowGot!=0)
								 	   	 		{
								 	    
								 	    			CommandBuffer.Index=CommandBuffer.NowGot-1;
								 	    
								 	    		}
								 	    		
								 	    		break;
								 	    	case Const:
								 	    		
								 	   			Sp--;
								 				*Sp=Var.Var[i-1];
								 				
								  				
							     				break;
							     			case Vartype:
							     				
								 	   			Sp--;
								 				*Sp=(unsigned int)&Var.Var[i-1];
								 				
							     				break;
								 	    	case ArrayType:
								 	    		temp=0;
								 	    	    PStack((int *)&temp);
								 	    	    temp=Var.Var[i-1]+CELL*temp;
								 	    	    SStack(temp);
							     				break;
							     			case CArrayType:
								 	    		temp=0;
								 	    	    PStack((int *)&temp);
								 	    	    temp=Var.Var[i-1]+temp;
								 	    	    SStack(temp);
							     				break;
								 	    	default:
								 	    		EnterError;
								 	   	 		Sp=	(unsigned int *)StackPattern.U+1;
								 				uart_printf("Input word is undefine !\n\r");
								 				break;
								 		}
								 		
								 	}
								 	else
								 	{
								 	    Sp=(unsigned int *)StackPattern.U+1;
								 	    EnterError
								 		uart_printf("Input word is undefine !\n\r");
								 		exit=1;
								 	   
								 	}
								 	
								
								
								
								 break;
								default: 
								 break;
							}
						
							if(exit)
							break;
						}
							
					}

				}
				break;
			case NumType:
				Sp--;
				*Sp=StrToInt(Data);
				Enter;
				break;
			case NoInput:
			    Enter;
				break;
			default:
				break;
		}
		OSTimeDly(10);
	}
	
	//return 1;
}






