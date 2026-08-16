#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "canopen_reference_od.h"

static const uint16_t requested_indices[] = {
    0x4800, 0x4801, 0x4802, 0x4803, 0x4804, 0x4805, 0x4806, 0x4807,
    0x4808, 0x4809, 0x480A, 0x480B, 0x480C, 0x480D, 0x480E, 0x480F,
    0x4810, 0x4811, 0x4812, 0x4813, 0x4819, 0x4850, 0x4851, 0x4852,
    0x4853, 0x4854, 0x4855, 0x4856, 0x4857, 0x4858, 0x4859, 0x485A,
    0x485B, 0x485C, 0x485D, 0x485E, 0x485F, 0x4860, 0x4861, 0x4862,
    0x4863, 0x4864, 0x4865, 0x4866, 0x4867, 0x4868, 0x4869, 0x486A,
    0x486B, 0x486C, 0x4880, 0x4881, 0x4882, 0x4883, 0x4900, 0x4901,
    0x4903, 0x4904, 0x4920, 0x4921
};

static OD_IO_t get_io(uint16_t index, uint8_t sub_index) {
    OD_IO_t io;
    OD_entry_t *entry = OD_find(OD, index);
    assert(entry != NULL);
    assert(OD_getSub(entry, sub_index, &io, false) == ODR_OK);
    return io;
}

int main(void) {
    assert(sizeof(requested_indices) / sizeof(requested_indices[0]) == 60U);
    assert(OD != NULL);

    for (size_t position = 0U;
         position < sizeof(requested_indices) / sizeof(requested_indices[0]);
         ++position) {
        OD_entry_t *entry = OD_find(OD, requested_indices[position]);
        assert(entry != NULL);
    }

    OD_IO_t bq_count = get_io(0x4900U, 0U);
    assert(bq_count.stream.dataLength == 1U);
    assert((bq_count.stream.attribute & ODA_SDO_W) == 0U);
    OD_IO_t bq = get_io(0x4900U, 1U);
    assert(bq.stream.dataLength == 2U);
    assert((bq.stream.attribute & ODA_SDO_W) == 0U);

    OD_IO_t soh = get_io(0x4800U, 0U);
    assert(soh.stream.dataLength == 1U);
    assert((soh.stream.attribute & ODA_SDO_W) == 0U);

    OD_IO_t sleep = get_io(0x4819U, 0U);
    assert(sleep.stream.dataLength == 2U);
    assert((sleep.stream.attribute & ODA_SDO_W) != 0U);
    uint16_t sleep_value = 1U;
    OD_size_t count_written = 0U;
    assert(sleep.write != NULL);
    assert(sleep.write(&sleep.stream, &sleep_value, sizeof(sleep_value), &count_written) == ODR_OK);
    assert(count_written == sizeof(sleep_value));

    printf("inventus_battery_od: PASS (%u application objects)\n",
           (unsigned)(sizeof(requested_indices) / sizeof(requested_indices[0])));
    return 0;
}
