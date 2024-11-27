/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal_rtc.h"

/* USER CODE BEGIN Includes */
#include "W25Q16.h"
#include "tuwan.h"
#include "bluetooth.h"

uint8_t  RTC_Clock_Selector=1;  //0:外部24MHz 1：RTC由外部32.768K产生 2：由内部晶振32K产生
uint8_t Error_Code=0;

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
ADC_HandleTypeDef hadc3;

I2C_HandleTypeDef hi2c1;

I2S_HandleTypeDef hi2s2;
DMA_HandleTypeDef hdma_spi2_rx;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart5;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_uart5_tx;
DMA_HandleTypeDef hdma_usart3_tx;

osThreadId MainTaskHandle;
osThreadId ForthTemialTaskHandle;
osThreadId PositionTaskHandle;
osThreadId ActionTaskHandle;
osThreadId SnoreTaskHandle;
osThreadId LedFlashTaskHandle;
osThreadId WatchStackTaskHandle;

osMessageQId UART5QHandle;
osMessageQId UART3QHandle;



/* USER CODE BEGIN PV */
xQueueHandle  ExhalAdcQHandle,InhalAdcQHandle;
/* Private variables ---------------------------------------------------------*/
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S2_Init(void);
static void MX_SPI1_Init(void);
static void MX_UART5_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_RTC_Init(void);
static void MX_ADC3_Init(void);
//static void SystemClock_Config(void);
static void MX_TIM7_Init(void);
void MainTaskFunction(void const * argument);
void ForthTemialFunction(void const * argument);
void PositionTaskFunction(void const * argument);
void ActionTaskFunction(void const * argument);
void SnoreTaskFunction(void const * argument);
void LedFlashTaskFunction(void const * argument);
void WatchStackTaskFunction(void const * argument);

extern void INT_TIM7_Service_Code(void);
extern volatile uint32_t TimerTick;
extern void SoftWareReset(void);
extern RTC_DateTypeDef sdatestructure;
void SetINTinput(void);
void SetBLERESET(unsigned char);
		


