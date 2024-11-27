#include "ForthTeminal.h"
#include <stdio.h>
#include <string.h>
#include "flash.h"
#include "ForthDecode.h"
#include "ForthWords.h"
 #include "stdbool.h"
#ifdef ARM
//#include "ucos_ii.h"
//#include "usart.h"
#endif
bool LOGO_ON=false;
extern unsigned int Stack[StackSize],*Sp,HEX;
extern struct __Stack__{
	unsigned int U;
	unsigned int B;
}StackPattern;
//extern TableDef Table;
extern VarDef Var;
extern struct __Mempd{
char MemPad[PadTotal][1024];
unsigned char PadUsed[PadTotal];
}PadMemry;

extern char  Data[200];
extern ArrayTypedef ArrayBuffer;
extern firmware_parameter_bak_def g_firmwareParameter;
extern unsigned short volume;
//warning:when write data to stack ,please dec the stack at first;
unsigned int PStack(int *data)
{     
	
	if(!((unsigned int)Sp>StackPattern.U))
	{	
		*data=*Sp;
		Sp++;
		
	}
	else
	{ 
		return 0;
	}

return 1;


}


unsigned int SStack(unsigned int Data)
{


	if(!((unsigned int)Sp==StackPattern.B))
	{
		Sp--;
		*Sp=Data;
		return 1;
	}
	else
	{
		return 0;
	
	}



}

char Add(void)
{
	int  No1,No2;
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No2=*Sp;
		Sp++;
	}
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No1=*Sp;
		*Sp=(No1+No2);
	}
	else
	{ 
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
	return 1;

}

char dec(void)
{       int No1,No2;
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No2=*Sp;
		Sp++;
	}
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No1=*Sp;
		*Sp=(No1+No2);
	}
	else 
	{
	     EnterError;
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
	
	*Sp=No1-No2;
	
	return 1	;

}

char Mul(void)
{    int No1;
     int No2;
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No2=*Sp;
		Sp++;
	}
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No1=*Sp;
		*Sp=(No1*No2);
	}
	else
	{ EnterError;
	  uart_printf("no enough parameter in the stack! \n\r");
	
	  return 0;
	}
	
	return 1;
}


char Division(void)
{         int No1;
          int No2;
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No2=*Sp;
		Sp++;
	}
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No1=*Sp;
		*Sp=(No1+No2);
	}
	else 
	{
	    EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
	
	*Sp=No1/No2;

	return 1	;

}


char ViewStack(void)
{
	unsigned int *p;
	char data[50];
	p=(unsigned int *)StackPattern.U;
	
	if((unsigned int)Sp>StackPattern.U)
	{
		EnterError
		uart_printf("stack empty \n\r");
		return 0;
		
	
	}
	
	if(!HEX)
	{	uart_printf("      Stack:  ");
		while(p>=Sp)
		{
			uart_printf("%d  ",*p);
			p--;
		}
	
	}
	else
	{	uart_printf("      Stack:  ");
		while(p>=Sp)
		{
			sprintf(data,"%x",*p);
			uart_printf("%s  ",data);
			p--;
		}
	
	}
	
	return 1;
	
}

char Pop(void)
{   char data[20];
	
	
		if(!((unsigned int )Sp>StackPattern.U))
		{	
			
			if(HEX)
			{
				sprintf(data,"%x",*Sp);
				uart_printf("    %s",data);
				Sp++;
				
				
			}
			else
			{
				sprintf(data,"%d",*Sp);
				uart_printf("    %s",data);
				Sp++;
		  }
		}
		else
		{   EnterError
		    uart_printf("stack underflow\n\r");
	   		return 0;
	
		}
	
	
	
	
	
	return 1;
	
}


char ClrStack(void)
{
	Sp=	(unsigned int *)StackPattern.U+1;
	
	return 1;
	
}


char CmpareString(char *str1,char *str2)   //str2 中是否有包含 str1
{   
    char buf[20];
	
	if(*str2==0)
	return 0;
	
	
	while(*str2)
	{	if(!(strlen(str2)<strlen(str1)))
		{
		memcpy(buf,str2,strlen(str1));	
		buf[strlen(str1)]=0;
		if(strcmp(buf,str1)==0)
		return 1;
		str2++;
		}
		else
		break;
	
	}



return 0;

}




char Words(void)
{   char Buf[20],*p,find=0;
	int i,j,Len,k;
	EnterError
	p=Data;
	i=0;
	while(*p!=0)
	{
		Buf[i++]=*p++;  //
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i]=0;
		
		if(strcmp(Buf,"words")==0)
		{   if(*p++==' ')
			{	i=0;
				while((*p!=0)&&(*p!=' '))
				{
			
					Buf[i++]=*p++;  //
			
			
				}
				Buf[i]=0;
				break;
		
			}
			else 
			{    
					Buf[0]=0;
					break;
			
			}
		
		}
		else
		{
			
			p-=4;
		
		}
		
	
	}
	
	if(Buf[0]==0)
	{
	
		for(i=0;i<Var.UsedVar;i+=4)
		{
	 	
	 		for(j=0;j<4;j++)
	 		{
	 			if(i>=Var.UsedVar)
	 			{
	 				i=Var.UsedVar;
	 				break;
	 	
	 	
	 			}	
		    	Len=strlen(Var.VarName[i+j]);
		    
				uart_printf("    %s",Var.VarName[i+j]);
				k=10-Len;
				if(k<0)
					k=0;
				while(k--)
				{
			
					uart_printf(" ");
			
				}
			
			}
		
		
			EnterError
		
	
		}
	}
	else
	{
		
		j=0;
		for(i=0;i<Var.UsedVar;i++)
		{
	 			if(i>=Var.UsedVar)
	 			{
	 				i=Var.UsedVar;
	 				break;
	 			}	
	 			if(CmpareString(Buf,Var.VarName[i]))
	 			{
		    		Len=strlen(Var.VarName[i]);
					uart_printf("    %s",Var.VarName[i]);
					k=10-Len;
					while(k--)
					uart_printf(" ");
					j++;
					if(j==4)
					{
						EnterError
						j=0;
					}
					
					
					find=1;
				}
				
			
				
				
				
				
				
		}
		
		
		
		if(find==0)
		{
		
		
			
				
					
				EnterError
				uart_printf("cannot find this constant.");
				
				
				
		
		}
	
	
	
	
	}
	
	return 0x80|(strlen(Buf));

}

