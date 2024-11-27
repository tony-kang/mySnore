
#include "bluetooth.h"
#include "tuwan.h"
#include <string.h>

char * BleNameHeader="MP2";//"TW";//"MP2-";//���Ȳ��ܳ���6���ַ���MAC��ַ��12���ַ����ϼƲ��ܳ���18���ַ�

unsigned char Uart5TXBuffer[32];
unsigned char SetBleNameBuffer[14];
uint8_t BleNameVerifyOK_flag=0;
uint8_t rcvbufdata[50];
unsigned char CheckBLETask = 0;
uint8_t     usart3_rx_buf[50]; // �жϽ��ջ�����
__IO        uint8_t     usart3_rx_count = 0;

extern unsigned char buf_for_BLE_SN_modify[100];



/**************************************************************************/
/*                                                                        */
/**************************************************************************/

void SetINTinput(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;


  /*Configure GPIO pin : PA3 */
  GPIO_InitStruct.Pin = BTINT;	
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BTINT_GPIO, &GPIO_InitStruct);
}


void SetINToutput(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;


    
  /*Configure GPIO pins :  PA3 */
  GPIO_InitStruct.Pin =  BTINT;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BTINT_GPIO, &GPIO_InitStruct);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BTINT_GPIO, BTINT, GPIO_PIN_SET);
}

void SetBLEWAKEUP(unsigned char highlow)
{
	if(highlow==1)
//		HAL_GPIO_WritePin(BTWAKEUP_GPIO, BTWAKEUP, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BTWAKEUP_GPIO, BTWAKEUP, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(BTWAKEUP_GPIO, BTWAKEUP, GPIO_PIN_RESET);
}

