/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Examples_LL/DAC/DAC_GenerateWaveform_TriggerHW_Init/Src/main.c
  * @author  MCD Application Team
  * @brief   This example describes how to use the DAC peripheral to generate
  *          a voltage waveform from digital data stream transferred by DMA.
  *          This example is based on the STM32G4xx DAC LL API;
  *          Peripheral initialization done using LL initialization function.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "usart.h"
#include "opamp.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* Definitions of environment analog values */
  /* Value of analog reference voltage (Vref+), connected to analog voltage   */
  /* supply Vdda (unit: mV).                                                  */
  #define VDDA_APPLI                       ((uint32_t)3300)
  
/* Definitions of data related to this example */
/* Full-scale digital value with a resolution of 12 bits (voltage range     */
/* determined by analog voltage references Vref+ and Vref-,                 */
/* refer to reference manual).                                              */
#define DIGITAL_SCALE_12BITS             (__LL_DAC_DIGITAL_SCALE(LL_DAC_RESOLUTION_12B))

/* Definitions of waveform generation values */
/* Waveform generation: parameters of waveform */
/* Waveform amplitude (unit: mV) */
#define WAVEFORM_AMPLITUDE          (VDDA_APPLI)
/* Waveform amplitude (unit: Hz) */
#define WAVEFORM_FREQUENCY          ((uint32_t)1000)
/* Size of array containing DAC waveform samples */
#define WAVEFORM_SAMPLES_SIZE       (sizeof (WaveformSine_12bits_32samples) / sizeof (uint16_t))

/* Waveform generation: parameters of timer (used as DAC trigger) */
/* Timer frequency (unit: Hz). With a timer 16 bits and time base           */
/* freq min 1Hz, range is min=1Hz, max=32kHz.                               */
#define WAVEFORM_TIMER_FREQUENCY                (WAVEFORM_FREQUENCY * WAVEFORM_SAMPLES_SIZE)
/* Timer minimum frequency (unit: Hz), used to calculate frequency range.   */
/* With a timer 16 bits, maximum frequency will be 32000 times this value.  */
#define WAVEFORM_TIMER_FREQUENCY_RANGE_MIN      ((uint32_t)    1)
/* Timer prescaler maximum value (0xFFFF for a timer 16 bits) */
#define WAVEFORM_TIMER_PRESCALER_MAX_VALUE      ((uint32_t)0xFFFF-1)

/**
  * @brief Toggle periods for various blinking modes
  */
#define LED_BLINK_FAST  200
#define LED_BLINK_SLOW  500
#define LED_BLINK_ERROR 1000

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/**
  * @brief  Computation of a data from maximum value on digital scale 12 bits 
  *         (corresponding to voltage Vdda)
  *         to a value on the new scale
  *         (corresponding to voltage defined by WAVEFORM_AMPLITUDE).
  * @param  __DATA_12BITS__: Digital value on scale 12 bits
  * @retval None
  */
#define __WAVEFORM_AMPLITUDE_SCALING(__DATA_12BITS__)                                     \
  (__DATA_12BITS__                                                                        \
   * __LL_DAC_CALC_VOLTAGE_TO_DATA(VDDA_APPLI, WAVEFORM_AMPLITUDE, LL_DAC_RESOLUTION_12B) \
   / __LL_DAC_DIGITAL_SCALE(LL_DAC_RESOLUTION_12B)                                        \
  )

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

__IO uint8_t ubButtonPress = 0;

uint32_t timer_prescaler = 0;                   /* Time base prescaler to have timebase aligned on minimum frequency possible */
uint32_t timer_reload = 0;                      /* Timer reload value in function of timer prescaler to achieve time base period */

