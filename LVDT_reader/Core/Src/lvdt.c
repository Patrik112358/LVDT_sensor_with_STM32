#include "lvdt.h"
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "adc.h"
#include "dac.h"
#include "debugtools.h"
#include "goertzel.h"
#include "main.h"
#include "opamp.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_adc.h"
#include "stm32g4xx_hal_adc_ex.h"
#include "stm32g4xx_hal_dac.h"
#include "stm32g4xx_hal_def.h"
#include "stm32g4xx_hal_tim.h"
#include "stm32g4xx_ll_adc.h"
#include "tim.h"
#include "user_interface.h"

uint32_t      ADC_buffer[ADC_BUFFER_SIZE] = { 0 };
__IO uint32_t n_halfbuffers_sampled = 0;
uint32_t      tick_sampling_start = 0;
uint32_t      n_halfbuffers_processed = 0;
uint32_t      ADC_halfbuffer_for_processing[ADC_HALF_BUFFER_SIZE] = { 0 };
__IO _Bool    buffer_being_processed = 0;
__IO _Bool    buffer_ready_for_processing = 0;
typedef struct {
  float primary_drive_frequency;
  // float primary_drive_frequency_sin;
  // float primary_drive_frequency_cos;
  float secondary_sampling_frequency;
} LVDT_params_t;
LVDT_params_t lvdt_params = { 0 };

/**
 * @brief  Computation of a data from maximum value on digital scale 12 bits
 *         (corresponding to voltage Vdda)
 *         to a value on the new scale
 *         (corresponding to voltage defined by WAVEFORM_AMPLITUDE).
 * @param  __DATA_12BITS__: Digital value on scale 12 bits
 * @retval None
 */
#define __WAVEFORM_AMPLITUDE_SCALING(__DATA_12BITS__)                                                                  \
  (__DATA_12BITS__ * __LL_DAC_CALC_VOLTAGE_TO_DATA(VDDA_APPLI, WAVEFORM_AMPLITUDE, LL_DAC_RESOLUTION_12B)              \
          / __LL_DAC_DIGITAL_SCALE(LL_DAC_RESOLUTION_12B)                                                              \
      + __LL_DAC_CALC_VOLTAGE_TO_DATA(VDDA_APPLI, WAVEFORM_OFFSET, LL_DAC_RESOLUTION_12B))

