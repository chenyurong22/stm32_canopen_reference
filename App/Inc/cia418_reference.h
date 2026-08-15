/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Bounded CiA 418 battery-module personality adapter.
 */
#ifndef CIA418_REFERENCE_H
#define CIA418_REFERENCE_H

#include <stdbool.h>
#include <stdint.h>

#define CIA418_STATUS_READY 0x01U
#define CIA418_STATUS_FAULT 0x80U

typedef struct {
    uint8_t battery_status;
    uint8_t charger_status;
    int16_t temperature;
    uint8_t battery_type;
    uint16_t ah_capacity;
    uint16_t maximum_charge_current;
    uint16_t number_of_cells;
    uint32_t cumulative_total_ah_charge;
    uint16_t ah_expended_since_last_charge;
    uint16_t ah_returned_during_last_charge;
    uint16_t ah_since_last_equalization;
    uint32_t battery_voltage;
    uint16_t charge_current_requested;
    uint8_t charger_state_of_charge;
    uint8_t battery_state_of_charge;
    uint8_t water_level_status;
    uint32_t battery_serial_number[3];
    uint32_t battery_id[5];
    uint32_t vehicle_serial_number[5];
    uint32_t vehicle_id[5];
    uint16_t date_of_last_equalization[2];
    bool safe_fault;
} Cia418ReferenceState;

void Cia418Reference_Init(Cia418ReferenceState *state);
bool Cia418Reference_UpdateMeasurements(Cia418ReferenceState *state,
                                        uint32_t battery_voltage,
                                        int16_t temperature,
                                        uint8_t battery_state_of_charge,
                                        uint16_t charge_current_requested);
bool Cia418Reference_WriteObject(Cia418ReferenceState *state,
                                 uint16_t index,
                                 uint8_t sub_index,
                                 uint32_t value);
bool Cia418Reference_ReadObject(const Cia418ReferenceState *state,
                                uint16_t index,
                                uint8_t sub_index,
                                uint32_t *value);
void Cia418Reference_ForceSafe(Cia418ReferenceState *state);

#endif /* CIA418_REFERENCE_H */
