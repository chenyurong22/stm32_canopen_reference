# Architecture Decision Record 001 — CANopenNode Native Core with a Stable Project Facade

**Status:** Accepted  
**Date:** 2026-08-13  
**Scope:** STM32F767 CANopen device/reference firmware

## Decision

The project will retain **CANopenNode** as the protocol implementation and will continue to use its native lifecycle internally: `CO_new()`, `CO_CANinit()`, `CO_LSSinit()`, `CO_CANopenInit()`, `CO_CANopenInitPDO()`, `CO_process()`, `CO_process_SYNC()`, `CO_process_RPDO()`, and `CO_process_TPDO()`.[1]

A project-owned facade will expose the stable, stack-neutral operations required by application and integration code. It will not expose or emulate the unrelated legacy `CO_Data` data model. The facade presents initialization, non-real-time process, fixed-cycle process, CAN receive, safe-state, and diagnostic operations. The existing CANopenNode runtime remains the sole owner of protocol objects, Object Dictionary access, and driver-to-stack frame dispatch.

> The `CO_Data`/`CO_init()` signature cited in the supplied requirements represents a different CANopen stack API family. Combining it with CANopenNode’s native object model would create duplicate ownership of node state, OD access, and CAN reception. The facade is therefore an **adapter**, not a second stack implementation.

## Rationale

The existing firmware already has a successful Cortex-M7 build using the current CANopenNode STM32 binding. It has a generated-style OD, initial NMT/heartbeat/SDO/PDO/LSS setup, and a defined split between TIM7 real-time work and mainline processing. Replacing that model with a separately invented CANopen core would duplicate protocol responsibility and invalidate the previous integration work.

The facade preserves a straightforward application contract and supports a host transport without exposing CANopenNode internals to application modules. It also makes the project’s target-specific bxCAN binding replaceable with a SocketCAN adapter for integration testing.

## Interface contract

| Facade concern | Contract |
|---|---|
| Initialization | Configure identity/node parameters and initialize exactly one CANopenNode instance. |
| Mainline service | Execute CANopenNode non-real-time processing; it owns NMT, heartbeat, SDO, reset commands, and supervisory services. |
| Fixed-cycle service | Execute SYNC/RPDO/profile/TPDO work with a measured target-period argument. |
| CAN reception | Deliver validated classic-CAN frames only to the port/driver that owns the CANopenNode receive path. |
| Hardware safety | Force board-owned outputs and drives to safe state before initialization, reset, transport loss, or error escalation. |
| Diagnostics | Publish bounded counters/status only; logging must be optional and forbidden in interrupt context. |

## Consequences

The canonical STM32 code remains under `App/` and `Core/`. The repository will add a `middleware/canopen/` compatibility and host-test layer, but it does not reimplement CANopenNode protocol services. Generated OD sources remain compiled through the top-level CMake target. The exact `middleware/canopen/core`, `port`, `od`, and `examples` paths are provided as integration surfaces and documentation locations.

The next implementation step must add a testable `can_port` API and a SocketCAN transport without changing the generated OD contract or creating a parallel SDO/NMT implementation.

## Non-goals

This decision does not claim CiA 401, CiA 402, CiA 405, CiA 304, IEC 61508, ISO 13849, SIL, or PL conformity. It does not make a CANopen gateway, safety controller, motion-control loop, physical I/O board driver, or FreeRTOS port complete. Each remains a separately scoped product capability.

## References

[1]: https://canopennode.github.io/CANopenNode/ "CANopenNode documentation"
