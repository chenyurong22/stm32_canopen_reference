# STM32F767 Board Integration and Hardware-in-the-Loop Plan

**Status:** Required product-port work; not completed by the board-agnostic reference.

The source reference now has explicit board hooks, but it cannot select a real CAN transceiver, safe-output topology, drive feedback interface, or UART pin without the target schematic and safety architecture. The default implementation therefore starts with the transceiver disabled and all application hooks de-energized. This is intentional.

> A successful CANopen protocol initialization does not authorize physical I/O or torque production. The board port must enforce its own independent interlocks and power-stage safety behavior.

## 1. Required board overrides

| Reference symbol | Product-board responsibility | Default behavior |
|---|---|---|
| `CANopenReferenceBoard_SetCanTransceiverEnabled()` | Drive the actual transceiver EN/STB pin with verified polarity and startup timing. | Does nothing; transceiver remains logically disabled. |
| `CANopenReferenceBoard_InitSafe()` | Configure EN/STB, digital outputs, analog outputs, and drive enable pins before CAN startup. | Disables the transceiver and forces reference I/O/drive hooks safe. |
| `CANopenReferenceBoard_ForceSafe()` | Invoke the independent board/power-stage safe path on fatal error or reset. | Disables the transceiver and reference outputs. |
| `CANopenReferenceBoard_OnCanopenReady()` | Permit communication only after board diagnostics and policy checks. | Does nothing. |
| `CANopenReferenceHw_*()` | Bind CiA 401 channels and CiA 402 feedback/commands to validated drivers. | Weak fail-safe stubs; no healthy drive feedback. |
| `CANopenReferenceDiagnostics_Write()` | Optionally provide bounded UART/DMA output from mainline. | No output; diagnostics are disabled. |

The production board override must preserve exclusive ownership of CAN1 callbacks by `CO_driver_STM32.c`. Do not link `middleware/canopen/port/can_port.c` to the production CAN1 instance; it is an exclusive-owner diagnostic/test facade, not a second bxCAN driver.

## 2. Physical CAN acceptance procedure

The target firmware assumes **CAN1 on PA11/PA12**, a 25 MHz HSE, and 500 kbit/s derived from a 54 MHz APB1 clock. Recalculate and measure timing after any CubeMX clock-tree change. The bxCAN timing configuration must be reviewed against the actual oscillator tolerance, network length, transceiver, isolation, termination arrangement, and EMC requirements.[1] [2]

| Step | Evidence required | Pass criterion |
|---|---|---|
| Electrical review | Schematic review covering transceiver supply, EN/STB polarity, common-mode/isolator architecture, 120-ohm network-end termination, connector shielding, and ESD protection. | Approved by hardware owner; no reference default pin assumptions remain. |
| Safe startup | Probe EN/STB, drive-enable, and output rails from reset through first CANopen initialization. | No physical output or drive enable before independent board checks pass. |
| Bit timing | Capture CANH/CANL with a differential probe or CAN analyzer under bus traffic. | 500 kbit/s communication without error-frame accumulation; observed timing matches reviewed configuration. |
| Network interoperability | Connect at least one independent CANopen manager and one passive analyzer. | Boot-up, heartbeat, SDO `0x1018`, RPDO/TPDO, NMT reset, LSS discovery, and EMCY events decode correctly. |
| Error handling | Remove/reconnect bus, induce bus-off where safe, and reset the manager. | Application remains in board-defined safe state; error history and recovery match product requirements. |
| SDO block transfer | Transfer payloads smaller/larger than the configured 1,024-byte server/client buffers using an independent CANopen tool. | Valid CRC, expected abort code on invalid conditions, no watchdog/reset, and documented performance. |

## 3. Optional UART diagnostics

Set `CANOPEN_REFERENCE_UART_DIAGNOSTICS=1` only after overriding `CANopenReferenceDiagnostics_Write()` with a non-blocking, bounded board implementation—normally DMA-backed UART with explicit buffering. The publisher is called from the main CANopen process loop at most once per second and is never invoked from the 1 ms profile/CAN processing interrupt.

## 4. Release evidence

A product release must archive the CubeMX `.ioc`, generated pinout/clock reports, compiler version, firmware hash, CAN analyzer traces, HIL test logs, EDS/XDD version, and conformance-test records. This reference’s CI proves host-frame contracts and cross-build reproducibility; it does **not** replace physical-network interoperability, device-profile conformance, EMC, functional-safety, or production manufacturing tests.

## References

[1]: https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf "STM32F76xxx and STM32F77xxx reference manual"

[2]: https://github.com/CANopenNode/CANopenNode "CANopenNode source repository"
