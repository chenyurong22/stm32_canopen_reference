# Issue #13 Protocol Acceptance Procedure

Issue #13 contains protocol screenshots for the Inventus test personality. This procedure records the values that are accepted as standard CANopen wire-contract checks without replacing CANopenNode with a second protocol implementation.

## Accepted standard contract

For node ID `N` in the generic CANopen range `0x01..0x7F`, the host acceptance test checks the following formulas:

| Service | COB-ID or command |
|---|---|
| NMT | `0x000` |
| EMCY | `0x080 + N` |
| SDO client request | `0x600 + N` |
| SDO server response | `0x580 + N` |
| Heartbeat | `0x700 + N` |
| TPDO1–TPDO4 | `0x180/0x280/0x380/0x480 + N` |
| TPDO5–TPDO6 | `0x190/0x290 + N` when explicitly configured |
| LSS master/slave | `0x7E5` / `0x7E4` |

The SDO acceptance vectors cover upload request `0x40`, expedited upload responses `0x4F`, `0x4B`, and `0x43`, and successful download response `0x60`. Frame payload parsing remains in the existing CANopenNode-backed or Python contract tests; `middleware/canopen/core/canopen_reference_protocol.h` only centralizes formulas and constants for deterministic unit checks.

## Run the acceptance test

```sh
make -C tests/host test-protocol-contract
```

The test verifies node-ID boundaries, all six TPDO base formulas, the standard heartbeat calculation, the SDO command bytes, and the fixed LSS identifiers. It also asserts that the screenshot’s `0x780 + N` heartbeat interpretation is not adopted.

## Issue #13 boundaries

The screenshot table appears to propose a deployment range of `0x31..0x3F`. The repository does not narrow the global CANopen node-ID validity rule to that range. A product or test fixture may select that range as a deployment policy, but it must not become a protocol-wide restriction without hardware-owner approval.

The heartbeat screenshot contains a contradiction: the function-code row states `0x700`, while another cell shows `0x780 + N`. The standard `0x700 + N` formula is retained until the product owner supplies authoritative clarification.

LSS identifiers are documented and checked as constants only. Fastscan, persistent node-ID/bit-rate commissioning, and multi-node provisioning remain outside the current v1 product claim. EMCY is likewise not implemented from screenshots alone; error-code semantics, inhibit timing, manufacturer data, and trigger policy require a separate approved catalog and test plan.

TPDO COB-ID formulas do not establish payload mappings. PDO activation, transmission type, inhibit/event timers, and application mapping must be approved and tested separately. The Inventus TPDO5/TPDO6 records remain disabled by default.

## Evidence classification

A passing protocol-contract test is **software evidence only**. It does not prove bxCAN transmission, SocketCAN interoperability, arbitration behavior, heartbeat timing on silicon, bus-off behavior, EMCY electrical observability, LSS commissioning, or formal CANopen conformance. Those require the physical `vcan0`/HIL and qualification procedures described elsewhere in this repository.
