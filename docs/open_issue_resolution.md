# Open Issue Resolution Record

## Scope

This record resolves the two open issues identified in `mahdi-benhassen/stm32_canopen_reference`.

| Issue | Request | Resolution |
|---|---|---|
| #1 | Add CiA 402 motion-control Object Dictionary entries. | Added a declarative CiA 402 catalog and regenerated `Generated/OD.c`, `Generated/OD.h`, and the EDS from that source model. The validator now enforces the complete catalog, sorted OD ordering, generated shortcuts, EDS sections, and optional-object synchronization. |
| #2 | Add CANopen frames and the communication state machine. | Preserved CANopenNode as the protocol implementation and strengthened the deterministic SocketCAN model and regression suite with explicit NMT state transitions, reset/boot-up, broadcast start, malformed-frame rejection, state-gated SYNC/PDO behavior, canonical SDO aborts, LSS, heartbeat, PDO, and SDO frame contracts. |

## CiA 402 catalog coverage

The generated catalog covers the issue-requested motion-control groups, including the standard control/status objects, mode and set-point objects, homing, interpolation, profile-position, profile-velocity, profile-torque, motor/drive identification, scaling-factor, and digital I/O entries. Scalar, visible-string, record, and array representations are emitted from `scripts/cia402_catalog.py`; generated C storage, CANopenNode runtime objects, header shortcuts, and EDS sections are produced by `scripts/generate_reference_od.py`.

The expanded artifacts currently validate as **159 sorted OD entries**, **156 EDS optional objects**, and **126 synchronized profile indices**. The exact object-level contract is enforced by `scripts/validate_od.py`; these counts are validation outputs, not hand-entered metadata.

## CANopen communication coverage

The protocol PDF attached to issue #2 specifies NMT slave operation, COB-ID `0x000` NMT commands, configurable LSS node ID and baud rate, SDO server request/response IDs, expedited SDO access and error responses, and cyclic or event-driven TPDO behavior. The repository’s CANopenNode stack remains the implementation of those protocol services; the host model is explicitly a deterministic wire-contract harness, not a replacement for the embedded stack.

The issue #2 regression suite now verifies the following frame-level behavior when a Linux runner provides `vcan0`:

- NMT start, stop, pre-operational, reset-node, and broadcast-start commands.
- Boot-up and heartbeat state values on `0x700 + node-ID`.
- Rejection of malformed NMT frames that do not contain the required two-byte payload.
- SYNC-triggered TPDO behavior only in operational state.
- SDO expedited reads/writes, segmented upload, dynamic PDO mapping, and canonical aborts for missing and read-only objects.
- LSS Fastscan response and EMCY observability.

Hosted GitHub runners may lack the `vcan` kernel module. In that environment, CI keeps all deterministic OD, Python, C, static-analysis, and firmware-build checks mandatory and reports the wire-level suite as an explicit capability-gated skip. A standard Linux host or CAN HIL runner is required for the runtime frames.

## Validation evidence

The following local validation was run after the changes:

```text
./scripts/validate_reference.sh
python3 -m py_compile tests/host/test_sdo.py middleware/canopen/examples/canopen_vcan_device.py
make -C tests/host clean all
build/host/test_can_port_stm32
build/host/test_gateway_default_deny
```

The local matrix passed the OD generator and validator, firmware configuration contracts, CiA 401/CiA 402 profile tests, host façade and gateway tests, and default, CiA 402, and CiA 309 gateway firmware builds. SocketCAN runtime execution remains dependent on host `vcan0` capability.

## References

[1]: https://github.com/mahdi-benhassen/stm32_canopen_reference/issues/1 "Issue #1: CiA 402 Object Dictionary request"
[2]: https://github.com/mahdi-benhassen/stm32_canopen_reference/issues/2 "Issue #2: CANopen frames and state machine request"
[3]: https://github.com/user-attachments/files/31047613/DS402.pdf "CiA 402 reference attached to issue #1"
[4]: https://github.com/user-attachments/files/31047966/Piher_CANOpen_PROTOCOL_V0.pdf "CANopen protocol reference attached to issue #2"
[5]: https://canopennode.github.io/CANopenNode/ "CANopenNode documentation"
