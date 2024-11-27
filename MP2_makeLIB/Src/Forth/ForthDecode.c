#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "ForthTeminal.h"
#include <ctype.h>
#include "tuwan.h"
#include "ForthDecode.h"
#include <stdarg.h>
//#include "ucos_ii.h"
#include <stdlib.h>
#include <ctype.h> 
void SaveLog2Flash(char *log);
extern CommandBufferTypedef CommandBuffer;
extern unsigned char GetChar,Uart2Buffer;
void uart5Send(unsigned char *data,unsigned int len);

#define ZEROPAD        1    /* pad with zero */
#define SIGN           2    /* unsigned/signed long */
#define PLUS           4    /* show plus */
#define SPACE          8    /* space if plus */
#define LEFT           16   /* left justified */
#define SPECIAL        32   /* 0x */
#define LARGE          64   /* use 'ABCDEF' instead of 'abcdef' */

#define is_digit(c) ((c) >= '0' && (c) <= '9')

#define do_div(n,base) ({    \
         int __res;     \
         __res = ((unsigned long) n) % (unsigned) base; \
         n = ((unsigned long) n) / (unsigned) base; \
         __res; })
   
static unsigned char sprint_buf[1024];
extern unsigned int HEX;


void uart_putc(unsigned char ch) 
{
	//UART2SEND(ch);
	//RS485_PutData((const char*)&ch, 1);
	uart_send(&ch,1);
}

unsigned char uart_getc(void)
{
	while(GetChar==0);
	GetChar=0;
	return Uart2Buffer;//RS485_GetChar();
	
 }
   
void uart_puts(unsigned char* src)
{	
	uart5Send(src,strlen((char *)src));
}
   

   
int strnlen2(const char * s, int count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)

	return sc - s;
}

   
 //  unsigned char _ctype[] = {
 //    _C,_C,_C,_C,_C,_C,_C,_C,   /* 0-7 */
//     _C,_C|_S,_C|_S,_C|_S,_C|_S,_C|_S,_C,_C, /* 8-15 */
 //    _C,_C,_C,_C,_C,_C,_C,_C,   /* 16-23 */
 //    _C,_C,_C,_C,_C,_C,_C,_C,   /* 24-31 */
 //    _S|_SP,_P,_P,_P,_P,_P,_P,_P,  /* 32-39 */
 //    _P,_P,_P,_P,_P,_P,_P,_P,   /* 40-47 */
//     _D,_D,_D,_D,_D,_D,_D,_D,   /* 48-55 */
//     _D,_D,_P,_P,_P,_P,_P,_P,   /* 56-63 */
//     _P,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U, /* 64-71 */
 //    _U,_U,_U,_U,_U,_U,_U,_U,   /* 72-79 */
 //    _U,_U,_U,_U,_U,_U,_U,_U,   /* 80-87 */
 //    _U,_U,_U,_P,_P,_P,_P,_P,   /* 88-95 */
 //    _P,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L, /* 96-103 */
//     _L,_L,_L,_L,_L,_L,_L,_L,   /* 104-111 */
//     _L,_L,_L,_L,_L,_L,_L,_L,   /* 112-119 */
 //    _L,_L,_L,_P,_P,_P,_P,_C,   /* 120-127 */
 //    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 128-143 */
//     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  /* 144-159 */
//     _S|_SP,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,   /* 160-175 */
//     _P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,       /* 176-191 */
//     _U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,       /* 192-207 */
//     _U,_U,_U,_U,_U,_U,_U,_P,_U,_U,_U,_U,_U,_U,_U,_L,       /* 208-223 */
//     _L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,       /* 224-239 */
//     _L,_L,_L,_L,_L,_L,_L,_P,_L,_L,_L,_L,_L,_L,_L,_L};      /* 240-255 */
   