/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{
	uint32_t I_delay=0,k;
  /* USER CODE BEGIN 1 */
	
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	//for(k=0x200;k>0;k--)
	//{
	//  for(I_delay=0xffff;I_delay>0;I_delay--);
	//}
	
  HAL_Init();
  __set_PRIMASK(0);
	
  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
	
  MX_GPIO_Init();
	MX_TIM7_Init();
  MX_DMA_Init();
	
#ifndef SOFTIIC
  MX_I2C1_Init();
#endif
  MX_I2S2_Init();
  MX_SPI1_Init();
  MX_UART5_Init();
  MX_USART3_UART_Init();

  MX_ADC1_Init();
  MX_ADC2_Init();
  
  MX_ADC3_Init();
//	if(RTC_Clock_Selector!=2)
	{
	  MX_RTC_Init();
	}
	HAL_PWREx_DisableBkUpReg(); //临时添加，by ZJP 20220416
//  SystemClock_Config();
//  MX_TIM7_Init();

  /* USER CODE BEGIN 2 */
  FLASH_SPI_CS_DISABLE();
	__set_PRIMASK(0);
	//CheckSystemParameter(); //disabled by zjp for no bootloader
	InhalePump(0);
	ExhalePump(0);
	
	SetBLEWAKEUP(1);
//	SetBLEWAKEUP(1);
		SetINTinput();
		SetBLERESET(0);
		osDelay(10);
		SetBLERESET(1);
		osDelay(200);//上电等待蓝牙可操作,必须大于120毫秒以上
	
	ERP_ON();
//	BlueToolthEnabled(1);  //正式的要关闭
	
	/// BlueToolLed(1);
	 ErrorCode=0;
	 
	
	 PowerLed(1);
	 RunLed(1);
	 /*
	 ResetValuePump();
	
	 EntryStandbyMode();
	*/
  // while(1);
	
	 /*
	 while(1)
	 {
	 d=10000000;
	 PowerLed(0);
	 while(d--);
		  d=10000000;
	 PowerLed(1);
	 while(d--);
		 
	 }
	 */
	  
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of MainTask */
  osThreadDef(MainTask, MainTaskFunction, osPriorityNormal, 0, 1024);
  MainTaskHandle = osThreadCreate(osThread(MainTask), NULL);

  /* definition and creation of ForthTemialTask */
  osThreadDef(ForthTemialTask, ForthTemialFunction, osPriorityLow, 0, 256);
  ForthTemialTaskHandle = osThreadCreate(osThread(ForthTemialTask), NULL);

  /* definition and creation of PositionTask */
  osThreadDef(PositionTask, PositionTaskFunction, osPriorityBelowNormal, 0, 1024);
  PositionTaskHandle = osThreadCreate(osThread(PositionTask), NULL);

  /* definition and creation of ActionTask */
  osThreadDef(ActionTask, ActionTaskFunction, osPriorityLow, 0, 512);
  ActionTaskHandle = osThreadCreate(osThread(ActionTask), NULL);

  /* definition and creation of SnoreTask */
  osThreadDef(SnoreTask, SnoreTaskFunction, osPriorityHigh, 0, 2048);
  SnoreTaskHandle = osThreadCreate(osThread(SnoreTask), NULL);


  osThreadDef(LedFlashTask, LedFlashTaskFunction, osPriorityNormal, 0, 256);
  LedFlashTaskHandle = osThreadCreate(osThread(LedFlashTask), NULL);
	
	osThreadDef(WatchStackTask, WatchStackTaskFunction, osPriorityNormal, 0, 128);
  WatchStackTaskHandle = osThreadCreate(osThread(WatchStackTask), NULL);
  

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the queue(s) */
  /* definition and creation of UART5Q */
  osMessageQDef(UART5Q, 50, unsigned char);
  UART5QHandle = osMessageCreate(osMessageQ(UART5Q), NULL);

  /* definition and creation of UART3Q */
//  osMessageQDef(UART3Q, 300, uint8_t);
//  UART3QHandle = osMessageCreate(osMessageQ(UART3Q), NULL);
  UART3QHandle = xQueueCreate(3000,sizeof(uint8_t));
  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  ExhalAdcQHandle = xQueueCreate( 10 , sizeof( unsigned int) );  
	InhalAdcQHandle = xQueueCreate( 10 , sizeof( unsigned int) );  
  /* USER CODE END RTOS_QUEUES */
 

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{
//  uint32_t I_delay=0;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

  __HAL_RCC_PWR_CLK_ENABLE();

	
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	
//	RCC->CFGR |=RCC_CFGR_HPRE_DIV4;//GD32F405特殊代码
//	for(I_delay=0xffff;I_delay>0;I_delay--);
	if(RTC_Clock_Selector==0)
	{
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	}
  else if(RTC_Clock_Selector==1)
	{
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
		RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	}
	else 
	{
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSI;
		RCC_OscInitStruct.LSIState = RCC_LSI_ON;	
	}
  
	
//	RCC->CFGR|=0x00000002;//PLL??????  added by zjp at 20211106

	
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;  
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
		Error_Code=1;
    Error_Handler();
  }
  else
	{
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
		  Error_Code=2;
      Error_Handler();
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S|RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.PLLI2S.PLLI2SN = 96;
    PeriphClkInitStruct.PLLI2S.PLLI2SR = 6;
	  if(RTC_Clock_Selector==0)
	  {
	    PeriphClkInitStruct.RTCClockSelection=RCC_RTCCLKSOURCE_HSE_DIV30;
	  }
	  else if(RTC_Clock_Selector==1)
	  {
		  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;//LSE
	  }
	  else 
	  {
      PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;//LSE
	  }
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
		  Error_Code=3;
      Error_Handler();
    }

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
	}
}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
		Error_Code=4;
    Error_Handler();
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
		Error_Code=5;
    Error_Handler();
  }

}

/* ADC2 init function */
static void MX_ADC2_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.ScanConvMode = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
		Error_Code=6;
    Error_Handler();
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
		Error_Code=7;
    Error_Handler();
  }

}

/* ADC3 init function */
static void MX_ADC3_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
		Error_Code=8;
    Error_Handler();
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_11; //气压传感器采样
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
		Error_Code=9;
    Error_Handler();
  }

}

/* I2C1 init function */
extern void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
		Error_Code=10;
    Error_Handler();
  }

}

/* I2S2 init function */
static void MX_I2S2_Init(void)
{

  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_RX;
  hi2s2.Init.Standard = I2S_STANDARD_LSB;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_32K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
		Error_Code=11;
    Error_Handler();
  }

}

