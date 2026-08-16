"""Validate the isolated Inventus battery test-profile artifacts."""

from __future__ import annotations

import re
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))

from inventus_battery_catalog import (  # noqa: E402
    PDO_MAPPINGS,
    REQUESTED_INDICES,
    SOURCE,
    EXPECTED_APPLICATION_OBJECT_COUNT,
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

    print(f"inventus battery validation: PASS ({EXPECTED_APPLICATION_OBJECT_COUNT} objects, {len(PDO_MAPPINGS)} PDO maps)")


if __name__ == "__main__":
    main()
