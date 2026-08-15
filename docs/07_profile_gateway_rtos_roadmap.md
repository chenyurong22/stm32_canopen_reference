# Profile, Gateway, FreeRTOS, and Product-Completion Roadmap

**Status:** The CANopen transport, Object Dictionary workflow, profile seams, gateway adapter, host protocol suite, and target build are implemented. The product-specific items in this document are intentionally not represented as completed conformance claims.

## 1. Current implementation status

| Capability | Delivered implementation | Product-release work remaining |
|---|---|---|
| CiA 401 | The 1 ms `CiA401Reference_Process()` bridge samples hardware inputs and applies OD-controlled outputs only through fail-safe hardware hooks. | Define channels, polarity, debouncing/filtering, diagnostics, analog scaling, PDO maps, EDS/XDD semantics, and test every permitted I/O configuration against the purchased profile specification. |
| CiA 402 | The reference implements a bounded controlword/statusword state-machine seam with drive-enable interlocks, safe fault behavior, and mode-display handling. | Select and implement the required modes of operation, feedback scaling, trajectory/control-loop ownership, quick-stop behavior, fault-reaction timing, limits, watchdogs, and profile-conformance tests. |
| CiA 309-3 gateway | `canopen_reference_gateway.c` supplies an optional ASCII gateway adapter. It registers the CANopenNode response callback, accepts command bytes from a board worker, and passes a board authorization decision into the native `CO_process()` gateway enable flag. | Bind a bounded UART/USB/network worker, define authentication/physical-service policy, audit command permissions, perform protocol and security testing, and enable `CANOPEN_REFERENCE_ENABLE_GATEWAY=1` only in a designated gateway product personality. |
| CiA 304 | Not enabled. | Create a dedicated safety concept, safety OD, timing analysis, diagnostics, lifecycle evidence, and independent validation before considering GFC/SRDO. |
| CiA 405 | Not enabled. | Define the interface profile, mandatory OD, service behavior, physical interfaces, and test strategy before adding code. |
| FreeRTOS | The current implementation is a deterministic bare-metal loop with a 1 ms timer path. | Port only after task priorities, ISR safety, mutex/critical-section rules, stack budgets, watchdog behavior, and jitter limits are reviewed on the target board. |

## 2. CiA 401 completion plan

The present `cia401_reference.c` is a safe process-data bridge, not a complete I/O product. A product port must create a channel table that identifies every physical signal, whether it is input/output/analog/specialized, its electrical safety state, conversion function, diagnostic behavior, and associated OD/PDO mapping. The project must regenerate `Generated/OD.c`, `Generated/OD.h`, and the EDS together through the guarded import or generation workflow after profile decisions change.

| Work package | Acceptance evidence |
|---|---|
| Channel definitions | Reviewed channel table and generated OD/EDS with device identity/version updates. |
| Electrical behavior | HIL records for output reset behavior, under/over-voltage, open/short diagnostics where applicable, input filtering, and all enabled load conditions. |
| CAN behavior | Independent manager verifies PDO timing, event behavior, mapping restrictions, SDO read/write permissions, reset handling, and error reporting. |
| Conformance | Profile-spec checklist and test report under the organization’s licensed CiA 401 specification and applicable test tooling. |

## 3. CiA 402 completion plan

The CiA 402 source is intentionally a **reference state-machine seam**. It may be used to integrate an existing motor-control or power-stage subsystem, but it is not a certified motion-control implementation. The application owner must decide whether the product needs profile position, velocity, torque, homing, interpolated position, cyclic synchronous position/velocity/torque, or another permitted subset before completing the OD and PDO design.

