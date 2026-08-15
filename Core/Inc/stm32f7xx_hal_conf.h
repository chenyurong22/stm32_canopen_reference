/* SPDX-License-Identifier: Apache-2.0 */
#ifndef __STM32F7xx_HAL_CONF_H
#define __STM32F7xx_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_CAN_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED

#define HSE_VALUE       25000000U
#define HSE_STARTUP_TIMEOUT 100U
#define HSI_VALUE       16000000U
#define LSI_VALUE       32000U
#define LSE_VALUE       32768U
#define LSE_STARTUP_TIMEOUT 5000U
#define EXTERNAL_CLOCK_VALUE 12288000U
#define VDD_VALUE       3300U
#define TICK_INT_PRIORITY 0U
#define USE_RTOS        0U
#define PREFETCH_ENABLE 1U
#define INSTRUCTION_CACHE_ENABLE 1U
#define DATA_CACHE_ENABLE 1U

#ifndef assert_param
#define assert_param(expr) ((void)(expr))
#endif

/* HAL module headers establish the STM32 device register and common type
 * declarations before stm32f7xx_hal.h exports the generic HAL API. */
#include "stm32f7xx_hal_rcc.h"
#include "stm32f7xx_hal_gpio.h"
#include "stm32f7xx_hal_can.h"
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_tim.h"
#include "stm32f7xx_hal_tim_ex.h"
#include "stm32f7xx_hal_pwr.h"
#include "stm32f7xx_hal_pwr_ex.h"
#include "stm32f7xx_hal_cortex.h"
#include "stm32f7xx_hal_flash.h"
#ifdef HAL_IWDG_MODULE_ENABLED
#include "stm32f7xx_hal_iwdg.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STM32F7xx_HAL_CONF_H */
