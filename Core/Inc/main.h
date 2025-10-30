/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s8;

typedef const int32_t sc32;  /*!< Read Only */
typedef const int16_t sc16;  /*!< Read Only */
typedef const int8_t  sc8;   /*!< Read Only */

typedef __IO int32_t  vs32;
typedef __IO int16_t  vs16;
typedef __IO int8_t   vs8;

typedef __I int32_t vsc32;  /*!< Read Only */
typedef __I int16_t vsc16;  /*!< Read Only */
typedef __I int8_t  vsc8;   /*!< Read Only */

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef const uint32_t uc32;  /*!< Read Only */
typedef const uint16_t uc16;  /*!< Read Only */
typedef const uint8_t  uc8;   /*!< Read Only */

typedef __IO uint32_t vu32;
typedef __IO uint16_t vu16;
typedef __IO uint8_t  vu8;

typedef __I uint32_t vuc32;  /*!< Read Only */
typedef __I uint16_t vuc16;  /*!< Read Only */
typedef __I uint8_t  vuc8;   /*!< Read Only */
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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EVENT_COUNT (PWM_ARR*95/100)
#define PWM_HALF (PWM_ARR>>2)
#define TIM3_ARR 1000
#define TIM3_PSC 170
#define DEAD_TIME 51
#define PWM_ARR (4250)
#define R_EN_Pin GPIO_PIN_13
#define R_EN_GPIO_Port GPIOC
#define LCD_RES_Pin GPIO_PIN_14
#define LCD_RES_GPIO_Port GPIOC
#define LCD_DC_Pin GPIO_PIN_15
#define LCD_DC_GPIO_Port GPIOC
#define VA_Pin GPIO_PIN_0
#define VA_GPIO_Port GPIOC
#define VB_Pin GPIO_PIN_1
#define VB_GPIO_Port GPIOC
#define VC_Pin GPIO_PIN_2
#define VC_GPIO_Port GPIOC
#define IC_Pin GPIO_PIN_0
#define IC_GPIO_Port GPIOA
#define IB_Pin GPIO_PIN_1
#define IB_GPIO_Port GPIOA
#define IA_Pin GPIO_PIN_2
#define IA_GPIO_Port GPIOA
#define IBUS_Pin GPIO_PIN_3
#define IBUS_GPIO_Port GPIOA
#define RGB_Pin GPIO_PIN_5
#define RGB_GPIO_Port GPIOA
#define ABZ_A_Pin GPIO_PIN_6
#define ABZ_A_GPIO_Port GPIOA
#define ABZ_B_Pin GPIO_PIN_7
#define ABZ_B_GPIO_Port GPIOA
#define ADSPE_Pin GPIO_PIN_4
#define ADSPE_GPIO_Port GPIOC
#define VBUS_Pin GPIO_PIN_5
#define VBUS_GPIO_Port GPIOC
#define ABZ_Z_Pin GPIO_PIN_0
#define ABZ_Z_GPIO_Port GPIOB
#define ABZ_Z_EXTI_IRQn EXTI0_IRQn
#define NTC1_Pin GPIO_PIN_1
#define NTC1_GPIO_Port GPIOB
#define MOTOR_COM_Pin GPIO_PIN_2
#define MOTOR_COM_GPIO_Port GPIOB
#define NTC3_Pin GPIO_PIN_12
#define NTC3_GPIO_Port GPIOB
#define KEY4_Pin GPIO_PIN_6
#define KEY4_GPIO_Port GPIOC
#define KEY3_Pin GPIO_PIN_7
#define KEY3_GPIO_Port GPIOC
#define KEY2_Pin GPIO_PIN_8
#define KEY2_GPIO_Port GPIOC
#define KEY1_Pin GPIO_PIN_9
#define KEY1_GPIO_Port GPIOC
#define SPI3_CS_Pin GPIO_PIN_15
#define SPI3_CS_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_2
#define SPI1_CS_GPIO_Port GPIOD
#define LCD_CS_Pin GPIO_PIN_6
#define LCD_CS_GPIO_Port GPIOB
#define LCD_BLK_Pin GPIO_PIN_7
#define LCD_BLK_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