/* RTC init function */
static void MX_RTC_Init(void)
{

  //RTC_TimeTypeDef sTime;
  //RTC_DateTypeDef sDate;

    /**Initialize RTC and set the Time and Date 
    */
	
//	  Error_Code=12;
//	  Error_Handler();
		
	
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;	
	  if(RTC_Clock_Selector==0)
	  {
	
      hrtc.Init.AsynchPrediv = 127;
	    hrtc.Init.SynchPrediv = 6149;  //255：LSE 249：LSI  (127+1)*(255+1)=32768;32.768K
		
	  }	
	  else if(RTC_Clock_Selector==1)
	  {
	
      hrtc.Init.AsynchPrediv = 127;
	    hrtc.Init.SynchPrediv = 255;  //255：LSE 249：LSI  (127+1)*(255+1)=32768;32.768K
		
	  }	
	
	  else
	  {
		  hrtc.Init.AsynchPrediv = 112;
      hrtc.Init.SynchPrediv = 255;  //255：LSE 249  ((LSI 124+1)*(255+1)=32000;32K
	  }
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
		  
//		  SoftWareReset();		
		  Error_Code=12;
      Error_Handler();
  
		}
		
		
		
#if 0
	if (HAL_RTCEx_SetCalibrationOutPut(&hrtc, RTC_CALIBOUTPUT_512HZ) != HAL_OK)
  {
    Error_Handler();
  }
#endif
/*
  sTime.Hours = 0x8;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x10;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
	  
	
    Error_Handler();
  }

  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
*/
    /**Enable the WakeUp 
    */
#if 0	//disable by zjp at 20211028
  if (HAL_RTCEx_SetWakeUpTimer(&hrtc, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
  {
    Error_Handler();
  }
#endif	

}

/* SPI1 init function */
static void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;//SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
		Error_Code=13;
    Error_Handler();
  }

}

/* TIM7 init function */
static void MX_TIM7_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig;

  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 0;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 420;//840：10us  84:1us
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
		Error_Code=14;
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
		Error_Code=15;
    Error_Handler();
  }

}

/* UART5 init function */
static void MX_UART5_Init(void)
{

  huart5.Instance = UART5;
  huart5.Init.BaudRate = UART5_BAUDRATE;//115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
		Error_Code=16;
    Error_Handler();
  }

}

/* USART3 init function */

//蓝牙模块UART通讯接口
static void MX_USART3_UART_Init(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 38400;//256000;//38400;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
		Error_Code=17;
    Error_Handler();
  }
  
}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
  /* DMA1_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
  /* DMA1_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA2 PA8 PA9 PA2:BLE_WAKEUP设置为输出
                           PA10 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_8|GPIO_PIN_9 
                          |GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA3 */  //BLE_int,初始设置为输入
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB13 PB14 
                           PB15 PB3 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_13|GPIO_PIN_14 
                          |GPIO_PIN_15|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PC6 PC7 PC8 PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin Output Level */
 // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_8|GPIO_PIN_9 
 //                         |GPIO_PIN_10, GPIO_PIN_RESET);
  
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1,GPIO_PIN_RESET);
													
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_9 
                          |GPIO_PIN_10, GPIO_PIN_RESET);	//对于三代 GPIOA 8，9，10 GPIOC 9上电置为1，以接通气袋到空气，对于四代要相反											
	
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_11|GPIO_PIN_2, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_13|GPIO_PIN_14 
                          |GPIO_PIN_15|GPIO_PIN_3|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
//  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);
	
 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8, GPIO_PIN_SET);

 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);//对于三代 GPIOA 8，9，10 GPIOC 9上电置为1，以接通气袋到空气，对于四代要相反 
 
	
	
	
	
	/*Configure GPIO pins : PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* MainTaskFunction function */
__weak void MainTaskFunction(void const * argument)
{

  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(10);
  }
  /* USER CODE END 5 */ 
}

/* ForthTemialFunction function */
__weak void ForthTemialFunction(void const * argument)
{
  /* USER CODE BEGIN ForthTemialFunction */
  /* Infinite loop */
  for(;;)
  {
    osDelay(10);
  }
  /* USER CODE END ForthTemialFunction */
}

/* PositionTaskFunction function */
__weak void PositionTaskFunction(void const * argument)
{
  /* USER CODE BEGIN PositionTaskFunction */
  /* Infinite loop */
  for(;;)
  {
    osDelay(10);
  }
  /* USER CODE END PositionTaskFunction */
}

__weak void LedFlashTaskFunction(void const * argument)
{
  /* USER CODE BEGIN PositionTaskFunction */
  /* Infinite loop */
  for(;;)
  {
    osDelay(10);
  }
  /* USER CODE END PositionTaskFunction */
}

/* ActionTaskFunction function */
__weak void ActionTaskFunction(void const * argument)
{
  /* USER CODE BEGIN ActionTaskFunction */
  /* Infinite loop */
  for(;;)
  {
    osDelay(10);
  }
  /* USER CODE END ActionTaskFunction */
}

