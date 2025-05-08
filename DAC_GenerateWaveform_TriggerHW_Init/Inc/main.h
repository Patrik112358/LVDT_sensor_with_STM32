/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    Examples_LL/DAC/DAC_GenerateWaveform_TriggerHW_Init/Inc/main.h
  * @author  MCD Application Team
  * @brief   Header for main.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

#include "stm32g4xx_ll_dac.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_crs.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_exti.h"
#include "stm32g4xx_ll_cortex.h"
#include "stm32g4xx_ll_utils.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* IRQ Handler treatment */
void UserButton_Callback(void);
void DacDmaTransferError_Callback(void);
void DacUnderrunError_Callback(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define tim7_prescaler 0
#define tim7_period 1250
#define USER_BUTTON_Pin LL_GPIO_PIN_13
#define USER_BUTTON_GPIO_Port GPIOC
#define USER_BUTTON_EXTI_IRQn EXTI15_10_IRQn
#define LVDT_SEC1_IN_Pin LL_GPIO_PIN_1
#define LVDT_SEC1_IN_GPIO_Port GPIOA
#define LVDT_PRIMARY_DRIVE_Pin LL_GPIO_PIN_4
#define LVDT_PRIMARY_DRIVE_GPIO_Port GPIOA
#define LED2_Pin LL_GPIO_PIN_5
#define LED2_GPIO_Port GPIOA
#define LVDT_SEC2_IN_Pin LL_GPIO_PIN_7
#define LVDT_SEC2_IN_GPIO_Port GPIOA
#define OLED_DC_Pin LL_GPIO_PIN_10
#define OLED_DC_GPIO_Port GPIOB
#define OLED_RES_Pin LL_GPIO_PIN_8
#define OLED_RES_GPIO_Port GPIOA
#define OLED_SPI_SCK_Pin LL_GPIO_PIN_3
#define OLED_SPI_SCK_GPIO_Port GPIOB
#define OLED_CS_Pin LL_GPIO_PIN_4
#define OLED_CS_GPIO_Port GPIOB
#define OLED_SPI_MOSI_Pin LL_GPIO_PIN_5
#define OLED_SPI_MOSI_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
