#ifndef __BLUETOOTH_H__  
  #define __BLUETOOTH_H__
	
#include 	"stm32f4xx.h"


#define BT_ChangeName_Enabled 1 // 1:enable other:disable
//#define   BTREST_GPIO       GPIOA
//#define   BTREST            GPIO_PIN_3
#define   BTINT_GPIO        GPIOA
#define   BTINT             GPIO_PIN_3
#define   BTWAKEUP_GPIO     GPIOA
#define   BTWAKEUP          GPIO_PIN_2	
#define   BTRESET_GPIO      GPIOA
#define   BTRESET           GPIO_PIN_11

extern unsigned char Uart5TXBuffer[32];
extern unsigned char CheckBLETask;
extern uint8_t     usart3_rx_buf[50]; // 中断接收缓冲区
extern __IO        uint8_t     usart3_rx_count;

extern void CheckBleName(void);
extern void BLEInit(void);
extern void SetBLEWAKEUP(unsigned char highlow);
#endif

