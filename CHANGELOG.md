# Changelog

All notable changes to this project are documented here. The project follows semantic-versioning principles for tagged reference baselines; firmware conformance and product compatibility still depend on the exact board, Object Dictionary, toolchain, and build personality.

## [Unreleased]

- Continue development on the `main` branch.

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
