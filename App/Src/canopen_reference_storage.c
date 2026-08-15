/* SPDX-License-Identifier: Apache-2.0 */
#include "canopen_reference_storage.h"

#include <string.h>

#include "OD.h"
#include "storage/CO_storage.h"

#define CANOPEN_REFERENCE_STORAGE_MAGIC       0x434F5354UL
#define CANOPEN_REFERENCE_STORAGE_VERSION     1UL

/* This image is deliberately bounded to the generated communication group. */
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t length;
    uint32_t crc32;
    OD_PERSIST_COMM_t payload;
} canopen_reference_storage_image_t;

static canopen_reference_storage_image_t s_image;
static OD_PERSIST_COMM_t s_factory_defaults;
static bool s_factory_defaults_valid;
static CO_storage_t s_storage;
static CO_storage_entry_t s_entries[1];

static uint32_t
storage_crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFFUL;

    for (size_t i = 0U; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t bit = 0U; bit < 8U; ++bit) {
            crc = ((crc & 1U) != 0U) ? ((crc >> 1U) ^ 0xEDB88320UL) : (crc >> 1U);
        }
    }
    return ~crc;
}

__attribute__((weak)) bool
CANopenReferenceStorage_BoardStore(const void *data, size_t length) {
    if (data == NULL || length != sizeof(s_image.payload)) {
        return false;
    }
    s_image.magic = CANOPEN_REFERENCE_STORAGE_MAGIC;
    s_image.version = CANOPEN_REFERENCE_STORAGE_VERSION;
    s_image.length = (uint32_t)length;
    (void)memcpy(&s_image.payload, data, length);
    s_image.crc32 = storage_crc32((const uint8_t *)&s_image.payload, length);
    return true;
}

__attribute__((weak)) bool
CANopenReferenceStorage_BoardRestore(void *data, size_t length) {
    if (data == NULL || length != sizeof(s_image.payload)
        || s_image.magic != CANOPEN_REFERENCE_STORAGE_MAGIC
        || s_image.version != CANOPEN_REFERENCE_STORAGE_VERSION
        || s_image.length != (uint32_t)length
        || s_image.crc32 != storage_crc32((const uint8_t *)&s_image.payload, length)) {
        return false;
    }
    (void)memcpy(data, &s_image.payload, length);
    return true;
}

static ODR_t
storage_store(CO_storage_entry_t *entry, CO_CANmodule_t *can_module) {
    (void)can_module;
    if (entry == NULL || entry->addr == NULL || entry->len != sizeof(OD_PERSIST_COMM_t)) {
        return ODR_DEV_INCOMPAT;
    }
    return CANopenReferenceStorage_BoardStore(entry->addr, entry->len) ? ODR_OK : ODR_HW;
}

static ODR_t
storage_restore(CO_storage_entry_t *entry, CO_CANmodule_t *can_module) {
    (void)can_module;
    if (entry == NULL || entry->addr == NULL || entry->len != sizeof(OD_PERSIST_COMM_t)) {
        return ODR_DEV_INCOMPAT;
    }
    /* A missing image is a normal first-boot condition: restore compiled-in
     * defaults. A board override may return true after loading a real image. */
    if (!CANopenReferenceStorage_BoardRestore(entry->addr, entry->len)) {
        if (!s_factory_defaults_valid) {
            return ODR_HW;
        }
        (void)memcpy(entry->addr, &s_factory_defaults, entry->len);
    }
    return ODR_OK;
}

CO_ReturnError_t
CANopenReferenceStorage_Init(CO_t *co) {
    OD_entry_t *store_entry;
    OD_entry_t *restore_entry;
    CO_ReturnError_t result;

    if (co == NULL || co->CANmodule == NULL || OD == NULL) {
        return CO_ERROR_ILLEGAL_ARGUMENT;
    }
    if (!s_factory_defaults_valid) {
        (void)memcpy(&s_factory_defaults, &OD_PERSIST_COMM, sizeof(s_factory_defaults));
        s_factory_defaults_valid = true;
    }
    (void)CANopenReferenceStorage_BoardRestore(&OD_PERSIST_COMM, sizeof(OD_PERSIST_COMM));

    store_entry = OD_find(OD, 0x1010U);
    restore_entry = OD_find(OD, 0x1011U);
    s_entries[0].addr = &OD_PERSIST_COMM;
    s_entries[0].len = sizeof(OD_PERSIST_COMM);
    s_entries[0].subIndexOD = 2U;
    s_entries[0].attr = (uint8_t)CO_storage_cmd | (uint8_t)CO_storage_restore;
    s_entries[0].addrNV = &s_image;

    result = CO_storage_init(&s_storage, co->CANmodule, store_entry, restore_entry,
                             storage_store, storage_restore, s_entries, 1U);
    if (result == CO_ERROR_NO) {
        s_storage.enabled = true;
    }
    return result;
}