void SetBLERESET(unsigned char highlow)
{
	if(highlow==1)
		HAL_GPIO_WritePin(BTRESET_GPIO, BTRESET, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(BTRESET_GPIO, BTRESET, GPIO_PIN_RESET);
}

void SetBLEINT(unsigned char highlow)
{
	if(highlow==1)
		HAL_GPIO_WritePin(BTINT_GPIO, BTINT, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(BTINT_GPIO, BTINT, GPIO_PIN_RESET);
}

uint8_t ReadBLEINT(void)
{
	uint8_t state=0;
	state=HAL_GPIO_ReadPin(BTINT_GPIO, BTINT);
	return state;	
}

void ChangeBLEToCommandMode()
{
    SetINToutput();
    SetBLEINT(1);
    osDelay(1);    // 1ms
	  SetBLEWAKEUP(0);
    osDelay(20);   // 20ms
	  SetBLEINT(0);
    osDelay(220);   // 20ms
    SetBLEINT(1);
    osDelay(2);     // 2ms
	  SetBLEWAKEUP(1);    
    SetINTinput();
}


/**************************************************************************/
/*                                                                        */
/**************************************************************************/
void SendATCommandToBLE(unsigned char *data, unsigned int len)
{
    usart3_rx_count = 0;
    //EnUsart2Rcv = 0;
	  SetBLEWAKEUP(0);
    osDelay(2);  // 2ms
    uart3Send(data,len);
	  SetBLEWAKEUP(1);
    
	  //�ȴ�int��Ӧ
//	  while(ReadBLEINT()==1)
//		{
//		}
//		SetBLEWAKEUP(0);
//		while(ReadBLEINT()==0)
//		{
//		}
//		SetBLEWAKEUP(1);	
}

/**************************************************************************/
/*                                                                        */
/**************************************************************************/
//#define cncharM  'J'

void BLEInit(void)
{
	   uint8_t counter;
	  osDelay(200);//�ϵ�ȴ������ɲ���,�������120��������
	  CheckBLETask=1;
    ChangeBLEToCommandMode();
    osDelay(20);//���ʱ����Ҫ����ȷ��

	  //�ȶ�ȡ������ʶͷ���Ƿ��Ѿ��޸ĺ�
	  memset(usart3_rx_buf,0xff,50);
	  SendATCommandToBLE("AT+NAME=?\r\n", strlen("AT+NAME=?\r\n"));
	  counter=0;
	  while(1)
		{
	    if(usart3_rx_buf[0x1a]==0x0A && usart3_rx_buf[0x19]==0x0D && usart3_rx_count>=0x1b)
		  { 
			  break;
		  }
			counter++;
			if(counter>20)
				break;
			osDelay(10); 
			
	  }
		memset(usart3_rx_buf,0xff,50);	
		counter=0;

#if 0		
		//���͹ر�OAT(��������������
	  memset(usart3_rx_buf,0xff,50);
	  SendATCommandToBLE("AT+SETUP=0DC2012D\r\n", strlen("AT+SETUP=0DC2012D\r\n"));
	  counter=0;
	  while(1)
		{
	    if(usart3_rx_buf[0x5]==0x0D && usart3_rx_buf[0x6]==0x0A )
		  { 
			  break;
		  }
			counter++;
			if(counter>20)
				break;
			osDelay(10); 
			
	  }
		
		
		memset(usart3_rx_buf,0xff,50);	
		counter=0;
#endif		
		
		
    SendATCommandToBLE("AT+MODE=DATA\r\n", strlen("AT+MODE=DATA\r\n"));
	  while(1)
	  {
	    if(usart3_rx_count>=0x07)
		  { 
			  break;
		  }
			counter++;
			if(counter>20)
				break;
			osDelay(10);
		}			
	 		
//	 SetBLEWAKEUP(1); //�����޸Ļ�ȷ������ͷ��ʾ������
	 CheckBLETask=0;
		
	
}


uint8_t len;
void CheckBleName(void)
{
    uint8_t RetError=0;
	  uint8_t len,i,j,temp,counter;
	
	if(BT_ChangeName_Enabled==1)
	{
    CheckBLETask=1;
    len=strlen(BleNameHeader);	
    //===========û�г�ʼ����============ 
    osDelay(200);//�ϵ�ȴ������ɲ���,�������120��������
	  
    ChangeBLEToCommandMode();
    osDelay(20);//���ʱ����Ҫ����ȷ��

	  //�ȶ�ȡ������ʶͷ���Ƿ��Ѿ��޸ĺ�
	  memset(usart3_rx_buf,0xff,50);
	  SendATCommandToBLE("AT+NAME=?\r\n", strlen("AT+NAME=?\r\n"));
	  counter=0;
	  while(1)
		{
	    if(usart3_rx_buf[0x1a]==0x0A && usart3_rx_buf[0x19]==0x0D && usart3_rx_count>=0x1b)
		  { 
			  break;
		  }
			counter++;
			if(counter>20)
				break;
			osDelay(10); 
			
	  }
	  
	  memset(rcvbufdata,0xff,50);
    //---------------
    memcpy(rcvbufdata,usart3_rx_buf,usart3_rx_count);

    temp=0;
    for(i=0;i<len;i++)
    {
      if(rcvbufdata[7+i] != BleNameHeader[i])
			{
				temp=1;
				break;
			}
    }	
		if(temp==0)
		{
			//������ʶͷ��ȷ
			goto exitat;			
		}	    
	
    //�� bt-mac
		memset(usart3_rx_buf,0xff,50);
    SendATCommandToBLE("AT+DEVID=?\r\n", strlen("AT+DEVID=?\r\n"));
    counter=0;
	  while(1)
		{
	    if(usart3_rx_buf[0x1f]==0x0A && usart3_rx_buf[0x1e]==0x0D && usart3_rx_count>=0x20)
		  { 
			  break;
		  }
			counter++;
			if(counter>20)
				break;
			osDelay(10); 
			
	  }
//    osDelay(200);//���ʱ����Ҫ����ȷ��
	
    memset(rcvbufdata,0xff,50);
    //---------------
    memcpy(rcvbufdata,usart3_rx_buf,usart3_rx_count);

    if((rcvbufdata[10]==0x44)&&(rcvbufdata[14]==0x44)) //�����ַҪ������
    {
    }
    else
    {
        RetError = 1;
        goto exitat;
    }
		
    
    //д bt-id MT+mac
    //AT+NAME=TM1234567890AB\r\n
		
		//�ڷ��ص�MAC��ַ����ӱ�ʶͷ
		
		i=8-(len-2);
		
    rcvbufdata[i] = 'A';
    rcvbufdata[i+1] = 'T';
    rcvbufdata[i+2] = '+';
    rcvbufdata[i+3] = 'N';
    rcvbufdata[i+4] = 'A';
    rcvbufdata[i+5] = 'M';
    rcvbufdata[i+6] = 'E';
    rcvbufdata[i+7] = '=';
		for(j=0;j<len;j++)
		{
			rcvbufdata[i+8+j]= BleNameHeader[j];
		}
//    rcvbufdata[16] = 'T';
//    rcvbufdata[17] = cncharM;
    rcvbufdata[32] = 0;
    // 24: 1234567890AB\r\n
		memset(usart3_rx_buf,0xff,50);
		counter=0;
		SendATCommandToBLE(&rcvbufdata[i], 32-i);
    while(1)
		{
	    if(usart3_rx_buf[0x6]==0x0A && usart3_rx_buf[0x5]==0x0D && usart3_rx_count>=0x07)
		  { 
			  break;
		  }
			counter++;
			if(counter>20)
				break;
			osDelay(10);			
	  }
exitat: 
   
   memset(usart3_rx_buf,0xff,50);	
	 counter=0;
   SendATCommandToBLE("AT+MODE=DATA\r\n", strlen("AT+MODE=DATA\r\n"));
	 while(1)
	 {
	    if(usart3_rx_count>=0x07)
		  { 
			  break;
		  }
			counter++;
			if(counter>20)
				break;
			osDelay(10);
		}			
	 		
//	 SetBLEWAKEUP(1); //�����޸Ļ�ȷ������ͷ��ʾ������
	 CheckBLETask=0;	
		
   /*
    if(RetError == 0)
    {
        WrokingLed(OFF_LED);
        osDelay(500); 
        WrokingLed(GREE_MAX_LED);
        osDelay(500); 
        WrokingLed(OFF_LED);
        osDelay(500); 
        WrokingLed(GREE_MAX_LED);
        osDelay(500); 
        WrokingLed(OFF_LED);
        osDelay(500);         
    }
		*/
	}
	else
	{
		SetBLEWAKEUP(1);
		SetINTinput();
		SetBLERESET(0);
		osDelay(10);
		SetBLERESET(1);
		osDelay(200);//�ϵ�ȴ������ɲ���,�������120��������
	}
}



void SetBleName(void)
{
	  uint8_t RetError=0;
	  uint8_t len,i,j,temp,counter;
	
	
    CheckBLETask=1;
	  osDelay(200);//�ϵ�ȴ������ɲ���,�������120��������    
	  
    ChangeBLEToCommandMode();
    osDelay(20);//���ʱ����Ҫ����ȷ��

	  //�ȶ�ȡ������ʶͷ���Ƿ��Ѿ��޸ĺ�
	  
//	  memset(rcvbufdata,0xff,50);    
    rcvbufdata[0] = 'A';
    rcvbufdata[1] = 'T';
    rcvbufdata[2] = '+';
    rcvbufdata[3] = 'N';
    rcvbufdata[4] = 'A';
    rcvbufdata[5] = 'M';
    rcvbufdata[6] = 'E';
    rcvbufdata[7] = '=';
		//����Ҫ�޸ĵ�name
		for(j=0;j<14;j++)
		{
			SetBleNameBuffer[j]=rcvbufdata[j+8];
		}
		rcvbufdata[22]=0x0D;
		rcvbufdata[23]=0x0A;
		rcvbufdata[24]=0x00;

		memset(usart3_rx_buf,0xff,50);	
		
		
		SendATCommandToBLE(&rcvbufdata[0], 24);
		
		memset(usart3_rx_buf,0xff,50);
		
		
		
		SendATCommandToBLE(&rcvbufdata[0], 24);
		
		
		counter=0;
    while(1)
		{
	    if(usart3_rx_buf[0x6]==0x0A && usart3_rx_buf[0x5]==0x0D && usart3_rx_count>=0x07)
		  { 
			  break;
		  }
			counter++;
			if(counter>20)
				break;
			osDelay(10);			
	  }
   
   memset(usart3_rx_buf,0xff,50);		
   SendATCommandToBLE("AT+MODE=DATA\r\n", strlen("AT+MODE=DATA\r\n"));
	 counter=0;
	 while(1)
	 {
	    if(usart3_rx_count>=0x07)
		  { 
			  break;
		  }
			counter++;
			if(counter>20)
				break;
			osDelay(10);
		}			
	 		
//	 SetBLEWAKEUP(1); //�����޸Ļ�ȷ������ͷ��ʾ������
	 CheckBLETask=0;
	
}
void VerifyBleName(void)
{
	  uint8_t RetError=0;
	  uint8_t len,i,j,temp,counter;
	
	
    CheckBLETask=1;
	  osDelay(200);//�ϵ�ȴ������ɲ���,�������120��������    
	  
    ChangeBLEToCommandMode();
    osDelay(20);//���ʱ����Ҫ����ȷ��

	  //�ȶ�ȡ������ʶͷ���Ƿ��Ѿ��޸ĺ�
	  
//	  memset(rcvbufdata,0xff,50);    
    rcvbufdata[0] = 'A';
    rcvbufdata[1] = 'T';
    rcvbufdata[2] = '+';
    rcvbufdata[3] = 'N';
    rcvbufdata[4] = 'A';
    rcvbufdata[5] = 'M';
    rcvbufdata[6] = 'E';
    rcvbufdata[7] = '=';
		rcvbufdata[8] = '?';
		rcvbufdata[9] = 0x0D;
		rcvbufdata[10] =0x0A;
		memset(usart3_rx_buf,0xff,50);
		SendATCommandToBLE(&rcvbufdata[0], 11);
		counter=0;
    while(1)
		{
	    if(usart3_rx_buf[0x25]==0x0D && usart3_rx_buf[0x26]==0x0A)
		  { 
			  break;
		  }
			counter++;
			if(counter>20)
				break;
			osDelay(10);			
	  }
   //�Ƚ��Ƿ��޸ĳɹ�
	 temp=1;
	 for(i=0;i<1;i++)
   {		
		 if(SetBleNameBuffer[i]==usart3_rx_buf[i+7])
		 {
		 }
		 else
		 {
       temp=0;
			 break;
		 }			 
	 }
	 if(temp==1)
		 BleNameVerifyOK_flag=1;
	 else
		 BleNameVerifyOK_flag=0;
   memset(usart3_rx_buf,0xff,50);		
   SendATCommandToBLE("AT+MODE=DATA\r\n", strlen("AT+MODE=DATA\r\n"));
	 counter=0;
	 while(1)
	 {
	    if(usart3_rx_count>=0x07)
		  { 
			  break;
		  }
			counter++;
			if(counter>20)
				break;
			osDelay(10);
		}			
	 		
//	 SetBLEWAKEUP(1); //�����޸Ļ�ȷ������ͷ��ʾ������
	 CheckBLETask=0;
	
}
