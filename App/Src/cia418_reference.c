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

void
Cia418Reference_SyncToGeneratedOd(const Cia418ReferenceState *state,
                                   CIA418_OD_APP_t *od_app) {
    if (state == NULL || od_app == NULL) {
        return;
    }

    (void)memset(od_app, 0, sizeof(*od_app));
    od_app->x6000_batteryStatus = state->battery_status;
    od_app->x6001_chargerStatus = state->charger_status;
    od_app->x6010_temperature = state->temperature;
    od_app->x6050_cumulativeTotalAhCharge = state->cumulative_total_ah_charge;
    od_app->x6051_ahExpendedSinceLastCharge = state->ah_expended_since_last_charge;
    od_app->x6052_ahReturnedDuringLastCharge = state->ah_returned_during_last_charge;
    od_app->x6053_ahSinceLastEqualization = state->ah_since_last_equalization;
    od_app->x6060_batteryVoltage = state->battery_voltage;
    od_app->x6070_chargeCurrentRequested = state->charge_current_requested;
    od_app->x6080_chargerStateOfCharge = state->charger_state_of_charge;
    od_app->x6081_batteryStateOfCharge = state->battery_state_of_charge;
    od_app->x6090_waterLevelStatus = state->water_level_status;
    od_app->x6020_batteryParameters.batteryType = state->battery_type;
    od_app->x6020_batteryParameters.ahCapacity = state->ah_capacity;
    od_app->x6020_batteryParameters.maximumChargeCurrent = state->maximum_charge_current;
    od_app->x6020_batteryParameters.numberOfCells = state->number_of_cells;

    (void)memcpy(&od_app->x6030_batterySerialNumber[0], state->battery_serial_number,
                 sizeof(state->battery_serial_number));
    (void)memcpy(&od_app->x6031_batteryId[0], state->battery_id, sizeof(state->battery_id));
    (void)memcpy(&od_app->x6040_vehicleSerialNumber[0], state->vehicle_serial_number,
                 sizeof(state->vehicle_serial_number));
    (void)memcpy(&od_app->x6041_vehicleId[0], state->vehicle_id, sizeof(state->vehicle_id));
    (void)memcpy(&od_app->x6054_dateOfLastEqualization[0], state->date_of_last_equalization,
                 sizeof(state->date_of_last_equalization));
}
