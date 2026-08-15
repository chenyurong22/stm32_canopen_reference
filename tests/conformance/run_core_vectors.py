#!/usr/bin/env python3
"""Validate the repository's machine-readable core CANopen regression vectors.

This is a deterministic software contract runner. It does not claim physical,
transceiver, timing, HIL, or official CANopen conformance evidence.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path


def fail(message: str) -> None:
    raise AssertionError(message)


def validate_vector(node_id: int, vector: dict) -> None:
    request = vector["request"]
    expected = vector["expected"]
    cob_id = int(request["cob_id"])
    payload_hex = request["data"]
    if not 0 <= cob_id <= 0x7FF:
        fail(f"{vector['name']}: COB-ID out of range")
    if len(payload_hex) % 2:
        fail(f"{vector['name']}: payload has odd hex length")
    payload = bytes.fromhex(payload_hex)
    if len(payload) > 8:
        fail(f"{vector['name']}: classic CAN payload exceeds 8 bytes")

    service = expected["service"]
    if service == "nmt":
        if cob_id != 0 or payload != bytes((1, node_id)):
            fail(f"{vector['name']}: invalid NMT start vector")
    elif service == "sync":
        if cob_id != 0x80 or payload:
            fail(f"{vector['name']}: invalid SYNC vector")
    elif service == "sdo_upload_request":
        if cob_id != 0x600 + node_id or payload[:4] != bytes((0x40, 0x18, 0x10, 0)):
            fail(f"{vector['name']}: invalid SDO upload vector")
        if expected["index"] != 0x1018 or expected["subindex"] != 0:
            fail(f"{vector['name']}: invalid SDO object metadata")
    elif service == "emcy":
        if cob_id != 0x80 + node_id or len(payload) != 8:
            fail(f"{vector['name']}: invalid EMCY vector")
        if int.from_bytes(payload[:2], "little") != expected["error_code"]:
            fail(f"{vector['name']}: EMCY code mismatch")
    elif service == "heartbeat":
        if cob_id != 0x700 + node_id or payload != bytes((5,)):
            fail(f"{vector['name']}: invalid heartbeat vector")
    elif service == "lss":
        if cob_id != 0x7E4 or payload[:2] != bytes((4, 1)):
            fail(f"{vector['name']}: invalid LSS vector")
    else:
        fail(f"{vector['name']}: unsupported service {service!r}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("path", nargs="?", type=Path, default=Path(__file__).with_name("core_vectors.json"))
    args = parser.parse_args()
    document = json.loads(args.path.read_text(encoding="utf-8"))
    if document.get("schema") != "canopen-reference-core-vector-v1":
        fail("unsupported vector schema")
    node_id = int(document["node_id"])
    if not 1 <= node_id <= 127:
        fail("node_id must be in the CANopen range 1..127")
    vectors = document.get("vectors")
    if not isinstance(vectors, list) or not vectors:
        fail("vectors must be a non-empty list")
    for vector in vectors:
        validate_vector(node_id, vector)
    print(f"validated {len(vectors)} core CANopen vectors (software contract only)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