/* python3.12
import math
A = 0xFFF
for i in range(100):
    print(f"__WAVEFORM_AMPLITUDE_SCALING({int(A*(math.cos(i/100*2*math.pi)+1)/2)}),")
*/
const uint16_t DAC_buffer_sine[DAC_BUFFER_SIZE] = { __WAVEFORM_AMPLITUDE_SCALING(4095),
  __WAVEFORM_AMPLITUDE_SCALING(4090), __WAVEFORM_AMPLITUDE_SCALING(4078), __WAVEFORM_AMPLITUDE_SCALING(4058),
  __WAVEFORM_AMPLITUDE_SCALING(4030), __WAVEFORM_AMPLITUDE_SCALING(3994), __WAVEFORM_AMPLITUDE_SCALING(3951),
  __WAVEFORM_AMPLITUDE_SCALING(3900), __WAVEFORM_AMPLITUDE_SCALING(3841), __WAVEFORM_AMPLITUDE_SCALING(3776),
  __WAVEFORM_AMPLITUDE_SCALING(3703), __WAVEFORM_AMPLITUDE_SCALING(3625), __WAVEFORM_AMPLITUDE_SCALING(3540),
  __WAVEFORM_AMPLITUDE_SCALING(3449), __WAVEFORM_AMPLITUDE_SCALING(3352), __WAVEFORM_AMPLITUDE_SCALING(3250),
  __WAVEFORM_AMPLITUDE_SCALING(3144), __WAVEFORM_AMPLITUDE_SCALING(3033), __WAVEFORM_AMPLITUDE_SCALING(2919),
  __WAVEFORM_AMPLITUDE_SCALING(2801), __WAVEFORM_AMPLITUDE_SCALING(2680), __WAVEFORM_AMPLITUDE_SCALING(2556),
  __WAVEFORM_AMPLITUDE_SCALING(2431), __WAVEFORM_AMPLITUDE_SCALING(2304), __WAVEFORM_AMPLITUDE_SCALING(2176),
  __WAVEFORM_AMPLITUDE_SCALING(2047), __WAVEFORM_AMPLITUDE_SCALING(1918), __WAVEFORM_AMPLITUDE_SCALING(1790),
  __WAVEFORM_AMPLITUDE_SCALING(1663), __WAVEFORM_AMPLITUDE_SCALING(1538), __WAVEFORM_AMPLITUDE_SCALING(1414),
  __WAVEFORM_AMPLITUDE_SCALING(1293), __WAVEFORM_AMPLITUDE_SCALING(1175), __WAVEFORM_AMPLITUDE_SCALING(1061),
  __WAVEFORM_AMPLITUDE_SCALING(950), __WAVEFORM_AMPLITUDE_SCALING(844), __WAVEFORM_AMPLITUDE_SCALING(742),
  __WAVEFORM_AMPLITUDE_SCALING(645), __WAVEFORM_AMPLITUDE_SCALING(554), __WAVEFORM_AMPLITUDE_SCALING(469),
  __WAVEFORM_AMPLITUDE_SCALING(391), __WAVEFORM_AMPLITUDE_SCALING(318), __WAVEFORM_AMPLITUDE_SCALING(253),
  __WAVEFORM_AMPLITUDE_SCALING(194), __WAVEFORM_AMPLITUDE_SCALING(143), __WAVEFORM_AMPLITUDE_SCALING(100),
  __WAVEFORM_AMPLITUDE_SCALING(64), __WAVEFORM_AMPLITUDE_SCALING(36), __WAVEFORM_AMPLITUDE_SCALING(16),
  __WAVEFORM_AMPLITUDE_SCALING(4), __WAVEFORM_AMPLITUDE_SCALING(0), __WAVEFORM_AMPLITUDE_SCALING(4),
  __WAVEFORM_AMPLITUDE_SCALING(16), __WAVEFORM_AMPLITUDE_SCALING(36), __WAVEFORM_AMPLITUDE_SCALING(64),
  __WAVEFORM_AMPLITUDE_SCALING(100), __WAVEFORM_AMPLITUDE_SCALING(143), __WAVEFORM_AMPLITUDE_SCALING(194),
  __WAVEFORM_AMPLITUDE_SCALING(253), __WAVEFORM_AMPLITUDE_SCALING(318), __WAVEFORM_AMPLITUDE_SCALING(391),
  __WAVEFORM_AMPLITUDE_SCALING(469), __WAVEFORM_AMPLITUDE_SCALING(554), __WAVEFORM_AMPLITUDE_SCALING(645),
  __WAVEFORM_AMPLITUDE_SCALING(742), __WAVEFORM_AMPLITUDE_SCALING(844), __WAVEFORM_AMPLITUDE_SCALING(950),
  __WAVEFORM_AMPLITUDE_SCALING(1061), __WAVEFORM_AMPLITUDE_SCALING(1175), __WAVEFORM_AMPLITUDE_SCALING(1293),
  __WAVEFORM_AMPLITUDE_SCALING(1414), __WAVEFORM_AMPLITUDE_SCALING(1538), __WAVEFORM_AMPLITUDE_SCALING(1663),
  __WAVEFORM_AMPLITUDE_SCALING(1790), __WAVEFORM_AMPLITUDE_SCALING(1918), __WAVEFORM_AMPLITUDE_SCALING(2047),
  __WAVEFORM_AMPLITUDE_SCALING(2176), __WAVEFORM_AMPLITUDE_SCALING(2304), __WAVEFORM_AMPLITUDE_SCALING(2431),
  __WAVEFORM_AMPLITUDE_SCALING(2556), __WAVEFORM_AMPLITUDE_SCALING(2680), __WAVEFORM_AMPLITUDE_SCALING(2801),
  __WAVEFORM_AMPLITUDE_SCALING(2919), __WAVEFORM_AMPLITUDE_SCALING(3033), __WAVEFORM_AMPLITUDE_SCALING(3144),
  __WAVEFORM_AMPLITUDE_SCALING(3250), __WAVEFORM_AMPLITUDE_SCALING(3352), __WAVEFORM_AMPLITUDE_SCALING(3449),
  __WAVEFORM_AMPLITUDE_SCALING(3540), __WAVEFORM_AMPLITUDE_SCALING(3625), __WAVEFORM_AMPLITUDE_SCALING(3703),
  __WAVEFORM_AMPLITUDE_SCALING(3776), __WAVEFORM_AMPLITUDE_SCALING(3841), __WAVEFORM_AMPLITUDE_SCALING(3900),
  __WAVEFORM_AMPLITUDE_SCALING(3951), __WAVEFORM_AMPLITUDE_SCALING(3994), __WAVEFORM_AMPLITUDE_SCALING(4030),
  __WAVEFORM_AMPLITUDE_SCALING(4058), __WAVEFORM_AMPLITUDE_SCALING(4078), __WAVEFORM_AMPLITUDE_SCALING(4090) };
