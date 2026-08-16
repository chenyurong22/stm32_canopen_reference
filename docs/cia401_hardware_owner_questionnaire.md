# CiA 401 Hardware-Owner Questionnaire

## Purpose

This questionnaire is the completion record for the unresolved production-freeze inputs in [`PRODUCT_CIA401.md`](../PRODUCT_CIA401.md). It is intended for the hardware owner, with product, firmware, safety, manufacturing, and test owners contributing where indicated.

The selected v1 personality is a **CiA 401 I/O device**. The answers must describe the released product board, not a generic STM32 reference design. Do not infer values from the firmware defaults. If an item is not applicable, record `N/A` together with a technical rationale; do not leave a blank field. A `TBD` answer does not close a production-freeze gate.

> This questionnaire records product decisions. It does not replace electrical review, laboratory qualification, HIL testing, formal conformance testing, or safety approval.

## Completion rules

Each answer should identify the source of truth, such as a schematic revision, PCB revision, BOM line, MCU datasheet, calculation, laboratory report, or approved product requirement. Attach or link the evidence where possible. Values must be stated with units, tolerances, limits, polarity, and revision identifiers. Where the answer affects firmware, record the required firmware, Object Dictionary, EDS/XDD, configuration, or test change.

Use the following status values throughout the form:

| Status | Meaning |
|---|---|
| `OPEN` | No approved answer exists. |
| `PROPOSED` | A technical answer exists but requires owner approval. |
| `APPROVED` | The responsible owner has approved the answer and evidence is identified. |
| `N/A` | The item does not apply, with a written rationale. |

## 1. Record metadata

| Field | Response |
|---|---|
| Product name / product code |  |
| Product revision |  |
| Board assembly number and revision |  |
| Schematic revision |  |
| PCB revision |  |
| BOM revision |  |
| Hardware owner |  |
| Product owner |  |
| Firmware owner |  |
| Safety/reliability owner, if applicable |  |
| Manufacturing owner, if applicable |  |
| Questionnaire date |  |
| Target release or milestone |  |
| Overall questionnaire status (`OPEN` / `PROPOSED` / `APPROVED`) |  |
| Related change request, issue, or approval record |  |

## 2. Hardware definition

Complete every row. Attach the released schematic, PCB, BOM, datasheet excerpt, calculation, or review record that supports the answer.

| Item | Required production answer | Response | Evidence / source revision | Owner | Status |
|---|---|---|---|---|---|
| MCU ordering code | Exact STM32F767 ordering code, package, temperature grade, and revision, for example `STM32F767...` |  |  |  |  |
| MCU package | Package name, pin count, exposed pad requirements, and assembly constraints |  |  |  |  |
| MCU memory density | Exact Flash and SRAM density for the fitted part |  |  |  |  |
| Flash sector map | Sector addresses, sizes, erase constraints, and the two persistence-slot locations |  |  |  |  |
| Flash option bytes | Readout protection, write protection, watchdog-related options, boot configuration, and reset behavior |  |  |  |  |
| Bootloader reservation | Bootloader presence, start/end addresses, image size allowance, update mechanism, and whether the reference linker reservation remains valid |  |  |  |  |
| SRAM map | SRAM banks, addresses, sizes, ECC/parity behavior if applicable, and reserved regions |  |  |  |  |
| CAN transceiver | Manufacturer, exact part number, variant, supply voltage, common-mode range, standby behavior, and approved replacement policy |  |  |  |  |
| Transceiver control wiring | MCU pins and active polarity for enable, standby, silent, slope, or mode controls; define reset defaults |  |  |  |  |
| CAN protection | TVS/ESD device, common-mode choke, filtering, galvanic isolation if present, surge rating, and grounding path |  |  |  |  |
| CAN connector | Connector manufacturer and part number, mating part, keying, locking, pin numbering, service access, and field-replacement policy |  |  |  |  |
| CAN pinout | CANH, CANL, signal ground, shield, power pins if present, and unused-pin treatment |  |  |  |  |
| CAN MCU pins | Confirm `PA11/PA12` or the actual routed pins, alternate function, remap configuration, and board revision |  |  |  |  |
| CAN electrical limits | Common-mode limits, differential limits, dominant/recessive levels, maximum bus length, stub limits, and allowable node count |  |  |  |  |
| Oscillator | Exact HSE part, nominal frequency, tolerance, aging, temperature drift, load capacitance, startup time, and failure behavior |  |  |  |  |
| Clock safety margin | CAN bit-timing margin, oscillator tolerance budget, PLL configuration, clock-monitor policy, and acceptance limits |  |  |  |  |
| Supply rails | Rail names, nominal/minimum/maximum voltages, current limits, ramp rates, sequencing, and load-transient limits |  |  |  |  |
| Brownout/reset | BOR threshold, external reset supervisor if present, reset pulse width, reset release condition, and brownout recovery behavior |  |  |  |  |
| Termination | Fixed or switchable 120-ohm termination, split termination values, common-mode capacitor, switch/jumper control, and measurement points |  |  |  |  |
| Grounding/shielding | Signal-ground reference, chassis/shield connection, isolation boundary, bonding points, and allowed cable shield termination |  |  |  |  |
| Environment | Operating and storage temperature, humidity, vibration, shock, altitude, condensation, ingress rating, and chemical exposure limits |  |  |  |  |
| Board diagnostics | Power-good, transceiver fault, thermal, supply, connector, or other board-level diagnostic signals and their fault values |  |  |  |  |

