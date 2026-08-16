# STM32F767 CANopen Execution Model and Timing Analysis

## Executive conclusion

The checked-in STM32F767 CANopen reference firmware is **HAL bare-metal firmware with a superloop and interrupt-driven real-time paths**. It is **not FreeRTOS-based**, does not create RTOS tasks, does not start a scheduler, and does not use `taskENTER_CRITICAL()` or RTOS mutexes.

The execution model has four relevant contexts:

1. The **main superloop**, which runs `canopen_app_process()` and watchdog servicing.
2. The **TIM7 1 ms interrupt**, which runs CANopen SYNC/RPDO processing, CiA 401/CiA 402 application processing, CiA 418 model synchronization when enabled, and TPDO processing.
3. The **bxCAN interrupt callbacks**, which receive CAN frames, software-match CANopen RX buffers, and invoke CANopenNode receive callbacks directly.
4. The **SysTick/HAL time base**, which provides the millisecond tick consumed by the mainline timing logic.

The implementation uses **short global-interrupt critical sections** for CANopenNode shared data, not FreeRTOS critical sections. The important production timing risk is therefore not a missing FreeRTOS task lock; it is the need to preserve bounded interrupt execution and prevent synchronous Flash operations or optional formatted diagnostics from being invoked in real-time contexts.

## Evidence summary

| Evidence | Repository location | Finding |
|---|---|---|
| `#define USE_RTOS 0U` | `Core/Inc/stm32f7xx_hal_conf.h:27-28` | HAL is configured without an RTOS time base. |
| No scheduler/task symbols | Repository-wide source inventory | No FreeRTOS headers, task creation, scheduler startup, CMSIS-RTOS layer, or task notifications are used by the target firmware. |
| Main processing | `App/Src/CO_app_STM32_reference.c:340-379` | `canopen_app_process()` is called from the superloop and advances CANopenNode once per changed HAL millisecond tick. |
| 1 ms processing | `App/Src/CO_app_STM32_reference.c:381-410` | TIM7 invokes SYNC, RPDO, application, CiA 418 synchronization, and TPDO work in interrupt context. |
| TIM7 cadence | `Core/Src/main.c:14-19, 88-96` | 108 MHz TIM7 input divided by 108000 produces a statically checked 1 kHz / 1 ms interrupt. |
| Interrupt priorities | `Core/Src/stm32f7xx_hal_msp.c:31-38, 54-60` | CAN IRQs use priority 5; TIM7 uses priority 6; SysTick is configured at priority 0 by `TICK_INT_PRIORITY`. Lower numerical priority is more urgent on Cortex-M. |
| OD atomicity | `third_party/CanOpenSTM32/CANopenNode_STM32/CO_driver_target.h:139-162` | CAN send, EMCY, and OD locks save PRIMASK, disable interrupts, and restore PRIMASK. |
| CAN RX context | `third_party/CanOpenSTM32/CANopenNode_STM32/CO_driver_STM32.c:556-590` | bxCAN FIFO callbacks read a frame, scan the RX table, and call the registered CANopen callback directly in interrupt context. |
| Flash persistence | `App/Src/canopen_reference_storage.c:113-157, 225-246` | OD store callbacks synchronously erase and program internal Flash; they are not deferred to a worker task. |
| Watchdog liveness | `App/Src/canopen_reference_watchdog.c:59-84` | The 1 ms ISR increments progress; the mainline refreshes the IWDG only after observing ISR progress. |

## Execution architecture

### Main superloop

`canopen_app_process()` obtains `HAL_GetTick()`, processes CAN recovery, and returns immediately if the millisecond tick has not advanced. When it does advance, it computes:

```text
elapsedUs = (current_tick - previous_tick) * 1000
```

It then performs CiA 302 pre-processing, calls `CO_process()`, performs CiA 302 post-processing, updates status LEDs and diagnostics, and handles communication/application reset requests.

