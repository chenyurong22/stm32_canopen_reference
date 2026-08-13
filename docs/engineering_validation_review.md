# STM32F767 CANopen Reference — Engineering Validation Review

**Review date:** 2026-08-13
**Scope:** STM32F767 bare-metal firmware, CANopenNode integration, Object Dictionary artifacts, CiA 401/402 reference behavior, host SocketCAN harness, build system, and CI workflow.

## Executive conclusion

The reviewed repository is a **well-bounded reference implementation**, not a hardware-qualified production device. The reviewed source builds successfully for the default CiA 401 personality and the optional CiA 402 personality. The Object Dictionary is internally consistent, host-independent profile tests pass, and deterministic source-contract tests now protect the principal hardware assumptions.

A material timing defect was discovered and corrected during this review. The original TIM7 divider treated the APB1 peripheral clock of 54 MHz as the timer kernel clock. With APB1 divided by four, STM32F7 general-purpose timers run at twice PCLK1; TIM7 therefore runs from 108 MHz. The original divider produced a 0.5 ms interrupt while CANopen processing was told that 1 ms had elapsed. The corrected divider is 108000, restoring the intended 1 ms cadence. The timing contract is now protected by a compile-time assertion and a host-side source-contract test. STM32F7 clock and timer behavior must still be confirmed against the selected device’s reference manual and the actual board clock source.[1]

> **Assurance boundary:** No software-only review can establish 100% functional or safety assurance for a CAN-connected embedded device. Target hardware, a real CAN transceiver and network, power-stage safety hardware, and a CiA conformance test environment remain necessary before product release.

## Review coverage and outcomes

| Area | Review outcome | Automated evidence |
|---|---|---|
| STM32 clock tree | 25 MHz HSE, 216 MHz SYSCLK, 54 MHz APB1 contract reviewed | `tests/test_firmware_configuration.py` |
| TIM7 CANopen cadence | Corrected from an effective 2 kHz interrupt to 1 kHz | Compile-time assertion and source-contract test |
| bxCAN timing | `54 MHz / (6 × (1 + 15 + 2)) = 500 kbit/s` | Source-contract test |
| CAN1 wiring and IRQs | PA11/PA12 AF9; TX/RX0/RX1/SCE IRQ configuration reviewed | Source-contract test |
| CANopenNode features | SDO segmented/block configuration, 1024-byte buffers, FIFO CRC prerequisites, dynamic PDO, LSS, LEDs reviewed | Source-contract test; pinned upstream source audit |
| Object Dictionary | Generated C/header ordering and required CiA 401/402 EDS objects checked | `scripts/validate_od.py` |
| CiA 401 bridge | Safe initialization, input acquisition, output application, and force-safe behavior checked | `tests/test_profiles.c` |
| CiA 402 seam | Interlocks, fault behavior, reset, quick stop, mode validation, feedback propagation, and safe disable checked | `tests/test_profiles.c` |
| Default firmware personality | ARM cross-build completed | `scripts/validate_reference.sh`; CI |
| CiA 402 personality | Separate ARM cross-build completed | `scripts/validate_reference.sh`; CI |
| CiA 309 gateway personality | Build remains covered in CI; authorization defaults deny access | CI |
| SocketCAN protocol model | SDO, NMT, heartbeat, PDO, LSS, and EMCY model contracts exist | `tests/host/test_sdo.py`, when `vcan` is available |

## Corrective action implemented

### TIM7 cadence correction

The timing contract is now explicit in `Core/Src/main.c`:

| Parameter | Correct value | Rationale |
|---|---:|---|
| APB1 peripheral clock | 54 MHz | 216 MHz HCLK divided by four |
| TIM7 kernel clock | 108 MHz | APB prescaler is not one, so the timer kernel uses twice PCLK1 |
| TIM7 prescaler divider | 108000 | 108 MHz divided by 108000 equals 1 kHz |
| TIM7 period ticks | 1 | One update event per prescaled tick |
| CANopen interrupt cadence | 1 ms | Matches the `1000U` microsecond argument passed to SYNC/RPDO/TPDO processing |

