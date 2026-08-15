/* SPDX-License-Identifier: Apache-2.0 */
#ifndef CANOPEN_REFERENCE_DIAGNOSTICS_H
#define CANOPEN_REFERENCE_DIAGNOSTICS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Board override: write a bounded diagnostic byte sequence from mainline only. */
void CANopenReferenceDiagnostics_Write(const uint8_t *bytes, uint16_t length);

/** Publish a rate-limited stack summary from the non-real-time mainline. */
void CANopenReferenceDiagnostics_Process(uint8_t node_id, uint16_t can_error_status, uint8_t led_green,
                                         uint8_t led_red, uint32_t now_ms);

/** Record a HAL CAN fault from an interrupt callback without performing I/O. */
void CANopenReferenceDiagnostics_ReportCanHardwareError(uint32_t hal_error_code);

/** Return the number of HAL CAN fault callbacks observed since boot. */
uint32_t CANopenReferenceDiagnostics_CanHardwareErrorCount(void);

/** Return the most recent HAL CAN error bitmask. */
uint32_t CANopenReferenceDiagnostics_LastCanHardwareError(void);

#ifdef __cplusplus
}
#endif

#endif /* CANOPEN_REFERENCE_DIAGNOSTICS_H */
