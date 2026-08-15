/* SPDX-License-Identifier: Apache-2.0 */
#ifndef CANOPEN_REFERENCE_WATCHDOG_H
#define CANOPEN_REFERENCE_WATCHDOG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize the optional IWDG and reset the dual-rate progress state. */
void CANopenReferenceWatchdog_Init(void);

/** Record one TIM7 tick; safe to call from the 1 ms interrupt callback. */
void CANopenReferenceWatchdog_TickISR(void);

/** Observe timer progress and refresh IWDG only from the mainline. */
void CANopenReferenceWatchdog_Process(void);

/** Return the number of timer ticks observed by the mainline. */
uint32_t CANopenReferenceWatchdog_MainlineTicks(void);

/** Return reset flags captured before HAL initialization. */
uint32_t CANopenReferenceWatchdog_ResetFlags(void);

#ifdef __cplusplus
}
#endif

#endif /* CANOPEN_REFERENCE_WATCHDOG_H */
