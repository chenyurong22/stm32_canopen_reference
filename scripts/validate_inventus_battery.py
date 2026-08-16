"""Validate the isolated Inventus battery test-profile artifacts."""

from __future__ import annotations

import re
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from inventus_battery_catalog import (  # noqa: E402
    DIAGNOSTIC_INDICES,
    EXPECTED_APPLICATION_OBJECT_COUNT,
    EXPECTED_DIAGNOSTIC_ARRAY_COUNT,
    IDENTITY_INDICES,
    PDO_MAPPINGS,
    RECORDS,
    REQUESTED_INDICES,
    SOURCE,
)


def fail(message: str) -> None:
    raise SystemExit(f"inventus validation failed: {message}")


def main() -> None:
    if len(REQUESTED_INDICES) != EXPECTED_APPLICATION_OBJECT_COUNT:
        fail("catalog does not contain exactly 60 populated application indices")
    if len(set(REQUESTED_INDICES)) != len(REQUESTED_INDICES):
        fail("catalog contains duplicate application indices")
    if not all(0x4800 <= index <= 0x4921 for index in REQUESTED_INDICES):
        fail("application index is outside the requested 0x4800-0x4921 range")
    if not SOURCE.exists():
        fail(f"source catalog is missing: {SOURCE}")

    header = (ROOT / "Generated" / "inventus_battery_OD.h").read_text(encoding="utf-8")
    source = (ROOT / "Generated" / "inventus_battery_OD.c").read_text(encoding="utf-8")
    eds = (ROOT / "ObjectDictionary" / "stm32f767_inventus_battery_test.eds").read_text(encoding="utf-8")

    for index in REQUESTED_INDICES:
        token = f"x{index:04X}_"
        if token not in header or f"{{0x{index:04X}," not in source:
            fail(f"generated OD is missing application object 0x{index:04X}")
        if f"[{index:04X}]" not in eds:
            fail(f"generated EDS is missing application object 0x{index:04X}")

    for index, length in ((0x1008, 32), (0x1009, 16), (0x100A, 16)):
        if index not in IDENTITY_INDICES:
            fail(f"identity catalog is missing 0x{index:04X}")
        if f"{{0x{index:04X}, 0x01, ODT_VAR" not in source:
            fail(f"generated OD is missing identity object 0x{index:04X}")
        if f"[{index:04X}]" not in eds or f"DataType=9" not in eds.split(f"[{index:04X}]", 1)[1].split("[", 1)[0]:
            fail(f"generated EDS is missing VISIBLE_STRING identity object 0x{index:04X}")
        if f"dataLength = {length}" not in source:
            fail(f"generated OD identity length is wrong for 0x{index:04X}")

    expected_records = {0x1804, 0x1805, 0x1A04, 0x1A05}
    if {index for index, *_ in RECORDS} != expected_records:
        fail("Inventus catalog record set is not exactly TPDO5/TPDO6 communication and mapping records")
    for index in sorted(expected_records):
        if f"{{0x{index:04X}," not in source or f"[{index:04X}]" not in eds:
            fail(f"generated OD/EDS is missing record 0x{index:04X}")
    if ".COB_IDUsedByTPDO = 0xC0000000" not in source:
        fail("TPDO5/TPDO6 communication defaults are not disabled")
    for reserved_index in (0x1804, 0x1805):
        record_section = eds.split(f"[{reserved_index:04X}]", 1)[1].split("[", 1)[0]
        if f"[{reserved_index:04X}sub4]" in record_section:
            fail(f"reserved TPDO sub-index 4 was incorrectly emitted for 0x{reserved_index:04X}")

    if DIAGNOSTIC_INDICES != (0xD000, 0xD001) or len(DIAGNOSTIC_INDICES) != EXPECTED_DIAGNOSTIC_ARRAY_COUNT:
        fail("catalog must contain exactly bounded diagnostic arrays 0xD000 and 0xD001")
    for index, access in ((0xD000, "ro"), (0xD001, "rw")):
        token = f"x{index:04X}_"
        if token not in header or f"{{0x{index:04X}, 0xFF, ODT_ARR" not in source:
            fail(f"generated OD is missing bounded diagnostic array 0x{index:04X}")
        if f"[{index:04X}]" not in eds or f"SubNumber=0xFF" not in eds.split(f"[{index:04X}]", 1)[1].split("[", 1)[0]:
            fail(f"generated EDS is missing bounded diagnostic array 0x{index:04X}")
        section = eds.split(f"[{index:04X}]", 1)[1].split("[", 1)[0]
        if f"AccessType={access}" not in section or not re.search(r"^DataType=(?:0x0005|5)$", section, re.MULTILINE):
            fail(f"diagnostic array 0x{index:04X} has incorrect raw-byte metadata")
        if f"[{index:04X}sub255]" in eds:
            fail(f"diagnostic array 0x{index:04X} incorrectly emits unsupported sub-index 255")
    if "diagnostic" not in SOURCE.read_text(encoding="utf-8"):
        fail("source catalog does not disclose diagnostic-array provenance")

    for mapping_index, mappings in PDO_MAPPINGS.items():
        if len(mappings) == 0 or sum(value & 0xFF for value in mappings) > 64:
            fail(f"invalid PDO mapping definition 0x{mapping_index:04X}")
        for value in mappings:
            app_index = (value >> 16) & 0xFFFF
            if app_index not in REQUESTED_INDICES:
                fail(f"PDO mapping 0x{mapping_index:04X} references absent object 0x{app_index:04X}")

    optional_section = eds.split("[OptionalObjects]", 1)[-1].split("[", 1)[0]
    supported_match = re.search(r"^SupportedObjects=(\d+)$", optional_section, re.MULTILINE)
    if supported_match is None or int(supported_match.group(1)) < EXPECTED_APPLICATION_OBJECT_COUNT:
        fail("EDS OptionalObjects SupportedObjects is smaller than the 60 requested application objects")

    print(f"inventus battery validation: PASS ({EXPECTED_APPLICATION_OBJECT_COUNT} application objects, {len(DIAGNOSTIC_INDICES)} bounded diagnostic arrays, {len(PDO_MAPPINGS)} PDO maps, {len(IDENTITY_INDICES)} identity objects)")


if __name__ == "__main__":
    main()