char ClearScreen(void)
{
	ClrScreen
	
    return 1;

}

char Store(void)
{  
  unsigned int *p32,No1,No2;
 
 
  
  
  if(!((unsigned int)Sp>StackPattern.U))
	{
		No2=*Sp;
		Sp++;
	}
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No1=*Sp;
		Sp++;
		
	}
	else 
	{
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
  	
 
  			p32=(unsigned int *)No2;
  			*p32=No1;
  			
  	
	

return 1;

}




char CStore(void)
{  
  unsigned int No1,No2;

  unsigned char *p8;
  
  
 
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No2=*Sp;
		Sp++;
		
	}
	else 
	{
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
  	if(!((unsigned int)Sp>StackPattern.U))
	{
		No1=*Sp;
		Sp++;
	}
	else 
	{	EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
  
 
  			p8=(unsigned char  *)No2;
  			*p8=No1;
  		
  
  

	

return 1;

}




char WStore(void)
{  
  unsigned int No1,No2;
  unsigned short *p16;

  
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No2=*Sp;
		Sp++;
		
	}
	else 
	{
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
  	if(!((unsigned int)Sp>StackPattern.U))
	{
		No1=*Sp;
		Sp++;
	}
	else 
	{	EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
  
  			p16=(unsigned short *)No2;
  			*p16=No1;
  			

return 1;

}

















int Fetch(void)
{
		
  unsigned int *p32,No1;
 
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No1=*Sp;
		
	}
	else 
	{
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
  
  
 
  			p32=(unsigned int *)No1;
  			*Sp=*p32;
  			
	
			
			return 1;


}
int CFetch(void)
{
		
  unsigned int No1;
   unsigned char *p8;
  
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No1=*Sp;
		
	}
	else 
	{
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
  
  
 
  			p8=(unsigned char *)No1;
  			*Sp=*p8;
  			
	
			
			return 1;


}
int WFetch(void)
{
		
  unsigned int No1;
  unsigned short *p16;
 
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No1=*Sp;
		
	}
	else 
	{
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
  
  
 
  			p16=(unsigned short *)No1;
  			*Sp=*p16;
  			
	
			
			return 1;


}


char  Hex(void)
{
	HEX=1;
	
    return 1;

}

char  Decimal(void)
{
	HEX=0;
	
	return 1;
}


char Dump(void)
{
	unsigned char *addr,t;
	char buf[10];
	unsigned int len,j,k,l; //i,
	t=HEX;
	HEX=1;
	if(!((unsigned int)Sp>StackPattern.U))
	{
		len=*Sp;
		Sp++;
		
	}
	else 
	{   HEX=t;
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		addr=(unsigned char *)*Sp;
		Sp++;
		
	}
	else 
	{	 HEX=t;
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
  			uart_printf("\n\r");
  			//i=0;
  			for(k=0;k<len;k+=16)
  			{	   sprintf(buf,"%x",(unsigned int )addr);
  					switch(strlen(buf))
  					{
  						case 0:
  						uart_printf("%x|",(unsigned int)addr);
  						break;
  						case 1:
  						uart_printf("0000000%x|",(unsigned int)addr);
  						break;
  						case 2:
  						uart_printf("000000%x|",(unsigned int)addr);
  						break;
  						case 3:
  						uart_printf("00000%x|",(unsigned int)addr);
  						break;
  						case 4:
  						uart_printf("0000%x|",(unsigned int)addr);
  						break;
  						case 5:
  						uart_printf("000%x|",(unsigned int)addr);
  						break;
  						case 6:
  						uart_printf("00%x|",(unsigned int)addr);
  						break;
  						case 7:
  						uart_printf("0%x|",(unsigned int)addr);
  						break;
  						case 8:
  						uart_printf("%x|",(unsigned int)addr);
  						break;
  					
  					}
  				
  				   for(l=0;l<16;l++)
  				   {	
  				   		if(k+l<len)
  				   		{	if(*addr>0x0f)
  				   			{
  				   				 sprintf(buf,"%x",(unsigned int )*addr);
  				   				 uart_printf(" %s",buf);
  				   			}
  				   			else
  				   			{ sprintf(buf,"%x",(unsigned int )*addr);
  				   			 uart_printf(" 0%s",buf);
  				   			}
  					 		addr++;
  					 	}
  					 	else
  					 	{
  					 	
  					 		uart_printf("   ");
  					 		addr++;
  					 	
  					 	}
  				   }
  					uart_printf("|");
  					addr-=16;
  					for(j=0;j<16;j++)
  					{
  							if(k+j<len)
  							{
  								if((*addr<32)||(*addr>127))
  								{
  									uart_printf(".");
  						
  								}
  								else
  								uart_printf("%c",*addr);
  								addr++;
  							}
  							else
  							{
  							
  									uart_printf(" ");
  									addr++;
  							
  							}
  					}
  					uart_printf("|");
  					EnterError
  					//i=0;
  			
  			}

			 HEX=t;
			return 1;
}


char dup(void)
{  int No1;
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No1=*Sp;
		
	}
	else 
	{
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
	
	
	Sp--;
	*Sp=No1;
	
return 1;
}

char Swap(void)
{	int No1,No2;
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No2=*Sp;
		Sp++;
		
	}
	else 
	{
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
  	if(!((unsigned int)Sp>StackPattern.U))
	{
		No1=*Sp;
		
	}
	else 
	{	EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
	
	
	*Sp=No2;
	Sp--;
	*Sp=No1;
	
return 1;
}
/*
char Over(void)
{
	int No1,No2;
	
	if(!((unsigned int)Sp>StackPattern.U))
	{
		No2=*Sp;
		Sp++;
		
	}
	else 
	{
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
  	if(!((unsigned int)Sp>StackPattern.U))
	{
		No1=*Sp;
		
	}
	else 
	{	EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	}
	
	
	*Sp=No1;
	Sp--;
	*Sp=No2;
	Sp--;
	*Sp=No1;
	
	
return 1;
	
}
*/

char Over(void)
{
	int No1,No2;
	if(PStack(&No2)==0)
	
	{
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	
	}
	
	if(PStack(&No1)==0)
	
	{
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		return 0;
	
	}

    SStack(No1);
    SStack(No2);
    SStack(No1);
  
    return 1;
}

char Drop(void)
{
	int No1;

	if(PStack(&No1)==0)
	
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	
	}
	
	return 1;


}



char Rot(void)
{
	int No1,No2,No3;
	
	if(PStack(&No3)==0)
	
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	
	}
	
	if(PStack(&No2)==0)
	
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	
	}
	
	if(PStack(&No1)==0)
	
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	
	}
	
	
	
	if(SStack(No2))
	{
		if(SStack(No3))
		{
		
			if(SStack(No1))
			{  
				return 1;
			
			}
			else
			{
			  EnterError;
			  uart_printf("Stack is full! \n\r");
			 
			}
		
		
		}
		else
		{
			  EnterError;
			  uart_printf("Stack is full! \n\r");
			 
			
		}
	
	
	
	}
	else
	{
			  EnterError;
			  uart_printf("Stack is full! \n\r");
			
	}
	
	
	
return 0;

}

