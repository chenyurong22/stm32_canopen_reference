# Inventus Battery Test Profile

This profile contains the **60 populated application indices** from the Inventus workbook attached to GitHub issues #6, #9, and #10, plus the requested standard identity/PDO objects, a structured diagnostic record at `0xD000`, and a bounded raw diagnostic array at `0xD001`. It is available only through the opt-in CMake option `CANOPEN_REFERENCE_ENABLE_INVENTUS_BATTERY=ON`.

> This is a test-only, non-commercial interoperability profile. It is not part of the frozen CiA 401 v1 personality and must not be used to claim production conformance.

## Source and isolation

The reviewable sources are `product/inventus_battery_od.csv` for the application/identity/PDO catalog and `product/inventus_battery_d000.csv` for the workbook-derived structured D000 fields. The deterministic generator is `scripts/generate_inventus_battery_od.py`; it emits `Generated/inventus_battery_OD.h`, `Generated/inventus_battery_OD.c`, and `ObjectDictionary/stm32f767_inventus_battery_test.eds`. The default `Generated/OD.c` and generic `Generated/cia418_OD.c` artifacts are not modified by this profile.

The generated application indices are the 60 populated rows between `0x4800` and `0x4921`: the range is sparse and does **not** mean that every numeric index in the interval exists. The workbook’s stated byte widths are preserved as raw `UNSIGNED8` or `UNSIGNED16` wire values. Signedness, enumerations, scaling limits, persistence, and runtime safety semantics remain unapproved test-profile assumptions and must be resolved before any product use.

The profile also exposes standard identity objects `0x1008` (Manufacturer Device Name), `0x1009` (Manufacturer Hardware Version), and `0x100A` (Manufacturer Software Version) as fixed-length read-only `VISIBLE_STRING` fields of 32, 16, and 16 bytes. Their defaults are zero-filled/empty because the latest issue request supplied the object names but no approved product strings. They must be populated by the test harness or hardware owner before identity interoperability is assessed.

The workbook supplies these defaults, including nonzero values such as `0x485B=20000`, `0x485D=28000`, `0x486C=0x04`, `0x4880=15`, `0x4881=58100`, `0x4882=4150`, `0x4883=58100`, and `0x4903=9`. They are transcribed for test reproducibility only; they are not hardware-owner approval.

## PDO mapping

The catalog retains the six workbook-declared TPDO mapping groups `0x1A00` through `0x1A05` and validates that every referenced application index exists and that each map is at most 64 bits. The Inventus test OD now emits `0x1A04` and `0x1A05` with the workbook mappings, and emits the corresponding TPDO5/TPDO6 communication records at `0x1804` and `0x1805`. The communication records use disabled `0xC0000000` COB-ID defaults, transmission type `0xFE`, and zero timers; they are available for SDO/PDO configuration testing but are not activated by firmware. Mapping activation and COB-ID/node-ID policy require an explicit test-harness decision and are not product configuration.

## 0x4900 limitation

The workbook writes `0x4900` sub-indices as `0x00~0xFF`. CANopenNode represents the array count and data sub-index with an 8-bit sub-index, so the generated standard array exposes the representable data range `0x01..0xFE` with sub-index `0` as the count. The `0xFF` endpoint requires a separate vendor-specific transport definition and is intentionally not guessed.

## D000/D001 diagnostic scope

Issue #11’s attached workbook provides a machine-readable sparse table for `0xD000`. The generated test profile now exposes sub-index `0x00` plus the workbook-defined entries through `0x70`, preserving the named fields, mixed 8/16/32-bit widths, documented defaults, and Read/Read/Write access modes. Undefined gaps remain absent from the OD and therefore return `ODR_SUB_NOT_EXIST`.

The workbook contains a source inconsistency: `0xD000:00` says the highest supported sub-index is `0x29`, while the listed entries reach `0x70` and omit `0x29`. The test profile resolves the generated highest-sub-index value to `0x70`, because that is the maximum defined entry, and records the discrepancy in the catalog, validator, and tests. This is a test-profile resolution, not vendor approval.

Workbook details that are not fully specified—version byte order, signedness of `0xD000:64`, scaling limits, persistence, PDO mapping, and runtime side effects—remain provisional. The four writable fields are ordinary in-memory OD variables with no hardware callbacks. This profile is test-only and makes no production-conformance claim.

`0xD001` remains a bounded raw-byte diagnostic array with count `0xFE` and data sub-indices `0x01..0xFE`; sub-index `0xFF` is intentionally absent. The workbook identifies it only as a generic BMS command interface and does not provide the per-command metadata needed for a typed implementation.

## Build and test

```sh
python3 scripts/generate_inventus_battery_od.py
python3 scripts/validate_inventus_battery.py
make -C tests/host test-inventus-battery
cmake -S . -B build/firmware-inventus \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake \
  -DSTM32_CUBE_F7_DIR="$PWD/third_party/STM32CubeF7" \
  -DSTM32_F7_LINKER_SCRIPT="$PWD/linker/STM32F767_2M_512K_FLASH.ld" \
  -DCANOPEN_REFERENCE_ENABLE_INVENTUS_BATTERY=ON
cmake --build build/firmware-inventus --parallel 2
```

The default build remains the CiA 401 reference personality. The Inventus option is mutually exclusive with CiA 401, CiA 402, and generic CiA 418 at the generated-OD level.
