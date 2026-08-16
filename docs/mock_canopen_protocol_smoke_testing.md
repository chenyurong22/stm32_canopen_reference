# In-process CANopen protocol smoke testing

`scripts/mock_canopen_runner.py` is a deterministic, user-space protocol smoke test for the Inventus test personality. It models a single CANopen node and an in-process lossless bus, records every simulated transmit and receive frame, and checks protocol exchanges without opening SocketCAN or requiring the host `vcan` kernel module.

The implementation is intentionally small enough to reuse as a pattern for future opt-in personalities. The reusable boundary is the transport-neutral model: `Frame` validates classic-CAN identifiers and DLC, `MockBus` records TX/RX traffic, `MockNode` implements bounded NMT/heartbeat/SDO/PDO behavior, and an Object Dictionary adapter supplies the personality-specific objects. A future profile can retain the frame/bus/node machinery while replacing the catalog adapter and scenario assertions.

## What it verifies

| Area | Current checks |
|---|---|
| NMT and heartbeat | Boot-up, start-remote-node, operational heartbeat, and node-ID addressing |
| SDO | Expedited and segmented uploads, typed expedited writes, read-only rejection, unknown-object rejection, and missing-sub-index rejection |
| Inventus identity | Zero-filled placeholder widths for `0x1008–0x100A` |
| Structured D000 | Resolved highest sub-index `0x70`, typed defaults, writable field readback, read-only enforcement, and sparse-gap rejection |
| Provisional D001 | Bounded `0x01..0xFE` array behavior and intentional `0xFF` rejection |
| PDO | TPDO1–TPDO6 disabled defaults, opt-in COB-ID enabling, mapping widths, payload lengths, and COB-ID calculation |

Run the default scenario with:

```sh
python3 scripts/mock_canopen_runner.py
```

Use a different valid node ID or print every simulated frame when diagnosing a scenario:

```sh
python3 scripts/mock_canopen_runner.py --node-id 0x01
python3 scripts/mock_canopen_runner.py --node-id 0x7f --verbose
```

The runner is also wired into the repository checks:

```sh
make -C tests/host test-mock-canopen
bash scripts/validate_reference.sh
```

The Make target is the preferred quick check when only the protocol model has changed. The full validation script additionally runs static validators, native host tests, contract suites, and the selected ARM builds.

## Interpretation of results

A passing run is **protocol-model evidence**. It shows that the modeled request/response sequences and the generated profile contract agree. It does not prove that the STM32 firmware executes identically, that bxCAN bit timing is correct, or that a transceiver will pass physical-layer, arbitration, error-frame, bus-off, EMC, persistence-interruption, HIL, or formal conformance testing.

When `vcan0` is available, run the SocketCAN procedure separately. Treat the in-process runner as a deterministic baseline and a fallback for environments without `CAP_NET_ADMIN` or the `vcan` kernel module, not as a reason to skip physical testing.

## Adding a future personality

A future profile should avoid copying the whole runner. First extract or reuse the transport-neutral frame, bus, NMT, SDO, and PDO components. Then provide an Object Dictionary adapter backed by the profile’s checked-in catalog and add scenario functions for only the behavior justified by that catalog. Keep the profile-specific assertions and source paths explicit so a passing generic harness cannot conceal missing vendor semantics.
