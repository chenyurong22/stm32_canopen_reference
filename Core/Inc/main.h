/* SPDX-License-Identifier: Apache-2.0 */
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"

extern CAN_HandleTypeDef hcan1;
extern TIM_HandleTypeDef htim7;

void SystemClock_Config(void);
void Error_Handler(void);
void MX_CAN1_Init(void);
void MX_TIM7_Init(void);
void MX_GPIO_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