This is a **cooperative mainline**. There is no scheduler time slice and no deterministic maximum loop frequency independent of workload. Under normal conditions the CANopen mainline is called once per HAL millisecond. If the loop is delayed, the next call receives the accumulated elapsed time rather than executing one catch-up iteration per missed millisecond.

### TIM7 real-time path

TIM7 is configured for a 1 kHz interrupt. The interrupt path first checks CANopen readiness, locks the CANopen OD critical section, then executes:

```text
CO_process_SYNC(CO, 1000 us, ...)
CO_process_RPDO(CO, syncWas, 1000 us, ...)
Cia401Reference_Process1ms()
Cia402Reference_Process1ms()
CO_process_TPDO(CO, syncWas, 1000 us, ...)
```

The application comment explicitly prohibits blocking drivers, Flash operations, and `printf` from this context. This is the correct architectural boundary for a 1 ms real-time path.

The critical timing requirement is:

```text
WCET(TIM7 ISR including lock) < 1.000 ms
```

A production target should reserve margin for higher-priority CAN and SysTick interrupts. The repository provides the cadence configuration and static checks, but it does not contain silicon-based cycle or jitter measurements.

### bxCAN interrupt path

The STM32 CANopenNode driver enables CAN RX FIFO, TX mailbox, and error notifications. For a received frame, the driver reads the FIFO entry, software-matches the standard identifier against the configured CANopen RX buffer table, and calls the registered callback directly.

The receive search is linear in the configured RX table size:

```text
O(number of configured CANopen RX buffers)
```

The hardware global filter accepts standard frames broadly and the driver performs software matching. Consequently, a hostile or noisy bus can increase interrupt CPU load even when frames do not belong to this node. The HIL campaign should measure RX interrupt occupancy and FIFO overflow under maximum expected and abnormal bus load.

### SysTick

The HAL configuration uses `TICK_INT_PRIORITY 0U` and `USE_RTOS 0U`. SysTick is therefore the HAL millisecond time base, not an RTOS scheduler tick. The mainline uses this tick for elapsed-time calculation, recovery delays, watchdog grace timing, diagnostics rate limiting, and storage-store throttling.

## Atomicity and critical sections

The recommendation to use `taskENTER_CRITICAL()` is **not applicable to the current firmware architecture**, because there are no FreeRTOS tasks.

The project’s CANopenNode STM32 target header implements the relevant protection as follows:

```c
primask = __get_PRIMASK();
__disable_irq();
/* protected CAN/OD operation */
__set_PRIMASK(primask);
```

This protects CAN-send buffers and OD accesses against interrupt concurrency. In particular, the 1 ms path wraps PDO and application OD work with `CO_LOCK_OD()` / `CO_UNLOCK_OD()`, while CANopenNode mainline SDO/OD paths use the same lock contract.

This is sufficient as an **interrupt-versus-mainline protection mechanism** provided that:

- the protected region remains short and bounded;
- no blocking operation is added inside the OD lock;
- application code does not access PDO-mappable OD variables outside the lock;
- all ISR-shared application state uses appropriate volatile/atomic or lock-protected access;
- the target remains single-core Cortex-M execution without an RTOS task context.

The current design does not provide a FreeRTOS-style priority-aware mutex or scheduler suspension. Porting the application to FreeRTOS would require replacing or adapting these macros and re-evaluating ISR-safe APIs, priority inversion, and task/ISR ownership.

## Timing constants and theoretical bus timing