// const uint16_t DAC_buffer_sine[DAC_BUFFER_SIZE] = { __WAVEFORM_AMPLITUDE_SCALING(2048),
//   __WAVEFORM_AMPLITUDE_SCALING(2447), __WAVEFORM_AMPLITUDE_SCALING(2831), __WAVEFORM_AMPLITUDE_SCALING(3185),
//   __WAVEFORM_AMPLITUDE_SCALING(3495), __WAVEFORM_AMPLITUDE_SCALING(3750), __WAVEFORM_AMPLITUDE_SCALING(3939),
//   __WAVEFORM_AMPLITUDE_SCALING(4056), __WAVEFORM_AMPLITUDE_SCALING(4095), __WAVEFORM_AMPLITUDE_SCALING(4056),
//   __WAVEFORM_AMPLITUDE_SCALING(3939), __WAVEFORM_AMPLITUDE_SCALING(3750), __WAVEFORM_AMPLITUDE_SCALING(3495),
//   __WAVEFORM_AMPLITUDE_SCALING(3185), __WAVEFORM_AMPLITUDE_SCALING(2831), __WAVEFORM_AMPLITUDE_SCALING(2447),
//   __WAVEFORM_AMPLITUDE_SCALING(2048), __WAVEFORM_AMPLITUDE_SCALING(1649), __WAVEFORM_AMPLITUDE_SCALING(1265),
//   __WAVEFORM_AMPLITUDE_SCALING(911), __WAVEFORM_AMPLITUDE_SCALING(601), __WAVEFORM_AMPLITUDE_SCALING(346),
//   __WAVEFORM_AMPLITUDE_SCALING(157), __WAVEFORM_AMPLITUDE_SCALING(40), __WAVEFORM_AMPLITUDE_SCALING(0),
//   __WAVEFORM_AMPLITUDE_SCALING(40), __WAVEFORM_AMPLITUDE_SCALING(157), __WAVEFORM_AMPLITUDE_SCALING(346),
//   __WAVEFORM_AMPLITUDE_SCALING(601), __WAVEFORM_AMPLITUDE_SCALING(911), __WAVEFORM_AMPLITUDE_SCALING(1265),
//   __WAVEFORM_AMPLITUDE_SCALING(1649) };

/**
 * @brief  Calculate prescaler and reload values for timer 6 (used by DAC).
 * @param  None
 * @retval None
 */
// void TIM6_PrescalerReloadCalculation(void);

void LVDT_Push_info_to_UI(void);

void LVDT_Init(void)
{
  HAL_ADC_MspInit(&hadc1); // TODO: check if this is needed (both of these rows)
  HAL_ADC_MspInit(&hadc2); // TODO: check if this is needed (both of these rows)
  // TIM6_PrescalerReloadCalculation(); // TODO: do not recalculate. Timer params were set manually
}


/**
 * @brief  Calculate prescaler and reload values for timer.
 * @param  None
 * @retval None
 */
