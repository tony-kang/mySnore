#include "stm32f4xx_hal.h"
void RunLed(unsigned char s);
void BlueToolLed(unsigned char s);
void OnOffLed(unsigned char s);
void PowerLed(unsigned char s);
GPIO_PinState GetPowerKey(void);
GPIO_PinState GetBlueToothKey(unsigned char key);
void HeadLeft(GPIO_PinState s);
void HeadRight(GPIO_PinState s);
void BodyLeft(GPIO_PinState s);
void BodyRigth(GPIO_PinState s);
void ValueDriver(unsigned char value,GPIO_PinState s);
void ValueDriver1(unsigned char value,GPIO_PinState s);
void InhalePump(unsigned char s);
void ExhalePump(unsigned char s);
void BlueToolthEnabled(unsigned char s);