### Hardware decisions requiring explicit narrative

| Question | Response |
|---|---|
| Which board revision is the release candidate, and how is it uniquely identified in manufacturing? |  |
| Which electrical values differ from the generic reference assumptions in `PRODUCT_CIA401.md`? |  |
| Which hardware failures force a reset, disable outputs, isolate CAN, or latch a service fault? |  |
| Are there approved component substitutions? If yes, what equivalence criteria and requalification are required? |  |
| What is the maximum permitted uncertainty for each safety- or timing-relevant electrical value? |  |

## 3. I/O definition

Complete one subsection for every implemented channel or channel bank. Add rows for additional channels; do not compress unlike channels into one answer if their electrical behavior differs.

### 3.1 Digital input bank — CANopen object `0x6000:01`

| Field | Response |
|---|---|
| Number of channels |  |
| Connector pins and signal names |  |
| Input electrical type |  |
| Nominal input voltage range |  |
| Guaranteed low/high thresholds |  |
| Absolute maximum and fault limits |  |
| Active polarity |  |
| Pull-up/pull-down and default state |  |
| Filtering and debounce time |  |
| Sampling period and worst-case reporting latency |  |
| Disconnected/open-circuit behavior |  |
| Short-to-ground behavior |  |
| Short-to-supply behavior |  |
| Overvoltage/ESD protection |  |
| Diagnostic coverage and diagnostic object(s) |  |
| Fault value exposed through the Object Dictionary |  |
| Safe-state interpretation |  |
| Calibration requirement |  |
| Evidence reference |  |

### 3.2 Digital output bank — CANopen object `0x6200:01`

| Field | Response |
|---|---|
| Number of channels |  |
| Connector pins and signal names |  |
| Output type |  |
| Nominal load voltage |  |
| Maximum continuous and peak current |  |
| Active polarity |  |
| Default state during reset/startup |  |
| Safe state during watchdog reset |  |
| Safe state during bus-off |  |
| Safe state during loss of communication |  |
| Short-circuit response and recovery |  |
| Open-load behavior |  |
| Overtemperature behavior |  |
| Output-enable interlock or external hardware override |  |
| Update period and worst-case latency |  |
| Fault value exposed through the Object Dictionary |  |
| Evidence reference |  |

### 3.3 Analog input 1 — CANopen object `0x6401:01`

| Field | Response |
|---|---|
| Sensor/input type |  |
| Connector pin and signal name |  |
| Input range and absolute maximum |  |
| Units |  |
| ADC resolution and reference |  |
| Scaling equation and engineering-unit range |  |
| Gain, offset, and calibration method |  |
| Calibration points and acceptance tolerance |  |
| Analog filtering and cutoff frequency |  |
| Sampling period and worst-case reporting latency |  |
| Saturation behavior |  |
| Disconnected/open-circuit behavior |  |
| Short/fault behavior |  |
| Invalid-value encoding and diagnostic indication |  |
| Safe-state interpretation |  |
| Evidence reference |  |

### 3.4 Analog input 2 — CANopen object `0x6411:01`

| Field | Response |
|---|---|
| Sensor/input type |  |
| Connector pin and signal name |  |
| Input range and absolute maximum |  |
| Units |  |
| ADC resolution and reference |  |
| Scaling equation and engineering-unit range |  |
| Gain, offset, and calibration method |  |
| Calibration points and acceptance tolerance |  |
| Analog filtering and cutoff frequency |  |
| Sampling period and worst-case reporting latency |  |
| Saturation behavior |  |
| Disconnected/open-circuit behavior |  |
| Short/fault behavior |  |
| Invalid-value encoding and diagnostic indication |  |
| Safe-state interpretation |  |
| Evidence reference |  |

### 3.5 Analog output 1 — CANopen object `0x6422:01`

