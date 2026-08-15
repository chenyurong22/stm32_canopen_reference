# Hardware Test Procedure: UDS/ISO-TP and CiA 302

## Purpose

This procedure validates the embedded UDS/ISO-TP diagnostic path and CiA 302 NMT-master behavior on an STM32F767 target using a real CAN bus. It is intended for engineering acceptance and regression testing after a firmware build has passed the host and cross-build checks.

The executable runner is:

```text
tests/hardware/run_uds_cia302_acceptance.py
```

The runner uses Linux SocketCAN raw frames so the tester can observe ISO-TP and NMT traffic directly with an independent CAN analyzer. It does not configure the CAN interface, power-cycle the board, or control external loads.

## Bench requirements

The bench shall include the STM32F767 device under test, a compatible high-speed CAN transceiver, a Linux host with a SocketCAN adapter, an independent CAN trace or analyzer, a current-limited power supply, and a controlled reset method. The CAN bus shall have 120-ohm termination at both physical ends and approximately 60 ohms measured between CAN-H and CAN-L when powered down.

The operator shall record the firmware Git SHA, build personality, compiler version, CAN bitrate, node-ID, diagnostic request/response identifiers, managed-node list, heartbeat settings, UDS session timers, and DID configuration before starting. Any drive, battery, or actuator power stage shall be disconnected or replaced with a safe simulated load unless the hazard analysis authorizes live operation.

## Pre-test checklist

| Check | Required result |
|---|---|
| Firmware identity | Git SHA and build artifact match the test request |
| CAN wiring | CAN-H/CAN-L polarity and common ground verified |
| Termination | Approximately 60 ohms across the powered-down bus |
| Interface | SocketCAN interface exists and can transmit/receive frames |
| Power | Current limit and safe-state behavior verified |
| Node configuration | DUT node-ID, remote node-ID, bitrate, and diagnostic IDs recorded |
| Capture | Independent CAN capture starts before DUT power-up |
| Safety | Destructive DID write and ECU reset gates are disabled initially |

Configure the bus before running the runner. For example:

```bash
sudo ip link set can0 down 2>/dev/null || true
sudo ip link set can0 up type can bitrate 500000
```

## Non-destructive execution

First validate the runner without opening the CAN interface:

```bash
python3 tests/hardware/run_uds_cia302_acceptance.py --dry-run
```

Then run the safe default test set:

```bash
mkdir -p results
GIT_SHA="$(git rev-parse HEAD)" \
python3 tests/hardware/run_uds_cia302_acceptance.py \
  --iface can0 \
  --remote-node 2 \
  --uds-tx-id 0x7E0 \
  --uds-rx-id 0x7E8 \
  --json-out "results/uds_cia302_$(date -u +%Y%m%dT%H%M%SZ).json"
```

The safe set covers UDS default and extended sessions, TesterPresent, DID read, unknown-service negative response, a Classical CAN multi-frame request, remote-node boot-up, NMT start, pre-operational, stop, reset-node, broadcast start, heartbeat observation, and malformed-NMT rejection.

The process returns zero only when no test fails. A skipped destructive test is reported as `SKIP` and does not cause failure. Any transport timeout, unexpected response, malformed frame, unexpected NMT transition, or missing heartbeat is a failure.

## Destructive tests

DID writes and ECU reset are disabled by default. Before enabling them, the operator shall confirm that the DID is explicitly writable, the value is safe, the target is not connected to an uncontrolled actuator, and the reset recovery path is understood.

Run a controlled DID write only after approval:

```bash
GIT_SHA="$(git rev-parse HEAD)" \
python3 tests/hardware/run_uds_cia302_acceptance.py \
  --iface can0 \
  --remote-node 2 \
  --enable-destructive \
  --write-did 0xF190 \
  --write-value 54455354 \
  --json-out results/uds_write.json \
  --tests uds-write-did
```

Run the reset test only with a controlled reset and, when applicable, a boot-up heartbeat identifier:

```bash
GIT_SHA="$(git rev-parse HEAD)" \
python3 tests/hardware/run_uds_cia302_acceptance.py \
  --iface can0 \
  --remote-node 2 \
  --enable-reset \
  --bootup-id 0x702 \
  --json-out results/uds_reset.json \
  --tests uds-reset
```

## Independent trace checks

The JSON result is an execution summary; the independent CAN trace is authoritative for wire-level acceptance. The reviewer shall confirm that:

1. ISO-TP single frames use a valid payload length and do not exceed seven UDS bytes.
2. Multi-frame transfers contain a First Frame, Flow Control, correctly sequenced Consecutive Frames, and a complete response.
3. Flow Status, block size, and STmin are respected.
4. UDS positive responses echo the expected service semantics, while negative responses contain `7F`, the requested SID, and a valid NRC.
5. NMT frames use exactly two data bytes: command and node-ID.
6. NMT state indications use the expected heartbeat bytes: `00` boot-up, `04` stopped, `05` operational, and `7F` pre-operational.
7. Malformed NMT frames do not produce a state change.
8. Diagnostic traffic does not corrupt heartbeat, SYNC, PDO, or emergency traffic.

## Acceptance decision

The hardware run is accepted only when all mandatory tests pass, measured response and heartbeat timing are within the configured product limits, no unexpected frame or state transition is present, no unsafe output is observed, and the firmware metadata, JSON results, and raw trace are archived together.

The run is rejected for any unexplained ISO-TP sequence error, timeout violation, wrong UDS response, malformed-NMT acceptance, lost mandatory heartbeat, unsafe reset behavior, bus lockup, hard fault, watchdog reset, or queue/resource exhaustion.

## Result archive

Archive a directory named with the firmware SHA and UTC timestamp:

```text
results/<git-sha>_<utc-time>/
  uds_cia302.json
  can_trace.asc or can_trace.blf
  firmware_metadata.txt
  bench_configuration.txt
  operator_notes.md
```

## Scope and references

This is a hardware acceptance procedure for the embedded activation of the bounded UDS/ISO-TP and CiA 302 features. Host contract tests remain necessary but do not replace physical CAN validation. ISO-TP timing and transport terminology should be interpreted against the applicable product standard and the project’s declared configuration [1] [2]. CANopen NMT behavior should be interpreted against the approved CiA 301/CiA 302 revisions used by the product [3].

## References

[1]: https://www.iso.org/standard/66574.html "ISO 15765-2 standard record"

[2]: https://docs.kernel.org/networking/iso15765-2.html "Linux ISO-TP implementation reference"

[3]: https://www.can-cia.org/cia-groups/technical-documents "CAN in Automation technical documents"
