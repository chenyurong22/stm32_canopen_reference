/* SPDX-License-Identifier: Apache-2.0 */
#include "canopen_reference_watchdog.h"
#include "canopen_reference_config.h"
#include "main.h"

#if CANOPEN_REFERENCE_ENABLE_IWDG
static IWDG_HandleTypeDef s_iwdg;
#endif

static volatile uint32_t s_timer_ticks;
static uint32_t s_last_mainline_tick;
static uint32_t s_mainline_ticks;

void
CANopenReferenceWatchdog_Init(void) {
#if CANOPEN_REFERENCE_ENABLE_IWDG
    const uint32_t nominal_lsi_hz = 32000U;
    uint32_t reload = (nominal_lsi_hz * CANOPEN_REFERENCE_IWDG_TIMEOUT_MS) / (64U * 1000U);

    if (reload < 1U) {
        reload = 1U;
    }
    if (reload > 0x0FFFU) {
        reload = 0x0FFFU;
    }
    __HAL_RCC_LSI_ENABLE();
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY) == RESET) {
        /* LSI readiness is required before starting the independent watchdog. */
    }
    s_iwdg.Instance = IWDG;
    s_iwdg.Init.Prescaler = IWDG_PRESCALER_64;
    s_iwdg.Init.Reload = reload;
    s_iwdg.Init.Window = IWDG_WINDOW_DISABLE;
    if (HAL_IWDG_Init(&s_iwdg) != HAL_OK) {
        Error_Handler();
    }
#endif
    s_timer_ticks = 0U;
    s_last_mainline_tick = 0U;
    s_mainline_ticks = 0U;
}

void
CANopenReferenceWatchdog_TickISR(void) {
    (void)__atomic_fetch_add(&s_timer_ticks, 1U, __ATOMIC_RELAXED);
}

void
CANopenReferenceWatchdog_Process(void) {
    uint32_t timer_ticks = __atomic_load_n(&s_timer_ticks, __ATOMIC_ACQUIRE);

    if (timer_ticks == s_last_mainline_tick) {
        return;
    }
    s_last_mainline_tick = timer_ticks;
    s_mainline_ticks++;
#if CANOPEN_REFERENCE_ENABLE_IWDG
    if (HAL_IWDG_Refresh(&s_iwdg) != HAL_OK) {
        Error_Handler();
    }
#endif
}

uint32_t
CANopenReferenceWatchdog_MainlineTicks(void) {
    return s_mainline_ticks;
}