The original divisor of 54000 produced 2 kHz, which would have advanced interrupt-driven CANopen timers twice as quickly as their declared elapsed time. The correction preserves the upstream STM32 integration model, which expects a 1 ms timer callback that invokes `canopen_app_interrupt()`.[2]

## Validation added during this review

The validation workflow now includes the following additions.

| Addition | Purpose |
|---|---|
| `tests/test_firmware_configuration.py` | Protects clock tree, CAN bitrate, TIM7 cadence, pins/IRQs, CANopenNode feature dependencies, selected profile defaults, and fail-safe board defaults. |
| Expanded `tests/test_profiles.c` | Adds CiA 401 stale-output initialization checks and CiA 402 quick-stop, unsupported-mode, hardware-fault, and fault-reset assertions. |
| CiA 402 build in `scripts/validate_reference.sh` | Compiles a standalone CiA 402 personality in addition to the default CiA 401 firmware. |
| CiA 402 build in CI | Ensures the optional drive personality continues to compile on every hosted workflow run. |
| CI host validation extension | Runs Object Dictionary checks, Python syntax checks, configuration contracts, CiA profile unit tests, and C SocketCAN transport compilation before conditional runtime testing. |

## Functional limitations and release evidence still required

The items below are intentional boundaries of a reference project and must be closed with target-specific evidence before declaring a commercial device production-ready.

| Open evidence item | Why host/CI validation is insufficient | Required release evidence |
|---|---|---|
| Physical CAN operation | The repository has only weak, fail-safe board hooks; it cannot validate transceiver standby polarity, termination, isolation, EMC, or bus-off recovery on the actual board. | Bench test with the selected STM32F767 board, transceiver, power supply, and a second CAN node. |
| CANopen conformance | The host vcan model is a controlled model, not the compiled ARM firmware or an independent conformance implementation. | CiA CANopen conformance test report for the released EDS and firmware revision. |
| SDO block transfer interoperability | Feature flags and buffer dependencies are compiled, but full wire-level block-transfer sequences are not executed against the STM32 target in CI. | Target capture/test covering upload, download, CRC, timeout, sequence errors, and abort codes. CANopenNode documents the segmented/block and buffer prerequisites.[3] |
| LSS commissioning | CI exercises only the bounded virtual-model response contract. | Target test covering Fastscan, node-ID/bitrate configuration, store/reset, and reboot behavior. |
| CiA 402 motion safety | This project provides a state-machine seam, not a trajectory generator, torque loop, STO path, or certified safety function. | Product-specific drive-control, limit, encoder, braking, fault-reaction, and independent safety validation. |
| CI SocketCAN runtime suite | The GitHub-hosted Azure kernel used by the workflow lacks the `vcan` driver, so CI explicitly skips runtime vcan tests while retaining deterministic checks. | Run `make -C tests/host test` on a Linux runner with `CONFIG_CAN_VCAN`, or use a self-hosted hardware-in-the-loop runner. |

## Reproducible commands

Run the complete local software validation with the STM32CubeF7 source tree available:

```sh
scripts/validate_reference.sh
```

Run the host SocketCAN suite on a Linux system that supports virtual CAN:

```sh
scripts/setup_vcan.sh vcan0
CAN_PORT_IFACE=vcan0 make -C tests/host test
```

The GitHub workflow runs deterministic validation on every push. It runs the SocketCAN runtime suite only when its runner can create `vcan0`, and emits an explicit warning when that kernel capability is unavailable.

## References

[1]: https://www.st.com/resource/en/reference_manual/dm00224583-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf "STMicroelectronics RM0410 / STM32F76xxx and STM32F77xxx reference manual"
[2]: https://github.com/CANopenNode/CanOpenSTM32 "CANopenNode CanOpenSTM32 integration guidance"
[3]: https://canopennode.github.io/CANopenNode/group__CO__STACK__CONFIG__SDO.html "CANopenNode SDO server/client configuration"
