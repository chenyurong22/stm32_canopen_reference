"""Host-independent CANopen frame and communication-state contract tests."""
from __future__ import annotations

import importlib.util
import sys
from pathlib import Path
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
HOST = ROOT / "tests" / "host"
sys.path.insert(0, str(HOST))
from can_socket import Frame  # noqa: E402

MODEL_PATH = ROOT / "middleware" / "canopen" / "examples" / "canopen_vcan_device.py"
SPEC = importlib.util.spec_from_file_location("canopen_vcan_device_under_test", MODEL_PATH)
assert SPEC is not None and SPEC.loader is not None
MODEL = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODEL)


NODE_ID = 0x0A


def build_device(sent: list[tuple[int, bytes]]) -> object:
    with patch.object(MODEL, "open_socket", return_value=object()), patch.object(
        MODEL, "send", side_effect=lambda _sock, arbitration_id, data: sent.append((arbitration_id, data))
    ):
        device = MODEL.Device("fake", NODE_ID, 100, False)
    return device


def test_nmt_state_machine_and_bootup_contract() -> None:
    sent: list[tuple[int, bytes]] = []
    device = build_device(sent)
    sent.clear()

    device.handle(Frame(0x000, bytes((0x01, NODE_ID))))
    assert device.state == 0x05
    assert sent[-1] == (0x700 + NODE_ID, b"\x05")

    device.handle(Frame(0x000, bytes((0x02, NODE_ID))))
    assert device.state == 0x04
    assert sent[-1] == (0x700 + NODE_ID, b"\x04")

    device.handle(Frame(0x000, bytes((0x80, 0x00))))
    assert device.state == 0x7F
    assert sent[-1] == (0x700 + NODE_ID, b"\x7f")

    device.handle(Frame(0x000, bytes((0x81, NODE_ID))))
    assert device.state == 0x7F
    assert sent[-1] == (0x700 + NODE_ID, b"\x00")


def test_malformed_nmt_and_state_gated_sync_are_ignored() -> None:
    sent: list[tuple[int, bytes]] = []
    device = build_device(sent)
    sent.clear()

    device.handle(Frame(0x000, b"\x01"))
    assert device.state == 0x7F
    assert sent == []

    device.handle(Frame(0x080, b""))
    assert sent == []

    device.handle(Frame(0x000, bytes((0x01, NODE_ID))))
    sent.clear()
    device.handle(Frame(0x080, b""))
    assert sent == [(0x180 + NODE_ID, b"\x00\x00")]


def test_sdo_abort_contracts() -> None:
    sent: list[tuple[int, bytes]] = []
    device = build_device(sent)
    sent.clear()

    device.handle(Frame(0x600 + NODE_ID, bytes((0x40, 0x99, 0x99, 0, 0, 0, 0, 0))))
    assert sent[-1] == (0x580 + NODE_ID, bytes((0x80, 0x99, 0x99, 0)) + (0x06020000).to_bytes(4, "little"))

    device.handle(Frame(0x600 + NODE_ID, bytes((0x23, 0x18, 0x10, 1, 0, 0, 0, 0))))
    assert sent[-1] == (0x580 + NODE_ID, bytes((0x80, 0x18, 0x10, 1)) + (0x06010002).to_bytes(4, "little"))