char RotRot(void)
{

	if(Rot())
	{
		if(Rot())
		{
		
			return 1;
		
		}
		else
		{
			EnterError
			uart_printf("no enough parameter in the stack! \n\r");
			//return 0;
		
		}
	
	}
	else
	{
	
		EnterError
		uart_printf("no enough parameter in the stack! \n\r");
		//return 0;
	
	}
	
	return 0;

}


char Drop2(void)
{
	
	if(Drop())
	{
		if(Drop())
		{
			
			return 1;
		
		}
		else
		{
			EnterError
			uart_printf("Stack underflow! \n\r");
			//return 0;
		
		}
	
	}
	else
	{
	
		EnterError
		uart_printf("Stack underflow! \n\r");
		//return 0;
	
	}
	
	return 0;

}

char Dup2(void)
{
   int r1,r2;
   
   if(PStack(&r2))
   {
   		if(PStack(&r1))
   		{
   				if(SStack(r1))
   				{
   					if(SStack(r2))
   					{
   						
   						if(SStack(r1))
   						{
   							if(SStack(r2))
   							{
   					
   							
   								return 1;
   							}
   							else
   							{
   				
   								EnterError
								uart_printf("Stack is full ! \n\r");
								return 0;
   				
   							}
   				
   						}
   						else
   						{
   				
   							EnterError
							uart_printf("Stack is full ! \n\r");
							return 0;
   				
   						}
   		
   					
   					}
   					else
   					{
   				
   					EnterError
					uart_printf("Stack is full ! \n\r");
					return 0;
   				
   					}
   				
   				}
   				else
   				{
   				
   					EnterError
					uart_printf("Stack is full ! \n\r");
					return 0;
   				
   				}
   		
   		}
   		else
		{
			EnterError
			uart_printf("no enough parameter in the stack! \n\r");
			return 0;
		
		}
   
   }
   else
	{
			EnterError
			uart_printf("no enough parameter in the stack! \n\r");
			return 0;
		
	}
   
	


}

char Mod(void)
{  int r1,r2;
	
 if(PStack(&r2))
   {
   		if(PStack(&r1))
   		{
   		
   			if(SStack(r1%r2))
   			{
   			
   				
   				return 1;
   			
   			}
   		
   		
   		}
   		else
   		{
   			EnterError
			uart_printf("no enough parameter in the stack! \n\r");
			return 0;
   		}
   	}
   	else
   	{
   	
   			EnterError
			uart_printf("no enough parameter in the stack! \n\r");
			return 0;
   		
   	
   	
   	}
   		

return 0;
}

