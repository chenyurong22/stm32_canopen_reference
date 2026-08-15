/* SPDX-License-Identifier: Apache-2.0 */
#ifndef CANOPEN_REFERENCE_STORAGE_H
#define CANOPEN_REFERENCE_STORAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "CANopen.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize OD 1010h/1011h storage and restore a validated communication image.
 * Must run after CO_CANinit() and before CO_CANopenInit().
 */
CO_ReturnError_t CANopenReferenceStorage_Init(CO_t *co);

/**
 * Board persistence seam. The default implementation is CRC-validated RAM and
 * therefore survives communication resets but not power loss. A product board
 * must override both hooks with Flash/EEPROM wear-managed storage.
 */
bool CANopenReferenceStorage_BoardStore(const void *data, size_t length);
bool CANopenReferenceStorage_BoardRestore(void *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* CANOPEN_REFERENCE_STORAGE_H */
