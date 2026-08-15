#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "cia418_reference.h"

int main(void) {
    Cia418ReferenceState state;
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

    puts("cia418_reference: PASS");
    return 0;
}