char Pad(void)
{
        
		if( SStack((unsigned int)&ArrayBuffer.Buffer[ArrayBuffer.Base]))
			{
				
				return 1;
			}
			else
			{
				EnterError
				uart_printf("Stack is full ! \n\r");
				//return 0;
			}
		
		

   EnterError
   uart_printf("no enough memory to allot! \n\r");
   return 0;


}

char Fill(void)
{

	int r1,r2,r3,i;
	char *p;
	if(PStack(&r3)==0)
	
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	
	}
	
	
	if(PStack(&r2)==0)
	
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	
	}
	
	if(PStack(&r1)==0)
	
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	
	}
	
	p=( char *)r1;
	for(i=0;i<r2;i++)
	{
	
	
	  *p++=r3;
	
	}

 
   return 1;

}

char Erase(void)
{
	
	int r1,r2,i;
	char *p;
	if(PStack(&r2)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	p=( char *)r1;
	for(i=0;i<r2;i++)
	{
	  *p++=0;
	}

   return 1;
}

char Cmove(void)
{
    int r1,r2,r3,i;
    unsigned char *s,*d;
    
   
	
	if(PStack(&r3)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	
	if(PStack(&r2)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	
	
	s=(unsigned char *) r1;
    d=(unsigned char *) r2;
	for(i=0;i<r3;i++)
	{
	
		*d++=*s++;
	
	
	}
	

	return 1;

}

char CAddStore(void)
{
	int r1,r2;
	unsigned char *s;
	if(PStack(&r2)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	
	s=(unsigned char *)r2;
	
	if(SStack(*s))
	{			SStack(r1);
				Add();
				PStack(&r2);
				*s=r2;
			
				//return 1;
	}
	else
	{
				EnterError
				uart_printf("Stack is full ! \n\r");
				return 0;
	}
	
	
	return 1;	
}

char AddStore(void)
{
	int r1,r2;
	unsigned int *s;
	if(PStack(&r2)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	
	s=(unsigned int *)r2;
	
	if(SStack(*s))
	{			SStack(r1);
				Add();
				PStack(&r2);
				*s=r2;
				
				//return 1;
	}
	else
	{
				EnterError
				uart_printf("Stack is full ! \n\r");
				return 0;
	}
	
	
	return 1;	
}


char Equal(void)
{

	int r1,r2;
	
	if(PStack(&r2)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}

	if(r1==r2)
	{	
		r1=-1;
		if(SStack(r1))
		{			
					
					return 1;
		}
		else
		{
					EnterError
					uart_printf("Stack is full ! \n\r");
					return 0;
		}
	
		
	}
	else 
	{
		r1=0;
		if(SStack(r1))
		{			
					
					return 1;
		}
		else
		{
					EnterError
					uart_printf("Stack is full ! \n\r");
					return 0;
		}
	   //return 0;
	 }

}

char LessThanZero(void)
{
	int r1;
	
	
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}

	if(r1<0)
	{	
		r1=-1;
		if(SStack(r1))
		{			
					
					return 1;
		}
		else
		{
					EnterError
					uart_printf("Stack is full ! \n\r");
					return 0;
		}
	
		
	}
	else 
	{
		r1=0;
		if(SStack(r1))
		{			
					
					return 1;
		}
		else
		{
					EnterError
					uart_printf("Stack is full ! \n\r");
					return 0;
		}
	   //return 0;
	 }




}

char MoreThanZero(void)
{
	int r1;
	
	
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}

	if(r1>0)
	{	
		r1=-1;
		if(SStack(r1))
		{			
					
					return 1;
		}
		else
		{
					EnterError
					uart_printf("Stack is full ! \n\r");
					return 0;
		}
	
		
	}
	else 
	{
		r1=0;
		if(SStack(r1))
		{			
				
					return 1;
		}
		else
		{
					EnterError
					uart_printf("Stack is full ! \n\r");
					return 0;
		}
	   //return 0;
	 }




}


char Invert(void)
{
	int r1;
	
	
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}

	
		if(r1==0)
		{
		
			r1=-1;
		
		}
		else r1=~r1;
		if(SStack(r1))
		{			
					
					return 1;
		}
		else
		{
					EnterError
					uart_printf("Stack is full ! \n\r");
					return 0;
		}
	

}


char Ms(void)
{  int r1;
		if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	
	if(r1==0)
	{
	
	
	
	}else
	{
	
		 OSTimeDly(r1);
	
	}
	
	
	return 1;


}

char UPop(void)
{
    char data[20];
	unsigned int r1;
	if(PStack((int *)&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}


		
			sprintf(data,"%x",r1);
			uart_printf("    %s\n\r",data);
return 1;
}

char Cr(void)
{
	EnterError
	
	return 1;

}

char Blank(void)
{

	uart_printf("%c",32);

	return 1;
}
char BlankN(void)
{
	int i,r1;
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	
  for(i=0;i<r1;i++)
  Blank();

return 1;
}

char RPop(void)
{
    char a[10];
	
	int r1,r2,i;
	
	if(PStack(&r2)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}

	if(HEX)
	{
		sprintf(a,"%x",r1);
		
	
	}
	else
	{
		sprintf(a,"%d",r1);
	
	
	}

	i=strlen(a);
	
	if(i<r2)
	{
	
		r2=r2-i;
	
	}
	else r2=1;
	
	
        if(SStack(r2))
		{			
					
						BlankN();
						uart_printf("%d",r1);
						return 1;
						
		}
		else
		{
					EnterError
					uart_printf("Stack is full ! \n\r");
					return 0;
		}
	




}

char Key(void)
{  int data;
  	while(1)
  	{
	 	data=uart_getc();
	 	if(data!=0)
	 	break;
	 	OSTimeDly(1);	
	}
	if(SStack(data))
		{			
					
						return 1;
						
		}
		else
		{
					EnterError
					uart_printf("Stack is full ! \n\r");
					return 0;
		}
	

}

char Emit(void)
{
	int r1;
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	
	uart_printf("%c",r1);
	return 1;

}

char Type(void)
{   int r1,r2,i;
    char *p;

	if(PStack(&r2)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	p=(char *)r1;

	for(i=0;i<r2;i++)
	{
		if((*p<32)||(*p>127))
  		{
  			uart_printf(".");
  						
  		}
  		else
  		uart_printf("%c",*p);
	
	
		
		p++;
	}
	
	return 1;
		
}

char HOME(void)
{

	Home 
	return 1;



}


unsigned int  CheckAllTheWord(char *p)
{
	int i;
	
	for(i=Var.UsedVar;i>0;i--)
	{
		if(strcmp(p,Var.VarName[i-1])==0)
		{
			if(Var.attribute[i-1]==CommandType)
			return (0x20000000|(i-1));	
			return (0x10000000|(i-1));	
		}
		
	
	}
	
	
	return 0;
}

char Constant(void)
{
	char Buf[20],*p,i=0,len;
	
	int temp,r1;
	
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	
	
	
	p=Data;
	
	
	
	while(*p!=0)
	{   i=0;
		Buf[i++]=*p++;  //
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i]=0;
		
		if(strcmp(Buf,"constant")==0)
		{   if(*p++==' ')
			{	i=0;
				while((*p!=0)&&(*p!=' '))
				{
			
					Buf[i++]=*p++;  //
			
			
				}
				Buf[i]=0;
				break;
		
			}
			else 
			{    
					EnterError
					uart_printf("redefine error,please input the constant name! \n\r");
					return 0;
					
			
			
			}
		
		}
		else
		{
			
			p-=7;
		
		}
		
	}
	
	temp=CheckAllTheWord(Buf);
	//if(temp==0)
	//{
		if(Var.UsedVar<VarSize)
		{
			Var.Var[Var.UsedVar]=r1;
			strcpy(Var.VarName[Var.UsedVar],Buf);
			Var.attribute[Var.UsedVar]=Const;
			if(temp!=0)
			uart_printf("\n%s is redefined !",Var.VarName[Var.UsedVar]);
			len=strlen(Var.VarName[Var.UsedVar]);
			Var.UsedVar++;
			return 0x80|len;
		}
		else
		{
			EnterError
			uart_printf("No enough space to store constant ! \n\r");
			return 0;
		}
	/*
	}
	else
	{
	
		if(temp&0x10000000)   //Var
		{
			//temp&=~0x10000000;
			//Var.Var[temp]=r1;
			//strcpy(Var.VarName[temp],Buf);
			//Var.attribute[temp]=Const;
			uart_printf("\n%s is redefined !",Var.VarName[temp]);
			//len=strlen(Var.VarName[temp]);//
			
			Var.Var[Var.UsedVar]=r1;
			strcpy(Var.VarName[Var.UsedVar],Buf);
			Var.attribute[Var.UsedVar]=Const;
			
			//uart_printf("\n%s is redefined !",Var.VarName[Var.UsedVar]);
			len=strlen(Var.VarName[Var.UsedVar]);
			Var.UsedVar++;
			
			
			return 0x80|len;
			
		
		}
		else                //command
		{	temp&=~0x20000000;
			Table.Address[temp]=Table.Address[Table.Used-1];
			strcpy(Table.CMD[temp],Table.CMD[Table.Used-1]);
		    Table.Used--;
		    
		    
		    Var.Var[Var.UsedVar]=r1;
			strcpy(Var.VarName[Var.UsedVar],Buf);
			Var.attribute[Var.UsedVar]=Const;
			EnterError
			uart_printf("\n%s is redefined !",Var.VarName[Var.UsedVar]);
			len=strlen(Var.VarName[Var.UsedVar]);
			Var.UsedVar++;
			return 0x80|len;
		
		
		}
	
	
	
	
	}
	
	*/
	
	
	
}


char Variable(void)
{
	char Buf[20],*p,i=0,len;
	
	int temp,r1;
	
	
	
	p=Data;
	
	
	
	while(*p!=0)
	{	 i=0;
		Buf[i++]=*p++;  //
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i]=0;
		
		if(strcmp(Buf,"variable")==0)
		{   if(*p++==' ')
			{	i=0;
				while((*p!=0)&&(*p!=' '))
				{
			
					Buf[i++]=*p++;  //
			
			
				}
				Buf[i]=0;
				break;
		
			}
			else 
			{    
					EnterError
					uart_printf("redefine error,please input the variable name! \n\r");
					return 0;
					
				
			
			
			}
		
		}
		else
		{
			
			p-=7;
		
		}
		
	}
	
	temp=CheckAllTheWord(Buf);
	//if(temp==0)
	//{
		if(Var.UsedVar<VarSize)
		{
			Var.Var[Var.UsedVar]=r1;
			strcpy(Var.VarName[Var.UsedVar],Buf);
			Var.attribute[Var.UsedVar]=Vartype;
			if(temp)
			uart_printf("\n\r%s is redefined !",Var.VarName[Var.UsedVar]);
			
			len=strlen(Var.VarName[Var.UsedVar]);
			Var.UsedVar++;
			return 0x80|len;
		}
		else
		{
			EnterError
			uart_printf("No enough space to store variable ! \n\r");
			return 0;
		}
	
	//}
	/*
	else
	{
	
		if(temp&0x10000000)   //Var
		{
			temp&=~0x10000000;
			Var.Var[temp]=r1;
			strcpy(Var.VarName[temp],Buf);
			Var.attribute[temp]=Vartype;
			uart_printf("\n%s is redefined !",Var.VarName[temp]);
			len=strlen(Var.VarName[temp]);
			return 0x80|len;
		}
		else                //command
		{	temp&=~0x20000000;
			Table.Address[temp]=Table.Address[Table.Used-1];
			strcpy(Table.CMD[temp],Table.CMD[Table.Used-1]);
		    Table.Used--;
		    
		    
		    Var.Var[Var.UsedVar]=r1;
			strcpy(Var.VarName[Var.UsedVar],Buf);
			Var.attribute[Var.UsedVar]=Vartype;
			EnterError
			uart_printf("\n%s is redefined !",Var.VarName[Var.UsedVar]);
			len=strlen(Var.VarName[Var.UsedVar]);
			Var.UsedVar++;
			return 0x80|len;
		
		}
	
	}*/
	
	
}

char Equ(void)
{
	char Buf[20],*p,i=0,len;
	
	int temp,r1;
	
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	
	
	
	p=Data;
	
	
	
	while(*p!=0)
	{   i=0;
		Buf[i++]=*p++;  //
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		
		Buf[i]=0;
		
		if(strcmp(Buf,"equ")==0)
		{   if(*p++==' ')
			{	i=0;
				while((*p!=0)&&(*p!=' '))
				{
			
					Buf[i++]=*p++;  //
			
			
				}
				Buf[i]=0;
				break;
		
			}
			else 
			{    
					EnterError
					uart_printf("redefine error,please input the constant name! \n\r");
					return 0;
					
			
			
			}
		
		}
		else
		{
			
			p-=2;
		
		}
		
	}
	
	temp=CheckAllTheWord(Buf);
//	if(temp==0)
	//{
		if(Var.UsedVar<VarSize)
		{
			Var.Var[Var.UsedVar]=r1;
			strcpy(Var.VarName[Var.UsedVar],Buf);
			Var.attribute[Var.UsedVar]=Const;
			if(temp)
			uart_printf("\n\r%s is redefined !",Var.VarName[Var.UsedVar]);
			
			len=strlen(Var.VarName[Var.UsedVar]);
			Var.UsedVar++;
			return 0x80|len;
		}
		else
		{
			EnterError
			uart_printf("No enough space to store constant ! \n\r");
			return 0;
		}
	
	//}
	/*
	else
	{
	
		if(temp&0x10000000)   //Var
		{
			temp&=~0x10000000;
			Var.Var[temp]=r1;
			strcpy(Var.VarName[temp],Buf);
			Var.attribute[temp]=Const;
			uart_printf("\n%s is redefined !",Var.VarName[temp]);
			len=strlen(Var.VarName[temp]);
			return 0x80|len;
			
		
		}
		else                //command
		{	temp&=~0x20000000;
			Table.Address[temp]=Table.Address[Table.Used-1];
			strcpy(Table.CMD[temp],Table.CMD[Table.Used-1]);
		    Table.Used--;
		    
		    
		    Var.Var[Var.UsedVar]=r1;
			strcpy(Var.VarName[Var.UsedVar],Buf);
			Var.attribute[Var.UsedVar]=Const;
			EnterError
			uart_printf("\n%s is redefined !",Var.VarName[Var.UsedVar]);
			len=strlen(Var.VarName[Var.UsedVar]);
			Var.UsedVar++;
			return 0x80|len;
		
		
		}
	
	
	
	
	}*/
	
	
	
	
	
}


char Cell(void)
{
	if(SStack(CELL))
		{			
					
						return 1;
						
		}
		else
		{
					EnterError
					uart_printf("Stack is full ! \n\r");
					return 0;
		}
	
	

}

char Cells(void)
{
  int r1;

	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	

	if(SStack(CELL*r1))
		{			
					
						return 1;
						
		}
		else
		{
					EnterError
					uart_printf("Stack is full ! \n\r");
					return 0;
		}
	



}
char Array(void)
{

	char Buf[20],*p,i=0,len;
	
	int temp,r1;
	
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
	
	
	p=Data;
	
	
	
	while(*p!=0)
	{	 i=0;
		Buf[i++]=*p++;  //
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		
		Buf[i]=0;
		
		if(strcmp(Buf,"array")==0)
		{   if(*p++==' ')
			{	i=0;
				while((*p!=0)&&(*p!=' '))
				{
			
					Buf[i++]=*p++;  //
			
			
				}
				Buf[i]=0;
				break;
		
			}
			else 
			{    
					EnterError
					uart_printf("redefine error,please input the variable name! \n\r");
					return 0;
					
				
			
			
			}
		
		}
		else
		{
			
			p-=4;
		
		}
		
	}
	
	temp=CheckAllTheWord(Buf);
	
	if((ArrayBuffer.Base+r1)>ArrayBufferSize )
	{
			
				EnterError
				uart_printf("No enough space to store Array ! \n\r");
				return 0;
			
			
	}	
	//if(temp==0)
	//{
		if(Var.UsedVar<VarSize)
		{	
			Var.Size=r1;
			Var.Var[Var.UsedVar]=(unsigned int)&ArrayBuffer.Buffer[ArrayBuffer.Base];
			strcpy(Var.VarName[Var.UsedVar],Buf);
			Var.attribute[Var.UsedVar]=ArrayType;
			ArrayBuffer.Base+=r1;
			if(temp)
			uart_printf("\n\r%s is redefined !",Var.VarName[Var.UsedVar]);
			len=strlen(Var.VarName[Var.UsedVar]);
			Var.UsedVar++;
			return 0x80|len;
		}
		else
		{
			EnterError
			uart_printf("No enough space to store variable ! \n\r");
			return 0;
		}
	
	//}
	/*
	else
	{
	
		if(temp&0x10000000)   //Var
		{	temp&=~0x10000000;
			if((Var.attribute[temp]==ArrayType)&&(Var.Size>=r1))
			{
				strcpy(Var.VarName[temp],Buf);
				uart_printf("\n%s is redefined !",Var.VarName[temp]);
				len=strlen(Var.VarName[temp]);
				return 0x80|len;
			}
			Var.Size=r1;
			Var.Var[temp]=(unsigned int)&ArrayBuffer.Buffer[ArrayBuffer.Base];
			strcpy(Var.VarName[temp],Buf);
			Var.attribute[temp]=ArrayType;
			ArrayBuffer.Base+=r1;
			uart_printf("\n%s is redefined !",Var.VarName[temp]);
			len=strlen(Var.VarName[temp]);
			Var.UsedVar++;
			return 0x80|len;
		}
		else                //command
		{	temp&=~0x20000000;
			Table.Address[temp]=Table.Address[Table.Used-1];
			strcpy(Table.CMD[temp],Table.CMD[Table.Used-1]);
		    Table.Used--;
		    
		    
		    Var.Size=r1;
			Var.Var[Var.UsedVar]=(unsigned int)&ArrayBuffer.Buffer[ArrayBuffer.Base];
			strcpy(Var.VarName[Var.UsedVar],Buf);
			Var.attribute[Var.UsedVar]=ArrayType;
			EnterError
			ArrayBuffer.Base+=r1;
			uart_printf("\n%s is redefined !",Var.VarName[Var.UsedVar]);
			len=strlen(Var.VarName[Var.UsedVar]);
			Var.UsedVar++;
			return 0x80|len;
		
		}
	
	}
	

*/



}



char CArray(void)
{

	char Buf[20],*p,i=0,len;
	
	int temp,r1;

	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}

	
	p=Data;
	
	
	
	while(*p!=0)
	{	 i=0;
		Buf[i++]=*p++;  //
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		
		Buf[i]=0;
		
		if(strcmp(Buf,"carray")==0)
		{   if(*p++==' ')
			{	i=0;
				while((*p!=0)&&(*p!=' '))
				{
			
					Buf[i++]=*p++;  //
			
			
				}
				Buf[i]=0;
				break;
		
			}
			else 
			{    
					EnterError
					uart_printf("redefine error,please input the variable name! \n\r");
					return 0;
					
				
			
			
			}
		
		}
		else
		{
			
			p-=5;
		
		}
		
	}
	
	temp=CheckAllTheWord(Buf);
	
	r1=r1/4+1;
	
	
	
	if((ArrayBuffer.Base+r1)>ArrayBufferSize )
	{
			
				EnterError
				uart_printf("No enough space to store Array ! \n\r");
				return 0;
			
			
	}	
	//if(temp==0)
	//{
		if(Var.UsedVar<VarSize)
		{	
			Var.Size=r1;
			Var.Var[Var.UsedVar]=(unsigned int)&ArrayBuffer.Buffer[ArrayBuffer.Base];
			strcpy(Var.VarName[Var.UsedVar],Buf);
			Var.attribute[Var.UsedVar]=CArrayType;
			ArrayBuffer.Base+=r1;
			if(temp)
			uart_printf("\n\r%s is redefined !",Var.VarName[Var.UsedVar]);
			len=strlen(Var.VarName[Var.UsedVar]);
			Var.UsedVar++;
			return 0x80|len;
		}
		else
		{
			EnterError
			uart_printf("No enough space to store variable ! \n\r");
			return 0;
		}
	
	//}
	/*
	else
	{
	
		if(temp&0x10000000)   //Var
		{	temp&=~0x10000000;
			if((Var.attribute[temp]==ArrayType)&&(Var.Size>=r1))
			{
				strcpy(Var.VarName[temp],Buf);
				uart_printf("\n%s is redefined !",Var.VarName[temp]);
				len=strlen(Var.VarName[temp]);
				return 0x80|len;
			}
			Var.Size=r1;
			Var.Var[temp]=(unsigned int)&ArrayBuffer.Buffer[ArrayBuffer.Base];
			strcpy(Var.VarName[temp],Buf);
			Var.attribute[temp]=ArrayType;
			ArrayBuffer.Base+=r1;
			uart_printf("\n%s is redefined !",Var.VarName[temp]);
			len=strlen(Var.VarName[temp]);
			Var.UsedVar++;
			return 0x80|len;
		}
		else                //command
		{	temp&=~0x20000000;
			Table.Address[temp]=Table.Address[Table.Used-1];
			strcpy(Table.CMD[temp],Table.CMD[Table.Used-1]);
		    Table.Used--;
		    
		    
		    Var.Size=r1;
			Var.Var[Var.UsedVar]=(unsigned int)&ArrayBuffer.Buffer[ArrayBuffer.Base];
			strcpy(Var.VarName[Var.UsedVar],Buf);
			Var.attribute[Var.UsedVar]=ArrayType;
			EnterError
			ArrayBuffer.Base+=r1;
			uart_printf("\n%s is redefined !",Var.VarName[Var.UsedVar]);
			len=strlen(Var.VarName[Var.UsedVar]);
			Var.UsedVar++;
			return 0x80|len;
		
		}
	
	}
	



*/


}




char Help(void)
{
	char Buf[20],*p,len,i=0;
	int temp;
	p=Data;
	while(*p!=0)
	{   i=0;
		Buf[i++]=*p++;  //
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i]=0;
		if(strcmp(Buf,"help")==0)
		{   if(*p++==' ')
			{	i=0;
				while((*p!=0)&&(*p!=' '))
				{
					Buf[i++]=*p++;  //
				}
				Buf[i]=0;
				break;
			}
			else 
			{    
					EnterError
					uart_printf("parameter error! \n\r");
					return 0;
			}
		}
		else
		{
			p-=3;
		}
	}
	temp=CheckAllTheWord(Buf);
	if(temp==0)
	{
			EnterError
			uart_printf("command no define ! \n\r");
			return 0;
	}
	else
	{
		if(temp&0x10000000)   //Var
		{
			EnterError
			uart_printf("can not find help information ! \n\r");
			return 0;
		}
		else                //command
		{	temp&=~0x20000000;
			uart_printf("   %s \n\r",Var.Help[temp]);
			len=strlen(Var.VarName[temp]);
			return 0x80|len;
		}
	}
}


