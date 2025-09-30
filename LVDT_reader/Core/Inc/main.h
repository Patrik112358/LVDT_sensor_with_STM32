/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "onebutton.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern uint32_t     tim6_prescaler;
extern uint32_t     tim6_period;
extern uint32_t     tim7_prescaler;
extern uint32_t     tim7_period;
extern __IO uint8_t ubButtonPress;
extern OnebuttonHandler_t onebutton_handle;

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
/**
 * @brief Toggle periods for various blinking modes
 */
#define LED_BLINK_FAST  200
#define LED_BLINK_SLOW  500
#define LED_BLINK_ERROR 1000


/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void LED_On(void);
void LED_Off(void);
void LED_Blinking(uint32_t Period);
void WaitForUserButtonPress(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define tim6_prescaler               24
#define tim6_period                  5
#define tim7_prescaler               249
#define tim7_period                  4
#define USER_BUTTON_Pin              GPIO_PIN_13
#define USER_BUTTON_GPIO_Port        GPIOC
#define USER_BUTTON_EXTI_IRQn        EXTI15_10_IRQn
#define LVDT_SEC1_IN_Pin             GPIO_PIN_1
#define LVDT_SEC1_IN_GPIO_Port       GPIOA
#define LVDT_PRIMARY_DRIVE_Pin       GPIO_PIN_4
#define LVDT_PRIMARY_DRIVE_GPIO_Port GPIOA
#define LED2_Pin                     GPIO_PIN_5
#define LED2_GPIO_Port               GPIOA
#define LVDT_SEC2_IN_Pin             GPIO_PIN_7
#define LVDT_SEC2_IN_GPIO_Port       GPIOA
#define OLED_DC_Pin                  GPIO_PIN_1
#define OLED_DC_GPIO_Port            GPIOB
#define OLED_Res_Pin                 GPIO_PIN_2
#define OLED_Res_GPIO_Port           GPIOB
#define OLED_CS_Pin                  GPIO_PIN_14
#define OLED_CS_GPIO_Port            GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
