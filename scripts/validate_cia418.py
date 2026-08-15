"""Validate the bounded CiA 418 catalog and generated artifacts."""
from pathlib import Path
import re
import sys

from cia418_catalog import ARRAYS, MANDATORY_APPLICATION_INDICES, RECORDS, REQUESTED_INDICES, SCALARS

ROOT = Path(__file__).resolve().parents[1]


def main():
    scalar_indices = [entry[0] for entry in SCALARS]
    record_indices = [entry[0] for entry in RECORDS]
    array_indices = [entry[0] for entry in ARRAYS]
    all_indices = scalar_indices + record_indices + array_indices
    assert len(all_indices) == len(set(all_indices)), "duplicate CiA 418 index"
    assert set(MANDATORY_APPLICATION_INDICES).issubset(set(all_indices))
    assert REQUESTED_INDICES == sorted(all_indices)
    for index, _ident, _ctype, _eds, _access, count in ARRAYS:
        assert 1 <= count <= 5, (hex(index), count)
    for index, _ident, fields in RECORDS:
        assert fields and [field[0] for field in fields] == list(range(1, len(fields) + 1)), hex(index)

    header = (ROOT / "Generated" / "cia418_OD.h").read_text()
    source = (ROOT / "Generated" / "cia418_OD.c").read_text()
    eds = (ROOT / "ObjectDictionary" / "stm32f767_cia418_reference.eds").read_text()
    for index in REQUESTED_INDICES:
        token = f"0x{index:04X}"
        assert token in source or f"x{index:04X}_" in header, token
        assert token in eds, token
    assert "SupportedObjects=7" in eds
    assert "SupportedObjects=14" in eds
    print(f"CiA 418 validation passed: {len(REQUESTED_INDICES)} application objects")


if __name__ == "__main__":
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    main()