| Item | Configured or derived value | Interpretation |
|---|---:|---|
| CPU clock | 216 MHz | Reference PLL/system-clock configuration. |
| APB1 clock | 54 MHz | CAN1 peripheral bus clock domain. |
| TIM7 timer input | 108 MHz | APB timer doubling applies because APB prescaler is not 1. |
| TIM7 prescaler | 108000 divisor | Produces 1 kHz with period 1 tick. |
| TIM7 period | 1 ms | CANopen SYNC/RPDO/application/TPDO service cadence. |
| Default CAN bitrate | 500 kbit/s | Nominal bxCAN bitrate. |
| Default heartbeat | 1000 ms | CANopen heartbeat setting in project configuration. |
| Bus-off recovery wait | 100 ms | Mainline recovery delay before an attempt. |
| Bus-off maximum attempts | 3 | Bounded recovery before safe fault. |
| Default IWDG timeout | 200 ms | Opt-in; requires board validation of LSI and reset behavior. |
| IWDG startup grace | 100 ms | Opt-in startup grace, constrained to be shorter than timeout. |
| Storage minimum interval | Configuration-dependent | Store throttling is checked in mainline/OD callback context. |

At 500 kbit/s, an ordinary standard CAN data frame containing 8 data bytes occupies approximately 222 microseconds before considering worst-case bit stuffing and inter-frame details. A conservative engineering estimate is higher, approximately 250–270 microseconds for a heavily stuffed frame. This is a **wire-time estimate, not a measured firmware latency**. Acceptance must be based on captured bus traces and CPU/IRQ instrumentation at the product’s maximum frame rate.

## Blocking and latency risks

### Flash storage

The OD store callback calls `HAL_FLASHEx_Erase()` and then programs the image synchronously. This path is mainline/OD-callback work, not a task or background worker. It must not be called from the TIM7 ISR or a CAN RX callback.

During erase/program, the mainline can be delayed substantially relative to ordinary processing. The design’s CANopen timing impact depends on STM32F7 Flash execution behavior, interrupt masking inside the HAL Flash driver, CAN FIFO depth, and bus load. These values are not safely inferable from a host build. The required hardware campaign is power-loss, interrupted-write, endurance, and maximum observed store latency testing.

### Optional UART diagnostics

Diagnostics are disabled by default. If enabled, formatted output is rate-limited to approximately once per 1000 ms and runs from mainline, but the board override must be bounded and non-blocking. A blocking UART writer would violate the intended superloop timing model and could starve CANopen mainline/watchdog servicing.

### Reset and bus recovery

Communication reset stops the timer, changes CAN configuration, deletes the CANopen object, and reinitializes the stack. Application reset invokes safe application handling and then a system reset. These are exceptional paths, but their duration must be excluded from normal 1 ms WCET claims and measured separately during HIL.

Bus-off recovery is driven from mainline time checks. The configured wait is 100 ms and the retry bound is 3 attempts. This is a bounded recovery policy, not a 1 ms response guarantee.

## Watchdog timing interpretation

The watchdog is optional and disabled by default in the central configuration. When enabled:

- the TIM7 ISR increments an atomic progress counter every 1 ms;
- the mainline compares that counter against its last observed value;
- the IWDG refresh is allowed only after the startup grace period and only when ISR progress has been observed;
- a stalled mainline stops refresh even if the timer ISR is still running;
- a stalled timer interrupt path prevents the mainline from observing new progress and also stops refresh.

This is a sound dual-context liveness design for bare-metal firmware. It does not prove a 200 ms silicon watchdog margin; the LSI frequency, HAL initialization time, interrupt starvation, reset cause, and recovery path require board measurements.

## Bare-metal versus FreeRTOS conclusion

| Question | Answer |
|---|---|
| Is FreeRTOS linked or started? | **No evidence; target is configured `USE_RTOS 0U`.** |
| Are tasks created? | **No.** |
| Is there a scheduler? | **No.** |
| Is there a cooperative main loop? | **Yes.** |
| Is CAN reception interrupt-driven? | **Yes.** |
| Is 1 ms CANopen processing interrupt-driven? | **Yes, via TIM7.** |
| Are PDO/OD accesses protected? | **Yes, with PRIMASK-based interrupt masking in CANopenNode target macros.** |
| Is `taskENTER_CRITICAL()` required? | **No for the current architecture; it would only be relevant after an RTOS port.** |
| Is timing fully characterized? | **No. Cadence is statically configured, but WCET, jitter, FIFO overflow, Flash latency, and physical bus-load margins require hardware measurement.** |