| Field | Response |
|---|---|
| Required for this product? If not, provide rationale and mark `N/A` in the main document. |  |
| Connector pin and signal name |  |
| DAC, PWM, or other implementation |  |
| Output range and units |  |
| Load limits and output impedance |  |
| Scaling equation |  |
| Gain, offset, and calibration method |  |
| Calibration points and acceptance tolerance |  |
| Update period and worst-case latency |  |
| Default state during reset/startup |  |
| Safe state during watchdog, bus-off, brownout, and loss of communication |  |
| Open-load, short-circuit, and overtemperature behavior |  |
| Invalid-command behavior |  |
| Evidence reference |  |

## 4. CANopen product policy

These are product decisions, not merely software defaults. Record the approved value, permitted range, reaction, commissioning authority, and required test evidence.

| Policy | Required production decision | Response | Evidence / test reference | Owner | Status |
|---|---|---|---|---|---|
| Product profile | Confirm CiA 401 I/O device as the v1 personality and record any declared optional services |  |  |  |  |
| Node-ID | Allowed range, default value, persistence, source at boot, production programming method, duplicate-ID handling, and field-service override policy |  |  |  |  |
| Default bitrate | Default bitrate, allowed alternatives, tolerance, production programming method, and invalid-value behavior |  |  |  |  |
| Heartbeat | Producer period, consumer entries, timeout values, timeout reaction, startup behavior, and acceptance limits |  |  |  |  |
| EMCY | Product error-code list, inhibit/repetition policy, clear policy, reset behavior, service procedure, and traceability to diagnostics |  |  |  |  |
| SDO server | Timeout, abort codes, access policy, writable-object restrictions, block-transfer policy, and service-tool expectations |  |  |  |  |
| SDO client | Is SDO client part of the v1 claim? If yes, define peers, objects, timeouts, retries, and acceptance tests. If no, define disabled behavior. |  |  |  |  |
| PDO defaults | Approved RPDO/TPDO count, COB-IDs, mappings, transmission types, inhibit times, event timers, and product revision |  |  |  |  |
| TPDO transmission | Event, inhibit, synchronous, timer, and change-of-state behavior |  |  |  |  |
| RPDO behavior | Timeout, invalid-data, disable, stale-data, and safe-output reactions |  |  |  |  |
| SYNC | Producer/consumer role, period, jitter tolerance, counter behavior, and PDO synchronization policy |  |  |  |  |
| LSS | Enabled or disabled in production, authorized operations, node-ID/bitrate storage policy, service-tool authorization, and Fastscan boundary |  |  |  |  |
| NMT | Startup state, reset behavior, invalid-command behavior, boot-up timing, and safe-state policy |  |  |  |  |
| Persistence | Persistent parameters, authorized use of `0x1010`/`0x1011`, minimum store interval, commissioning behavior, interrupted-write policy, and both-slots-invalid behavior |  |  |  |  |

### Node-ID and commissioning record

| Question | Response |
|---|---|
| Who is authorized to assign or change the production node-ID? |  |
| What prevents duplicate node-IDs on a deployed network? |  |
| Is the node-ID field-programmable, factory-programmed, strap-selected, or fixed? |  |
| What is the recovery procedure for an invalid or corrupted node-ID? |  |
| Which parameters may be persisted, and what is the minimum permitted write interval? |  |
| Which production tool, fixture, or script performs commissioning? |  |
| What evidence proves the programmed identity and bitrate for each board? |  |

## 5. Safe-state and fault-reaction definition

The board-level design must be safe independently of weak firmware hooks. For each scenario, describe the electrical state of every controlled output and the required diagnostic/lifecycle response.

| Scenario | Required board state | Required firmware/CANopen state | Detection threshold or timeout | Recovery / latching policy | Evidence reference | Owner | Status |
|---|---|---|---|---|---|---|---|
| Power-on reset |  |  |  |  |  |  |  |
| MCU reset |  |  |  |  |  |  |  |
| Watchdog/IWDG reset |  |  |  |  |  |  |  |
| TIM7 or scheduler progress fault |  |  |  |  |  |  |  |
| CAN interrupt or receive-path fault |  |  |  |  |  |  |  |
| CAN bus-off |  |  |  |  |  |  |  |
| Repeated bus-off or exhausted recovery |  |  |  |  |  |  |  |
| Heartbeat loss / peer loss |  |  |  |  |  |  |  |
| NMT reset or stop command |  |  |  |  |  |  |  |
| Brownout or supply out of range |  |  |  |  |  |  |  |
| Flash write interruption |  |  |  |  |  |  |  |
| Both persistence slots invalid |  |  |  |  |  |  |  |
| Digital-output overload or short |  |  |  |  |  |  |  |
| Analog-input disconnected or over-range |  |  |  |  |  |  |  |
| Transceiver standby/fault indication |  |  |  |  |  |  |  |
| Loss of connector, shield, or CAN reference |  |  |  |  |  |  |  |