| Area | Required product decision and evidence |
|---|---|
| State behavior | Trace every controlword transition, statusword value, fault reaction, reset, quick stop, and safe torque/power command on the physical power stage. |
| Modes | Implement only selected modes, report unavailable modes consistently, and test mode switching with the actual controller and feedback chain. |
| Units and limits | Specify encoder resolution, gear ratio, current/torque scaling, velocity/acceleration/jerk units, limit sources, and saturation behavior in the EDS/XDD and integration manual. |
| Timing | Budget PDO, control-loop, interrupt, watchdog, and fieldbus latency; verify worst-case jitter with a loaded CAN bus and target compiler optimization. |
| Fault handling | Make hardware safety signals authoritative; demonstrate behavior for feedback loss, bus loss, over-current/over-temperature signals, reset, and controller faults. |

## 4. Optional CiA 309-3 gateway personality

CANopenNode documents its ASCII gateway as an application-bound input/output stream: the application supplies output through `CO_GTWA_initRead()`, feeds newline-terminated commands with `CO_GTWA_write()`, and the stack processes the gateway cyclically.[1] The implementation in this project follows that model.

The default setting remains `CANOPEN_REFERENCE_ENABLE_GATEWAY=0`. When set to `1`, the build enables the upstream ASCII, SDO-client, NMT, and descriptive-error portions of the gateway. `CANopenReferenceGateway_Authorized()` remains `false` until a board port overrides it. This prevents an accidentally compiled gateway from accepting commands. LSS-master gateway commands are not enabled in this personality; provisioning is handled separately through the existing LSS-slave workflow.

> The gateway should never be treated as an authentication mechanism. The product must define who may access it, how access is enabled and revoked, how input is bounded, how audit records are retained, and what SDO/NMT functions are permitted.

The upstream documentation explicitly supports application-owned serial, stdio, or socket streams and a callback-based output path.[1] The project limits the adapter to a mainline worker interface; do not call `CANopenReferenceGateway_WriteCommand()` from a UART or CAN interrupt.

## 5. FreeRTOS migration design

CANopenNode supports hardware-specific integrations outside the base stack, and its device-support documentation lists prior RTOS-oriented integrations while making clear that device drivers remain project-owned.[2] For this STM32F767 reference, migration should preserve the stack’s execution ownership rather than introduce parallel protocol processing.

| Execution context | Bare-metal reference | Proposed FreeRTOS mapping |
|---|---|---|
| CAN RX/TX and bus status ISR | bxCAN driver callbacks; short preprocessing only. | Keep ISR work bounded; use direct task notification only where necessary. No SDO, gateway, diagnostics, or application blocking work in ISR context. |
| Real-time 1 ms path | TIM7 scheduling and profile process boundary. | High-priority periodic timer/notification to a CANopen real-time task. Measure wake-up jitter and preserve the single owner of PDO/profile timing. |
| CANopen main process | Main loop calls `CO_process()` and application hooks. | One CANopen service task owns `CO_process()`, reset handling, LEDs, gateway processing, and non-real-time profile work. |
| UART gateway and diagnostics | Disabled, optional mainline hooks. | Separate bounded DMA/worker task. Transfer command bytes into the gateway only outside ISR context; never hold a stack lock while waiting for output. |
| Product I/O/motion | Board-specific hooks. | Dedicated application task(s) with explicit data ownership and atomic snapshot/command handoff at the CANopen profile boundary. |

A FreeRTOS port is ready to implement only after completing a target-specific priority map, worst-case execution/jitter measurement plan, interrupt-priority policy compatible with the selected FreeRTOS port, static stack budgets, reset/watchdog strategy, and a concurrency review of each CANopenNode callback and application hook. It remains a future product change, not a default build option.

## 6. HIL and interoperability exit criteria

The full hardware procedure is in [`06_board_integration_and_hil.md`](06_board_integration_and_hil.md). The release decision must additionally include independent CANopen manager traces for all enabled profiles, a regression log for valid and invalid SDO/PDO/LSS cases, gateway authorization/permission tests where enabled, and evidence that product hardware remains safe when the CAN controller, bus, feedback, or external manager fails.

## References

[1]: https://canopennode.github.io/CANopenNode/group__CO__CANopen__309__3.html "CANopenNode Gateway ASCII mapping"

[2]: https://canopennode.github.io/CANopenNode/md_doc_2deviceSupport.html "CANopenNode device support"
