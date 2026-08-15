# Final-status review findings and remediation scope

This file converts the attached final-status review into implementation work items. It distinguishes defects that can be fixed in the STM32F767 reference baseline from evidence or product-scope items that require hardware, a conformance suite, or a separate protocol implementation.

| Priority | Finding | Action in this remediation |
| --- | --- | --- |
| P0 | CAN bus-off is detected but not recovered. | Implement bounded STOP/WAIT/REINIT recovery, CANopen restart, output-safe handling, and tests. |
| P0 | Default storage backend is RAM-backed. | Implement STM32F7 internal-Flash dual-slot persistence with CRC and sequence selection; retain weak board hooks. |
| P0 | Communication reset ignores reinitialization failure. | Return and latch a safe fault state; disable CAN, force safe outputs, and expose fault diagnostics. |
| P0 | Hardware filtering assumes default COB-IDs. | Build acceptance filters from active OD PDO mappings and retain mandatory NMT/SDO/LSS traffic. |
| P0/P1 | Wire-level and fault-injection coverage is incomplete. | Add deterministic host contracts for error mapping, recovery state machine, filters, and persistence image logic; document HIL requirements. |
| P1 | `can_port_poll(timeout_ms)` ignores timeout. | Implement bounded timeout semantics or rename the API; retain a nonblocking zero-timeout path. |
| P1 | Non-F767 builds can silently skip timing configuration. | Fail clearly for unsupported STM32 targets instead of starting an unconfigured controller. |
| P1 | Watchdog timing/reset reason are under-specified. | Add explicit deadlines, startup grace, fault counters, and reset-reason diagnostics where supported; keep default disabled. |
| P1 | CiA 402/401/302/LSS status needs clearer boundaries. | Add feature matrix and implementation-status tables; do not claim unsupported profile modes or full Fastscan. |
| P1 | Transport implementations can drift. | Document and test the shared wire-contract boundary; do not falsely claim identical hardware behavior without HIL. |
| P2 | Full UDS, ISO-TP, CiA 418 firmware, NMEA 2000, CAN-FD, and bootloader remain absent. | Keep explicitly marked as out of scope and unimplemented in the feature matrix. |
| Evidence | Recognized CANopen conformance and physical HIL are missing. | Add acceptance procedures, result templates, and CI gates that distinguish software contracts from hardware evidence. |

The implementation will not claim that host tests replace physical bus conformance, nor that a reference profile seam is a complete industrial product profile.