For each scenario, explicitly answer whether the safe state is **de-energized**, **held**, **last commanded**, or another defined state. If the hardware imposes a stricter state than firmware, state that precedence clearly.

## 6. Required evidence package

Mark each item with its document identifier, revision, and location. Evidence must be associated with the exact firmware SHA and board serial used for qualification.

| Evidence item | Required content | Document / artifact ID | Revision / hash | Status |
|---|---|---|---|---|
| Released schematic | CAN, supply, reset, I/O protection, termination, connector, and safety paths |  |  |  |
| Released PCB | Board revision, routing, layer stack, controlled impedance if applicable |  |  |  |
| Released BOM | MCU, transceiver, oscillator, protection, connectors, termination, substitutions |  |  |  |
| MCU documentation | Ordering code, memory map, sector map, option-byte policy |  |  |  |
| Electrical calculations | Supply, termination, CAN timing, protection, thermal, and load calculations |  |  |  |
| Component datasheets | Transceiver, oscillator, protection, output drivers, ADC/DAC circuitry |  |  |  |
| Manufacturing programming record | Node-ID, bitrate, firmware SHA, board serial, calibration data |  |  |  |
| CAN physical-layer report | Differential levels, bit timing, sample point, oscillator tolerance, error frames, interoperability |  |  |  |
| Bus-off report | Fault-injection trials, recovery timing, final safety state, retry/fault latch behavior |  |  |  |
| CiA 302 peer-loss report | Independent peer, heartbeat loss, peer reboot, NMT reset, recovery, persistence |  |  |  |
| IWDG report | LSI measurement, timeout, startup grace, reset latency, fault-injection repetitions |  |  |  |
| Flash power-loss report | Interrupted erase/program/commit cases, reboot results, CRC and fallback behavior |  |  |  |
| CiA 401 HIL report | Every I/O channel, boundary/fault cases, latency, calibration, safe-state behavior |  |  |  |
| OD/EDS/XDD package | Generated OD, EDS/XDD, manifest, hashes, and product revision consistency |  |  |  |
| Safety review | Board-independent safe-state analysis and approval |  |  |  |
| Deviations and waivers | Open deviations, rationale, risk acceptance, expiry, and owner |  |  |  |

### Evidence identity fields

| Field | Response |
|---|---|
| Firmware commit SHA |  |
| Build-manifest SHA-256 |  |
| Generated OD hash |  |
| EDS hash |  |
| XDD hash, or reason it is not yet available |  |
| Board serial number(s) |  |
| Board revision |  |
| Transceiver/BOM revision |  |
| Instrument identifiers and calibration due dates |  |
| Operator and test date |  |
| Ambient temperature, humidity, and supply conditions |  |
| Test procedure revision |  |
| Raw trace/log/archive location |  |

## 7. Freeze decision and approvals

The product-definition freeze is not complete until all applicable fields are `APPROVED`, all required evidence is identified, and unresolved deviations have an explicitly approved disposition.

| Approval role | Name | Decision (`APPROVE` / `REJECT` / `CONDITIONAL`) | Conditions or open actions | Signature / date |
|---|---|---|---|---|
| Hardware owner |  |  |  |  |
| Product owner |  |  |  |  |
| Firmware owner |  |  |  |  |
| Safety/reliability owner |  |  |  |  |
| Manufacturing owner |  |  |  |  |
| Test/HIL owner |  |  |  |  |
| Release approver |  |  |  |  |

### Final disposition

| Question | Response |
|---|---|
| Are any `TBD` values remaining in `PRODUCT_CIA401.md`? |  |
| Are any fields only `PROPOSED` rather than `APPROVED`? |  |
| Are any required hardware or HIL reports missing? |  |
| Are any deviations or waivers open? |  |
| Is the product definition ready to be copied into `PRODUCT_CIA401.md` as a freeze decision? |  |
| If not, list the blocking actions, owner, and due date. |  |

## 8. Transfer instructions

After approval, copy the final answers into the corresponding rows of [`PRODUCT_CIA401.md`](../PRODUCT_CIA401.md), update the product revision and traceability references, regenerate or validate the OD/EDS artifacts where required, and record the exact evidence-package revision. Do not remove `TBD` markers merely because a proposal exists; remove them only after the responsible owner has approved the answer and the required evidence is archived.
