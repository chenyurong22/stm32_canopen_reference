# STM32F767 CANopenNode Reference Firmware

**Author:** Manus AI  
**Status:** Build-validated engineering reference  
**Default role:** Bare-metal CiA 401 generic I/O device with a selectable CiA 402 drive-interface reference

This repository is a clean, build-validated STM32F767 firmware reference built around [CANopenNode](https://github.com/CANopenNode/CANopenNode) and the `CanOpenSTM32` HAL/bxCAN binding. It establishes a disciplined separation between the CANopen communication stack, generated Object Dictionary (OD), profile-specific application logic, and physical board/power-stage control. The reference uses classic CAN on **CAN1**, a 25 MHz HSE clock, a 216 MHz system clock, APB1 at 54 MHz, and 500 kbit/s CAN bit timing with 18 time quanta per bit.

> **Important:** This is not a declaration of CiA 401, CiA 402, CiA 304, IEC 61508, ISO 13849, or SIL/PL conformity. It is a robust, traceable implementation starting point. Product compliance requires the applicable specifications, an exact EDS/XDD, hardware design review, fault analysis, timing measurements, protocol/profile conformance testing, and—where safety is claimed—an independent safety lifecycle and evidence.

## Implemented reference boundary

CANopenNode supplies the CiA 301 communication mechanisms used by this project. The default device configuration enables SDO segmented and block transfers, CRC16 for block transfer, SDO client support, dynamic and bitwise PDO mapping, LSS slave services, and CiA 303 status LEDs. CiA 304 GFC/SRDO remains disabled. A separately buildable, disabled-by-default CiA 309 ASCII gateway personality is now provided behind explicit product and runtime authorization controls; it requires a dedicated UART/USB/network transport, command-permission policy, and security/interoperability evidence before release.[1] [2]

| Capability | Reference status | Location | Product completion required |
|---|---:|---|---|
| CiA 301 NMT, heartbeat, EMCY, SDO, SYNC, PDO | Enabled through CANopenNode | `App/Inc/CO_driver_custom.h` | Establish node-ID policy, COB-IDs, timeouts, EDS/XDD, network load, and conformance evidence. |
| SDO block transfer | Enabled with 1024-byte buffer and CRC16 | `CO_driver_custom.h` | Size buffers and test abort paths, update policy, and worst-case latency. |
| Dynamic/bitwise PDO mapping | Enabled | `CO_driver_custom.h`, `Generated/OD.*` | Constrain permitted mappings and test every released map and persistence behavior. |
| LSS slave/Fastscan | Stack-enabled | `CO_driver_custom.h` | Provision identity, node-ID/bit-rate storage, reset behavior, and production commissioning procedure. |
| CiA 401 generic I/O | Selected by default | `App/Src/cia401_reference.c` | Implement board drivers, electrical diagnosis, scaling, channel objects, and full profile object set. |
| CiA 402 drive interface | Selectable, unit-tested state-reference | `App/Src/cia402_reference.c` | Implement actual modes, loops, units, limits, brakes, sensors, faults, and the complete required profile behavior. |
| CiA 304 SRDO/GFC | Disabled | `CO_driver_custom.h` | Safety concept, independent channels, SRDO OD, watchdogs, FMEA/FM(E)DA, timing, and certification evidence. |
| CANopen gateway | Optional, disabled-by-default CiA 309 ASCII adapter | `App/Src/canopen_reference_gateway.c` | Bind a bounded transport, define authorization/permissions, and perform security, load, and interoperability testing. |

## Repository layout

| Path | Purpose |
|---|---|
| `Core/` | STM32F767 entry point, bxCAN/TIM7 setup, interrupts, clocks, HAL configuration, linker/runtime policy. |
| `App/` | CANopen runtime wrapper, profile bindings, hardware/board adapter interfaces, optional diagnostics, gateway bridge, and feature configuration. |
| `middleware/canopen/` | Project-owned CANopen lifecycle facade, portable CAN-port contract, SocketCAN transport, and host wire-level device harness. |
| `Generated/` | Generated-style OD C and header artifacts compiled into firmware. Do not hand-edit in a production workflow. |
| `ObjectDictionary/` | Editable EDS artifact used for communication/profile review. |
| `scripts/generate_reference_od.py` | Deterministically derives the reference OD artifacts from the pinned upstream example and profile overlay. |
| `scripts/validate_od.py` | Checks EDS and generated OD synchronization and index ordering. |
| `scripts/validate_reference.sh` | Runs the OD check, host profile tests, ARM firmware build, and artifact checks. |
| `tools/import_objdict.sh` | Guarded import/staging command for generated objdictgen C/H artifacts or ZIP files. |
| `tests/` | Host-side deterministic profile tests plus vcan CANopen wire-contract regression tests. |
| `.github/workflows/ci.yml` | GitHub Actions vcan regression and default/optional-gateway Cortex-M7 build workflow. |
| `docs/` | Architecture, CubeMX/build, middleware, board/HIL, profile/RTOS, audit, and remediation documentation. |
| `third_party/` | Pinned upstream CANopenNode/STM32 binding plus an optional local STM32CubeF7 checkout for validation. |

## Hardware assumptions and timing

The reference uses **PA11** for CAN1 RX and **PA12** for CAN1 TX. A physical high-speed CAN transceiver is mandatory and its enable/standby control is deliberately board-specific. The bxCAN timing values are `Prescaler=6`, `BS1=15 TQ`, `BS2=2 TQ`, and `SJW=1 TQ`; with APB1 at 54 MHz this produces 500 kbit/s and an approximately 88.9% sample point. The 1 ms application service is driven by TIM7 using a 54 MHz timer kernel clock.

| Parameter | Reference value | Review required before hardware use |
|---|---:|---|
| MCU family | STM32F767, 2 MiB flash / 512 KiB SRAM linker reference | Confirm exact part/package and memory partitions. |
| External clock | 25 MHz HSE | Replace `SystemClock_Config()` if the board differs. |
| System/APB1 | 216 MHz / 54 MHz | Recalculate bxCAN and TIM7 settings after clock changes. |
| CAN pins | CAN1 PA11/PA12 | Confirm AF9 routing, transceiver supply, termination, common-mode range, ESD, and standby behavior. |
| Nominal bit rate | 500 kbit/s | Commission using the actual network topology and requirements. |
| Main-loop scheduling | Non-blocking; TIM7 1 ms real-time path | Measure jitter, ISR priority, bus load, watchdog latency, and application execution time. |

## Build and validation

The project is provided with CMake and an ARM GCC toolchain definition. A standard STM32CubeF7 firmware package is expected at `STM32_CUBE_F7_DIR`; it supplies the HAL sources, CMSIS device startup file, and system file. The reference linker script is for the 2 MiB/512 KiB STM32F767 memory map only.

```sh
export STM32_CUBE_F7_DIR=/absolute/path/to/STM32CubeF7
export STM32_F7_LINKER_SCRIPT=$PWD/linker/STM32F767_2M_512K_FLASH.ld
scripts/validate_reference.sh
```

The validation script checks the OD/EDS consistency, compiles and runs the application profile tests on the host, configures the Cortex-M7 target, builds the ELF, and verifies that HEX and BIN outputs exist. The validated image reports the following static size in the supplied configuration.

| Image component | Size | Notes |
|---|---:|---|
| `.text` | 53,472 bytes | Code and read-only data. |
| `.data` | 1,216 bytes | Initialized RAM. |
| `.bss` | 9,624 bytes | Static RAM, including statically allocated CANopen objects. |
| Total (`text + data + bss`) | 64,312 bytes | Does not constitute a stack-watermark, WCET, or hardware qualification result. |

The CMake build defines `CO_USE_GLOBALS`, using CANopenNode’s supported global/static-object configuration for this single generated OD. The application’s newlib stubs intentionally reject file I/O and heap growth; this reference must not be converted to heap-dependent runtime behavior without an explicit memory, failure, and test strategy.

## Object Dictionary workflow

The EDS at `ObjectDictionary/stm32f767_canopen_reference.eds` is the editable external representation supplied with this reference. The `Generated/OD.c` and `Generated/OD.h` files demonstrate the firmware artifact layout expected by CANopenNode. A production project should retain its authoritative `objdictgen` project and regenerate both EDS/XDD and C artifacts whenever OD, profile, PDO-map, datatype, access, persistence, or default-value changes are made.

```sh
python3 scripts/generate_reference_od.py
python3 scripts/validate_od.py
tools/import_objdict.sh /path/to/objdictgen-output --stage
```

On a Linux host with permission to create a virtual CAN interface, run the wire-level protocol regression suite with `sudo scripts/setup_vcan.sh vcan0` followed by `CAN_PORT_IFACE=vcan0 make -C tests/host test`. The GitHub Actions workflow provisions that interface and also builds the optional gateway personality. See [`08_remediation_completion.md`](docs/08_remediation_completion.md) for exact validation status and the sandbox limitation.

The reference adds the following profile-facing indices. Whether each object is mandatory, optional, manufacturer-specific, PDO-mappable, or persistent in a released product must be decided against the licensed profile specification and product requirement.

| Index | Reference meaning |
|---:|---|
| `0x6000`, `0x6200`, `0x6401`, `0x6411`, `0x6422` | CiA 401-style digital/analogue process data bridge. |
| `0x603F`, `0x6040`, `0x6041` | CiA 402 error/control/status seam. |
| `0x6060`, `0x6061` | Requested and displayed modes of operation. |
| `0x6064`, `0x606C`, `0x6077` | Position, velocity, and torque feedback seam. |
| `0x6071`, `0x607A`, `0x60FF` | Torque, position, and velocity command seam. |

## Profile selection and hardware ownership

Profile switches are declared in `App/Inc/canopen_reference_config.h`. CiA 401 is enabled by default; CiA 402 is disabled by default. Enabling both requires explicit `CANOPEN_REFERENCE_ALLOW_COMBINED_PROFILES=1` because a combined device must define a coherent device type, identity, OD, EDS/XDD, PDO policy, lifecycle, and conformance target.

All physical I/O and drive control terminate at `App/Inc/canopen_reference_hw.h`. The weak reference implementations intentionally read safe/zero values, write safe/zero values, report unhealthy drive interlocks, and refuse to enable a drive. Replace each relevant hook in a board-specific implementation. Do not make raw GPIO, ADC, PWM, encoder, power-stage, STO, brake, or safety-monitor calls from the CANopen stack or OD code.

## Remediation and integration records

The native CANopenNode lifecycle decision and compatibility facade are described in [`docs/05_architecture_decision.md`](docs/05_architecture_decision.md). Board safe-state and hardware-in-the-loop requirements are in [`docs/06_board_integration_and_hil.md`](docs/06_board_integration_and_hil.md), while the CiA 401/402, gateway, and FreeRTOS completion roadmap is in [`docs/07_profile_gateway_rtos_roadmap.md`](docs/07_profile_gateway_rtos_roadmap.md). The completed remediation traceability and reproducible validation results are in [`docs/08_remediation_completion.md`](docs/08_remediation_completion.md).

## Engineering release gate

Before using this reference in a fielded product, execute the following engineering activities and retain their outputs in configuration control.

| Area | Required release evidence |
|---|---|
| Hardware | Schematic/layout review, transceiver and termination validation, EMC/ESD plan, power/reset behavior, boot/update architecture, fault injection, and temperature/voltage testing. |
| CANopen | Exact EDS/XDD, DCF policy, node-ID/bit-rate provisioning, startup/error behavior, network load calculation, interoperability tests, protocol conformance results, and regression traces. |
| CiA 401 | Signal scaling, channel diagnosis, output safe state, wiring-fault behavior, debounce/filtering, update timing, and I/O profile conformance evidence. |
| CiA 402 | Mode-specific behavior, physical units, limits, homing, state transitions, following error, encoder/feedback loss, brake policy, fault reaction, quick stop, watchdog, and drive-profile conformance evidence. |
| Safety | A separate safety plan, hazard/risk analysis, independent protection path, safety requirements, lifecycle records, coverage analysis, safety validation, and certification/assessment as required. |
| Cybersecurity | Firmware signing/update policy, debug-port lifecycle, device identity/provisioning policy, configuration protection, diagnostics, and incident/update process. |

## References

[1]: https://github.com/CANopenNode/CANopenNode "CANopenNode repository"
[2]: https://github.com/CANopenNode/CANopenNode/blob/master/doc/overview.md "CANopenNode overview and module documentation"
