/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define PRIMARY_BOOST_ENABLE_PIN_Pin GPIO_PIN_0
#define PRIMARY_BOOST_ENABLE_PIN_GPIO_Port GPIOA
#define PEIMARY_BOOST_PWM_CH_Pin GPIO_PIN_1
#define PEIMARY_BOOST_PWM_CH_GPIO_Port GPIOA
#define DETECT_LEVEL_HIGH_ADC_Pin GPIO_PIN_2
#define DETECT_LEVEL_HIGH_ADC_GPIO_Port GPIOA
#define DETECT_LEVEL_LOW_ADC_Pin GPIO_PIN_3
#define DETECT_LEVEL_LOW_ADC_GPIO_Port GPIOA
#define VREF1_2_Pin GPIO_PIN_4
#define VREF1_2_GPIO_Port GPIOA
#define DETECT_ZONE1_TOUCH_Pin GPIO_PIN_5
#define DETECT_ZONE1_TOUCH_GPIO_Port GPIOA
#define DETECT_ZONE2_TOUCH_Pin GPIO_PIN_6
#define DETECT_ZONE2_TOUCH_GPIO_Port GPIOA
#define DETECT_LEVEL_HIGH_Pin GPIO_PIN_0
#define DETECT_LEVEL_HIGH_GPIO_Port GPIOB
#define DETECT_LEVEL_LOW_Pin GPIO_PIN_1
#define DETECT_LEVEL_LOW_GPIO_Port GPIOB
#define DETECT_A3_LINE_PIN_Pin GPIO_PIN_10
#define DETECT_A3_LINE_PIN_GPIO_Port GPIOB
#define DETECT_A3_LINE_PIN_EXTI_IRQn EXTI15_10_IRQn
#define DETECT_A4_LINE_PIN_Pin GPIO_PIN_11
#define DETECT_A4_LINE_PIN_GPIO_Port GPIOB
#define DETECT_A4_LINE_PIN_EXTI_IRQn EXTI15_10_IRQn
#define DETECT_B1_LINE_PIN_Pin GPIO_PIN_12
#define DETECT_B1_LINE_PIN_GPIO_Port GPIOB
#define DETECT_B1_LINE_PIN_EXTI_IRQn EXTI15_10_IRQn
#define DETECT_B2_LINE_PIN_Pin GPIO_PIN_13
#define DETECT_B2_LINE_PIN_GPIO_Port GPIOB
#define DETECT_B2_LINE_PIN_EXTI_IRQn EXTI15_10_IRQn
#define DETECT_B3_LINE_PIN_Pin GPIO_PIN_14
#define DETECT_B3_LINE_PIN_GPIO_Port GPIOB
#define DETECT_B3_LINE_PIN_EXTI_IRQn EXTI15_10_IRQn
#define DETECT_B4_LINE_PIN_Pin GPIO_PIN_15
#define DETECT_B4_LINE_PIN_GPIO_Port GPIOB
#define DETECT_B4_LINE_PIN_EXTI_IRQn EXTI15_10_IRQn
#define ZONE1_PULSE_RELEASE_PIN_Pin GPIO_PIN_11
#define ZONE1_PULSE_RELEASE_PIN_GPIO_Port GPIOA
#define ZONE2_PULSE_RELEASE_PIN_Pin GPIO_PIN_12
#define ZONE2_PULSE_RELEASE_PIN_GPIO_Port GPIOA
#define DETECT_A1_LINE_PIN_Pin GPIO_PIN_8
#define DETECT_A1_LINE_PIN_GPIO_Port GPIOB
#define DETECT_A1_LINE_PIN_EXTI_IRQn EXTI9_5_IRQn
#define DETECT_A2_LINE_PIN_Pin GPIO_PIN_9
#define DETECT_A2_LINE_PIN_GPIO_Port GPIOB
#define DETECT_A2_LINE_PIN_EXTI_IRQn EXTI9_5_IRQn

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
