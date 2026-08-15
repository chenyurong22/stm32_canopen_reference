/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Bounded CiA 302 NMT-master foundation for the reference project.
 * This module is transport-neutral: the application supplies frame/event
 * callbacks and remains the owner of the production bxCAN driver.
 */
#ifndef CIA302_NMT_MASTER_H
#define CIA302_NMT_MASTER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CIA302_MAX_NODES 127U
#define CIA302_NMT_CAN_ID 0x000U
#define CIA302_HEARTBEAT_BASE 0x700U

#define CIA302_NMT_START 0x01U
#define CIA302_NMT_STOP 0x02U
#define CIA302_NMT_PREOP 0x80U
#define CIA302_NMT_RESET_NODE 0x81U
#define CIA302_NMT_RESET_COMMUNICATION 0x82U

#define CIA302_HEARTBEAT_BOOTUP 0x00U
#define CIA302_HEARTBEAT_STOPPED 0x04U
#define CIA302_HEARTBEAT_OPERATIONAL 0x05U
#define CIA302_HEARTBEAT_PREOP 0x7FU

typedef enum {
    CIA302_NODE_UNASSIGNED = 0,
    CIA302_NODE_WAITING_BOOTUP,
    CIA302_NODE_PREOP,
    CIA302_NODE_OPERATIONAL,
    CIA302_NODE_STOPPED,
    CIA302_NODE_TIMED_OUT,
} cia302_node_state_t;

typedef enum {
    CIA302_EVENT_BOOTUP = 1,
    CIA302_EVENT_HEARTBEAT,
    CIA302_EVENT_HEARTBEAT_TIMEOUT,
    CIA302_EVENT_BOOT_TIMEOUT,
    CIA302_EVENT_NETWORK_READY,
    CIA302_EVENT_INVALID_FRAME,
} cia302_event_type_t;

typedef struct {
    uint16_t can_id;
    uint8_t dlc;
    uint8_t data[8];
} cia302_frame_t;

typedef struct {
    cia302_event_type_t type;
    uint8_t node_id;
    uint8_t state;
    uint32_t timestamp_ms;
} cia302_event_t;

typedef bool (*cia302_send_fn)(void *context, const cia302_frame_t *frame);
typedef void (*cia302_event_fn)(void *context, const cia302_event_t *event);

typedef struct {
    bool assigned;
    bool mandatory;
    bool auto_start;
    uint16_t heartbeat_timeout_ms;
    uint32_t last_heartbeat_ms;
    cia302_node_state_t state;
} cia302_node_t;

typedef struct {
    uint32_t nmt_startup;
    uint32_t boot_time_ms;
    uint8_t master_node_id;
    uint32_t now_ms;
    uint32_t started_at_ms;
    bool running;
    bool network_ready;
    bool boot_timeout_reported;
    cia302_node_t nodes[CIA302_MAX_NODES + 1U];
    cia302_send_fn send;
    cia302_event_fn event;
    void *callback_context;
} cia302_nmt_master_t;

void cia302_nmt_master_init(cia302_nmt_master_t *master,
                            uint8_t master_node_id,
                            cia302_send_fn send,
                            cia302_event_fn event,
                            void *callback_context);

bool cia302_nmt_master_configure(cia302_nmt_master_t *master,
                                 uint8_t node_id,
                                 bool mandatory,
                                 bool auto_start,
                                 uint16_t heartbeat_timeout_ms);

bool cia302_nmt_master_request(cia302_nmt_master_t *master,
                               uint8_t command,
                               uint8_t node_id);

bool cia302_nmt_master_start(cia302_nmt_master_t *master, uint32_t now_ms);
void cia302_nmt_master_receive(cia302_nmt_master_t *master,
                               uint16_t can_id,
                               const uint8_t *data,
                               uint8_t dlc,
                               uint32_t now_ms);
void cia302_nmt_master_process(cia302_nmt_master_t *master, uint32_t now_ms);

#endif /* CIA302_NMT_MASTER_H */
