/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    cordic.c
  * @brief   This file provides code for the configuration
  *          of the CORDIC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "cordic.h"

/* USER CODE BEGIN 0 */
CORDIC_ConfigTypeDef cordic_active_config;
_Bool                cordic_active_config_valid = 0;
_Bool                CORDIC_configs_are_same(const CORDIC_ConfigTypeDef* config1, const CORDIC_ConfigTypeDef* config2);
#define Q31_MAX ((int32_t)((1LL << 31) - 1))
#define Q31_MIN ((int32_t)(-1LL << 31))
/* USER CODE END 0 */

CORDIC_HandleTypeDef hcordic;

/* CORDIC init function */
void MX_CORDIC_Init(void)
{

  /* USER CODE BEGIN CORDIC_Init 0 */

  /* USER CODE END CORDIC_Init 0 */

  /* USER CODE BEGIN CORDIC_Init 1 */

  /* USER CODE END CORDIC_Init 1 */
  hcordic.Instance = CORDIC;
  if (HAL_CORDIC_Init(&hcordic) != HAL_OK) { Error_Handler(); }
  /* USER CODE BEGIN CORDIC_Init 2 */

  /* USER CODE END CORDIC_Init 2 */
}

void HAL_CORDIC_MspInit(CORDIC_HandleTypeDef* cordicHandle)
{

  if (cordicHandle->Instance == CORDIC) {
    /* USER CODE BEGIN CORDIC_MspInit 0 */

    /* USER CODE END CORDIC_MspInit 0 */
    /* CORDIC clock enable */
    __HAL_RCC_CORDIC_CLK_ENABLE();
    /* USER CODE BEGIN CORDIC_MspInit 1 */

    /* USER CODE END CORDIC_MspInit 1 */
  }
}

void HAL_CORDIC_MspDeInit(CORDIC_HandleTypeDef* cordicHandle)
{

  if (cordicHandle->Instance == CORDIC) {
    /* USER CODE BEGIN CORDIC_MspDeInit 0 */

    /* USER CODE END CORDIC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CORDIC_CLK_DISABLE();
    /* USER CODE BEGIN CORDIC_MspDeInit 1 */

    /* USER CODE END CORDIC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

HAL_StatusTypeDef cordic_compute_sqrt(float input, float* sqrt_out)
{
  extern CORDIC_HandleTypeDef hcordic;
  CORDIC_ConfigTypeDef        config;
  int32_t                     input_q31, output_q31;

  if (!sqrt_out || input < 0.0f) { return HAL_ERROR; }

  // Convert input from float to Q1.31 fixed-point
  input_q31 = (int32_t)(input * (1LL << 31));

  config.Function = CORDIC_FUNCTION_SQUAREROOT;
  config.Precision = CORDIC_PRECISION_6CYCLES;
  config.Scale = CORDIC_SCALE_0;
  config.NbWrite = CORDIC_NBWRITE_1;
  config.NbRead = CORDIC_NBREAD_1;
  if (!cordic_active_config_valid || !CORDIC_configs_are_same(&cordic_active_config, &config)) {
    // Configure CORDIC for square root
    if (HAL_CORDIC_Configure(&hcordic, &config) != HAL_OK) {
      cordic_active_config_valid = 0;
      return HAL_ERROR;
    }
    cordic_active_config = config;
    cordic_active_config_valid = 1;
  }

  if (HAL_CORDIC_Calculate(&hcordic, &input_q31, &output_q31, 1, HAL_MAX_DELAY) != HAL_OK) { return HAL_ERROR; }

  // Convert result from Q1.31 to float
  *sqrt_out = output_q31 / (float)(1LL << 31);

  return HAL_OK;
}

HAL_StatusTypeDef CORDIC_compute_magnitude(float x, float y, float* magnitude_out)
{
  if (!magnitude_out) { return HAL_ERROR; }

  // Compute the magnitude using the CORDIC algorithm
  // float x_q31 = (int32_t)(x * (1LL << 31));
  // float y_q31 = (int32_t)(y * (1LL << 31));
  // float mag_q31 = sqrtf(x_q31 * x_q31 + y_q31 * y_q31);

  if (x < 0.0f) { x *= -1; }
  if (y < 0.0f) { y *= -1; }
  float* larger_operand = (x > y) ? &x : &y;
  float* smaller_operand = (x < y) ? &x : &y;
  *smaller_operand = *smaller_operand / *larger_operand;

  int32_t x_q31 = (int32_t)(*smaller_operand * (1LL << 31));
  int32_t y_q31 = Q31_MAX;

  CORDIC_ConfigTypeDef target_config = {
    .Function = CORDIC_FUNCTION_MODULUS,
    .Precision = CORDIC_PRECISION_6CYCLES,
    .Scale = CORDIC_SCALE_0,
    .NbWrite = CORDIC_NBWRITE_1,
    .NbRead = CORDIC_NBREAD_2,
    .InSize = CORDIC_INSIZE_32BITS,
    .OutSize = CORDIC_OUTSIZE_32BITS,
  };
  if (!cordic_active_config_valid || !CORDIC_configs_are_same(&cordic_active_config, &target_config)) {
    // Configure CORDIC for square root
    if (HAL_CORDIC_Configure(&hcordic, &target_config) != HAL_OK) {
      cordic_active_config_valid = 0;
      return HAL_ERROR;
    }
    cordic_active_config = target_config;
    cordic_active_config_valid = 1;
  }

  // Perform the CORDIC calculation
  int32_t in_buffer[2] = { x_q31, y_q31 };
  int32_t out_buffer = 0;
  if (HAL_CORDIC_Calculate(&hcordic, in_buffer, &out_buffer, 1, HAL_MAX_DELAY) != HAL_OK) { return HAL_ERROR; }

  // Convert result from Q1.31 to float
  *magnitude_out = (float)(out_buffer / (float)(1LL << 31)) * (*larger_operand);

  return HAL_OK;
}


_Bool CORDIC_configs_are_same(const CORDIC_ConfigTypeDef* config1, const CORDIC_ConfigTypeDef* config2)
{
  return (config1->Function == config2->Function) && (config1->Precision == config2->Precision)
      && (config1->Scale == config2->Scale) && (config1->NbWrite == config2->NbWrite)
      && (config1->NbRead == config2->NbRead);
}

/* USER CODE END 1 */
