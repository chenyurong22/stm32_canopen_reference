# Engineering audit remediation

This note records the reference-firmware changes made in response to the 2026 engineering audit. Each control is intentionally isolated in a focused commit so that the safety and integration review can trace the change to source and tests.

## Implemented controls

| Audit area | Remediation | Evidence |
| --- | --- | --- |
| Interrupt priority | CAN1 interrupts use pre-emption priority 5 and TIM7 uses priority 6 under `NVIC_PRIORITYGROUP_4`. | `Core/Src/stm32f7xx_hal_msp.c`; `test_can_pins_and_interrupts_match_can1_contract` |
| CAN timing | The default 500 kbit/s configuration uses prescaler 6, BS1 14 TQ, and BS2 3 TQ: 18 total TQ and an 83.33% sample point from the 54 MHz APB1 clock. | `Core/Src/main.c`; `test_bxcan_bit_timing_is_500_kbit` |
| Standalone bitrate selection | The object-only STM32 facade validates 10, 20, 50, 125, 250, 500, 800, and 1000 kbit/s and applies a bounded timing table. | `middleware/canopen/port/can_port.c` |
| CAN diagnostics | HAL CAN error callbacks propagate bus-off, warning, receive overflow, and transmit/receive warning conditions to CANopenNode diagnostics and bounded hardware-error counters. | `App/Src/CO_app_STM32_reference.c`; diagnostics API |
| OD persistence | CANopenNode storage is re-enabled through the project-owned driver override. OD 1010h/1011h are initialized before `CO_CANopenInit()`, with a bounded CRC-validated backend and weak board hooks. | `App/Src/canopen_reference_storage.c` |
| Watchdog supervision | An opt-in IWDG requires progress from both TIM7 and the mainline before refresh. It is disabled by default and enabled with `-DCANOPEN_REFERENCE_ENABLE_IWDG=ON`. | `App/Src/canopen_reference_watchdog.c`; `CMakeLists.txt` |
| Hardware acceptance filtering | bxCAN uses three 16-bit list-filter banks for NMT, SYNC/TIME, EMCY, node-specific PDO/SDO/heartbeat, and LSS frames rather than the previous accept-all filter. | `CANopenReference_ConfigureCanFilter()` |

## Persistence boundary

The default backend is deliberately safe for the reference target: it validates a magic value, version, length, and CRC-32, and it survives communication resets. It is RAM-backed and therefore does **not** claim power-loss persistence. A production board must override `CANopenReferenceStorage_BoardStore()` and `CANopenReferenceStorage_BoardRestore()` with a wear-managed Flash, EEPROM, or FRAM implementation and must reserve that storage region outside the linker-managed application image.

## Validation

The source-contract suite checks the clock tree, exact CAN timing, timer cadence, NVIC priorities, storage ordering, watchdog opt-in behavior, bitrate-table coverage, and filter construction. Host protocol regressions and ARM cross-builds remain required release gates; physical HIL acceptance remains board-dependent and is not substituted by the source-contract tests.