const uint16_t WaveformSine_12bits_32samples[] =
{
__WAVEFORM_AMPLITUDE_SCALING(2048),
__WAVEFORM_AMPLITUDE_SCALING(2447),
__WAVEFORM_AMPLITUDE_SCALING(2831),
__WAVEFORM_AMPLITUDE_SCALING(3185),
__WAVEFORM_AMPLITUDE_SCALING(3495),
__WAVEFORM_AMPLITUDE_SCALING(3750),
__WAVEFORM_AMPLITUDE_SCALING(3939),
__WAVEFORM_AMPLITUDE_SCALING(4056),
__WAVEFORM_AMPLITUDE_SCALING(4095),
__WAVEFORM_AMPLITUDE_SCALING(4056),
__WAVEFORM_AMPLITUDE_SCALING(3939),
__WAVEFORM_AMPLITUDE_SCALING(3750),
__WAVEFORM_AMPLITUDE_SCALING(3495),
__WAVEFORM_AMPLITUDE_SCALING(3185),
__WAVEFORM_AMPLITUDE_SCALING(2831),
__WAVEFORM_AMPLITUDE_SCALING(2447),
__WAVEFORM_AMPLITUDE_SCALING(2048),
__WAVEFORM_AMPLITUDE_SCALING(1649),
__WAVEFORM_AMPLITUDE_SCALING(1265),
__WAVEFORM_AMPLITUDE_SCALING(911),
__WAVEFORM_AMPLITUDE_SCALING(601),
__WAVEFORM_AMPLITUDE_SCALING(346),
__WAVEFORM_AMPLITUDE_SCALING(157),
__WAVEFORM_AMPLITUDE_SCALING(40),
__WAVEFORM_AMPLITUDE_SCALING(0),
__WAVEFORM_AMPLITUDE_SCALING(40),
__WAVEFORM_AMPLITUDE_SCALING(157),
__WAVEFORM_AMPLITUDE_SCALING(346),
__WAVEFORM_AMPLITUDE_SCALING(601),
__WAVEFORM_AMPLITUDE_SCALING(911),
__WAVEFORM_AMPLITUDE_SCALING(1265),
__WAVEFORM_AMPLITUDE_SCALING(1649)
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void Activate_DAC(void);
void LED_On(void);
void LED_Off(void);
void LED_Blinking(uint32_t Period);
void WaitForUserButtonPress(void);
void TIM_PrescalerReloadCalculation(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  TIM_PrescalerReloadCalculation();

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DAC1_Init();
  MX_TIM6_Init(timer_prescaler, timer_reload);
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_OPAMP1_Init();
  MX_OPAMP2_Init();
  MX_USART1_UART_Init();
  MX_LPUART1_UART_Init();
  MX_TIM7_Init(tim7_prescaler, tim7_period);
  /* USER CODE BEGIN 2 */

  /* Wait for User push-button press */
  WaitForUserButtonPress();

  /* Turn-off LED4 */
  LED_Off();

  /* Set DMA transfer addresses of source and destination */
  LL_DMA_ConfigAddresses(DMA1,
                         LL_DMA_CHANNEL_3,
                         (uint32_t)&WaveformSine_12bits_32samples,
                         LL_DAC_DMA_GetRegAddr(DAC1, LL_DAC_CHANNEL_1, LL_DAC_DMA_REG_DATA_12BITS_RIGHT_ALIGNED),
                         LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  
  /* Set DMA transfer size */
  LL_DMA_SetDataLength(DMA1,
                       LL_DMA_CHANNEL_3,
                       WAVEFORM_SAMPLES_SIZE);
  
  /* Enable DMA transfer interruption: transfer error */
  LL_DMA_EnableIT_TE(DMA1,
                     LL_DMA_CHANNEL_3);
  
  /* Note: In this example, the only DMA interruption activated is            */
  /*       transfer error.                                                     */
  /*       If needed, DMA interruptions of half of transfer                   */
  /*       and transfer complete can be activated.                            */
  /*       Refer to DMA examples.                                             */
  
  /* Activation of DMA */
  /* Enable the DMA transfer */
  LL_DMA_EnableChannel(DMA1,
                       LL_DMA_CHANNEL_3);

  /* Set DAC mode sample-and-hold timings */
  // LL_DAC_SetSampleAndHoldSampleTime (DAC1, LL_DAC_CHANNEL_1, 0x3FF);
  // LL_DAC_SetSampleAndHoldHoldTime   (DAC1, LL_DAC_CHANNEL_1, 0x3FF);
  // LL_DAC_SetSampleAndHoldRefreshTime(DAC1, LL_DAC_CHANNEL_1, 0xFF);
  
  /* Set the mode for the selected DAC channel */
  // LL_DAC_SetMode(DAC1, LL_DAC_CHANNEL_1, LL_DAC_MODE_NORMAL_OPERATION);
  
  /* Enable DAC channel DMA request */
  LL_DAC_EnableDMAReq(DAC1, LL_DAC_CHANNEL_1);
  
  /* Enable interruption DAC channel1 under-run */
  LL_DAC_EnableIT_DMAUDR1(DAC1);

  /* Activation of Timer */
  /* Enable counter */
  LL_TIM_EnableCounter(TIM6);

  /* Activation of DAC channel */
  Activate_DAC();

  /* Turn-on LED4 */
  LED_On();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  LL_RCC_HSI_Enable();
   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {
  }

  LL_RCC_HSI_SetCalibTrimming(64);
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_4, 75, LL_RCC_PLLR_DIV_2);
  LL_RCC_PLL_EnableDomain_SYS();
  LL_RCC_PLL_Enable();
   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {
  }

  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);
   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }

  /* Insure 1us transition state at intermediate medium speed clock*/
  for (__IO uint32_t i = (170 >> 1); i !=0; i--);

  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_SetSystemCoreClock(150000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
  * @brief  Perform DAC activation procedure to make it ready to generate
  *         a voltage (DAC instance: DAC1).
  * @note   Operations:
  *         - Enable DAC instance channel
  *         - Wait for DAC instance channel startup time
  * @param  None
  * @retval None
  */
void Activate_DAC(void)
{
  __IO uint32_t wait_loop_index = 0;
  
  /* Enable DAC channel */
  LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);
  
  /* Delay for DAC channel voltage settling time from DAC channel startup.    */
  /* Compute number of CPU cycles to wait for, from delay in us.              */
  /* Note: Variable divided by 2 to compensate partially                      */
  /*       CPU processing cycles (depends on compilation optimization).       */
  /* Note: If system core clock frequency is below 200kHz, wait time          */
  /*       is only a few CPU processing cycles.                               */
  wait_loop_index = ((LL_DAC_DELAY_STARTUP_VOLTAGE_SETTLING_US * (SystemCoreClock / (100000 * 2))) / 10);
  while(wait_loop_index != 0)
  {
    wait_loop_index--;
  }
  
  /* Enable DAC channel trigger */
  /* Note: DAC channel conversion can start from trigger enable:              */
  /*       - if DAC channel trigger source is set to SW:                      */
  /*         DAC channel conversion will start after trig order               */
  /*         using function "LL_DAC_TrigSWConversion()".                      */
  /*       - if DAC channel trigger source is set to external trigger         */
  /*         (timer, ...):                                                    */
  /*         DAC channel conversion can start immediately                     */
  /*         (after next trig order from external trigger)                    */
  LL_DAC_EnableTrigger(DAC1, LL_DAC_CHANNEL_1);
}

