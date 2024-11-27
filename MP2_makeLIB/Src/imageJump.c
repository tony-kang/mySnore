#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "tuwan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"
#include "I2s.h"
#include "snore.h"
#include "InputOutput.h"
#include "W25Q16.h"
#include "cmsis_os.h"
#include "task.h"
#include "flash.h"

//你好

typedef void (*iapfun)(void); 
iapfun jump2app;
  void iap_load_app(unsigned int  appxaddr)
  {
      if(((*(unsigned int*)appxaddr)&0x2FFE0000)==0x20000000) //??????????
			{ __set_PRIMASK(1);
				SCB->VTOR=appxaddr;
				jump2app = (iapfun)*(unsigned int *)(appxaddr+4);//????????????????
				__set_MSP(*(volatile unsigned int*) appxaddr);//???APP????,????????????????ÖÐ¹úÈË
				jump2app(); //???APP,????????
			}
  }





unsigned char JumpToImage(unsigned char type)
{
	if(type==IMAGEA)
	{
		iap_load_app(IMAGE_A_ADDRESS);
	
	}
	else if(type==IMAGEB)
	{
		iap_load_app(IMAGE_B_ADDRESS);
	
	}
	else return 0;

return 1;
}
