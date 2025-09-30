#include "goertzel.h"
#include "debugtools.h"
#include <math.h>
#include <stdint.h>
#include "cordic.h"
#include "lvdt.h"
#include "stm32g4xx_hal_def.h"
#include "stm32g4xx_ll_adc.h"
int goertzel_magnitude(float *amplitude1, float *amplitude2, uint32_t *data, uint32_t data_size, float sample_rate,
    float target_frequency)
{
  float omega = 2.0f * M_PI * target_frequency / sample_rate;
  float sine = sin(omega);
  float cosine = cos(omega);
  float coeff = 2.0f * cosine;

  float adc1_q0 = 0.0f;
  float adc1_q1 = 0.0f;
  float adc1_q2 = 0.0f;

  float adc2_q0 = 0.0f;
  float adc2_q1 = 0.0f;
  float adc2_q2 = 0.0f;

  // Process all samples
  for (uint32_t i = 0; i < data_size; i++) {
    uint16_t adc1 = data[i] & 0xFFF; // Extract ADC1 value
    uint16_t adc2 = (data[i] >> 16) & 0xFFF; // Extract ADC2 value
    float    adc1_scaled = __LL_ADC_CALC_DATA_TO_VOLTAGE(VDDA_APPLI, (float)adc1, LL_ADC_RESOLUTION_12B);
    float    adc2_scaled = __LL_ADC_CALC_DATA_TO_VOLTAGE(VDDA_APPLI, (float)adc2, LL_ADC_RESOLUTION_12B);

    // Process ADC1
    adc1_q0 = coeff * adc1_q1 - adc1_q2 + (float)adc1_scaled;
    adc1_q2 = adc1_q1;
    adc1_q1 = adc1_q0;

    // Process ADC2
    adc2_q0 = coeff * adc2_q1 - adc2_q2 + (float)adc2_scaled;
    adc2_q2 = adc2_q1;
    adc2_q1 = adc2_q0;
    // Uncomment the following lines to see the intermediate values
    // printf("ADC1: q0: %f, q1: %f, q2: %f\n", adc1_q0, adc1_q1, adc1_q2);
    // printf("ADC2: q0: %f, q1: %f, q2: %f\n", adc2_q0, adc2_q1, adc2_q2);
  }

  // Calculate magnitude
  float adc1_real = adc1_q1 - adc1_q2 * cosine;
  float adc1_imag = adc1_q2 * sine;

  float adc2_real = adc2_q1 - adc2_q2 * cosine;
  float adc2_imag = adc2_q2 * sine;

  //   HAL_StatusTypeDef ret1 = cordic_compute_sqrt(adc1_real * adc1_real + adc1_imag * adc1_imag, amplitude1);
  //   HAL_StatusTypeDef ret2 = cordic_compute_sqrt(adc2_real * adc2_real + adc2_imag * adc2_imag, amplitude2);
  HAL_StatusTypeDef ret1 = CORDIC_compute_magnitude(adc1_real, adc1_imag, amplitude1);
  HAL_StatusTypeDef ret2 = CORDIC_compute_magnitude(adc2_real, adc2_imag, amplitude2);
  //   *amplitude1 = sqrtf(adc1_real * adc1_real + adc1_imag * adc1_imag);
  //   *amplitude2 = sqrtf(adc2_real * adc2_real + adc2_imag * adc2_imag);
  if (HAL_OK != ret1 || HAL_OK != ret2) { WARN_PRINT("Cordic error: %d %d\n", ret1, ret2); }
  return (ret1 == HAL_OK && ret2 == HAL_OK) ? 0 : -1;
}
