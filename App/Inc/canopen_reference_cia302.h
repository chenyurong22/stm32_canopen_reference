/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Project-owned opt-in CiA 302 NMT-master adapter.
 */
#ifndef CANOPEN_REFERENCE_CIA302_H
#define CANOPEN_REFERENCE_CIA302_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool enabled;
    bool running;
    bool network_ready;
    uint8_t monitored_node_id;
    uint8_t monitored_node_state;
    uint32_t event_count_bootup;
    uint32_t event_count_heartbeat;
    uint32_t event_count_heartbeat_timeout;
    uint32_t event_count_boot_timeout;
    uint32_t event_count_network_ready;
    uint32_t event_count_invalid_frame;
    uint32_t last_event_timestamp_ms;
    uint8_t last_event_type;
    uint8_t last_event_node_id;
    uint8_t last_event_state;
} CANopenReferenceCia302Snapshot;

/** Prepare the optional heartbeat-consumer OD entries before CANopen init. */
void CANopenReferenceCia302_PrepareOd(void);

/** Initialize the opt-in CiA 302 master adapter after CANopen init. */
void CANopenReferenceCia302_Init(void *canopen_stack, uint8_t master_node_id, uint32_t now_ms);

/** Stop and clear the opt-in adapter during communication reset. */
void CANopenReferenceCia302_Deinit(void);

/** Feed each received heartbeat to the master before CANopenNode clears its RX flag. */
void CANopenReferenceCia302_PreProcess(uint32_t now_ms);

/** Process the non-blocking master state machine from mainline context. */
void CANopenReferenceCia302_Process(uint32_t now_ms);

/** Return a stable copy of the bounded diagnostic state. */
void CANopenReferenceCia302_GetSnapshot(CANopenReferenceCia302Snapshot *snapshot);

#ifdef __cplusplus
}
#endif

#endif /* CANOPEN_REFERENCE_CIA302_H */