/**
  * @brief  Function to manage IRQ Handler
  * @param  None
  * @retval None
  */
void UserButton_Callback(void)
{
  /* On the first press on user button, update only user button variable      */
  /* to manage waiting function.                                              */
  if(ubButtonPress == 0)
  {
    /* Update User push-button variable : to be checked in waiting loop in main program */
    ubButtonPress = 1;
  }
}

/**
  * @brief  DMA transfer error callback
  * @note   This function is executed when the transfer error interrupt
  *         is generated during DMA transfer
  * @retval None
  */
void DacDmaTransferError_Callback()
{
  /* Error detected during DMA transfer */
  LED_Blinking(LED_BLINK_ERROR);
}

/**
  * @brief  DAC under-run interruption callback
  * @note   This function is executed when DAC channel under-run error occurs.
  * @retval None
  */
void DacUnderrunError_Callback(void)
{
  /* Note: Disable DAC interruption that caused this error before entering in */
  /*       infinite loop below.                                               */
  
  /* Disable interruption DAC channel1 under-run */
  LL_DAC_DisableIT_DMAUDR1(DAC1);
  
  /* Error from ADC */
  LED_Blinking(LED_BLINK_ERROR);
}

/**
  * @brief  Turn-on LED4.
  * @param  None
  * @retval None
  */