static int skip_atoi(const char **s)
{
	int i=0;

	while (is_digit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}
   /*
   static char * number(char * str, long num, int base, int size, int precision
          ,int type)
   {
     char c,sign,tmp[66];
     const char *digits="0123456789abcdefghijklmnopqrstuvwxyz";
     int i;
   
     if (type & LARGE)
       digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
     if (type & LEFT)
       type &= ~ZEROPAD;
     if (base < 2 || base > 36)
       return 0;
     c = (type & ZEROPAD) ? '0' : ' ';
     sign = 0;
     if (type & SIGN) {
       if (num < 0) {
         sign = '-';
         num = -num;
         size--;
       } else if (type & PLUS) {
         sign = '+';
         size--;
       } else if (type & SPACE) {
         sign = ' ';
         size--;
       }
     }
     if (type & SPECIAL) {
       if (base == 16)
         size -= 2;
       else if (base == 8)
         size--;
     }
     i = 0;
     if (num == 0)
       tmp[i++]='0';
     else while (num != 0)
     tmp[i++] = digits[do_div(num,base)];
     if (i > precision)
       precision = i;
     size -= precision;
     if (!(type&(ZEROPAD+LEFT)))
       while(size-->0)
         *str++ = ' ';
     if (sign)
       *str++ = sign;
     if (type & SPECIAL) {
       if (base==8)
         *str++ = '0';
       else if (base==16) {
         *str++ = '0';
         *str++ = digits[33];
       }
     }
     if (!(type & LEFT))
       while (size-- > 0)
         *str++ = c;
     while (i < precision--)
       *str++ = '0';
     while (i-- > 0)
       *str++ = tmp[i];
     while (size-- > 0)
       *str++ = ' ';
     return str;
   }
 */  
  
///////////////////////////////////////////////////////

int __vsprintf2(char *buf, const char *fmt, va_list args)
{
	int len;

	//int  base;
	char * str,Buff[20];
	const char *s;
	int t;
	int Ci;

	int flags;  /* flags to number() */

	//  int field_width; /* width of output field */
	int precision=100;    /* min. # of digits for integers; max number of chars for from string */
	// int qualifier;      /* 'h', 'l', or 'L' for integer fields */
	for(len=0;len<1024;len++)
	{
		*buf++=0;
	}
	buf-=1024;

	//memset(buf,1024,0);

	for (str=buf ; *fmt ; ++fmt) 
	{
		if (*fmt != '%') 
		{
			*str++ = *fmt;
			continue;
		}


		fmt++;
		switch (*fmt) 
		{
			case 's':
				s = va_arg(args, char *);  //????????????
				if (!s)
				s = "<NULL>";

				len = strnlen2(s, precision);  //
				if(len)
				{
				strcat(str,s);
				str+=len;
				}
				continue;

			case 'p':
				continue;


			case 'n':
				/*
				if (qualifier == 'l') {
				long * ip = va_arg(args, long *);
				*ip = (str - buf);
				} else {
				int * ip = va_arg(args, int *);
				*ip = (str - buf);
				}*/
				continue;

			/* integer number formats - set up the flags and "break" */
			case 'o':
				//base = 8;
				break;

			case 'X':
				flags |= LARGE;
			case 'x':
				t = va_arg(args, int);  //????????????

				sprintf(Buff,"%x",t);

				strcat(str,Buff);
				str+=strlen(Buff);
				continue;

			case 'd':
				t = va_arg(args, int);  //????????????
				if(HEX==0)
				sprintf(Buff,"%d",t);
				else   sprintf(Buff,"%x",t);

				strcat(str,Buff);
				str+=strlen(Buff);
				continue;

			case 'c':
				Ci= va_arg(args,int); 
				*str++=Ci;
				continue;

			default:
				if (*fmt != '%')
					*str++ = '%';
				if (*fmt)
					*str++ = *fmt;
				else
					--fmt;
				continue;
		}
	}
	*str = '\0';
	return str-buf;
}


///////////////////////////////////////////////////////  
int __vsprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	unsigned long num;
	int i;//, base;
	char * str;
	const char *s;

	int flags;  /* flags to number() */

	int field_width; /* width of output field */
	int precision;    /* min. # of digits for integers; max number of chars for from string */
	int qualifier;      /* 'h', 'l', or 'L' for integer fields */
   
	for (str=buf ; *fmt ; ++fmt) 
	{
		if (*fmt != '%') 
		{
			*str++ = *fmt;
			continue;
		}

		/* process flags */
		flags = 0;
	repeat:
		++fmt;  /* this also skips first '%' */
		switch (*fmt) 
		{
			case '-': flags |= LEFT; goto repeat;
			case '+': flags |= PLUS; goto repeat;
			case ' ': flags |= SPACE; goto repeat;
			case '#': flags |= SPECIAL; goto repeat;
			case '0': flags |= ZEROPAD; goto repeat;
		}
     
		/* get field width */
		field_width = -1;
		if (is_digit(*fmt))
		field_width = skip_atoi(&fmt);
		else if (*fmt == '*')
		{
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) 
			{
				field_width = -field_width;
				flags |= LEFT;
			}
		}
   
		/* get the precision */
		precision = -1;
		if (*fmt == '.') 
		{
			++fmt; 
			if (is_digit(*fmt))
			precision = skip_atoi(&fmt);
			else if (*fmt == '*')
			{
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}
   
		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') 
		{
			qualifier = *fmt;
			++fmt;
		}
   
		/* default base */
		//base = 10;
   
		switch (*fmt) 
		{
		case 'c':
			if (!(flags & LEFT))
			while (--field_width > 0)
			*str++ = ' ';
			*str++ = (unsigned char) va_arg(args, int);
			while (--field_width > 0)
			*str++ = ' ';
			continue;
   
		case 's':
			s = va_arg(args, char *);
			if (!s)
			s = "<NULL>";

			len = strnlen2(s, precision);

			if (!(flags & LEFT))
			while (len < field_width--)
				*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;
			while (len < field_width--)
				*str++ = ' ';
			continue;
   
		case 'p':
			continue;
   
   
		case 'n':
			if (qualifier == 'l') 
			{
				long * ip = va_arg(args, long *);
				*ip = (str - buf);
			} 
			else 
			{
				int * ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;
   
         /* integer number formats - set up the flags and "break" */
		case 'o':
			//base = 8;
			break;
   
		case 'X':
			flags |= LARGE;
		case 'x':
			//base = 16;
			break;
   
		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;
   
		default:
			if (*fmt != '%')
				*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			continue;
		}
		if (qualifier == 'l')
		num = va_arg(args, unsigned long);
		else if (qualifier == 'h') 
		{
			num = (unsigned short) va_arg(args, int);
			if (flags & SIGN)
				num = (short) num;
		} 
		else if (flags & SIGN)
			num = va_arg(args, int);
		else
			num = va_arg(args, unsigned int);
         
         
		// sprintf(Buff,"%d",num);
		// strcat(str,Buff);
		*str++=num;
		//   str = number(str, num, base, field_width, precision, flags);
	}
	*str = '\0';
	return str-buf;
}
   
   
void uart_printf(char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	__vsprintf2((char*)sprint_buf, fmt,args);
	va_end(args);
	uart_puts(sprint_buf);
}
   
   
void __vsscanf(char* buf,const char *fmt, va_list args)
{
	int len;//,*Output;
	char Buff[50],dat,*data; //*str,

   	 	
	data=(char *)Buff;
	for(len=0;len<50;len++)
	{
		Buff[len]=0;


	}
   	 	
 	for(len=0;len<1024;len++)
	{
		*buf++=0;


	}
   		
   		
	buf-=1024;
    
   
    
	for (; *fmt ; ++fmt)  //str=buf 
	{
		if (*fmt != '%') 
			continue;
	}	
   	fmt++;
   	switch(*fmt)
   	{
   		case 'd':
   			//Output=va_arg(args, int *); 
   			va_arg(args, int *); 
   			while(1)
   			{
   			
   				dat=uart_getc();
   				if(dat==0x0d)
   				break;
   				else
   				{
   					
   					*data=dat;
   					data++;
   				
   				}
   				
   		
   		
   			}
   			//????????????????????.
   			
   			
   		break;
   		case 'c':
   		break;
   		default:
   		break;
   	}	
}
   
   
   
void uart_scanf(char *fmt)
{
	char data,derection[3];
	unsigned int S,i;
	S=(unsigned int )fmt;
	while(1)
	{
   		
		data=uart_getc();
		
		switch(data)
		{
		case 0:
			*fmt=1;
			break;
		case 13:  
			*fmt=0;
			//Enter
			break;
		case 8:
			if((unsigned int)fmt>S)
			{	*fmt=0;
				fmt--;
				BackSpace 
				uart_printf(" ");
				BackSpace 
						
			}
			else
			{
				
				*fmt=1;
				
			}
			break;
		case 0x1b:   //up key
		  derection[0]=0x1b;
		  derection[1]=uart_getc();
		  derection[2]=uart_getc();
		  *fmt=1;
		  if(derection[1]==0x5b)
		  {
	  		switch(derection[2])
	  		{
	  			case 0x41:     //up key
	  			if(CommandBuffer.NowGot>0)
	  			{
	  					
	  					i=0;
	  					fmt=( char *)S;
	  					for(i=0;i<80;i++)
	  					{
	  						BackSpace 
							uart_printf(" ");
							BackSpace 
	  					}
	  					uart_printf("%s",CommandBuffer.Buffer[CommandBuffer.Index]);
	  					strcpy(fmt,CommandBuffer.Buffer[CommandBuffer.Index]);
	  					fmt+=strlen(CommandBuffer.Buffer[CommandBuffer.Index]);
	  					*fmt=1;
	  					
	  					if(CommandBuffer.Index<CommandBuffer.NowGot-1)
	  					CommandBuffer.Index++;
	  					 else    CommandBuffer.Index=0;
	  			}
	  			break;

	  			case 0x42:     //down key
   				  			
	  				if(CommandBuffer.NowGot>0)
	  				{
	  					i=0;
	  					fmt=( char *)S;
	  					for(i=0;i<80;i++)
	  					{
	  						BackSpace 
							uart_printf(" ");
							BackSpace 
	  					}
	  				
	  					uart_printf("%s",CommandBuffer.Buffer[CommandBuffer.Index]);
	  				
	  					strcpy(fmt,CommandBuffer.Buffer[CommandBuffer.Index]);
	  				
	  					fmt+=strlen(CommandBuffer.Buffer[CommandBuffer.Index]);
	  					*fmt=1;
	  				
	  					if(CommandBuffer.Index>0)
	  					CommandBuffer.Index--;
	  					else CommandBuffer.Index=CommandBuffer.NowGot-1;
	  				}
		  			break;
					
	  			default:
 		  			break;
   			}
	  	}
		break;

		default:
			if((data>31)&&(data<127))
			{
				uart_printf("%c",data);
				if((data>64)&&(data<91))
				{
					data+=32;
				}
				*fmt++=data;
				*fmt=1;
			}
			break;
   		}

		if(*fmt==0)
		{
			break;
		}
		OSTimeDly(10);
	}
}



void DebugLog(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	__vsprintf2((char*)sprint_buf, fmt,args);
	va_end(args);
	if(LOGO_ON)
	uart_puts(sprint_buf);
	SaveLog2Flash((char *)sprint_buf);
}

