/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * CANopenNode feature override. The STM32 driver includes this file when the
 * build defines CO_DRIVER_CUSTOM. Values use the public flag bits declared in
 * 301/CO_config.h, which is included before this header.
 */
#ifndef CO_DRIVER_CUSTOM_H
#define CO_DRIVER_CUSTOM_H

#include "canopen_reference_config.h"

/* A device uses NMT slave processing in all builds. The master helper is needed
 * only when the explicitly enabled CiA 309 gateway accepts authorized NMT
 * commands for remote nodes. */
#if CANOPEN_REFERENCE_ENABLE_GATEWAY
#define CO_CONFIG_NMT                 0x02U
#else
#define CO_CONFIG_NMT                 0U
#endif

/* SDO server: expedited (always present), segmented, block, and OD-triggered
 * reconfiguration. The 1024 byte buffer satisfies a full block segment. */
#define CO_CONFIG_SDO_SRV             (0x02U | 0x04U | 0x4000U)
#define CO_CONFIG_SDO_SRV_BUFFER_SIZE 1024U

/* The optional SDO client is enabled for a local manager/application. Gateway
 * ASCII transport is deliberately not enabled in this single-CAN device build. */
#define CO_CONFIG_SDO_CLI             (0x01U | 0x02U | 0x04U | 0x08U | 0x4000U)
#define CO_CONFIG_SDO_CLI_BUFFER_SIZE 1024U
#if CANOPEN_REFERENCE_ENABLE_GATEWAY
#define CO_CONFIG_FIFO                (0x01U | 0x02U | 0x04U | 0x08U | 0x10U)
#else
#define CO_CONFIG_FIFO                (0x01U | 0x02U | 0x04U)
#endif
#define CO_CONFIG_CRC16               0x01U

/* RPDO/TPDO timing, SYNC, OD I/O, dynamic configuration, and bitwise mapping.
 * Dynamic mapping must still be constrained by the product OD/EDS and tested
 * for every permitted map. */
#define CO_CONFIG_PDO                 (0x01U | 0x02U | 0x04U | 0x08U | 0x10U | 0x20U | 0x40U | 0x4000U)

/* LSS slave with direct Fastscan responses. Identity values are provisioned in
 * the Object Dictionary before the LSS object is initialized. */
#define CO_CONFIG_LSS                 (0x01U | 0x02U)

/* CiA 303-3 status indications are calculated by the stack. */
#define CO_CONFIG_LEDS                0x01U

/* CiA 304 SRDO/GFC require additional standard OD records and a dedicated
 * safety lifecycle, so they are not enabled by this reference. CiA 309-3 is an
 * explicit gateway personality with SDO and NMT access; a board authorization
 * hook still gates every command path at runtime. */
#define CO_CONFIG_GFC                 0U
#define CO_CONFIG_SRDO                0U
#if CANOPEN_REFERENCE_ENABLE_GATEWAY
#define CO_CONFIG_GTW                 (0x02U | 0x04U | 0x08U | 0x40U)
#define CO_CONFIG_GTW_BLOCK_DL_LOOP   1U
#define CO_CONFIG_GTWA_COMM_BUF_SIZE  1024U
#else
#define CO_CONFIG_GTW                 0U
#endif

#endif /* CO_DRIVER_CUSTOM_H */
