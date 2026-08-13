# STM32F767 CANopen Reference Architecture

**Author:** Manus AI  
**Status:** Engineering reference; not a conformance certificate

This repository is a clean bare-metal reference for an STM32F767 node using the bxCAN peripheral, STM32 HAL, and the pinned CANopenNode STM32 port. The CANopenNode project provides the CiA 301 communication mechanisms, including SDO server/client, PDO, LSS, gateway, and safety modules as selectable components; each product must still provide its device-specific Object Dictionary, application behavior, and verification evidence.[1] The STM32 binding uses the HAL CAN driver and supports the legacy CAN controller used by STM32F767.[2]

> **Safety boundary.** CiA 304/SRDO is a safety-related data transport mechanism, not a safety-certified product. No source file, compiler option, or CANopen conformance test can substitute for the product safety lifecycle, hardware architecture, hazard analysis, diagnostics, timing analysis, and independent evidence required for a safety claim.[3]

| Layer | Repository location | Responsibility | Product-team action |
|---|---|---|---|
| Hardware and safety | Board project and `canopen_reference_hw.c` overrides | Transceiver, termination, I/O conditioning, power-stage enable, independent safe state, feedback | Replace every applicable weak hook; review fault containment and EMC. |
| CubeMX/HAL platform | `Core/` | Clock, CAN1, TIM7, GPIO, IRQ forwarding | Generate from the exact part/package and board pinout, then preserve the user code boundary. |
| CANopen driver | `third_party/CanOpenSTM32/CANopenNode_STM32` | bxCAN buffering, HAL notifications, CAN error handling, critical sections | Preserve the pinned dependency and perform supply-chain review. |
| CANopen communication | `third_party/CanOpenSTM32/CANopenNode` | CiA 301 NMT, heartbeat, SDO, PDO, EMCY, SYNC, LSS | Configure only services backed by matching OD entries and test them on the target. |
| Object Dictionary | `ObjectDictionary/` and `Generated/` | CANopen-visible data, access control, PDO eligibility | Edit the EDS/XDD in the OD editor, regenerate, and review generated source. |
| Device profile | `App/Src/cia401_reference.c`, `App/Src/cia402_reference.c` | I/O process data or drive state logic | Select one product profile by default; complete the profile-specific object set and conformance tests. |

## Deterministic execution model

The reference separates CAN frame reception, 1 ms real-time handling, and non-real-time protocol work. This follows CANopenNode’s intended model: receive handling should be prompt, PDO and SYNC processing belongs to the constant-period path, and SDO/NMT/heartbeat work belongs to the cyclic mainline path.[1]

| Context | Entry point | Allowed work | Prohibited work |
|---|---|---|---|
| CAN1 IRQ | HAL callbacks owned by the STM32 driver | Frame reception, mailbox completion, error/status handling | Blocking, flash writes, formatted logging, application control loops. |
| TIM7 IRQ, 1 ms | `canopen_app_interrupt()` | SYNC, RPDO, bounded hardware sampling, profile state transition, TPDO packing | Blocking driver calls, heap allocation, flash/EEPROM, console output. |
| Main loop | `canopen_app_process()` | SDO, NMT, heartbeat, reset command processing, supervisory functions | Long unbounded tasks without a latency budget. |

TIM7 derives a 1 ms tick from a 54 MHz APB1 timer clock. CAN1 uses 54 MHz / 6 prescaler / 18 time quanta = **500 kbit/s**, with 15 TQ segment 1 and 2 TQ segment 2, yielding an 88.9% nominal sample point. This timing is valid only if the generated clock tree maintains PCLK1 = 54 MHz; recalculate it after every clock-tree change.

## Object Dictionary workflow

`ObjectDictionary/stm32f767_canopen_reference.eds` is the editable external representation. `scripts/generate_reference_od.py` demonstrates a repeatable derivation of the compiled `Generated/OD.h` and `Generated/OD.c` from the upstream CANopenNode V4 base plus the selected reference profile entries. It is intentionally a reference generator, not a replacement for release-controlled Object Dictionary tooling.

