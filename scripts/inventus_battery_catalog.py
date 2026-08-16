"""Inventus battery test-profile Object Dictionary catalog.

The populated application rows are transcribed from the issue workbook
CANopen.cmd.xlsx (issue #10 attachment). The three identity strings are kept
in the same reviewable CSV source with empty defaults until the product owner
approves the actual device, hardware, and software version values. This is a
test-only catalog: application values remain raw wire-width values until
signed semantics, ranges, persistence, and runtime behavior are approved.
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


def _parse_default(value: str, *, preserve_empty: bool = False) -> str:
    value = value.strip()
    return value if value or preserve_empty else "0"


SCALARS = []
RECORDS = []
ARRAYS = []
SOURCE_ROWS = []
APPLICATION_SOURCE_ROWS = []
IDENTITY_INDICES = (0x1008, 0x1009, 0x100A)
with SOURCE.open(newline="", encoding="utf-8") as stream:
    for row in csv.DictReader(stream):
        index = int(row["index"], 0)
        name = row["name"].strip()
        access = row["access"].strip()
        kind = row.get("kind", "application").strip() or "application"
        width = int(row["bytes"], 10)
        default = _parse_default(row["default"], preserve_empty=(kind == "identity"))
        unit = row["unit"].strip()
        ident = _identifier(index, name)
        SOURCE_ROWS.append((index, name, access, width, unit, default, ident, kind))
        if kind == "identity":
            ctype = row["ctype"].strip()
            eds_type = int(row["eds_type"], 0)
            if not ctype.startswith("char[") or eds_type != 0x0009:
                raise ValueError(f"invalid visible-string identity metadata for {index:#06x}")
            SCALARS.append((index, ident, ctype, eds_type, access, default, 0))
            continue
        if kind == "diagnostic_array":
            ctype = row["ctype"].strip()
            eds_type = int(row["eds_type"], 0)
            count = int(row.get("count", ""), 0)
            if ctype != "uint8_t" or eds_type != 0x0005 or width != 1:
                raise ValueError(f"invalid raw diagnostic-array metadata for {index:#06x}")
            if access not in {"ro", "rw"} or count != 0xFE:
                raise ValueError(f"invalid bounded diagnostic-array metadata for {index:#06x}")
            ARRAYS.append((index, ident, ctype, eds_type, access, count, []))
            continue
        if kind != "application":
            raise ValueError(f"unsupported catalog row kind {kind!r} for {index:#06x}")
        APPLICATION_SOURCE_ROWS.append((index, name, access, width, unit, default, ident, kind))
        if width not in (1, 2):
            raise ValueError(f"unsupported width for {index:#06x}: {width}")
        ctype = "uint8_t" if width == 1 else "uint16_t"
        eds_type = 0x0005 if width == 1 else 0x0006
        if access == "array":
            # CANopen sub-index is uint8_t; sub-index 0 is the count, so the
            # largest representable data range is 1..0xFE. The workbook's
            # 0x00..0xFF notation is retained in the source CSV and called
            # out as a limitation in the test-profile documentation.
            ARRAYS.append((index, ident, ctype, eds_type, "ro", 0xFE, []))
        else:
            SCALARS.append((index, ident, ctype, eds_type, access, default, 0))

# The requested TPDO5/TPDO6 communication records are emitted with disabled
# COB-IDs and event-driven transmission defaults. These are test-safe defaults,
# not product configuration or node-ID policy.
def _tpdo_communication_record(number: int):
    return (
        0x1800 + number - 1,
        f"TPDOCommunicationParameter{number}",
        [
            (1, "COB_IDUsedByTPDO", "uint32_t", 0x0007, "rw", "0xC0000000"),
            (2, "transmissionType", "uint8_t", 0x0005, "rw", "0xFE"),
            (3, "inhibitTime", "uint16_t", 0x0006, "rw", "0"),
            (5, "eventTimer", "uint16_t", 0x0006, "rw", "0"),
            (6, "SYNCStartValue", "uint8_t", 0x0005, "rw", "0"),
        ],
    )


def _tpdo_mapping_record(index: int, values: list[int]):
    fields = [(0, "numberOfMappedApplicationObjectsInPDO", "uint8_t", 0x0005, "rw", str(len(values)))]
    fields.extend((position, f"applicationObject{position}", "uint32_t", 0x0007, "rw", f"0x{value:08X}")
                  for position, value in enumerate(values, start=1))
    fields.extend((position, f"applicationObject{position}", "uint32_t", 0x0007, "rw", "0")
                  for position in range(len(values) + 1, 9))
    return index, f"TPDOMappingParameter{index - 0x1A00 + 1}", fields


# The workbook maps these application objects into six TPDOs. The mappings are
# retained as checked-in source metadata and are applied to 0x1A00..0x1A05.
PDO_MAPPINGS = {
    0x1A00: [0x48500008, 0x48510008, 0x48520010, 0x48530010, 0x48540010],
    0x1A01: [0x48550010, 0x48560010, 0x48570010, 0x48580008, 0x48590008],
    0x1A02: [0x485A0010, 0x485B0010, 0x485C0010, 0x485D0010],
    0x1A03: [0x485E0008, 0x485F0008, 0x48600008, 0x48610008, 0x48620010, 0x48630010],
    0x1A04: [0x48640010, 0x48650010, 0x48660010, 0x48670010],
    0x1A05: [0x48680010, 0x48690008, 0x486A0010, 0x486B0010, 0x486C0008],
}

# The pinned DS301 template already supplies 0x1800..0x1803 and 0x1A00..0x1A03.
# The Inventus test profile adds TPDO5/TPDO6 communication and mapping records.
RECORDS = [_tpdo_communication_record(5), _tpdo_communication_record(6)]
RECORDS.extend(_tpdo_mapping_record(index, values)
               for index, values in PDO_MAPPINGS.items() if index >= 0x1A04)

for mapping in PDO_MAPPINGS.values():
    for value in mapping:
        index = (value >> 16) & 0xFFFF
        for item_position, item in enumerate(SCALARS):
            if item[0] == index:
                SCALARS[item_position] = (*item[:6], 1)
                break

APPLICATION_INDICES = sorted({row[0] for row in APPLICATION_SOURCE_ROWS})
REQUESTED_INDICES = APPLICATION_INDICES
DIAGNOSTIC_INDICES = tuple(sorted(index for index, *_ in ARRAYS if index >= 0xD000))
EXPECTED_APPLICATION_OBJECT_COUNT = 60
EXPECTED_DIAGNOSTIC_ARRAY_COUNT = 2
assert IDENTITY_INDICES == (0x1008, 0x1009, 0x100A)
assert len(REQUESTED_INDICES) == EXPECTED_APPLICATION_OBJECT_COUNT
assert len({index for index, *_ in APPLICATION_SOURCE_ROWS}) == EXPECTED_APPLICATION_OBJECT_COUNT
assert all(0x4800 <= index <= 0x4921 for index in REQUESTED_INDICES)
assert DIAGNOSTIC_INDICES == (0xD000, 0xD001)
assert len(DIAGNOSTIC_INDICES) == EXPECTED_DIAGNOSTIC_ARRAY_COUNT
