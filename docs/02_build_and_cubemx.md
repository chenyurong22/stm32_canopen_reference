# STM32CubeMX and Build Integration

**Status:** Board-port procedure for the reference firmware

This reference is intentionally not a complete `.ioc` project because **STM32F767 is a family, not a package or board definition**. Pin availability, flash/RAM map, HSE frequency, transceiver enable wiring, debug port, and production boot arrangement must be selected for the exact device and PCB. The STM32 CANopen integration expects the application to configure CAN/FDCAN bit rate, RX/TX interrupts, and a 1 ms timer before starting the stack.[1]

## CubeMX configuration

Create a bare-metal project for the exact STM32F767 package on the production board. Select HAL, disable an RTOS for this reference, and keep all generated files under source control. The following table is the baseline assumed by `Core/`.

| CubeMX area | Reference selection | Engineering note |
|---|---|---|
| RCC | HSE crystal, **25 MHz** | Change `SystemClock_Config()` and recompute CAN timing for any other HSE source. |
| Clock tree | SYSCLK 216 MHz, AHB 216 MHz, APB1 54 MHz, APB2 108 MHz | These values are the timing contract for `MX_CAN1_Init()` and TIM7. |
| CAN1 | Normal mode; PA11 RX / PA12 TX, AF9 | Connect a qualified CAN transceiver; STM32F767 does not provide a CAN physical layer. |
| CAN1 timing | Prescaler 6, SJW 1 TQ, BS1 15 TQ, BS2 2 TQ | 54 MHz / (6 × 18) = 500 kbit/s, sample point 88.9%. |
| CAN1 NVIC | TX, RX0, RX1, SCE enabled at priority 5 | All four IRQ handlers forward to `HAL_CAN_IRQHandler()`. |
| TIM7 | Internal clock; 1 ms update interrupt | Prescaler 54,000−1 and period 1−1 assuming a 54 MHz timer clock. |
| TIM7 NVIC | Enabled at priority 6 | The handler invokes `canopen_app_interrupt()` through HAL’s period callback. |
| GPIO | Board-specific transceiver enable, standby, LEDs, I/O | The reference does not guess actuator or transceiver safety polarities. |

The reference uses a 1 ms timer interrupt because CANopenNode’s recommended architecture separates time-critical SYNC/PDO processing from mainline SDO/NMT work.[2] Do not call blocking communication, flash, `printf`, dynamic allocation, or unbounded application code from the TIM7 callback.

## Preserving generated and owned files

The project-owned CANopen sources are deliberately separate from the generated CubeMX area. Add their include paths and source files to CubeIDE after generation. Compile `App/Src/CO_app_STM32_reference.c` **instead of** the upstream `CANopenNode_STM32/CO_app_STM32.c`; both define the same public runtime symbols.

| Category | Source to compile | Source to exclude | Reason |
|---|---|---|---|
| CANopen STM32 driver | `third_party/CanOpenSTM32/CANopenNode_STM32/CO_driver_STM32.c` | None | Supplies the bxCAN HAL driver and callbacks. |
| Runtime wrapper | `App/Src/CO_app_STM32_reference.c` | `third_party/.../CO_app_STM32.c` | Adds reference identity, safe profile scheduling, and reset handling. |
| CANopen stack | `third_party/CanOpenSTM32/CANopenNode/{301,303,304,305,309,storage}/*.c` | `CANopenNode/example/` | Includes the requested stack modules; feature flags remove unused internals. |
| Object Dictionary | `Generated/OD.c` | `CANopenNode/example/OD.c` | The generated reference OD carries the profile entries. |
| Application | `App/Src/*.c` | None | Owns profile binding and board hardware hooks. |

Define `STM32F767xx`, `USE_HAL_DRIVER`, and `CO_DRIVER_CUSTOM` for **every** translation unit, including third-party stack sources. `CO_DRIVER_CUSTOM` activates `App/Inc/CO_driver_custom.h`, which enables SDO block transfer, SDO client block transfer, dynamic/bitwise PDO processing, LSS slave Fastscan, and CANopen LED processing. CANopenNode documents block-transfer buffer requirements and the relevant feature dependencies.[3]

## Hardware acceptance prerequisites

The following actions must be completed before placing the node on a production or safety-relevant CAN network. They are not optional firmware TODOs.

| Area | Required acceptance evidence |
|---|---|
| CAN physical layer | Correct transceiver supply and logic level, common-mode range, termination plan, stub-length budget, ESD/surge protection, connector/pinout, and tested bus-off recovery. |
| Clock | Measured HSE frequency and clock-tree review; re-derived CAN timing and oscillator tolerance budget. |
| Electrical I/O | Schematics and tests for input range, output power, short-circuit behavior, diagnostics, startup state, EMC, and fail-safe defaults. |
| Drive enable | Independent hardwired safe torque/power removal path, interlock diagnostics, watchdog behavior, and fault-response test evidence. The CANopen `Controlword` cannot be the sole safety control. |
| Nonvolatile parameters | Atomic, wear-managed, power-fail-safe storage implementation before exposing CiA 301 store/restore behavior. |
| Identity | Unique vendor ID, product code, revision, serial number, and approved EDS/XDD. Do not release the reference identity constants. |

## CMake cross-build

The repository includes a toolchain-agnostic `CMakeLists.txt`. It expects an ARM GCC toolchain file, the official STM32CubeF7 firmware package, and a linker script for the exact part/package. A representative invocation is shown below; alter paths and the linker script for the real board.

```sh
cmake -S . -B build/f767 \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake \
  -DSTM32_CUBE_F7_DIR=/opt/STM32CubeF7 \
  -DSTM32_F7_LINKER_SCRIPT=/path/to/STM32F767xI_FLASH.ld \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/f767 --parallel
```

The resulting ELF, HEX, BIN, and map file are inputs to the release review. Retain the map file to check stack/heap placement, OD buffer memory, and linker-script consistency. Do not use a linker script selected for a different STM32F767 package.

## References

[1]: https://github.com/CANopenNode/CanOpenSTM32 "CANopenNode STM32: porting checklist"
[2]: https://github.com/CANopenNode/CANopenNode "CANopenNode execution flowchart"
[3]: https://canopennode.github.io/CANopenNode/group__CO__STACK__CONFIG__SDO.html "CANopenNode SDO configuration"