A product release should make the OD editor project (such as the CANopenEditor project) the source of truth, regenerate the EDS/XDD and C artifacts from that tool, and treat all generated outputs as a reviewed set. CANopenNode documents `OD.h` and `OD.c` as generated files and supports Object Dictionary access from local C code and the CAN network.[1]

| Object range | Included reference objects | Intended role | Release requirement |
|---|---|---|---|
| `0x1000–0x1FFF` | CiA 301 base, one SDO server/client, four RPDO/TPDO records, LSS identity | Communication profile | Replace identity and define the approved PDO set. |
| `0x6000`, `0x6200`, `0x6401`, `0x6411`, `0x6422` | Digital/analogue I/O sample points | CiA 401 reference subset | Add channel descriptors, polarity, diagnostics, scaling, and all applicable profile objects. |
| `0x603F`, `0x6040`, `0x6041`, `0x6060–0x60FF` subset | State, control, target, and feedback variables | CiA 402 reference subset | Add the full chosen mode-specific objects, limits, units, homing/trajectory, errors, and manufacturer data. |

## Profile selection

The default configuration enables CiA 401 and disables CiA 402. `canopen_reference_config.h` rejects a combined build unless `CANOPEN_REFERENCE_ALLOW_COMBINED_PROFILES` is deliberately set. This rule is engineering guidance: a combined I/O/drive node may be legitimate, but it needs a coherent product definition, device type, EDS/XDD, PDO mapping, and conformance scope rather than a claim of two independently conformant devices.

The CiA 402 module is deliberately limited to a bounded controlword/statusword transition reference. It samples feedback through board hooks, refuses to enable the drive without healthy interlocks, and requires a hardware adapter to own actual motion control and power-stage control. The CiA 402 profile family standardizes functional behavior for controller types such as servo drives, frequency inverters, and stepper-motor controllers; completing one demands a selected controller type and mode-specific engineering.[4]

## Enabled protocol features and guarded exclusions

| Capability | Reference status | Notes |
|---|---|---|
| SDO server expedited/segmented/block | Enabled | Server buffer is 1024 bytes, sufficient for the stack’s maximum block segment buffering guidance.[5] |
| SDO client block transfer | Enabled | Provided for an application-managed client; no gateway command transport is exposed. |
| Dynamic and bitwise PDO mapping | Enabled | Dynamic mapping must be constrained and verified by the final OD and network timing analysis. |
| LSS slave and Fastscan direct response | Enabled | Provision a unique, non-reference identity before production. CiA 305 defines Fastscan for discovery of unconfigured LSS servers.[6] |
| CiA 309 ASCII gateway | Disabled | A product gateway needs an authenticated/controlled host interface, transport design, and command lifecycle. |
| CiA 304 GFC/SRDO | Disabled | Requires the complete SRDO OD and a dedicated functional-safety development and validation program. |
| Persistent OD storage | Disabled | The upstream STM32 target disables generic storage; implement atomic, wear-managed, power-fail-safe NVM before enabling store/restore commands. |

## References

[1]: https://github.com/CANopenNode/CANopenNode "CANopenNode — CANopen protocol stack"
[2]: https://github.com/CANopenNode/CanOpenSTM32 "CANopenNode STM32 integration"
[3]: https://canopennode.github.io/CANopenNode/group__CO__CANopen__304.html "CANopenNode CiA 304 module documentation"
[4]: https://www.can-cia.org/can-knowledge/cia-402-series-canopen-device-profile-for-drives-and-motion-control "CAN in Automation: CiA 402 series"
[5]: https://canopennode.github.io/CANopenNode/group__CO__STACK__CONFIG__SDO.html "CANopenNode SDO configuration"
[6]: https://www.can-cia.org/can-knowledge/cia-305-layer-setting-services-lss "CAN in Automation: CiA 305 Layer Setting Services"
