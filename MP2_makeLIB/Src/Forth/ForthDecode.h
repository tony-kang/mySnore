#ifndef __FORTH_DECODE_H_
#define __FORTH_DECODE_H_

//#include ""
void OSTimeDly(unsigned short t);
void uart_printf(char *fmt, ...);
void uart_scanf(char *fmt);
unsigned char uart_getc(void);

/////Hyper linker command//////////////
#define ClrScreen      uart_printf("%c",0x0c);
#define BackSpace      uart_printf("%c",0x08);
#define Tab     	   uart_printf("%c",0x09);
#define Home     	   uart_printf("%c",0x0d);
#define Enter    	   uart_printf("   ok"); uart_printf("%c",0x0d); uart_printf("%c",0x0a); 
#define EnterError     uart_printf("%c",0x0d); uart_printf("%c",0x0a); 

//////////////////////////////////////// 
#define ARM     1  
    
#endif 

#if 0
int UartInitialization(int baudrate);
	void UART1SEND(char a);
	int RS232_PutData(char* Data,int Count);
	char  Uart1ReceiveByte(void);
	char  ReceiveByte(void);
	int RS232_GetData(char* strBuf,int timeout);
	void Uart2_Isr(void);
	char Receive485Byte(void);
	void UART2SEND(char a);
	int RS485_PutData(const char* buffer, int count);
	int RS485_GetData(char* strBuf,int timeout);
	
	void UartTest(void);
    void uart_printf(char *fmt, ...);
    unsigned char uart_getc(void);
     void uart_scanf(char *fmt);
#endif
