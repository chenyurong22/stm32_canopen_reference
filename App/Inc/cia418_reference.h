/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Bounded CiA 418 battery-module personality adapter.
 */
#ifndef CIA418_REFERENCE_H
#define CIA418_REFERENCE_H

#include <stdbool.h>
#include <stdint.h>

#include "cia418_OD.h"

#define CIA418_STATUS_READY 0x01U
#define CIA418_STATUS_FAULT 0x80U

/** Bounded application state exposed by the CiA 418 reference adapter. */
typedef struct {
    /** Battery status bit field using the project’s CiA 418 reference encoding. */
    uint8_t battery_status;
    /** Charger status bit field. */
    uint8_t charger_status;
    /** Battery temperature in the configured signed engineering unit. */
    int16_t temperature;
    /** Product-specific battery type code. */
    uint8_t battery_type;
    /** Nominal capacity in ampere-hours. */
    uint16_t ah_capacity;
    /** Maximum permitted charge current. */
    uint16_t maximum_charge_current;
    /** Number of monitored battery cells. */
    uint16_t number_of_cells;
    /** Cumulative charge throughput in ampere-hours. */
    uint32_t cumulative_total_ah_charge;
    /** Charge consumed since the last charge cycle. */
    uint16_t ah_expended_since_last_charge;
    /** Charge returned during the last charge cycle. */
    uint16_t ah_returned_during_last_charge;
    /** Charge throughput since the last equalization cycle. */
    uint16_t ah_since_last_equalization;
    /** Battery voltage in the configured engineering unit. */
    uint32_t battery_voltage;
    /** Charge current requested by the battery/application policy. */
    uint16_t charge_current_requested;
    /** Charger state of charge percentage or encoded value. */
    uint8_t charger_state_of_charge;
    /** Battery state of charge percentage or encoded value. */
    uint8_t battery_state_of_charge;
    /** Water-level status code for supported battery types. */
    uint8_t water_level_status;
    /** Battery serial number words. */
    uint32_t battery_serial_number[3];
    /** Battery identity words. */
    uint32_t battery_id[5];
    /** Vehicle serial number words associated with the battery. */
    uint32_t vehicle_serial_number[5];
    /** Vehicle identity words associated with the battery. */
    uint32_t vehicle_id[5];
    /** Date of the last equalization cycle, encoded as two words. */
    uint16_t date_of_last_equalization[2];
    /** True when the adapter has forced a safe fault condition. */
    bool safe_fault;
} Cia418ReferenceState;

/** Initialize a battery reference state to safe defaults. */
void Cia418Reference_Init(Cia418ReferenceState *state);
/** Update measured battery values and return false for invalid inputs. */
bool Cia418Reference_UpdateMeasurements(Cia418ReferenceState *state, uint32_t battery_voltage,
                                        int16_t temperature, uint8_t battery_state_of_charge,
                                        uint16_t charge_current_requested);
/** Write one bounded profile object and return false if it is unsupported. */
bool Cia418Reference_WriteObject(Cia418ReferenceState *state, uint16_t index, uint8_t sub_index,
                                 uint32_t value);
/** Read one bounded profile object and return false if it is unsupported. */
bool Cia418Reference_ReadObject(const Cia418ReferenceState *state, uint16_t index,
                                uint8_t sub_index, uint32_t *value);
/** Force the adapter into its safe-fault state. */
void Cia418Reference_ForceSafe(Cia418ReferenceState *state);
/** Copy adapter state into the generated CiA 418 model artifact.
 *
 * This model is intentionally not the live CANopenNode OD. A dedicated
 * battery-personality OD is required before CiA 418 SDO access can be claimed.
 */
void Cia418Reference_SyncToGeneratedOd(const Cia418ReferenceState *state,
                                       CIA418_OD_APP_t *od_app);

#endif /* CIA418_REFERENCE_H */
