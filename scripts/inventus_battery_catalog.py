"""Inventus battery test-profile Object Dictionary catalog.

The populated application rows are transcribed from the issue workbook
CANopen.cmd.xlsx (issue #10 attachment). This is a test-only catalog. The
workbook supplies byte widths, so this profile deliberately represents values
as raw UNSIGNED8/UNSIGNED16 wire values until a product owner approves signed
semantics, ranges, persistence, and runtime behavior.
"""

from __future__ import annotations

import csv
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "product" / "inventus_battery_od.csv"


def _identifier(index: int, name: str) -> str:
    stem = re.sub(r"[^A-Za-z0-9]+", "_", name).strip("_").lower()
    return f"inventus_{index:04x}_{stem}"


def _parse_default(value: str) -> str:
    return value.strip() or "0"

SCALARS = []
ARRAYS = []
SOURCE_ROWS = []
with SOURCE.open(newline="", encoding="utf-8") as stream:
    for row in csv.DictReader(stream):
        index = int(row["index"], 0)
        name = row["name"].strip()
        access = row["access"].strip()
        width = int(row["bytes"], 10)
        default = _parse_default(row["default"])
        unit = row["unit"].strip()
        if width not in (1, 2):
            raise ValueError(f"unsupported width for {index:#06x}: {width}")
        ctype = "uint8_t" if width == 1 else "uint16_t"
        eds_type = 0x0005 if width == 1 else 0x0006
        ident = _identifier(index, name)
        SOURCE_ROWS.append((index, name, access, width, unit, default, ident))
        if access == "array":
            # CANopen sub-index is uint8_t; sub-index 0 is the count, so the
            # largest representable data range is 1..0xFE. The workbook's
            # 0x00..0xFF notation is retained in the source CSV and called
            # out as a limitation in the test-profile documentation.
            ARRAYS.append((index, ident, ctype, eds_type, "ro", 0xFE, []))
        else:
            SCALARS.append((index, ident, ctype, eds_type, access, default, 0))

# The workbook maps these application objects into six TPDOs. The mappings are
# kept disabled at the communication layer by default, while the mapping
# records remain available for SDO/PDO configuration testing.
PDO_MAPPINGS = {
    0x1A00: [0x48500008, 0x48510008, 0x48520010, 0x48530010, 0x48540010],
    0x1A01: [0x48550010, 0x48560010, 0x48570010, 0x48580008, 0x48590008],
    0x1A02: [0x485A0010, 0x485B0010, 0x485C0010, 0x485D0010],
    0x1A03: [0x485E0008, 0x485F0008, 0x48600008, 0x48610008, 0x48620010, 0x48630010],
    0x1A04: [0x48640010, 0x48650010, 0x48660010, 0x48670010],
    0x1A05: [0x48680010, 0x48690008, 0x486A0010, 0x486B0010, 0x486C0008],
}

for position, row in enumerate(SOURCE_ROWS):
    index = row[0]
    if any(index == ((mapping >> 16) & 0xFFFF) for values in PDO_MAPPINGS.values() for mapping in values):
        for item_position, item in enumerate(SCALARS):
            if item[0] == index:
                SCALARS[item_position] = (*item[:6], 1)
                break




REQUESTED_INDICES = sorted({index for index, *_ in SCALARS} | {index for index, *_ in ARRAYS})
EXPECTED_APPLICATION_OBJECT_COUNT = 60
assert len(REQUESTED_INDICES) == EXPECTED_APPLICATION_OBJECT_COUNT
assert len({index for index, *_ in SOURCE_ROWS}) == EXPECTED_APPLICATION_OBJECT_COUNT