## Recommended timing-instrumentation campaign

The next timing work should be measurement rather than another scheduler abstraction. Add a hardware-validation build or board-specific instrumentation that records:

1. TIM7 ISR entry/exit cycle count using DWT `CYCCNT` or a GPIO pulse.
2. Maximum, minimum, and percentile TIM7 ISR duration over a long CAN load campaign.
3. TIM7 period jitter and missed/late interrupt count.
4. CAN RX callback latency from FIFO arrival to callback completion.
5. CAN FIFO occupancy and overflow counters under maximum legitimate and invalid traffic.
6. Mainline loop period, `CO_process()` duration, and longest reset/recovery path.
7. Flash erase/program duration and the longest interval during which CANopen service is delayed.
8. Watchdog refresh interval, observed LSI-derived timeout, reset cause, and post-reset recovery duration.
9. PDO data consistency under concurrent SDO writes and 1 ms PDO processing.

The resulting evidence should record firmware SHA, toolchain, board revision, transceiver, bitrate/sample point, bus load, instrument configuration, temperature, supply voltage, and pass/fail criteria approved by the product owner.

## Final assessment

The current project is a **bare-metal, interrupt-driven CANopen reference implementation**, not a FreeRTOS task-based controller. The supplied recommendation about FreeRTOS critical sections should therefore be translated to this project as: **retain and verify the existing PRIMASK-based CANopenNode critical sections, keep PDO/application work bounded within the 1 ms ISR, and measure worst-case timing on hardware**.

The repository provides a credible software timing architecture and explicit external-validation boundaries. It does not yet provide physical WCET, jitter, CAN-load, Flash-latency, watchdog, or EMC evidence, so those must remain open hardware/product qualification gates.

## Sources

- `Core/Inc/stm32f7xx_hal_conf.h`
- `Core/Src/main.c`
- `Core/Src/stm32f7xx_hal_msp.c`
- `App/Src/CO_app_STM32_reference.c`
- `App/Src/canopen_reference_storage.c`
- `App/Src/canopen_reference_watchdog.c`
- `App/Inc/canopen_reference_config.h`
- `third_party/CanOpenSTM32/CANopenNode_STM32/CO_driver_STM32.c`
- `third_party/CanOpenSTM32/CANopenNode_STM32/CO_driver_target.h`
- `third_party/CanOpenSTM32/CANopenNode/301/CO_driver.h`

## Implemented opt-in measurement path

The repository now contains a default-off DWT cycle-counter measurement path controlled by `CANOPEN_REFERENCE_ENABLE_TIMING_INSTRUMENTATION`. When enabled in a board-qualification build, it records maximum TIM7 IRQ duration, TIM7 period spacing, TIM7 budget overruns, per-source bxCAN IRQ counts and maximum durations, and mainline sample counts and maximum duration. Instrumentation surrounds the complete generated TIM7 and bxCAN IRQ handlers and the application mainline entry/exit, so the measured samples include HAL dispatch and project-owned callbacks.

The production default remains disabled. Enabling this option provides measurement data, not automatic conformance or a hardware PASS result. A board campaign must export `CANopenReferenceTiming_GetStats()`, correlate the counter values with firmware SHA and clock configuration, and record bus load, temperature, voltage, transceiver, and instrument setup. DWT cycle counts wrap at 32 bits; all elapsed calculations are unsigned wrap-safe, while aggregate counters saturate at `UINT32_MAX`.

Example qualification build option:

```text
-DCANOPEN_REFERENCE_ENABLE_TIMING_INSTRUMENTATION=ON
```

The enabled and disabled ARM production configurations have both been compile-tested. No universal ISR-duration claim is made until the instrumented image is executed on the target board.
