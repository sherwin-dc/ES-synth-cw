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
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "debug.h"
#include "FreeRTOS.h"
#include "semphr.h" // Contains the SemaphoreHandle_t type
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef uint8_t boardkeys_t[28]; // Type used to hold the state of keys
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern volatile uint8_t playedNotes [9*12]; // Global variable which is used to access the state notes

extern volatile uint8_t volume; // Global variable which stores volume of piano
extern volatile uint8_t octave; // Global variable which stores octave of piano
extern volatile uint8_t sound; // Global variable which stores sound type of piano
extern volatile uint8_t reverb;
typedef struct {
  uint32_t pitch;
  uint32_t modulation;
} ADC;
extern ADC joystick;
extern volatile uint8_t isRecording;

// handles incoming messages
extern QueueHandle_t msgInQ;
extern QueueHandle_t msgOutQ;
extern SemaphoreHandle_t CAN_TX_Semaphore;

// CAN recieve buffer
extern uint8_t RX_Message[8];
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define C0_Pin GPIO_PIN_3
#define C0_GPIO_Port GPIOA
#define Row_Sel_En_Pin GPIO_PIN_6
#define Row_Sel_En_GPIO_Port GPIOA
#define C2_Pin GPIO_PIN_7
#define C2_GPIO_Port GPIOA
#define Row_Sel_0_Pin GPIO_PIN_0
#define Row_Sel_0_GPIO_Port GPIOB
#define Row_Sel_1_Pin GPIO_PIN_1
#define Row_Sel_1_GPIO_Port GPIOB
#define C1_Pin GPIO_PIN_8
#define C1_GPIO_Port GPIOA
#define C3_Pin GPIO_PIN_9
#define C3_GPIO_Port GPIOA
#define LED_BUILTIN_Pin GPIO_PIN_3
#define LED_BUILTIN_GPIO_Port GPIOB
#define Row_Sel_2_Pin GPIO_PIN_4
#define Row_Sel_2_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
