/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32h7xx_hal.h"

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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_RESET_Pin GPIO_PIN_0
#define LCD_RESET_GPIO_Port GPIOC
#define PWM1_Pin GPIO_PIN_0
#define PWM1_GPIO_Port GPIOA
#define PWM2_Pin GPIO_PIN_2
#define PWM2_GPIO_Port GPIOA
#define MUTE_Pin GPIO_PIN_14
#define MUTE_GPIO_Port GPIOE
#define LCD_CS_Pin GPIO_PIN_12
#define LCD_CS_GPIO_Port GPIOB
#define LCD_SCK_Pin GPIO_PIN_13
#define LCD_SCK_GPIO_Port GPIOB
#define LCD_SDA_Pin GPIO_PIN_14
#define LCD_SDA_GPIO_Port GPIOB
#define LCD_WR_Pin GPIO_PIN_15
#define LCD_WR_GPIO_Port GPIOB
#define PWM5_Pin GPIO_PIN_14
#define PWM5_GPIO_Port GPIOD
#define PWM3_Pin GPIO_PIN_7
#define PWM3_GPIO_Port GPIOC
#define PWM2C8_Pin GPIO_PIN_8
#define PWM2C8_GPIO_Port GPIOC
#define SPI_FLASH_CS_Pin GPIO_PIN_15
#define SPI_FLASH_CS_GPIO_Port GPIOA
#define SPI_FLASH_SCK_Pin GPIO_PIN_3
#define SPI_FLASH_SCK_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define ff_malloc  pvPortMalloc
#define ff_free  vPortFree
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
