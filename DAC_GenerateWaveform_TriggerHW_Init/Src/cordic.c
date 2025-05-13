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
#include "debugtools.h"

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
  if (HAL_CORDIC_Init(&hcordic) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CORDIC_Init 2 */

  /* USER CODE END CORDIC_Init 2 */

}

void HAL_CORDIC_MspInit(CORDIC_HandleTypeDef* cordicHandle)
{

  if(cordicHandle->Instance==CORDIC)
  {
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

  if(cordicHandle->Instance==CORDIC)
  {
  /* USER CODE BEGIN CORDIC_MspDeInit 0 */

  /* USER CODE END CORDIC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CORDIC_CLK_DISABLE();
  /* USER CODE BEGIN CORDIC_MspDeInit 1 */

  /* USER CODE END CORDIC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/**
 * @brief Computes sine and cosine of an angle using STM32 CORDIC in fixed-point mode.
 * 
 * @param[in]  angle_rad  Input angle in radians.
 * @param[out] sin_out    Pointer to store sine result (can be NULL).
 * @param[out] cos_out    Pointer to store cosine result (can be NULL).
 * 
 * @retval HAL_OK     Success.
 * @retval HAL_ERROR  HAL configuration or calculation error.
 */
HAL_StatusTypeDef cordic_compute_sin_cos(float angle_rad, float* sin_out, float* cos_out)
{
    CORDIC_ConfigTypeDef config;
    int32_t input_q31, output_q31[2];
    
    DEBUG_EXP("%f\n", angle_rad);
    DEBUG_EXP("%p\n", (void*)sin_out);
    DEBUG_EXP("%p\n", (void*)cos_out);
    
    // Convert angle from float to Q1.31 fixed-point
    input_q31 = (int32_t)(angle_rad * (1LL << 31) / PI);  // CORDIC expects angle in [−π, π] mapped to [−1, 1)
    DEBUG_EXP("%d\n", input_q31);

    // Configure CORDIC for sine/cosine
    config.Function  = CORDIC_FUNCTION_SINE;
    config.Precision = CORDIC_PRECISION_6CYCLES;
    config.Scale     = CORDIC_SCALE_0;
    config.NbWrite   = CORDIC_NBWRITE_2;
    config.NbRead    = CORDIC_NBREAD_1;

    if (HAL_CORDIC_Configure(&hcordic, &config) != HAL_OK)
        return HAL_ERROR;

    if (HAL_CORDIC_Calculate(&hcordic, &input_q31, output_q31, 1, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;

    // Convert results from Q1.31 to float
    if (sin_out) {*sin_out = output_q31[0] / (float)(1LL << 31);}
    if (cos_out) {*cos_out = output_q31[1] / (float)(1LL << 31);}

    return HAL_OK;
}

/**
 * @brief Computes the square root of a non-negative number using STM32 CORDIC in fixed-point mode.
 * 
 * @param[in]  input      Positive float value to compute the square root of.
 * @param[out] sqrt_out   Pointer to store the square root result (must not be NULL).
 * 
 * @retval HAL_OK     Success.
 * @retval HAL_ERROR  HAL configuration or calculation error.
 */
HAL_StatusTypeDef cordic_compute_sqrt(float input, float* sqrt_out)
{
    extern CORDIC_HandleTypeDef hcordic;
    CORDIC_ConfigTypeDef config;
    int32_t input_q31, output_q31;

    if (!sqrt_out || input < 0.0f)
        return HAL_ERROR;

    // Convert input from float to Q1.31 fixed-point
    input_q31 = (int32_t)(input * (1LL << 31));

    // Configure CORDIC for square root
    config.Function  = CORDIC_FUNCTION_SQUAREROOT;
    config.Precision = CORDIC_PRECISION_6CYCLES;
    config.Scale     = CORDIC_SCALE_0;
    config.NbWrite   = CORDIC_NBWRITE_1;
    config.NbRead    = CORDIC_NBREAD_1;

    if (HAL_CORDIC_Configure(&hcordic, &config) != HAL_OK)
        return HAL_ERROR;

    if (HAL_CORDIC_Calculate(&hcordic, &input_q31, &output_q31, 1, HAL_MAX_DELAY) != HAL_OK)
        return HAL_ERROR;

    // Convert result from Q1.31 to float
    *sqrt_out = output_q31 / (float)(1LL << 31);

    return HAL_OK;
}

/* USER CODE END 1 */
