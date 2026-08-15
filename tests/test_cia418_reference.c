#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "cia418_reference.h"

int main(void) {
    Cia418ReferenceState state;
    CIA418_OD_APP_t od_app = {0};
    uint32_t value = 0U;
    Cia418Reference_Init(&state);
    assert(state.battery_status == CIA418_STATUS_READY);
    assert(!state.safe_fault);

    assert(Cia418Reference_UpdateMeasurements(&state, 48000U, 250, 75U, 120U));
    assert(Cia418Reference_ReadObject(&state, 0x6060U, 0U, &value) && value == 48000U);
    assert(Cia418Reference_ReadObject(&state, 0x6081U, 0U, &value) && value == 75U);
    assert(Cia418Reference_ReadObject(&state, 0x6010U, 0U, &value) && value == 250U);

    assert(!Cia418Reference_UpdateMeasurements(&state, 48000U, 250, 101U, 120U));
    assert(state.safe_fault);
    assert(state.charge_current_requested == 0U);
    assert((state.battery_status & CIA418_STATUS_FAULT) != 0U);

    assert(Cia418Reference_WriteObject(&state, 0x6001U, 0U, 2U));
    assert(Cia418Reference_WriteObject(&state, 0x6080U, 0U, 80U));
    assert(Cia418Reference_WriteObject(&state, 0x6054U, 1U, 2026U));
    assert(!Cia418Reference_WriteObject(&state, 0x6081U, 0U, 50U));
    assert(!Cia418Reference_WriteObject(&state, 0x6080U, 0U, 101U));
    assert(Cia418Reference_ReadObject(&state, 0x6054U, 1U, &value) && value == 2026U);
    assert(!Cia418Reference_ReadObject(&state, 0x6054U, 3U, &value));

    state.battery_serial_number[0] = 0x12345678U;
    state.vehicle_id[0] = 0xABCDEF01U;
    Cia418Reference_SyncToGeneratedOd(&state, &od_app);
    assert(od_app.x6060_batteryVoltage == state.battery_voltage);
    assert(od_app.x6081_batteryStateOfCharge == state.battery_state_of_charge);
    assert(od_app.x6054_dateOfLastEqualization[0] == state.date_of_last_equalization[0]);
    assert(od_app.x6030_batterySerialNumber[0] == 0x12345678U);
    assert(od_app.x6041_vehicleId[0] == 0xABCDEF01U);
    Cia418Reference_SyncToGeneratedOd(NULL, &od_app);
    Cia418Reference_SyncToGeneratedOd(&state, NULL);

    puts("cia418_reference: PASS");
    return 0;
}