void LED_On(void)
{
  /* Turn LED4 on */
  LL_GPIO_SetOutputPin(LED2_GPIO_Port, LED2_Pin);
}

/**
  * @brief  Turn-off LED4.
  * @param  None
  * @retval None
  */
void LED_Off(void)
{
  /* Turn LED4 off */
  LL_GPIO_ResetOutputPin(LED2_GPIO_Port, LED2_Pin);
}

/**
  * @brief  Set LED4 to Blinking mode for an infinite loop (toggle period based on value provided as input parameter).
  * @param  Period : Period of time (in ms) between each toggling of LED
  *   This parameter can be user defined values. Pre-defined values used in that example are :
  *     @arg LED_BLINK_FAST : Fast Blinking
  *     @arg LED_BLINK_SLOW : Slow Blinking
  *     @arg LED_BLINK_ERROR : Error specific Blinking
  * @retval None
  */
void LED_Blinking(uint32_t Period)
{
  /* Turn LED4 on */
  LL_GPIO_SetOutputPin(LED2_GPIO_Port, LED2_Pin);
  
  /* Toggle IO in an infinite loop */
  while (1)
  {
    LL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    LL_mDelay(Period);
  }
}

/**
  * @brief  Wait for User push-button press to start transfer.
  * @param  None 
  * @retval None
  */
void WaitForUserButtonPress(void)
{
  while (ubButtonPress == 0)
  {
    LL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    LL_mDelay(LED_BLINK_FAST);
  }
  ubButtonPress = 0;
}

/**
  * @brief  Calculate prescaler and reload values for timer.
  * @param  None 
  * @retval None
  */
void TIM_PrescalerReloadCalculation(void)
{
  uint32_t timer_clock_frequency = 0;             /* Timer clock frequency */

  /* Configuration of timer as time base:                                     */ 
  /* Caution: Computation of frequency is done for a timer instance on APB1   */
  /*          (clocked by PCLK1)                                              */
  /* Timer frequency is configured from the following constants:              */
  /* - WAVEFORM_TIMER_FREQUENCY: timer frequency (unit: Hz).                  */
  /* - WAVEFORM_TIMER_FREQUENCY_RANGE_MIN: timer minimum frequency possible   */
  /*   (unit: Hz).                                                            */
  /* Note: Refer to comments at these literals definition for more details.   */
  
  /* Retrieve timer clock source frequency */
  /* If APB1 prescaler is different of 1, timers have a factor x2 on their    */
  /* clock source.                                                            */
  if (LL_RCC_GetAPB1Prescaler() == LL_RCC_APB1_DIV_1)
  {
    timer_clock_frequency = __LL_RCC_CALC_PCLK1_FREQ(SystemCoreClock, LL_RCC_GetAPB1Prescaler());
  }
  else
  {
    timer_clock_frequency = (__LL_RCC_CALC_PCLK1_FREQ(SystemCoreClock, LL_RCC_GetAPB1Prescaler()) * 2);
  }
  
  /* Timer prescaler calculation */
  /* (computation for timer 16 bits, additional + 1 to round the prescaler up) */
  timer_prescaler = ((timer_clock_frequency / (WAVEFORM_TIMER_PRESCALER_MAX_VALUE * WAVEFORM_TIMER_FREQUENCY_RANGE_MIN)) +1);
  /* Timer reload calculation */
  timer_reload = (timer_clock_frequency / (timer_prescaler * WAVEFORM_TIMER_FREQUENCY));
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
 
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