// void TIM6_PrescalerReloadCalculation(void)
// {
//   uint32_t timer_clock_frequency = 0; /* Timer clock frequency */

//   /* Configuration of timer as time base:                                     */
//   /* Caution: Computation of frequency is done for a timer instance on APB1   */
//   /*          (clocked by PCLK1)                                              */
//   /* Timer frequency is configured from the following constants:              */
//   /* - WAVEFORM_TIMER_FREQUENCY: timer frequency (unit: Hz).                  */
//   /* - WAVEFORM_TIMER_FREQUENCY_RANGE_MIN: timer minimum frequency possible   */
//   /*   (unit: Hz).                                                            */
//   /* Note: Refer to comments at these literals definition for more details.   */

//   /* Retrieve timer clock source frequency */
//   /* If APB1 prescaler is different of 1, timers have a factor x2 on their    */
//   /* clock source.                                                            */
//   // if (LL_RCC_GetAPB1Prescaler() == LL_RCC_APB1_DIV_1)
//   // {
//   //   timer_clock_frequency = __LL_RCC_CALC_PCLK1_FREQ(SystemCoreClock, LL_RCC_GetAPB1Prescaler());
//   // }
//   // else
//   // {
//   //   timer_clock_frequency = (__LL_RCC_CALC_PCLK1_FREQ(SystemCoreClock, LL_RCC_GetAPB1Prescaler()) * 2);
//   // }
//   timer_clock_frequency = HAL_RCC_GetPCLK1Freq();

//   /* Timer prescaler calculation */
//   /* (computation for timer 16 bits, additional + 1 to round the prescaler up) */
//   //   TODO: iirc there needs to be -1 added either here or in the call to init TIM6 (check compared to ver1)
//   tim6_prescaler =
//       ((timer_clock_frequency / (WAVEFORM_TIMER_PRESCALER_MAX_VALUE * WAVEFORM_TIMER_FREQUENCY_RANGE_MIN)) + 1);
//   /* Timer reload calculation */
//   tim6_period = (timer_clock_frequency / (tim6_prescaler * WAVEFORM_TIMER_FREQUENCY));
// }

void LVDT_Start(void)
{
  DEBUG_PRINT("LVDT starting...\n");
  /* Start DAC */
  LVDT_Start_DAC();
  /* Start ADC */
  LVDT_Start_ADC();
  //   /* Start ADC */
  //   HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_buffer, ADC_BUFFER_SIZE);
  //   HAL_ADC_Start_DMA(&hadc2, (uint32_t *)ADC_buffer, ADC_BUFFER_SIZE);

  //   /* Start OPAMP */
  //   HAL_OPAMP_Start(&hopamp1);
  //   HAL_OPAMP_Start(&hopamp2);

  //   /* Start TIM6 */
  //   HAL_TIM_Base_Start(&htim6);
  INFO_PRINT("LVDT started\n");
  lvdt_params.primary_drive_frequency = LVDT_GetPrimaryDriveFrequency();
  // lvdt_params.primary_drive_frequency_sin = sin(2 * M_PI * lvdt_params.primary_drive_frequency);
  // lvdt_params.primary_drive_frequency_cos = cos(2 * M_PI * lvdt_params.primary_drive_frequency);
  UI_SetPrimaryDriveFrequency(lvdt_params.primary_drive_frequency);
  lvdt_params.secondary_sampling_frequency = LVDT_GetSecondarySamplingFrequency();
  UI_SetSecondarySamplingFrequency(lvdt_params.secondary_sampling_frequency);
  UI_Update();
}

float LVDT_GetPrimaryDriveFrequency(void)
{
  return Get_Timer_Frequency(&htim6) / (float)DAC_BUFFER_SIZE;
}

float LVDT_GetSecondarySamplingFrequency(void)
{
  return Get_Timer_Frequency(&htim7);
}

