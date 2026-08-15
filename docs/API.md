# Project API Reference

This document covers project-owned interfaces. CANopenNode and STM32 HAL APIs remain documented by their upstream packages and are not duplicated here.

## Runtime ownership

`App/Src/CO_app_STM32_reference.c` owns the project lifecycle. It prepares the Object Dictionary, initializes the CANopenNode stack, starts the board transport, runs the 1 ms interrupt path, and calls the bounded mainline processing functions. The project wrapper must be linked instead of the upstream example wrapper.

| Interface | Responsibility | Calling context |
|---|---|---|
| `CO_app_STM32_init` / reset lifecycle | Initialize and reset the reference runtime | Startup or controlled reset path |
| `canopen_app_interrupt` | Service time-critical CAN/timer work | Interrupt callback; bounded only |
| `canopen_app_process` | Run CANopenNode and project mainline processing | Main loop |
| `canopen_reference_board_*` | Board-specific transceiver and safe-output hooks | Board integration layer |
| `CANopenReferenceCia302_*` | Optional CiA 302 master adapter and snapshot | Mainline; opt-in personality only |
| `CANopenReferenceDiagnostics_*` | Bounded status publication | Mainline or diagnostic service |

## CiA 302 master adapter

The adapter is disabled by default and is enabled with `CANOPEN_REFERENCE_ENABLE_CIA302_MASTER=ON`. It uses the existing CANopenNode heartbeat-consumer and NMT transmit facilities rather than taking ownership of the bxCAN driver.

The lifecycle is:

```c
CANopenReferenceCia302_PrepareOd();
/* create and initialize CANopenNode */
CANopenReferenceCia302_Init(CO, master_node_id, now_ms);

for (;;) {
    CANopenReferenceCia302_PreProcess(now_ms);
    CO_process(CO, false, &sleep_ms);
    CANopenReferenceCia302_Process(now_ms);
}

CANopenReferenceCia302_Deinit();
```

`CANopenReferenceCia302_PreProcess()` must run before `CO_process()` so the adapter can observe new heartbeat frames before the stack clears its receive flags. All adapter callbacks and diagnostic state are bounded and non-blocking.

`CANopenReferenceCia302_GetSnapshot()` returns a copy of the current status. The snapshot includes enable/running state, network readiness, monitored node state, event counters, and the last event. A product diagnostic transport may expose this data through a controlled Object Dictionary entry or service interface; the reference does not define an authenticated field diagnostic protocol.

## Configuration switches

| CMake option or macro | Default | Effect |
|---|---:|---|
| `CANOPEN_REFERENCE_ENABLE_CIA401` | On | Enables the default I/O personality |
| `CANOPEN_REFERENCE_ENABLE_CIA402` | Off | Enables the CiA 402 reference adapter |
| `CANOPEN_REFERENCE_ENABLE_CIA302_MASTER` | Off | Enables the bounded NMT-master adapter |
| `CANOPEN_REFERENCE_ENABLE_GATEWAY` | Off | Enables the bounded gateway foundation |
| `CANOPEN_REFERENCE_ENABLE_CIA418` | Separate | Builds the battery-profile artifacts outside the default OD |

Build each personality in a clean directory. Do not combine mutually exclusive profile Object Dictionaries without reviewing generated index and memory requirements.

## CAN-port boundary

The project transport boundary keeps hardware-specific CAN operations in `middleware/canopen/port/` and the application-owned CANopenNode binding in `App/` and `third_party/CanOpenSTM32/`. A transport implementation must report queue or send failure explicitly, preserve the configured standard CAN identifier and DLC, and avoid blocking in interrupt context.

## Generate API documentation

Install Doxygen on the development host and run the following from the repository root:

```sh
doxygen Doxyfile
```

The generated HTML is written under `build/doxygen/html/`. The configuration intentionally scans only project-owned public headers and excludes vendor, generated, and build sources.

## Error and safety expectations

Public project APIs use explicit Boolean or status returns where an operation can fail. Callers must handle CAN queue-full, invalid node-ID, invalid DLC, unavailable board hooks, and reset conditions. A successful NMT transition is not a safety authorization: power-stage enable, emergency shutdown, watchdog response, and interlock behavior remain board and product responsibilities.