/* SnoreTaskFunction function */
__weak void SnoreTaskFunction(void const * argument)
{
  /* USER CODE BEGIN SnoreTaskFunction */
  /* Infinite loop */
  for(;;)
  {
    osDelay(10);
  }
  /* USER CODE END SnoreTaskFunction */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
/* USER CODE BEGIN Callback 0 */

/* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
	else if (htim->Instance == TIM7) {
    INT_TIM7_Service_Code();
  }
/* USER CODE BEGIN Callback 1 */

/* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */

void SystemClock_Config_LSI(void)
{
//  uint32_t I_delay=0;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

  __HAL_RCC_PWR_CLK_ENABLE();

	
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	
//	RCC->CFGR |=RCC_CFGR_HPRE_DIV4;//GD32F405特殊代码
//	for(I_delay=0xffff;I_delay>0;I_delay--);
	
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSI;
		RCC_OscInitStruct.LSIState = RCC_LSI_ON;	
	
//  RCC->CFGR|=0x00000002;//PLL??????  added by zjp at 20211106

  RCC_OscInitStruct.HSEState = RCC_HSE_ON;  
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
		Error_Code=1;
    
  }
  else
	{
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
		  Error_Code=2;
    
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S|RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.PLLI2S.PLLI2SN = 96;
    PeriphClkInitStruct.PLLI2S.PLLI2SR = 6;
	
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;//LSE
	
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
		  Error_Code=3;
	  }

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
	}
}

void SystemClock_Config_HSE(void)
{
//  uint32_t I_delay=0;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

  __HAL_RCC_PWR_CLK_ENABLE();

	
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	
//	RCC->CFGR |=RCC_CFGR_HPRE_DIV4;//GD32F405特殊代码
//	for(I_delay=0xffff;I_delay>0;I_delay--);
	
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	
  
	
//	RCC->CFGR|=0x00000002;//PLL??????  added by zjp at 20211106

	
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;  
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
		Error_Code=1;
   
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
		Error_Code=2;
    
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S|RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 96;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 6;
	
	PeriphClkInitStruct.RTCClockSelection=RCC_RTCCLKSOURCE_HSE_DIV30;
	
	
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
		Error_Code=3;
   
  }

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

void SystemClock_Config_LSE(void)
{
//  uint32_t I_delay=0;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

  __HAL_RCC_PWR_CLK_ENABLE();

	
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	
//	RCC->CFGR |=RCC_CFGR_HPRE_DIV4;//GD32F405特殊代码
//	for(I_delay=0xffff;I_delay>0;I_delay--);
	if(RTC_Clock_Selector==0)
	{
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	}
  else if(RTC_Clock_Selector==1)
	{
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
		RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	}
	else 
	{
		RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSI;
		RCC_OscInitStruct.LSIState = RCC_LSI_ON;	
	}
  
	
//	RCC->CFGR|=0x00000002;//PLL??????  added by zjp at 20211106

	
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;  
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
		Error_Code=1;
    
  }
  else
	{
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
		  Error_Code=2;
      
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S|RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.PLLI2S.PLLI2SN = 96;
    PeriphClkInitStruct.PLLI2S.PLLI2SR = 6;
	  if(RTC_Clock_Selector==0)
	  {
	    PeriphClkInitStruct.RTCClockSelection=RCC_RTCCLKSOURCE_HSE_DIV30;
	  }
	  else if(RTC_Clock_Selector==1)
	  {
		  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;//LSE
	  }
	  else 
	  {
      PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;//LSE
	  }
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
		  Error_Code=3;
      
    }

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
	}
}

static void MX_RTC_Init_Again(void)
{

  
		
	
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;	
	  if(RTC_Clock_Selector==0)
	  {
	
      hrtc.Init.AsynchPrediv = 127;
	    hrtc.Init.SynchPrediv = 6149;  //255：LSE 249：LSI  (127+1)*(255+1)=32768;32.768K
		
	  }	
	  else if(RTC_Clock_Selector==1)
	  {
	
      hrtc.Init.AsynchPrediv = 127;
	    hrtc.Init.SynchPrediv = 255;  //255：LSE 249：LSI  (127+1)*(255+1)=32768;32.768K
		
	  }	
	
	  else
	  {
		  hrtc.Init.AsynchPrediv = 112;
      hrtc.Init.SynchPrediv = 255;  //255：LSE 249  ((LSI 124+1)*(255+1)=32000;32K
	  }
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
		  

		  Error_Code=12;
 
  
    }
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
		//初始化错误

		LOGO_system_error();

