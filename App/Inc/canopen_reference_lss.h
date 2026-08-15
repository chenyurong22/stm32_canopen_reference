/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Project policy hooks around the CANopenNode CiA 305 LSS slave.
 */
#ifndef CANOPEN_REFERENCE_LSS_H
#define CANOPEN_REFERENCE_LSS_H

#include <stdbool.h>
#include <stdint.h>

#include "CANopen.h"

/* CO_t is declared by CANopen.h. */

typedef struct {
    uint8_t node_id;
    uint16_t bitrate_kbps;
    uint16_t activation_delay_ms;
    bool store_requested;
} CANopenReferenceLssState;

bool CANopenReferenceLss_BitrateSupported(uint16_t bitrate_kbps);
void CANopenReferenceLss_ActivateBitrate(uint16_t delay_ms);
bool CANopenReferenceLss_StoreConfiguration(uint8_t node_id, uint16_t bitrate_kbps);
void CANopenReferenceLss_Init(CO_t *co, CANopenReferenceLssState *state);

#endif /* CANOPEN_REFERENCE_LSS_H */
