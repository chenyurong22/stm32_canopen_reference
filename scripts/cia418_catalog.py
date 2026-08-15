"""Bounded CiA 418 battery-module catalog.

The catalog is intentionally separate from the default CiA 401/402 OD because
several CiA 418 indexes overlap profile-specific objects already used by the
reference drive personality. A product selects one profile personality and
regenerates its OD/EDS from the corresponding catalog.
"""

# index, identifier, C type, CiA 306 data type, access, category
SCALARS = [
    (0x6000, "batteryStatus", "uint8_t", 0x0005, "ro", "M"),
    (0x6001, "chargerStatus", "uint8_t", 0x0005, "rw", "M"),
    (0x6010, "temperature", "int16_t", 0x0003, "ro", "M"),
    (0x6050, "cumulativeTotalAhCharge", "uint32_t", 0x0007, "ro", "O"),
    (0x6051, "ahExpendedSinceLastCharge", "uint16_t", 0x0006, "ro", "O"),
    (0x6052, "ahReturnedDuringLastCharge", "uint16_t", 0x0006, "rw", "C"),
    (0x6053, "ahSinceLastEqualization", "uint16_t", 0x0006, "rw", "O"),
    (0x6060, "batteryVoltage", "uint32_t", 0x0007, "ro", "C"),
    (0x6070, "chargeCurrentRequested", "uint16_t", 0x0006, "ro", "C"),
    (0x6080, "chargerStateOfCharge", "uint8_t", 0x0005, "rw", "C"),
    (0x6081, "batteryStateOfCharge", "uint8_t", 0x0005, "ro", "C"),
    (0x6090, "waterLevelStatus", "uint8_t", 0x0005, "ro", "O"),
]

# index, identifier, [(sub-index, member, C type, CiA 306 data type, access)]
RECORDS = [
    (0x6020, "batteryParameters", [
        (1, "batteryType", "uint8_t", 0x0005, "ro"),
        (2, "ahCapacity", "uint16_t", 0x0006, "ro"),
        (3, "maximumChargeCurrent", "uint16_t", 0x0006, "ro"),
        (4, "numberOfCells", "uint16_t", 0x0006, "ro"),
    ]),
]

# index, identifier, C element type, CiA 306 data type, access, max count
ARRAYS = [
    (0x6030, "batterySerialNumber", "uint32_t", 0x0007, "ro", 3),
    (0x6031, "batteryId", "uint32_t", 0x0007, "ro", 5),
    (0x6040, "vehicleSerialNumber", "uint32_t", 0x0007, "ro", 5),
    (0x6041, "vehicleId", "uint32_t", 0x0007, "ro", 5),
    (0x6054, "dateOfLastEqualization", "uint16_t", 0x0006, "rw", 2),
]

REQUESTED_INDICES = sorted(
    {index for index, *_ in SCALARS}
    | {index for index, *_ in RECORDS}
    | {index for index, *_ in ARRAYS}
)

MANDATORY_APPLICATION_INDICES = {0x6000, 0x6001, 0x6010, 0x6020}