#if 0		
		TimerTick=0;
		while(TimerTick<=100)
	  {
			__NOP;
		}
		TimerTick=0;
		WrokingLed(RED_LED,1);
		while(TimerTick<100)
	  {
			__NOP;
		}
		TimerTick=0;
		WrokingLed(RED_LED,0);
		while(TimerTick<=100)
	  {
			__NOP;
		}
		
		TimerTick=0;
		WrokingLed(RED_LED,1);
		while(TimerTick<=100)
	  {
			__NOP;
		}
		TimerTick=0;
		WrokingLed(RED_LED,0);
		while(TimerTick<=100)
	  {
			__NOP;
		}
		TimerTick=0;
		WrokingLed(RED_LED,1);
		while(TimerTick<=100)
	  {
			__NOP;
		}
#endif
	
	
		if(Error_Code==1)
		{
			//初始化LSE失败，复位不掉电domian      
			TimerTick=0;		
			SET_BIT(RCC->BDCR, RCC_BDCR_BDRST);
		  while(TimerTick<=100)
		  {
			  __NOP;
		  }
			while(TimerTick<=100)
		  {
			  __NOP;
		  }
			TimerTick=0;
			CLEAR_BIT(RCC->BDCR, RCC_BDCR_BDRST);
			while(TimerTick<=100)
		  {
			  __NOP;
		  }
			TimerTick=0;		
			SET_BIT(RCC->BDCR, RCC_BDCR_RTCEN);
		  while(TimerTick<=100)
		  {
			  __NOP;
		  }
			
			Error_Code=0;
			SystemClock_Config_LSE();
			if(Error_Code!=0)
			{	
        Error_Code=0;				
        SystemClock_Config_LSI();
			  RTC_Clock_Selector=2;
			}
//			RCC->CFGR&=0xfffffffc;//PLL??????  added by zjp at 20211106
//			SystemClock_Config_HSE();
//			SET_BIT(RCC->BDCR, RCC_BDCR_BDRST);
//		  while(TimerTick<=100)
//		  {
//			  __NOP;
//		  }
//			RCC->CFGR&=0xfffffffc;//PLL??????  added by zjp at 20211106
//			SystemClock_Config();
		}	
		if(Error_Code==12)
		{
		  TimerTick=0;		
			SET_BIT(RCC->BDCR, RCC_BDCR_BDRST);
		  while(TimerTick<=100)
		  {
			  __NOP;
		  }
			while(TimerTick<=100)
		  {
			  __NOP;
		  }
			TimerTick=0;
			CLEAR_BIT(RCC->BDCR, RCC_BDCR_BDRST);
			while(TimerTick<=100)
		  {
			  __NOP;
		  }
			TimerTick=0;		
			SET_BIT(RCC->BDCR, RCC_BDCR_RTCEN);
		  while(TimerTick<=100)
		  {
			  __NOP;
		  }
			MX_RTC_Init_Again();
		}
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

uint32_t nousedheap_SnoreTaskHandle,MSPvalue,PSPvalue;
uint32_t nousedheap_SnoreTaskHandle;
uint32_t nousedheap_MainTaskHandle;
uint32_t nousedheap_ForthTemialTaskHandle;
uint32_t nousedheap_PositionTaskHandle;
uint32_t nousedheap_ActionTaskHandle;
uint32_t nousedheap_LedFlashTaskHandle;

void WatchStackTaskFunction(void const *argument)
{
	for(;;)
	{
		osDelay(10);
		//监控freeRTOS分配给任务的stack空闲情况
		nousedheap_SnoreTaskHandle = (int32_t)uxTaskGetStackHighWaterMark(SnoreTaskHandle)*4;
		nousedheap_MainTaskHandle = (int32_t)uxTaskGetStackHighWaterMark(MainTaskHandle)*4;
		nousedheap_ForthTemialTaskHandle = (int32_t)uxTaskGetStackHighWaterMark(ForthTemialTaskHandle)*4;
		nousedheap_PositionTaskHandle = (int32_t)uxTaskGetStackHighWaterMark(PositionTaskHandle)*4;
		nousedheap_ActionTaskHandle = (int32_t)uxTaskGetStackHighWaterMark(ActionTaskHandle)*4;
		nousedheap_LedFlashTaskHandle = (int32_t)uxTaskGetStackHighWaterMark(LedFlashTaskHandle)*4;
		MSPvalue = __get_MSP();
		PSPvalue = __get_PSP();
	}	
}

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