char GetValue(void)
{  
	if(Fetch())
	{
	        if(HEX)
	        UPop();
	        else
	        
	         Pop();
	         return 1;
	
	
	}
    return 0;


}


char Forget(void)
{
	char Buf[20],*p,i=0,len;
	
	int temp;
	p=Data;
	while(*p!=0)
	{   i=0;
		Buf[i++]=*p++;  //
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		Buf[i++]=*p++;  //
		Buf[i++]=*p++;
		Buf[i++]=*p++;
		
		Buf[i]=0;
		
		if(strcmp(Buf,"forget")==0)
		{   if(*p++==' ')
			{	i=0;
				while((*p!=0)&&(*p!=' '))
				{
			
					Buf[i++]=*p++;  //
			
			
				}
				Buf[i]=0;
				break;
		
			}
			else 
			{    
					EnterError
					uart_printf("redefine error,please input the constant name! \n\r");
					return 0;
					
			
			
			}
		
		}
		else
		{
			
			p-=5;
		
		}
		
	}
	
	temp=CheckAllTheWord(Buf);
	if(temp==0)
	{
		
		
			EnterError
			uart_printf("word not define ! \n\r");
			return 0;
		
	
	}
	
	else
	{
	
		if(temp&0x10000000)   //Var
		{  
			temp&=~0x10000000;
			len=strlen(Buf);
			strcpy(Var.VarName[temp],"");
			return 0x80|len;
			
		
		}
		else                //command
		{	
			temp&=~0x20000000;
			len=strlen(Buf);
			strcpy(Var.VarName[temp],"");
			return 0x80|len;
		
		
		}
	
	
	
	
	}
	



}



