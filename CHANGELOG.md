# Changelog

All notable changes to this project are documented here. The project follows semantic-versioning principles for tagged reference baselines; firmware conformance and product compatibility still depend on the exact board, Object Dictionary, toolchain, and build personality.

## [Unreleased]

### Audit remediation

- Propagated STM32 HAL CAN errors into CANopenNode diagnostics and added bounded hardware-fault counters.
- Added regression assertions for CAN1/TIM7 interrupt priorities and corrected the default 500 kbit/s sample point to 83.33%.
- Added validated standalone-facade bitrate selection for common 54 MHz APB1 configurations.
- Enabled OD 1010h/1011h storage through a project-owned, CRC-validated persistence seam with weak board hooks.
- Added an opt-in dual-rate IWDG supervisor that requires both TIM7 and mainline progress.
- Replaced the bxCAN accept-all filter with node-specific CANopen and LSS acceptance lists.
- Replaced the RAM-only default persistence path with a CRC-validated two-slot STM32F7 Flash backend and reserved the final 512 KiB in the linker memory map.
- Added bounded CAN bus-off STOP/WAIT/REINIT/FAULT recovery, safe-fault latching on failed communication reset, and recovery diagnostics.
- Derived bxCAN acceptance filters from active OD PDO/SDO mappings while retaining mandatory CANopen and LSS traffic.
- Added bounded timeout behavior and explicit `-ENOTSUP` failure for unsupported standalone CAN targets.
- Hardened the opt-in IWDG path with startup grace, LSI readiness bounds, timer-progress deadlines, mainline-only refresh, and reset-cause capture.
- Added deterministic recovery state-machine tests and source contracts for error mapping, Flash reservation, filters, transport deadlines, watchdog behavior, and lifecycle ordering.
- Added `docs/final_status_remediation_2026.md`, which separates implemented reference controls from required HIL, electrical, endurance, and formal-conformance evidence.

The reference firmware remains a non-certified baseline. Production release still requires board-specific Flash endurance and power-loss testing, watchdog timing measurement, physical CAN/HIL validation, EMC testing, and applicable official conformance evidence.

## [0.1.0] - 2026-08-15

### Added

- STM32F767 CAN1/bxCAN CANopenNode reference firmware with a CiA 401 default personality.
- Optional CiA 402 drive-control reference, CiA 302 NMT-master adapter, and bounded gateway foundations.
- CMake ARM builds, host protocol tests, deterministic contract tests, static analysis, and GitHub Actions validation.
- UDS/ISO-TP and CiA 302/NMT hardware acceptance runner and procedure.
- Build, hardware, security, contribution, and third-party dependency documentation.

### Notes

- The repository is a reference implementation, not a device-profile, functional-safety, or production-board certification.
- The exact release artifact must retain its build manifest, Object Dictionary, linker script, dependency revisions, and hardware validation evidence.
