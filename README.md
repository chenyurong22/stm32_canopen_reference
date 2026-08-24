# STM32F767 CANopen Reference

A bare-metal CANopen reference firmware for the **STM32F767** microcontroller. The project combines STM32 HAL, the pinned [CANopenNode](https://github.com/CANopenNode/CANopenNode) stack, and project-owned application adapters for building CANopen devices, test nodes, and gateway prototypes.

The default firmware is configured as a **CiA 401 I/O device reference** using CAN1/bxCAN at **500 kbit/s**. Optional build personalities provide CiA 402 drive-control interfaces, CiA 302 NMT-master supervision, and a bounded CiA 309 gateway foundation. These personalities are integration references and require product-specific hardware validation, Object Dictionary approval, and conformance testing before production use.

## Project description

The firmware provides the communication and application boundaries needed to develop an STM32F767 CANopen device:

- CANopenNode supplies CANopen communication services such as NMT, heartbeat, EMCY, SDO, PDO, SYNC, and LSS.
- The STM32 HAL and CanOpenSTM32 binding connect CANopenNode to the STM32F767 bxCAN peripheral.
- Project-owned code defines the runtime lifecycle, board safety hooks, device-profile adapters, diagnostics, gateway boundaries, and hardware test tools.
- The Object Dictionary and generated CANopen sources define the network-visible interface.
- The default application starts in a safe output state and keeps optional functions disabled unless explicitly selected.

This repository is a **reference implementation**, not a device-profile or functional-safety certification. A product implementation must add its exact board support, electrical protection, production Object Dictionary, application behavior, HIL evidence, and applicable conformance testing. The formal supported and unsupported feature boundary is defined in [`PRODUCT_SCOPE.md`](PRODUCT_SCOPE.md), with implementation and evidence detail in [`docs/feature_matrix.md`](docs/feature_matrix.md).

Project-owned material is available under the [STM32 CANopen Reference Research and Education License](LICENSE): free use is limited to qualifying research and education, while industrial, commercial, production, and large-scale use requires a separate paid commercial license. This is source-available and is not an OSI-approved Open Source license. Third-party components retain their respective licenses, which are listed in [THIRD_PARTY.md](THIRD_PARTY.md). See [COMMERCIAL-LICENSE.md](COMMERCIAL-LICENSE.md) for commercial licensing requests.

## Hardware reference

The current STM32F767 reference assumes the following interface:

| Function | Reference assignment |
|---|---|
| CAN1 receive | PA11, alternate function AF9 |
| CAN1 transmit | PA12, alternate function AF9 |
| CAN nominal rate | 500 kbit/s |
| External oscillator | 25 MHz HSE assumption |
| System clock | 216 MHz reference configuration |
| Real-time service | TIM7, 1 ms cadence |
| CAN transceiver | External board-level device; not included in the MCU |

The CAN transceiver, termination, connector, power supply, standby control, isolation, and application I/O are board-specific and must be implemented by the hardware integration.

## Build

### Prerequisites

Install the following tools on a Linux development host:

- CMake
- GNU Arm Embedded Toolchain, including `arm-none-eabi-gcc` and `arm-none-eabi-size`
- Native GCC and Make for host tests
- Python 3
- Git
- An STM32CubeF7 package supplied separately from this repository

The repository contains the CANopenNode and CanOpenSTM32 sources as submodules. STM32CubeF7 is intentionally supplied externally through `STM32_CUBE_F7_DIR`.

### Clone the repository

```sh
git clone --recurse-submodules https://github.com/mahdi-benhassen/stm32_canopen_reference.git
cd stm32_canopen_reference
git submodule update --init --recursive
```

### Build the default firmware

Set `STM32_CUBE_F7_DIR` to the location of a controlled STM32CubeF7 package. The linker script shown below is a reference for an STM32F767 configuration with 2 MiB flash and 512 KiB SRAM; replace it when the target memory map differs.

```sh
cmake -S . -B build/f767 \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake \
  -DSTM32_CUBE_F7_DIR=/opt/STM32CubeF7 \
  -DSTM32_F7_LINKER_SCRIPT="$PWD/linker/STM32F767_2M_512K_FLASH.ld" \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build/f767 --parallel
arm-none-eabi-size build/f767/stm32f767_canopen_reference
```

The build produces an ELF image and post-build HEX, BIN, and MAP artifacts in the selected build directory.

### Build optional personalities

The default configuration enables CiA 401 and disables optional personalities. Select one personality deliberately and validate its Object Dictionary and hardware behavior before use.

#### CiA 402 drive reference

```sh
cmake -S . -B build/cia402 \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake \
  -DSTM32_CUBE_F7_DIR=/opt/STM32CubeF7 \
  -DSTM32_F7_LINKER_SCRIPT="$PWD/linker/STM32F767_2M_512K_FLASH.ld" \
  -DCMAKE_C_FLAGS="-DCANOPEN_REFERENCE_ENABLE_CIA401=0 -DCANOPEN_REFERENCE_ENABLE_CIA402=1"

cmake --build build/cia402 --parallel
```

#### CiA 302 NMT-master reference

```sh
cmake -S . -B build/cia302 \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake \
  -DSTM32_CUBE_F7_DIR=/opt/STM32CubeF7 \
  -DSTM32_F7_LINKER_SCRIPT="$PWD/linker/STM32F767_2M_512K_FLASH.ld" \
  -DCANOPEN_REFERENCE_ENABLE_CIA302_MASTER=ON

cmake --build build/cia302 --parallel
```

The CiA 302 master personality is opt-in. It monitors a configured peer, supervises boot-up and heartbeat timing, and exposes bounded diagnostic state. It should be tested against a second CANopen node or deterministic network simulator.

#### Optional CiA 309 gateway foundation

```sh
cmake -S . -B build/gateway \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake \
  -DSTM32_CUBE_F7_DIR=/opt/STM32CubeF7 \
  -DSTM32_F7_LINKER_SCRIPT="$PWD/linker/STM32F767_2M_512K_FLASH.ld" \
  -DCMAKE_C_FLAGS="-DCANOPEN_REFERENCE_ENABLE_GATEWAY=1"

cmake --build build/gateway --parallel
```

The gateway is a bounded foundation only. A product must provide an authenticated host transport and an explicit diagnostic-access policy.

### Run host validation

The contract runners import project packages from the repository root, so the two package-based commands explicitly set `PYTHONPATH=.:tests`. The wire-contract suite is executable as a standalone script and fails non-zero when any assertion fails.

```sh
python3 tests/test_firmware_configuration.py
python3 tests/test_canopen_wire_contract.py
PYTHONPATH=.:tests python3 tests/run_uds_isotp_contract.py
PYTHONPATH=.:tests python3 tests/run_nmea2000_gateway_contract.py
make -C tests/host all test-stm32-facade test-gateway-default-deny test-acceptance-filter
make -C tests/host test-sanitize test-coverage-report
python3 tests/conformance/run_core_vectors.py
```

The optional fuzz target is a build-only libFuzzer harness. It injects arbitrary classic-CAN frame fields into the transport-neutral CiA 302/NMT protocol surface and exercises the project LSS bitrate, node-ID, store, and activation policy. A clang toolchain with libFuzzer support is required:

```sh
make -C tests/host test-fuzz
```

The target is intended to be run separately with a bounded corpus, timeout, and sanitizer configuration. Its results are host robustness evidence only; they do not replace hardware, EMC, HIL, security, or official CANopen conformance evidence.

For the hardware acceptance runner, see [`tests/hardware/README.md`](tests/hardware/README.md) and [`docs/hardware/uds_cia302_test_procedure.md`](docs/hardware/uds_cia302_test_procedure.md). The complete board, Flash, watchdog, profile, security, and formal-evidence release procedure is [`docs/production_validation_plan.md`](docs/production_validation_plan.md). Release tags additionally require the mandatory SocketCAN job in GitHub Actions; a missing `vcan0` fails that release gate rather than being skipped.

## Brief source structure

```text
App/
├── Inc/                         Project configuration and public application APIs
└── Src/
    ├── CO_app_STM32_reference.c Runtime lifecycle and CANopenNode integration
    ├── canopen_reference_board.c Board safety and hardware abstraction hooks
    ├── canopen_reference_cia302.c Opt-in CiA 302 NMT-master adapter
    ├── canopen_reference_diagnostics.c Bounded diagnostic status publisher
    ├── cia401_reference.c       CiA 401 I/O application adapter
    ├── cia402_reference.c       CiA 402 drive-control reference adapter
    └── canopen_reference_lss.c  Project LSS policy hooks

Core/
└── Src/                         CubeMX/HAL clock, GPIO, CAN, timer, and IRQ code

Generated/
├── OD.c / OD.h                  Generated CANopen Object Dictionary implementation
└── cia418_OD.*                  Separate battery-profile reference artifacts

middleware/
├── canopen/core/                Project CANopen helpers and CiA 302 state machine
├── canopen/port/                CAN-port abstraction and transport tests
├── diagnostics/                 Host-side UDS/ISO-TP contract model
└── gateway/                     Host-side gateway models

third_party/
└── CanOpenSTM32/                Pinned CANopenNode STM32 binding and stack

tests/
├── host/                        Native C transport and protocol tests
├── hardware/                    SocketCAN HIL acceptance runner and procedure
└── test_*.py                    Deterministic source and contract tests

scripts/                         Object Dictionary and profile validation tools
CMakeLists.txt                   ARM firmware and host validation build definitions
```

Project-owned application code belongs in `App/` or the project middleware directories. CubeMX-generated platform code remains in `Core/`, and third-party stack code remains under `third_party/`. Do not link both the project runtime wrapper and the original CANopenNode STM32 application wrapper in the same firmware image.

The runtime has explicit startup, running, reset-requested, reinitializing, and safe-fault states. CAN bus-off recovery is bounded and mainline-only. OD 1010h/1011h persistence uses CRC-validated dual-slot Flash on the reference linker map; configure `CANOPEN_REFERENCE_STORAGE_MIN_STORE_INTERVAL_MS` for a board-specific write-rate policy before production use. The opt-in IWDG path requires measured LSI timing and reset-recovery validation. The UDS profile is disabled by default and its exact configuration, addressing, service, security, Flash, target, timing, HIL, and troubleshooting boundaries are indexed in [`docs/uds/`](docs/uds/).

## Application examples

### Industrial remote I/O

Use the default CiA 401 personality to build a distributed I/O node for machine panels, valve islands, sensor concentrators, or industrial controllers. Digital and analogue channels can be mapped into PDOs while configuration and diagnostics remain accessible through SDO.

### Motion-control interface

Use the CiA 402 reference seam as a starting point for a servo, actuator, pump, or positioning controller. The product must add the actual power-stage enable logic, feedback processing, limits, fault reactions, and supported operating modes before it can control machinery.

### CANopen commissioning and supervision

Use the CiA 302 personality with a second CANopen node to supervise boot-up, heartbeat availability, NMT state, startup policy, and network readiness. This is useful for controller prototypes, commissioning tools, and multi-node integration benches.

### Diagnostic test node

Use the host-side UDS/ISO-TP contract model and SocketCAN hardware runner to exercise diagnostic sessions, negative responses, CAN timing, NMT transitions, reset behavior, and heartbeat supervision during development and production test.

## UDS diagnostics

The opt-in UDS profile is a bounded classic-CAN ISO-TP and UDS reference subset. Begin with the [UDS architecture](docs/uds/architecture.md), [ISO-TP contract](docs/uds/isotp.md), [service matrix](docs/uds/services.md), and [configuration](docs/uds/configuration.md). The [STM32F767 hardware runner](tests/hardware/run_uds_stm32f767_acceptance.py) keeps reset and download operations disabled unless the operator explicitly enables them. This repository does not claim complete ISO 14229 or ISO 15765-2 conformance, a production bootloader, or production cryptographic update security.

### CANopen gateway prototype

Use the bounded CiA 309 and gateway foundations as an integration starting point for a service tool, protocol bridge, or PC-connected commissioning interface. Add authentication, transport limits, access control, and fault handling before exposing the gateway to a deployed network.

## Important limitations

The reference does not define a universal STM32F767 board pinout, external CAN transceiver design, production Object Dictionary, application safety case, or device-profile certificate. Hardware teams must validate the exact MCU package, clock source, CAN physical layer, bus termination, node identity, PDO map, timing margins, reset behavior, and safe-state response on the target board.

## Related documentation

Use the [documentation map](docs/README.md) as the complete index of architecture notes, profile procedures, qualification gates, release records, and historical reviews.

| Topic | Document |
|---|---|
| Reproducible build and flashing | [BUILD.md](BUILD.md) |
| Documentation index | [docs/README.md](docs/README.md) |
| CubeMX ownership and board porting | [Build and CubeMX notes](docs/02_build_and_cubemx.md) |
| CANopen wiring and hardware bring-up | [Hardware integration guide](docs/hardware.md) |
| Third-party Object Dictionary requests | [OD request handling procedure](docs/handling_third_party_od_requests.md) |
| In-process protocol smoke testing | [Mock CANopen smoke testing](docs/mock_canopen_protocol_smoke_testing.md) |
| Inventus battery test profile | [Inventus profile](docs/inventus_battery_test_profile.md) |
| v1 qualification and release gates | [Production validation plan](docs/production_validation_plan.md) |
| Protocol examples and frame sequences | [Examples guide](examples/README.md) |
| UDS/CiA 302 acceptance | [Hardware procedure](docs/hardware/uds_cia302_test_procedure.md) and [test runner](tests/hardware/README.md) |
| UDS/ISO-TP reference profile | [UDS documentation index](docs/uds/), including [architecture](docs/uds/architecture.md), [ISO-TP](docs/uds/isotp.md), [services](docs/uds/services.md), [security](docs/uds/security.md), [Flash programming](docs/uds/flash_programming.md), and [HIL testing](docs/uds/hil_testing.md) |
| Object Dictionary | [EDS](ObjectDictionary/stm32f767_canopen_reference.eds) |
| Dependencies and licenses | [THIRD_PARTY.md](THIRD_PARTY.md) and [LICENSE](LICENSE) |
| Contribution process | [CONTRIBUTING.md](CONTRIBUTING.md) |
| Security boundaries | [SECURITY.md](SECURITY.md) |
| Feature and evidence status | [Feature matrix](docs/feature_matrix.md) |
| Change history | [CHANGELOG.md](CHANGELOG.md) |

The upstream components are [CANopenNode](https://github.com/CANopenNode/CANopenNode) and [CanOpenSTM32](https://github.com/CANopenNode/CanOpenSTM32).