char At(void)
{
        int x,y,i;
        
	if(PStack(&y)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}

        if(PStack(&x)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}

      	ClrScreen

       for(i=0;i<y;i++)
       {

             EnterError

       }

       for(i=0;i<x;i++)
       {

             uart_printf(" ");

       }


    return 0;


}



char Not(void)
{
	int r1;
	if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n");
		return 0;
	}

	
		if(r1==0)
		{
		
			r1=-1;
		
		}
		else r1=0;
		if(SStack(r1))
		{
					return 1;
		}
		else
		{
					EnterError;
					uart_printf("Stack is full ! \n\r");
					return 0;
		}
}

char Bounds(void)
{
     /*
     int r1,r2;
     if(PStack(&r2)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n");
		return 0;
	}
       if(PStack(&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n");
		return 0;
	}
      */
   Over();
   Add();
   Swap();

	return 1;

}


char LogSwitch(void)
{  int i;
   // char data[20];
	unsigned int r1;
	if(PStack((int *)&r1)==0)
	{
		EnterError
		uart_printf("Stack underflow! \n\r");
		return 0;
	}
  if(r1==1)
		LOGO_ON=true;
	else if(r1==3)
	{
		uart_printf("\n\rcurrent Log N:%d \n\r",g_firmwareParameter.DebugLog.current);
		
		for(i=0;i<STORE_DEBUG_LOG;i++)
		{
				uart_printf("%d:%s\n\r",i,g_firmwareParameter.DebugLog.DebugStack[i]);
		}
	}
	else if(r1==4)
	{
			uart_printf("volume=%d\n\r",volume);
	
	
	
	}
	else 
	LOGO_ON=false;
		
			//sprintf(data,"%x",r1);
		//	uart_printf("    %s\n\r",data);
return 1;
}


void SaveLog2Flash(char *log)
{
	
	strcpy(g_firmwareParameter.DebugLog.DebugStack[g_firmwareParameter.DebugLog.current],log);
	g_firmwareParameter.DebugLog.current++;
	if(g_firmwareParameter.DebugLog.current==STORE_DEBUG_LOG)
		g_firmwareParameter.DebugLog.current=0;

}

