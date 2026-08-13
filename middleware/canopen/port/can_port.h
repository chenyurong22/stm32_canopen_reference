/* SPDX-License-Identifier: Apache-2.0 */
#ifndef CANOPEN_MIDDLEWARE_CAN_PORT_H
#define CANOPEN_MIDDLEWARE_CAN_PORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CAN_PORT_MAX_DLC 8U

typedef void (*can_port_rx_callback_t)(uint32_t id, uint8_t *data, uint8_t len);

/**
 * Initialize the selected CAN transport at the requested nominal bit rate.
 *
 * For SocketCAN builds, the interface name is supplied through CAN_PORT_IFACE
 * (default: vcan0) and the rate is informational because vcan has no bit-rate
 * controller. For STM32 builds, call can_port_stm32_bind() first.
 */
int can_port_init(uint32_t bitrate);

/** Send one classic-CAN standard-data frame. Returns 0 on acceptance. */
int can_port_send(uint32_t id, uint8_t *data, uint8_t len);

/** Register one receive callback. The callback is invoked in port context. */
void can_port_register_rx(can_port_rx_callback_t cb);

/**
 * Receive at most one frame and invoke the registered callback.
 * timeout_ms == 0 performs a non-blocking poll; returns 1 if a frame was
 * dispatched, 0 on timeout, and a negative errno-style value on failure.
 */
int can_port_poll(uint32_t timeout_ms);

/** Close the host transport or reset the portable port state. */
void can_port_deinit(void);

#ifdef CAN_PORT_STM32
#include "stm32f7xx_hal.h"

/** Bind the CubeMX-generated CAN handle before calling can_port_init(). */
int can_port_stm32_bind(CAN_HandleTypeDef *hcan);

/**
 * Dispatch a frame read by a board-owned HAL callback. This keeps callback
 * ownership explicit; it must not be installed concurrently with the
 * CANopenNode STM32 driver on the same bxCAN peripheral.
 */
void can_port_stm32_dispatch_rx_from_isr(uint32_t id, uint8_t *data, uint8_t len);
#endif

#ifdef __cplusplus
}
#endif

#endif /* CANOPEN_MIDDLEWARE_CAN_PORT_H */
