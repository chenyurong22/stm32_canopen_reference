/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Product-level configuration for the STM32F767 CANopen reference firmware.
 * This file is intentionally outside third_party so it can be owned and
 * versioned by the product team.
 */
#ifndef CANOPEN_REFERENCE_CONFIG_H
#define CANOPEN_REFERENCE_CONFIG_H

#include <stdint.h>

/* Exactly one certified device-profile personality is the normal product mode.
 * A combined I/O + drive build is useful for integration experiments only and
 * requires a product-specific device type, EDS/XDD, and conformance scope. */
#ifndef CANOPEN_REFERENCE_ENABLE_CIA401
#define CANOPEN_REFERENCE_ENABLE_CIA401 1U
#endif

#ifndef CANOPEN_REFERENCE_ENABLE_CIA402
#define CANOPEN_REFERENCE_ENABLE_CIA402 0U
#endif

#ifndef CANOPEN_REFERENCE_ALLOW_COMBINED_PROFILES
#define CANOPEN_REFERENCE_ALLOW_COMBINED_PROFILES 0U
#endif

#if ((CANOPEN_REFERENCE_ENABLE_CIA401 != 0U) && (CANOPEN_REFERENCE_ENABLE_CIA402 != 0U) \
     && (CANOPEN_REFERENCE_ALLOW_COMBINED_PROFILES == 0U))
#error "Select one device profile or explicitly authorize the non-conformant combined reference mode."
#endif

#if ((CANOPEN_REFERENCE_ENABLE_CIA401 == 0U) && (CANOPEN_REFERENCE_ENABLE_CIA402 == 0U))
#error "At least one application profile must be selected."
#endif

/* Default CiA 301 settings. Node-ID and bitrate remain reconfigurable through
 * LSS when a valid identity is provisioned. The F767 bxCAN peripheral is set
 * to 500 kbit/s by the CubeMX/HAL initialization. */
#define CANOPEN_REFERENCE_DEFAULT_NODE_ID       10U
#define CANOPEN_REFERENCE_DEFAULT_BITRATE_KBPS  500U
#define CANOPEN_REFERENCE_HEARTBEAT_MS          1000U

/* Replace before release. These are deliberately non-production reference
 * values. The same values must be updated in the EDS/XDD and production label. */
#define CANOPEN_REFERENCE_VENDOR_ID             UINT32_C(0x00000000)
#define CANOPEN_REFERENCE_PRODUCT_CODE          UINT32_C(0xF7670401)
#define CANOPEN_REFERENCE_REVISION              UINT32_C(0x00010000)
#define CANOPEN_REFERENCE_SERIAL                UINT32_C(0x00000001)

/* Fail safe by default. Hardware adapters must explicitly permit outputs and
 * drive power after their own interlocks and diagnostics have passed. */
#define CANOPEN_REFERENCE_OUTPUTS_DEFAULT_SAFE  1U

/* Optional mainline-only UART diagnostic summary. A board integration must
 * override CANopenReferenceDiagnostics_Write() with a bounded, non-blocking
 * implementation before enabling this switch. */
#ifndef CANOPEN_REFERENCE_UART_DIAGNOSTICS
#define CANOPEN_REFERENCE_UART_DIAGNOSTICS       0U
#endif

/* CiA 309-3 ASCII gateway support is disabled unless the product has an
 * authenticated/physical diagnostic access policy and a bounded UART bridge. */
#ifndef CANOPEN_REFERENCE_ENABLE_GATEWAY
#define CANOPEN_REFERENCE_ENABLE_GATEWAY         0U
#endif

#endif /* CANOPEN_REFERENCE_CONFIG_H */
