# CiA 401 and CiA 402 Reference Modules

**Status:** Tested application reference; incomplete device-profile implementation

The repository contains two selectable profile bindings. The default build selects the CiA 401 I/O reference. The CiA 402 code is compiled as a reference state machine but is disabled by default. To prevent accidental product ambiguity, the configuration rejects a combined build unless it is explicitly authorized. This is a configuration policy, not a substitute for an EDS/XDD, device type, or conformance strategy.

The CiA 401 family covers device profiles for generic I/O modules, while CiA 402 defines standardized behavior for drives and motion-control devices.[1] [2] Both standard families require a product-specific selection of mandatory, optional, and manufacturer-specific objects; the small set here is intentionally a testable implementation seam rather than a claim of full profile coverage.

## CiA 401 reference binding

`App/Src/cia401_reference.c` samples hardware inputs and applies CANopen-visible output commands once each millisecond. Process data resides in the generated Object Dictionary, so it can be accessed through SDO and mapped by the configured PDO mechanism. The implementation makes no assumption about voltage range, signal polarity, analogue scaling, output driver type, or diagnostic semantics.

| OD object | Reference direction | Hardware hook | Required product completion |
|---|---|---|---|
| `0x6000:01` | Input | `CANopenReferenceHw_ReadDigitalInputs()` | Define channel count, bit polarity, debounce, diagnostics, and safe indication. |
| `0x6200:01` | Output | `CANopenReferenceHw_WriteDigitalOutputs()` | Define output type, interlocks, initialization, fault response, and ownership rules. |
| `0x6401:01`, `0x6411:01` | Input | `CANopenReferenceHw_ReadAnalogInput()` | Define units, range, filtering, calibration, quality/validity, and update behavior. |
| `0x6422:01` | Output | `CANopenReferenceHw_WriteAnalogOutput()` | Define scale, limits, saturation, slew rate, diagnostics, and safe value. |

On initialization and after an explicit safe-output request, the module writes zero to the reference output objects and calls the physical output hooks with zero. A product must independently ensure that zero is actually the required safe signal; it may instead require a separate relay, enable line, de-energized state, or hardware safety function.

| CiA 401 engineering policy | Reference status | Product decision required |
|---|---|---|
| Sampling/update cadence | One application service call per 1 ms TIM7 cycle | Measure worst-case interrupt latency and define channel-specific deadlines. |
| Digital debounce | Not selected by the reference | Define per-channel debounce and event-loss behavior. |
| Analog scaling/filtering | Raw board hook values are bridged | Define units, calibration, filtering, saturation, and invalid-value handling. |
| Output interlock/fault behavior | Weak hooks remain safe and de-energized | Define independent enable, relay, current limit, and diagnostic reaction. |
| PDO examples | Generated OD and mapping seam are provided | Approve the product PDO map and update EDS/XDD. |

## CiA 402 reference binding

`App/Src/cia402_reference.c` contains a bounded controlword/statusword transition reference for the common states **switch-on disabled**, **ready to switch on**, **switched on**, **operation enabled**, **quick stop active**, and **fault**. It transfers feedback to the OD and only calls the power-stage command hook when both the CiA 402 state is operation enabled and the board-reported interlock is healthy. CiA 402 standardizes communication behavior and parameterization for drives; the physical torque, velocity, position, braking, and safety behavior remains product-specific.[2]

| Area | Included reference behavior | Explicitly not implemented |
|---|---|---|
| Status/control | Basic `0x6040` controlword interpretation and `0x6041` statusword reporting | Complete transition-table qualifiers, warnings, manufacturer-specific state semantics, and complete EMCY behavior. |
| Modes | Recognizes common profile and cyclic modes, mirrors a supported requested mode to `0x6061` | Mode-specific trajectory, homing, torque/current, velocity, position, interpolation, limit, and unit implementation. |
| Targets/feedback | Bridges `0x607A`, `0x60FF`, `0x6071`, `0x6064`, `0x606C`, and `0x6077` through hardware hooks | Servo loop, time synchronization analysis, sensor validation, following-error supervision, and deterministic motion control. |
| Fault handling | Forces software drive disable on reported fault or unhealthy interlocks | Safety-rated fault reaction, STO, brake control, watchdog coverage, fault history, and recovery authorization. |

### CiA 402 mode status

| Mode/capability | Reference status |
|---|---|
| Profile Position | Reference target bridge only; no product trajectory planner |
| Profile Velocity | Reference target bridge only; no product velocity loop |
| Profile Torque | Not implemented |
| Homing | Not implemented |
| Cyclic Synchronous Position | Not implemented |
| Cyclic Synchronous Velocity | Not implemented |
| Cyclic Synchronous Torque | Not implemented |
| Feedback and power stage | Board-specific hooks |

> **Do not use this module as the only drive safety function.** The default weak hardware hooks report unhealthy interlocks and refuse to enable a drive. A real integration must provide an independent hardware path that moves the actuator to its required safe state even if the MCU, CAN controller, software task, or network is faulty.

## Test coverage

`tests/test_profiles.c` uses test-only OD and hardware adapters to execute the profile code on a host compiler. The included tests verify safe I/O initialization, input/output bridging, CiA 402 fault handling when interlocks are unhealthy, fault reset, the normal reference transition sequence, command forwarding, feedback update, and disabling after an interlock loss. This validates the tested application logic only; it does not prove timing, HAL interrupts, bus behavior, field wiring, motion dynamics, or conformance.

## References

[1]: https://www.can-cia.org/can-knowledge/cia-401-series-canopen-device-profile-for-generic-i-o-modules "CAN in Automation: CiA 401 series"
[2]: https://www.can-cia.org/can-knowledge/cia-402-series-canopen-device-profile-for-drives-and-motion-control "CAN in Automation: CiA 402 series"
