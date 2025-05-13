#include "lvdt.h"
#include <stdint.h>
#include "dac.h"
#include "debugtools.h"
#include "main.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_dac.h"
#include "stm32g4xx_hal_tim.h"
#include "tim.h"

uint32_t ADC_buffer[ADC_BUFFER_SIZE] = { 0 };


/**
 * @brief  Computation of a data from maximum value on digital scale 12 bits
 *         (corresponding to voltage Vdda)
 *         to a value on the new scale
 *         (corresponding to voltage defined by WAVEFORM_AMPLITUDE).
 * @param  __DATA_12BITS__: Digital value on scale 12 bits
 * @retval None
 */
#define __WAVEFORM_AMPLITUDE_SCALING(__DATA_12BITS__) \
  (__DATA_12BITS__ * __LL_DAC_CALC_VOLTAGE_TO_DATA(VDDA_APPLI, WAVEFORM_AMPLITUDE, LL_DAC_RESOLUTION_12B) \
      / __LL_DAC_DIGITAL_SCALE(LL_DAC_RESOLUTION_12B))

const uint16_t DAC_buffer_sine[DAC_BUFFER_SIZE] = { __WAVEFORM_AMPLITUDE_SCALING(2048),
  __WAVEFORM_AMPLITUDE_SCALING(2447), __WAVEFORM_AMPLITUDE_SCALING(2831), __WAVEFORM_AMPLITUDE_SCALING(3185),
  __WAVEFORM_AMPLITUDE_SCALING(3495), __WAVEFORM_AMPLITUDE_SCALING(3750), __WAVEFORM_AMPLITUDE_SCALING(3939),
  __WAVEFORM_AMPLITUDE_SCALING(4056), __WAVEFORM_AMPLITUDE_SCALING(4095), __WAVEFORM_AMPLITUDE_SCALING(4056),
  __WAVEFORM_AMPLITUDE_SCALING(3939), __WAVEFORM_AMPLITUDE_SCALING(3750), __WAVEFORM_AMPLITUDE_SCALING(3495),
  __WAVEFORM_AMPLITUDE_SCALING(3185), __WAVEFORM_AMPLITUDE_SCALING(2831), __WAVEFORM_AMPLITUDE_SCALING(2447),
  __WAVEFORM_AMPLITUDE_SCALING(2048), __WAVEFORM_AMPLITUDE_SCALING(1649), __WAVEFORM_AMPLITUDE_SCALING(1265),
  __WAVEFORM_AMPLITUDE_SCALING(911), __WAVEFORM_AMPLITUDE_SCALING(601), __WAVEFORM_AMPLITUDE_SCALING(346),
  __WAVEFORM_AMPLITUDE_SCALING(157), __WAVEFORM_AMPLITUDE_SCALING(40), __WAVEFORM_AMPLITUDE_SCALING(0),
  __WAVEFORM_AMPLITUDE_SCALING(40), __WAVEFORM_AMPLITUDE_SCALING(157), __WAVEFORM_AMPLITUDE_SCALING(346),
  __WAVEFORM_AMPLITUDE_SCALING(601), __WAVEFORM_AMPLITUDE_SCALING(911), __WAVEFORM_AMPLITUDE_SCALING(1265),
  __WAVEFORM_AMPLITUDE_SCALING(1649) };

/**
 * @brief  Calculate prescaler and reload values for timer 6 (used by DAC).
 * @param  None
 * @retval None
 */
void TIM6_PrescalerReloadCalculation(void);


void LVDT_Init(void)
{
  TIM6_PrescalerReloadCalculation();
}


/**
 * @brief  Calculate prescaler and reload values for timer.
 * @param  None
 * @retval None
 */
void TIM6_PrescalerReloadCalculation(void)
{
  uint32_t timer_clock_frequency = 0; /* Timer clock frequency */

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
  // if (LL_RCC_GetAPB1Prescaler() == LL_RCC_APB1_DIV_1)
  // {
  //   timer_clock_frequency = __LL_RCC_CALC_PCLK1_FREQ(SystemCoreClock, LL_RCC_GetAPB1Prescaler());
  // }
  // else
  // {
  //   timer_clock_frequency = (__LL_RCC_CALC_PCLK1_FREQ(SystemCoreClock, LL_RCC_GetAPB1Prescaler()) * 2);
  // }
  timer_clock_frequency = HAL_RCC_GetPCLK1Freq();

  /* Timer prescaler calculation */
  /* (computation for timer 16 bits, additional + 1 to round the prescaler up) */
  //   TODO: iirc there needs to be -1 added either here or in the call to init TIM6 (check compared to ver1)
  tim6_prescaler =
      ((timer_clock_frequency / (WAVEFORM_TIMER_PRESCALER_MAX_VALUE * WAVEFORM_TIMER_FREQUENCY_RANGE_MIN)) + 1);
  /* Timer reload calculation */
  tim6_period = (timer_clock_frequency / (tim6_prescaler * WAVEFORM_TIMER_FREQUENCY));
}

void LVDT_Start(void)
{
  DEBUG("LVDT starting...\n");
  /* Start DAC */
  LVDT_Start_DAC();

  //   /* Start ADC */
  //   HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_buffer, ADC_BUFFER_SIZE);
  //   HAL_ADC_Start_DMA(&hadc2, (uint32_t *)ADC_buffer, ADC_BUFFER_SIZE);

  //   /* Start OPAMP */
  //   HAL_OPAMP_Start(&hopamp1);
  //   HAL_OPAMP_Start(&hopamp2);

  //   /* Start TIM6 */
  //   HAL_TIM_Base_Start(&htim6);
  INFO("LVDT started\n");
}

void LVDT_Stop(void)
{
  /* Stop DAC */
  LVDT_Stop_DAC();
  /* Stop ADC */
}

/**
* @brief  Perform DAC activation procedure to make it ready to generate
*         a voltage (DAC instance: DAC1).
* @note   Operations:
*         - Enable DAC instance channel
*         - Wait for DAC instance channel startup time
* @param  None
* @retval None
*/
void LVDT_Start_DAC(void)
{
  /* Start DAC */
  HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *)DAC_buffer_sine, DAC_BUFFER_SIZE, DAC_ALIGN_12B_R);

  __IO uint32_t wait_loop_index = 0;
  /* Delay for DAC channel voltage settling time from DAC channel startup.    */
  /* Compute number of CPU cycles to wait for, from delay in us.              */
  /* Note: Variable divided by 2 to compensate partially                      */
  /*       CPU processing cycles (depends on compilation optimization).       */
  /* Note: If system core clock frequency is below 200kHz, wait time          */
  /*       is only a few CPU processing cycles.                               */
  wait_loop_index = ((LL_DAC_DELAY_STARTUP_VOLTAGE_SETTLING_US * (SystemCoreClock / (100000 * 2))) / 10);
  while(wait_loop_index != 0) { wait_loop_index--; }

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

  HAL_TIM_Base_Start_IT(&htim6);
}

void LVDT_Stop_DAC(void)
{
  /* Stop DAC */
  HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
  /* Disable DAC channel trigger */
  LL_DAC_DisableTrigger(DAC1, LL_DAC_CHANNEL_1); // TODO is this needed?

  HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (uint32_t)0);
  HAL_DAC_Stop(&hdac1, DAC_CHANNEL_1);
}
