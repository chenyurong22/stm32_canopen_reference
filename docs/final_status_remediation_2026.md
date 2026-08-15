# Final-status remediation and evidence matrix

**Date:** 15 August 2026  
**Baseline:** STM32F767 CANopen reference, `main` branch  
**Purpose:** Record the implementation corrections made after the final-status review and separate source/build evidence from board, network, and formal-conformance evidence.

## Executive status

The defects identified as missing in the reference baseline have been addressed in project-owned code. The status is **not equivalent to production release or formal CANopen conformance**: physical HIL, EMC, electrical transceiver validation, and an official conformance test remain external release gates.

> Host tests demonstrate deterministic software behavior. They do not prove physical CAN timing, transceiver behavior, EMC immunity, power-loss behavior on a populated board, or official CiA conformance.

## Remediation matrix

| Review finding | Implementation status | Evidence | Remaining boundary |
|---|---|---|---|
| CAN bus-off was detected but not recovered | **Implemented** | `canopen_reference_can_recovery.[ch]` provides STOP/WAIT/REINIT/FAULT transitions; application reset failures are bounded and diagnosed. | Validate recovery on a real transceiver under bus-off and repeated-fault conditions. |
| Communication reset ignored reinitialization failure | **Implemented** | `CANopenReference_FailRuntime()` latches safe state, disables CAN processing, stops TIM7, forces safe outputs, and reports a runtime fault. | Confirm board-specific safe-output and transceiver-disable timing. |
| Default persistence was RAM-backed | **Implemented** | `canopen_reference_storage.c` uses two STM32F7 Flash sectors, CRC validation, sequence selection, and commit-last metadata; weak hooks remain available. | Perform power-interruption and Flash endurance testing on the target board. |
| Persistence could overlap executable image | **Implemented** | `linker/STM32F767_2M_512K_FLASH.ld` limits executable `FLASH` to 1536 KiB and declares `FLASH_NVM` at `0x08180000` for the final 512 KiB. | Verify the exact MCU density/package and bootloader reservation for each product board. |
| Hardware filters assumed default COB-IDs | **Implemented** | Filter construction reads active OD RPDO/SDO-client mappings and adds mandatory NMT, SYNC, TIME, EMCY, heartbeat, and LSS identifiers. | Recalculate filters after runtime remapping if the product permits dynamic PDO remapping while active. |
| CAN facade ignored timeout and tolerated unsupported targets | **Implemented** | `can_port.c` uses bounded `HAL_GetTick()` polling with a 1 ms wait and returns `-ENOTSUP` for unsupported targets. | Validate interrupt latency and timeout values against the actual RTOS or bare-metal integration. |
| Watchdog timing and reset reason were under-specified | **Implemented, opt-in** | IWDG startup grace, bounded LSI readiness, timer-progress deadline, mainline refresh gating, and RCC reset-cause capture are implemented; default remains disabled. | Measure LSI-derived timeout and deliberate reset behavior on each board revision. |
| Error mapping was incomplete | **Implemented** | HAL ACK, bus-off, warning/passive, framing, stuffing, form, FIFO overflow, arbitration, and transmit-error classes are mapped to CANopen diagnostics. | Correlate mappings with target silicon error registers during HIL fault injection. |
| Fault/recovery test coverage was incomplete | **Improved** | Firmware source contracts cover error mapping, recovery integration, Flash reservation, filters, transport deadlines, and watchdog configuration; `test_can_recovery.c` covers state transitions and tick wraparound. | Add hardware fault-injection results to the product verification repository. |

## Profile and protocol boundaries

| Capability | Baseline status | Claim permitted by this repository |
|---|---|---|
| CiA 301 core services | Integrated through pinned CANopenNode | Reference-level NMT, heartbeat, EMCY, SDO, PDO, SYNC, and LSS integration; not formal conformance certification. |
| CiA 401 | Default project personality | Reference I/O application seam with safe board hooks; channel count, electrical behavior, and profile-specific conformance remain product work. |
| CiA 402 | Optional reference adapter | State-machine and application seam only; no claim of complete drive power-stage behavior, limits, feedback, or safety reaction. |
| CiA 302 NMT master | Optional build personality | Bounded peer supervision and NMT-master reference behavior; requires multi-node HIL validation. |
| LSS | Policy and bounded stack integration | Basic configuration policy and stack support; no claim of complete product Fastscan commissioning acceptance without target tests. |
| UDS/ISO-TP | Host contract model and hardware procedure | Diagnostic contract and acceptance procedure; no embedded UDS/ISO-TP implementation claim. |
| CiA 418, NMEA 2000, CAN-FD, CiA 304, bootloader/update security | Out of scope or roadmap | These must remain explicitly unimplemented until separately designed, implemented, tested, and reviewed. |

## Required release evidence

A product release should attach the exact firmware hash, compiler/toolchain version, Object Dictionary and EDS hashes, linker script, enabled CMake personality options, board revision, transceiver part number, and test-run identifiers. The following evidence must be completed outside host-only CI:

1. Run the SocketCAN/vcan regression suite on a privileged CI runner and retain the logs.
2. Run the hardware procedure in `docs/hardware/uds_cia302_test_procedure.md` against the target and a second CANopen node or deterministic bus simulator.
3. Force bus-off, observe safe outputs, verify bounded recovery, and repeat until the configured terminal-fault policy is reached.
4. Store and restore OD communication parameters, interrupt power during each Flash commit phase, and verify CRC rejection plus factory-default fallback.
5. Measure CAN sample point and bit rate with a CAN analyzer or oscilloscope at the selected oscillator and transceiver configuration.
6. Enable the watchdog on the board, stop TIM7 and mainline execution independently, and record the reset cause and elapsed time.
7. Run applicable official CANopen conformance and product profile tests before making a compliance claim.

## Reproducible software gates

```sh
python3 scripts/validate_repository.py
python3 tests/test_firmware_configuration.py
python3 tests/test_canopen_wire_contract.py
PYTHONPATH=.:tests python3 tests/run_uds_isotp_contract.py
make -C tests/host all
make -C tests/host test-recovery test-stm32-facade test-gateway-default-deny
```

The software gates are necessary but not sufficient. A final status of **production-ready**, **formally conformant**, or **hardware-validated** must not be assigned from these commands alone.
