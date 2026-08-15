/*
 * SPDX-License-Identifier: Apache-2.0
 */
#include "cia418_reference.h"

#include <string.h>

void
Cia418Reference_Init(Cia418ReferenceState *state) {
    if (state == NULL) {
        return;
    }
    (void)memset(state, 0, sizeof(*state));
    state->battery_status = CIA418_STATUS_READY;
    state->charger_state_of_charge = 0U;
    state->battery_state_of_charge = 0U;
    state->safe_fault = false;
}

void
Cia418Reference_ForceSafe(Cia418ReferenceState *state) {
    if (state == NULL) {
        return;
    }
    state->safe_fault = true;
    state->battery_status |= CIA418_STATUS_FAULT;
    state->charge_current_requested = 0U;
    state->charger_status = 0U;
}

bool
Cia418Reference_UpdateMeasurements(Cia418ReferenceState *state,
                                   uint32_t battery_voltage,
                                   int16_t temperature,
                                   uint8_t battery_state_of_charge,
                                   uint16_t charge_current_requested) {
    if (state == NULL || battery_state_of_charge > 100U) {
        Cia418Reference_ForceSafe(state);
        return false;
    }
    state->battery_voltage = battery_voltage;
    state->temperature = temperature;
    state->battery_state_of_charge = battery_state_of_charge;
    state->charge_current_requested = charge_current_requested;
    state->safe_fault = false;
    state->battery_status &= (uint8_t)~CIA418_STATUS_FAULT;
    return true;
}

static bool
valid_array_sub_index(uint8_t sub_index, uint8_t max_sub_index) {
    return sub_index >= 1U && sub_index <= max_sub_index;
}

bool
Cia418Reference_WriteObject(Cia418ReferenceState *state,
                            uint16_t index,
                            uint8_t sub_index,
                            uint32_t value) {
    if (state == NULL) {
        return false;
    }
    switch (index) {
        case 0x6001U:
            if (value > UINT8_MAX) return false;
            state->charger_status = (uint8_t)value;
            return true;
        case 0x6052U:
            if (value > UINT16_MAX) return false;
            state->ah_returned_during_last_charge = (uint16_t)value;
            return true;
        case 0x6053U:
            if (value > UINT16_MAX) return false;
            state->ah_since_last_equalization = (uint16_t)value;
            return true;
        case 0x6080U:
            if (value > 100U) return false;
            state->charger_state_of_charge = (uint8_t)value;
            return true;
        case 0x6054U:
            if (!valid_array_sub_index(sub_index, 2U) || value > UINT16_MAX) return false;
            state->date_of_last_equalization[sub_index - 1U] = (uint16_t)value;
            return true;
        default:
            return false;
    }
}

bool
Cia418Reference_ReadObject(const Cia418ReferenceState *state,
                           uint16_t index,
                           uint8_t sub_index,
                           uint32_t *value) {
    if (state == NULL || value == NULL) {
        return false;
    }
    switch (index) {
        case 0x6000U: *value = state->battery_status; return true;
        case 0x6001U: *value = state->charger_status; return true;
        case 0x6010U: *value = (uint16_t)state->temperature; return true;
        case 0x6020U:
            switch (sub_index) {
                case 1U: *value = state->battery_type; return true;
                case 2U: *value = state->ah_capacity; return true;
                case 3U: *value = state->maximum_charge_current; return true;
                case 4U: *value = state->number_of_cells; return true;
                default: return false;
            }
        case 0x6050U: *value = state->cumulative_total_ah_charge; return true;
        case 0x6051U: *value = state->ah_expended_since_last_charge; return true;
        case 0x6052U: *value = state->ah_returned_during_last_charge; return true;
        case 0x6053U: *value = state->ah_since_last_equalization; return true;
        case 0x6054U:
            if (!valid_array_sub_index(sub_index, 2U)) return false;
            *value = state->date_of_last_equalization[sub_index - 1U]; return true;
        case 0x6060U: *value = state->battery_voltage; return true;
        case 0x6070U: *value = state->charge_current_requested; return true;
        case 0x6080U: *value = state->charger_state_of_charge; return true;
        case 0x6081U: *value = state->battery_state_of_charge; return true;
        case 0x6090U: *value = state->water_level_status; return true;
        default: return false;
    }
}
