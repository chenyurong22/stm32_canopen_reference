# v0.9.0-rc1 Baseline Record

## Purpose

This document records the immutable software baseline selected for controlled STM32F767 hardware validation. It does not certify hardware behavior, EMC performance, manufacturing repeatability, security, or formal CANopen conformance.

## Source lineage

| Item | Value |
|---|---|
| Historical tag | `v0.9.0` → `9c04ef22c93b13f88df37245d47e52926b516d5c` (`9c04ef2`) |
| Release-candidate tag | `v0.9.0-rc1` → `509b49c262097f7dbbeb9751c9c2a6a0647f7955` (`509b49c`) |
| Branch at record creation | `main` |
| Repository | `https://github.com/mahdi-benhassen/stm32_canopen_reference` |
| Tag policy | The historical `v0.9.0` tag is retained; `v0.9.0-rc1` is an additional immutable candidate tag. |

The RC1 tag identifies the reviewed source state before this record was added. The record itself is a subsequent traceability commit and must not be confused with the tagged firmware source SHA.

## Reproducibility inputs

| Input | Pinned value or path |
|---|---|
| Build system minimum | CMake `>= 3.22`; CI package pin `3.28.3-1build7` |
| ARM compiler package | `gcc-arm-none-eabi=15:13.2.rel1-2` |
| ARM C library package | `libnewlib-arm-none-eabi=4.4.0.20231231-2` |
| C static analysis | `cppcheck=2.13.0-2ubuntu3` |
| Format/lint packages | `clang-format=1:18.0-59~exp2`; `clang-tidy=1:18.0-59~exp2` |
| STM32CubeF7 revision | `c2ecfd2d863d4cb1a138e63be4c8c1c4acd43d4d` |
| CanOpenSTM32 revision | Submodule commit `b313b2be0df86950322310f515126de4958e1340` |
| Linker script | `linker/STM32F767_2M_512K_FLASH.ld` |
| Default profile | CiA 401 reference personality; production profile enabled for release builds |
| CAN target | bxCAN CAN1, 500 kbit/s reference configuration; exact board and transceiver remain integration inputs |

The CI workflow is the authoritative source for package-version pins and the STM32CubeF7 revision. The checked-in submodule pointer is the authoritative CanOpenSTM32 revision.

## Software evidence available at this baseline

The baseline contains the reviewed CAN filter remediation, the bounded CiA 418 adapter/model lifecycle, explicit protocol-claim boundaries, host contract tests, deterministic conformance vectors, sanitizer and coverage gates, production compiler hardening, memory-budget checks, and a production-profile ARM build path.

The software evidence does not substitute for physical evidence. In particular, the following remain pending until executed on the exact target hardware and recorded in the controlled evidence package:

| External gate | Status at RC1 |
|---|---|
| Physical CAN timing, transceiver behavior, and analyzer traces | Pending hardware/HIL |
| CiA 401, CANopen, LSS, and bounded CiA 302 peer-supervision physical tests | Pending hardware/HIL |
| Bus-off campaigns and recovery timing | Pending hardware/HIL |
| Flash power-loss, interrupted-write, endurance, and corner testing | Pending hardware |
| Watchdog, brownout, reset-cause, and power-cycle testing | Pending hardware |
| Stress, soak, environmental, electrical, and EMC testing | Pending hardware/laboratory |
| Security, manufacturing, and formal conformance evidence | Pending responsible owners |

A blank or generated evidence record is not a pass. Production labeling requires approved, machine-valid external evidence tied to the exact release SHA, board revision, hardware serial, firmware image, OD/EDS inputs, and test conditions.

## Follow-up release policy

After hardware and HIL qualification, create a new immutable candidate tag such as `v0.9.0-rc2` at the exact validated commit. Create `v1.0.0` only after the applicable production, security, manufacturing, environmental, and independent conformance gates have been completed and reviewed.
