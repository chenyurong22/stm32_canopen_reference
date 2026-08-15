# Hardware Test Procedure: UDS/ISO-TP and CiA 302

## Purpose

This procedure validates the embedded UDS/ISO-TP diagnostic path and CANopen NMT behavior on an STM32F767 target using a real CAN bus. It has two explicitly separated scopes: (1) external NMT-master control of the DUT as a CANopen NMT slave, and (2) the embedded CiA 302 NMT-master personality, which is opt-in and requires an independent CANopen peer. It is intended for engineering acceptance and regression testing after a firmware build has passed the host and cross-build checks.

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
| Node configuration | DUT node-ID, remote node-ID(s), bitrate, diagnostic IDs, and heartbeat producer settings recorded |
| Master personality | Record whether the DUT is the default NMT slave or `CANOPEN_REFERENCE_ENABLE_CIA302_MASTER=ON` |
| Peer network | Embedded-master tests have at least one independent CANopen peer and a second observer/simulator |
| Timing limits | Heartbeat period, maximum gap, jitter tolerance, NMT response deadline, boot deadline, and heartbeat timeout recorded |
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
  --additional-remote-node 3 \
  --heartbeat-period 1.0 \
  --heartbeat-max-gap 2.0 \
  --uds-tx-id 0x7E0 \
  --uds-rx-id 0x7E8 \
  --json-out "results/uds_cia302_$(date -u +%Y%m%dT%H%M%SZ).json"
```

The safe set covers UDS default and extended sessions, TesterPresent, DID read, unknown-service negative response, a Classical CAN multi-frame request, remote-node boot-up, targeted NMT start/pre-operational/stop/reset-node, broadcast start/pre-operational/stop/reset-communication, targeted reset-communication, targeted-node isolation, heartbeat validity and timing, and a malformed-NMT matrix. Tests that require a second peer skip unless `--additional-remote-node` is supplied.

The process returns zero only when no test fails. A skipped destructive test or intentionally unavailable multi-node test is reported as `SKIP` and does not cause failure. Any transport timeout, unexpected response, malformed frame, unexpected NMT transition, invalid heartbeat state, heartbeat gap above the configured maximum, or missing heartbeat is a failure.

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

## CiA 302 embedded-master acceptance

The external runner’s NMT tests do not prove that the STM32F767 is acting as a CiA 302 master. To test the embedded master, build and flash the opt-in personality:

```bash
cmake -S . -B build-cia302 \
  -DSTM32_CUBE_F7_DIR=/path/to/STM32CubeF7 \
  -DSTM32_F7_LINKER_SCRIPT=/path/to/STM32F767xx_FLASH.ld \
  -DCANOPEN_REFERENCE_ENABLE_CIA302_MASTER=ON
cmake --build build-cia302
```

The default configuration monitors peer node 11 with a 1500 ms heartbeat timeout and a 10 s mandatory boot deadline. The peer assignment and timing values must be recorded with the firmware metadata. Enable the bounded UART diagnostic line for bench evidence with `CANOPEN_REFERENCE_UART_DIAGNOSTICS=1`.

Use an independent peer or deterministic simulator and a separate CAN trace. The following criteria are mandatory for the embedded-master personality:

| Criterion | Stimulus | Required evidence |
|---|---|---|
| Mandatory boot | Power or reset the assigned peer | `BOOTUP` event, valid pre-operational heartbeat, and exactly one readiness transition after all mandatory peers are present |
| Mandatory boot timeout | Keep the assigned peer absent | No `network_ready`; one boot-timeout event after the configured deadline; no auto-start command |
| Heartbeat supervision | Stop peer heartbeat after boot | One heartbeat-timeout event after the configured timeout; no repeated event flood while the peer remains absent |
| Recovery | Restore peer heartbeat | Heartbeat/boot recovery is observed and the project-defined readiness policy is restored without duplicate start commands |
| Optional node policy | Omit an optional configured peer | Network readiness remains according to the configured optional-node policy; omission is traceable |
| Auto-start | Enable the configured auto-start policy | Targeted `01 <node-ID>` is emitted exactly once per configured auto-start peer after readiness |
| Reset communication | Request targeted and broadcast communication reset | Exact `82 <node-ID>` / `82 00` frames, boot recovery, and no unsafe output transition |
| Invalid heartbeat | Inject DLC-not-one or invalid state-byte heartbeat | Invalid-frame counter/event increments; tracked valid state is not replaced by invalid data |
| Unassigned node | Transmit heartbeat from a node not assigned to the master | No readiness, timeout, or auto-start state is attributed to the unassigned node |

The acceptance record shall include the UART diagnostic output, independent CAN trace, peer simulator log, and final JSON runner output. If the adapter is not enabled, these criteria are marked **not hardware-tested**, not passed.

## Independent trace checks

The JSON result is an execution summary; the independent CAN trace is authoritative for wire-level acceptance. The reviewer shall confirm that:

1. ISO-TP single frames use a valid payload length and do not exceed seven UDS bytes.
2. Multi-frame transfers contain a First Frame, Flow Control, correctly sequenced Consecutive Frames, and a complete response.
3. Flow Status, block size, and STmin are respected.
4. UDS positive responses echo the expected service semantics, while negative responses contain `7F`, the requested SID, and a valid NRC.
5. NMT frames use exactly two data bytes: command and node-ID; broadcast uses node-ID `00`.
6. Targeted commands affect only the addressed node; broadcast commands affect every listed node and no unlisted node.
7. NMT state indications use the expected heartbeat bytes: `00` boot-up, `04` stopped, `05` operational, and `7F` pre-operational.
8. Heartbeat acceptance records sample count, first-heartbeat delay, maximum inter-heartbeat gap, and period/jitter when configured.
9. Malformed NMT frames cover DLC 0, DLC 1, DLC 3+, invalid command, invalid node-ID, and reserved broadcast combinations; none produces a state change.
10. Diagnostic traffic does not corrupt heartbeat, SYNC, PDO, or emergency traffic.
11. Embedded-master tests correlate every readiness, timeout, auto-start, and invalid-frame event with the independent CAN trace and UART diagnostic counters.

## Acceptance decision

The hardware run is accepted only when all mandatory tests for the selected scope pass. For external NMT-slave scope, this means correct targeted/broadcast transitions, valid heartbeat state and timing, malformed-frame rejection, and safe reset behavior. For embedded CiA 302-master scope, it additionally means mandatory/optional policy, boot timeout, heartbeat timeout, timeout de-duplication, readiness, auto-start, reset-communication, invalid-heartbeat handling, and unassigned-node behavior are evidenced by both the diagnostic output and independent trace. No unexpected frame or state transition may be present, no unsafe output may be observed, and firmware metadata, JSON results, peer logs, and raw trace shall be archived together.

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

This is a hardware acceptance procedure for the activated UDS/ISO-TP path, NMT-slave behavior, and the opt-in embedded CiA 302 master adapter. Host contract tests remain necessary but do not replace physical CAN validation. A default firmware image without `CANOPEN_REFERENCE_ENABLE_CIA302_MASTER=ON` cannot claim embedded-master acceptance. ISO-TP timing and transport terminology should be interpreted against the applicable product standard and the project’s declared configuration [1] [2]. CANopen NMT behavior should be interpreted against the approved CiA 301/CiA 302 revisions used by the product [3].

## References

[1]: https://www.iso.org/standard/66574.html "ISO 15765-2 standard record"

[2]: https://docs.kernel.org/networking/iso15765-2.html "Linux ISO-TP implementation reference"

[3]: https://www.can-cia.org/cia-groups/technical-documents "CAN in Automation technical documents"
