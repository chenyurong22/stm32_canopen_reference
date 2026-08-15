# Application Examples

This directory documents practical ways to exercise the reference firmware. The examples are protocol and integration patterns, not production actuator or safety implementations.

## NMT and heartbeat supervision

Use the opt-in CiA 302 personality with a second CANopen node. Start the STM32 node in pre-operational mode, observe the boot-up heartbeat on `0x700 + node-ID` with payload `00`, and issue NMT commands on CAN-ID `0x000`:

```text
01 <node-id>    Start operational
80 <node-id>    Enter pre-operational
02 <node-id>    Stop
81 <node-id>    Reset node
82 <node-id>    Reset communication
```

The expected heartbeat state bytes are `05` for operational, `7F` for pre-operational, and `04` for stopped. The target node-ID and heartbeat period must come from the exact product configuration.

## SDO configuration

Use an SDO client to read the identity and application configuration objects from the Object Dictionary. A typical expedited upload request has the form:

```text
CAN-ID: 0x600 + node-ID
Data:   40 <index-low> <index-high> <sub-index> 00 00 00 00
```

A write operation must use the correct command specifier, data length, access type, and Object Dictionary data type. Validate the response and handle abort codes; never assume that an object is writable because it exists in an EDS file.

## PDO-mapped I/O

For an I/O device, map application inputs into TPDOs and outputs into RPDOs according to the approved Object Dictionary. Verify that PDOs are disabled in pre-operational and stopped states when required by the product profile, and that the board-level output path remains in its safe state until the application authorization and interlocks are satisfied.

Use a CAN analyzer to record the COB-ID, DLC, mapped byte order, transmission type, inhibit time, event timer, and state transition that enabled the PDO. The reference project does not select a universal board I/O map.

## EMCY and fault handling

Inject an application fault through the board adapter or test seam and verify that the node emits the configured EMCY frame, records the fault state, and applies the board’s safe response. Confirm that fault reset, NMT reset, and power-cycle behavior match the approved product fault matrix.

An EMCY frame is a diagnostic notification; it is not a substitute for an independent hardware safety shutdown or power-stage interlock.

## UDS/ISO-TP diagnostics

Use the host-side contract model and SocketCAN runner to test diagnostic session transitions, tester-present, DID reads, negative responses, multi-frame transfer, write authorization, and reset gating. Run the detailed [hardware acceptance procedure](../docs/hardware/uds_cia302_test_procedure.md) and archive the JSON result plus an independent CAN trace.

The host model is not an embedded UDS server. A product that needs UDS on the STM32 must add an embedded ISO-TP transport, diagnostic server, timing configuration, authorization policy, and Object Dictionary or application integration.

## Reproducible development loop

For each example, retain the exact firmware commit, build personality, node-ID, bitrate, Object Dictionary revision, CAN trace, and hardware wiring. Start with a current-limited bench supply and the transceiver disabled, then enable application power only after protocol and safe-state checks pass.
