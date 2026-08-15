# STM32F767 CANopen Reference Implementation

## Cover
STM32F767 CANopenNode Reference Firmware
Architecture, hardware setup, build workflow, and validation
Engineering reference overview

## Slide 1
### A validated reference—not a finished product
- Bare-metal STM32F767 CANopenNode integration with STM32 HAL and bxCAN
- Default personality: CiA 401 generic I/O reference at 500 kbit/s
- Selectable CiA 402 state-machine seam and optional CiA 309 gateway personality
- Clear boundary: conformance, HIL, safety, and board-specific implementation remain product work

## Slide 2
### Ownership boundaries keep the design maintainable
- CANopenNode owns CiA 301 communication services: NMT, SDO, PDO, SYNC, LSS, EMCY, and heartbeat
- Generated OD artifacts define the network contract; application modules own profile semantics
- Board hooks own transceiver control, I/O, feedback, power stages, diagnostics, and safe state
- Pinned CanOpenSTM32 binding owns bxCAN callback and interrupt integration

## Slide 3
### The runtime separates real-time CANopen from service work
- CAN1 IRQ: receive frames, complete mailboxes, and process CAN status/errors
- TIM7 at 1 ms: bounded SYNC/PDO cadence, hardware sampling, profile transition, and TPDO packing
- Main loop: SDO, NMT, heartbeat, reset supervision, diagnostics, and optional gateway processing
- Rule: no blocking I/O, heap allocation, flash access, or formatted logging in IRQ/timer paths

## Slide 4
### Hardware setup is intentionally narrow and explicit
| Function | Reference assignment |
|---|---|
| CAN1_RX | PA11 · alternate function AF9 |
| CAN1_TX | PA12 · alternate function AF9 |
| CAN physical layer | External CAN transceiver; CANH/CANL board-specific |
| Real-time service | TIM7, 1 ms, internal timer |
| CAN IRQ priorities | CAN TX/RX/status priority 5; TIM7 priority 6 |
- 25 MHz HSE reference assumption; 216 MHz system clock; 54 MHz APB1
- CAN timing: prescaler 6, BS1 15 TQ, BS2 2 TQ, SJW 1 TQ → 500 kbit/s
- Transceiver EN/STB, UART, I/O, feedback, and power-stage pins remain unassigned until board review

## Slide 5
### The Object Dictionary is the controlled network contract
- Editable EDS: `ObjectDictionary/stm32f767_canopen_reference.eds`
- Firmware artifacts: `Generated/OD.c` and `Generated/OD.h`
- Reproducible generation: `scripts/generate_reference_od.py`
- Synchronization validation: `scripts/validate_od.py`
- Guarded objdictgen import: `tools/import_objdict.sh --stage`
- Release rule: review EDS/XDD, access flags, PDO mapping, defaults, persistence, and identity together

## Slide 6
### Profile scope is configurable, but not overstated
| Profile / service | Reference posture |
|---|---|
| CiA 401 | Default I/O bridge with safe hardware adapter boundary |
| CiA 402 | Optional bounded controlword/statusword state reference; not a complete drive |
| CiA 309-3 | Optional ASCII gateway; disabled by default and authorization-gated |
| CiA 304 | SRDO/GFC deliberately disabled pending safety architecture |
- Profile selection must produce one coherent device type, OD, EDS/XDD, PDO policy, and test target
- Physical drive loops, limits, feedback, brakes, STO, and fault reactions remain product-owned

## Slide 7
### The build is reproducible with CMake and ARM GCC
```sh
export STM32_CUBE_F7_DIR=/opt/STM32CubeF7
export STM32_F7_LINKER_SCRIPT=$PWD/linker/STM32F767_2M_512K_FLASH.ld
./scripts/validate_reference.sh
```
- Official STM32CubeF7 package is external; CANopenSTM32 is pinned as a submodule
- CMake produces ELF, HEX, BIN, and MAP firmware artifacts
- Default build uses CANopenNode static/global object allocation (`CO_USE_GLOBALS`)
- Optional CiA 309 build is compiled separately to prevent accidental default exposure

## Slide 8
### CI combines wire-level regression with target builds
| CI job | What it executes |
|---|---|
| `host-vcan` | Creates `vcan0`; validates OD/import tooling; compiles C SocketCAN transport; runs Python wire tests |
| `cortex-m7-build` | Builds default firmware and optional gateway personality; uploads ELF/HEX/BIN/MAP artifacts |
- Protocol suite covers SDO identity read (`0x1018`), expedited/segmented SDO, NMT, heartbeat, PDO, mapping, LSS Fastscan, and EMCY
- vcan model validates wire contracts; it does not execute the compiled STM32 image
- Physical HIL, timing/jitter, CAN interoperability, and formal conformance remain required

## Slide 9
### Safe defaults make board integration deliberate
- Reference startup keeps the CAN transceiver disabled and application outputs de-energized
- Weak hooks force safe I/O, report unhealthy drive interlocks, and refuse drive enable
- Board port must implement verified EN/STB polarity, output safe state, feedback, watchdog, and independent power-stage protection
- Release evidence: schematic/EMC review, CAN traces, fault injection, HIL logs, profile conformance, and safety lifecycle records

## Slide 10
### Engineering handoff
- Start with the verified MCU assignments, then bind the exact STM32F767 package and board schematic
- Freeze the OD/EDS and profile personality before interoperability testing
- Run local validation and CI; retain the map, firmware hash, CAN traces, and HIL evidence
- Treat this repository as an integration baseline—not as a certification artifact

## Slide 11
### References and project records
- CANopenNode repository and module documentation [1]
- STM32F76xxx/F77xxx reference manual [2]
- CiA 402 device-profile overview [3]
- CANopenNode CiA 304 module documentation [4]
- Project records: `docs/01_architecture.md`, `docs/06_board_integration_and_hil.md`, `docs/07_profile_gateway_rtos_roadmap.md`, and `docs/08_remediation_completion.md`

[1]: https://github.com/CANopenNode/CANopenNode
[2]: https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
[3]: https://www.can-cia.org/can-knowledge/cia-402-series-canopen-device-profile-for-drives-and-motion-control
[4]: https://canopennode.github.io/CANopenNode/group__CO__CANopen__304.html
