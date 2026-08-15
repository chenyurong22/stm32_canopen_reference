# Product Scope

## Purpose

This repository is a **reference firmware platform** for STM32F767 CANopen development. It is not, by itself, a finished device, drive, gateway, safety product, or field-update system. A production product must select one declared personality, freeze its Object Dictionary and hardware design, and produce the required hardware, reliability, security, manufacturing, electrical, and conformance evidence.

## Declared reference personalities

| Personality | Default status | Supported reference behavior | Not supported as a production claim |
|---|---|---|---|
| CiA 401 I/O device | Default | CANopenNode communication services, generated OD integration, board I/O seams, safe startup hooks, and profile-oriented host tests | Product-specific electrical limits, channel calibration, debounce requirements, board diagnostics, HIL evidence, and formal CiA 401 conformance |
| CiA 402 drive interface | Optional | Reference state/controlword/statusword/fault-reset behavior and Profile Position/Velocity adapter seams | Complete drive product, torque/homing/CSP/CSV/CST behavior, motor feedback, power-stage safety, limits, following-error behavior, HIL, and formal conformance |
| CiA 302 NMT-master supervision | Optional | Bounded configured-peer boot-up and heartbeat supervision with deterministic host tests | Complete network list, configuration manager, commissioning workflow, startup configuration manager, multi-node production evidence, and formal conformance |
| CiA 309 gateway foundation | Optional | Bounded gateway foundation with deny-by-default policy tests | Authenticated production transport, authorization model, audit trail, security approval, and gateway conformance |

## Protocol and feature boundaries

| Feature | Repository status | Production interpretation |
|---|---|---|
| NMT, heartbeat, EMCY, SDO, PDO, SYNC | Integrated through CANopenNode and project configuration | Requires product OD approval, board testing, stress testing, and applicable conformance evidence |
| LSS | Stack integration and project policy hooks | Complete Fastscan commissioning and multi-node provisioning are not claimed |
| UDS / ISO-TP | Host-side contract models | No embedded UDS server or embedded ISO-TP implementation is claimed |
| CiA 418 | Generated/reference artifacts only | No complete CiA 418 device profile or physical device model is claimed |
| NMEA 2000 | Host gateway contract only | No embedded NMEA 2000 stack or field interoperability is claimed |
| CAN-FD/FDCAN | Not implemented; target uses bxCAN | No CAN-FD capability is claimed |
| Bootloader and firmware update | Not implemented | No secure update, signature verification, rollback, or anti-rollback capability is claimed |
| Secure boot and product security | Repository policy and release boundaries only | No secure boot, key provisioning, firmware authentication, or production debug-lock evidence is claimed |

## Evidence rule

A host test, source-contract test, sanitizer run, fuzz run, or cross-build is **software evidence only**. It does not establish physical CAN behavior, electrical compliance, EMC performance, watchdog timing on silicon, Flash power-loss tolerance, manufacturing repeatability, security approval, or formal CANopen conformance.

The minimum product evidence for a selected personality includes the exact firmware SHA, OD/EDS/XDD hashes, build manifest, board revision, transceiver and termination details, captured CAN traces, environmental conditions, test results, and independent review or conformance records where applicable.

## Release levels

| Release level | Meaning |
|---|---|
| `v0.9.0` Hardware Validation Candidate | Software gates pass, production compiler/resource checks pass, and the repository is ready for controlled board/HIL validation. External evidence is explicitly tracked and may remain pending. |
| `v1.0.0` Production release | All mandatory product requirements pass implementation, automated verification where applicable, physical/HIL verification where applicable, documented acceptance criteria, recorded evidence, and approved release review. |

## Scope-change rule

Adding a protocol or profile to a build flag, test model, generated artifact, or example does not make it a supported production feature. A scope change requires an update to this document, the feature matrix, the Object Dictionary/evidence plan, the release checklist, and the corresponding implementation and verification records.