void LVDT_Stop(void)
{
  /* Stop DAC */
  LVDT_Stop_DAC();
  /* Stop ADC */
  LVDT_Stop_ADC();
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
  while (wait_loop_index != 0) { wait_loop_index--; }

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


void LVDT_Start_ADC(void)
{
  // ADC1 and ADC2 will work in dual regular simultaneous mode.
  // Therefore DMA1Ch1 will carry data from both at once.
  HAL_StatusTypeDef ret = HAL_OK;
  HAL_ADCEx_MultiModeStart_DMA(&hadc1, ADC_buffer, ADC_BUFFER_SIZE);
  if (ret != HAL_OK) { ERROR_PRINT("ADC1+2 start failed (HAL_ADCEx_MultiModeStart_DMA)\n"); }
  ret = ADC_Enable(&hadc1);
  if (ret != HAL_OK) { ERROR_PRINT("ADC1 enable failed\n"); }
  ret = ADC_Enable(&hadc2);
  if (ret != HAL_OK) { ERROR_PRINT("ADC2 enable failed\n"); }
  // HAL_ADC_Start_DMA(&hadc2, ADC_buffer, ADC_BUFFER_SIZE);
  ret = HAL_OPAMP_Start(&hopamp1);
  if (ret != HAL_OK) { ERROR_PRINT("OPAMP1 start failed\n"); }
  ret = HAL_OPAMP_Start(&hopamp2);
  if (ret != HAL_OK) { ERROR_PRINT("OPAMP2 start failed\n"); }
  ret = HAL_TIM_Base_Start_IT(&htim7);
  if (ret != HAL_OK) { ERROR_PRINT("TIM7 start failed\n"); }
}

void LVDT_Stop_ADC(void)
{
  HAL_ADCEx_MultiModeStop_DMA(&hadc1);
  // HAL_ADC_Stop_DMA(&hadc2);
  HAL_OPAMP_Stop(&hopamp1);
  HAL_OPAMP_Stop(&hopamp2);
  HAL_TIM_Base_Stop_IT(&htim7);
}

void HalfbufferConvCpltCallback(_Bool second_half)
{
  n_halfbuffers_sampled++;
  if (buffer_being_processed) { // TODO use locks?
    return;
  }

  memcpy(ADC_halfbuffer_for_processing, &ADC_buffer[second_half ? ADC_HALF_BUFFER_SIZE : 0],
      ADC_HALF_BUFFER_SIZE * sizeof(uint32_t));
  buffer_ready_for_processing = 1;
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
  // DEBUG_PRINT("ADC half conversion complete\n");
  HalfbufferConvCpltCallback(0);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  // DEBUG_PRINT("ADC conversion complete\n");
  HalfbufferConvCpltCallback(1);
}

void LVDT_ProcessData(void)
{
  if (!buffer_ready_for_processing) {
    DEBUG_PRINT("No data to process\n");
    return;
  }
  buffer_being_processed = 1;
  buffer_ready_for_processing = 0;
  float a1 = 0;
  float a2 = 0;

  goertzel_magnitude(&a1, &a2, ADC_halfbuffer_for_processing, ADC_HALF_BUFFER_SIZE,
      lvdt_params.secondary_sampling_frequency, lvdt_params.primary_drive_frequency);

  // a1 = __LL_ADC_CALC_DATA_TO_VOLTAGE(VDDA_APPLI, a1, LL_ADC_RESOLUTION_12B);
  // a2 = __LL_ADC_CALC_DATA_TO_VOLTAGE(VDDA_APPLI, a2, LL_ADC_RESOLUTION_12B);
  buffer_being_processed = 0;
  n_halfbuffers_processed++;

  UI_SetSec1Amplitude(a1);
  UI_SetSec2Amplitude(a2);
  uint32_t current_tick = HAL_GetTick();

  UI_SetNHalfbuffersSkipped((n_halfbuffers_sampled - n_halfbuffers_processed));
  UI_SetNHalfbuffersSkippedPerSecond(
      (float)(n_halfbuffers_sampled - n_halfbuffers_processed) / ((current_tick - tick_sampling_start) / 1000.0f));

  float ratio = (a1 - a2) / (a1 + a2);
  UI_SetRatio(ratio);
  // UI_SetPosition(a1 / a2);
  UI_Update();
}


void LVDT_Push_info_to_UI(void)
{
}
