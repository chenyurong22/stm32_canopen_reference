/* SPDX-License-Identifier: Apache-2.0 */
#include "canopen_reference_diagnostics.h"

#include <stdio.h>

#ifndef CANOPEN_REFERENCE_UART_DIAGNOSTICS
#define CANOPEN_REFERENCE_UART_DIAGNOSTICS 0U
#endif

#define CANOPEN_REFERENCE_DIAGNOSTICS_PERIOD_MS 1000U

__attribute__((weak)) void
CANopenReferenceDiagnostics_Write(const uint8_t *bytes, uint16_t length) {
    (void)bytes;
    (void)length;
}

void
CANopenReferenceDiagnostics_Process(uint8_t node_id, uint16_t can_error_status, uint8_t led_green, uint8_t led_red,
                                    uint32_t now_ms) {
#if CANOPEN_REFERENCE_UART_DIAGNOSTICS
    static uint32_t last_report_ms;
    char line[80];
    int written;

    if ((uint32_t)(now_ms - last_report_ms) < CANOPEN_REFERENCE_DIAGNOSTICS_PERIOD_MS) {
        return;
    }
    last_report_ms = now_ms;
    written = snprintf(line, sizeof(line), "CANopen node=%u err=0x%04X led=%u/%u\r\n", (unsigned)node_id,
                       (unsigned)can_error_status, (unsigned)led_green, (unsigned)led_red);
    if (written > 0) {
        uint16_t length = (uint16_t)((written >= (int)sizeof(line)) ? sizeof(line) - 1U : (unsigned)written);
        CANopenReferenceDiagnostics_Write((const uint8_t *)line, length);
    }
#else
    (void)node_id;
    (void)can_error_status;
    (void)led_green;
    (void)led_red;
    (void)now_ms;
#endif
}
