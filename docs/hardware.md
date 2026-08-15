# Hardware Integration Guide

This document defines the minimum board-level integration needed to use the STM32F767 CANopen reference. The repository does not define a universal Nucleo, Discovery, or production-board pinout, so the exact MCU package, schematic, transceiver, power tree, and connector must be reviewed before hardware is powered.

## CAN wiring

The STM32F767 CAN1 signals are digital controller pins and must connect to an external CAN transceiver.

| MCU signal | STM32F767 reference pin | Transceiver signal |
|---|---|---|
| CAN1_RX | PA11, AF9 | Transceiver `RXD` |
| CAN1_TX | PA12, AF9 | Transceiver `TXD` |
| Ground | Board-specific | Transceiver ground and CAN peer ground reference |
| Transceiver enable/standby | Board-specific GPIO | `EN`, `STB`, or device-specific control |
| CANH/CANL | External connector | Twisted pair to the bus |

Use a CAN transceiver rated for the bus voltage, common-mode range, data rate, temperature range, and EMC environment. Examples include devices from the SN65HVD or TJA105x families; select the exact part from the product electrical and compliance requirements rather than treating the example family as a mandatory design.

The MCU and transceiver must share a valid logic supply and ground reference. Check the transceiver’s logic-level thresholds and standby polarity. Keep the CAN pair routed as a controlled differential pair where practical, avoid stubs, and follow the selected transceiver data sheet for protection and decoupling.

## Termination

A conventional two-ended CAN bus uses one approximately 120 Ω termination resistor at each physical end of the trunk. Do not add a third termination on an intermediate node. With power removed, an ohmmeter across CANH and CANL should normally measure approximately 60 Ω for two parallel 120 Ω terminators. The exact measurement can vary with protection components and attached equipment.

Use split termination, common-mode filtering, galvanic isolation, or transient protection only after considering the transceiver data sheet, bus topology, EMC requirements, and timing budget. Do not connect CANH or CANL directly to an STM32 pin.

## Clock and CAN timing

The reference firmware assumes a 25 MHz HSE, a 216 MHz system clock, a 54 MHz APB1 clock, and a CAN timing configuration of 500 kbit/s. If the board uses a different crystal, oscillator, APB1 divider, or clock tree, recalculate the CAN bit timing and verify it with a CAN analyzer.

The reference uses PA11 and PA12 for CAN1 and TIM7 for the 1 ms application cadence. The exact alternate-function configuration, interrupt routing, and GPIO electrical settings must be checked against the selected STM32F767 package.

## ST-LINK and debug connection

Provide an accessible SWD header during development with SWDIO, SWCLK, reset, target voltage, and ground. Use the ST-LINK tool appropriate for the board and target device. Keep debug access controlled in production according to the product threat model; see [SECURITY.md](../SECURITY.md).

Before flashing, confirm the target voltage, reset wiring, exact device density, linker script, and boot configuration. A wrong linker script or target selection can produce an image that builds successfully but does not match the physical memory map.

## Safe bring-up sequence

1. Power the board with the transceiver held in its documented standby or disabled state.
2. Confirm the MCU supply, reset, clock source, and SWD connection.
3. Flash the default personality and verify that application outputs remain in their safe default state.
4. Connect the CAN analyzer or second CANopen node with correct bitrate and termination.
5. Enable the transceiver and observe the boot-up heartbeat on `0x700 + node-ID`.
6. Confirm pre-operational heartbeat state, then test NMT transitions, heartbeat timing, EMCY behavior, and reset recovery.
7. Only after the basic network is stable should application power stages or actuators be enabled.

## Bench acceptance evidence

Retain the firmware commit, build manifest, linker script, Object Dictionary revision, wiring photograph or schematic, CAN analyzer trace, measured bitrate, termination measurement, power/reset conditions, and HIL result JSON. Use the detailed [UDS/CiA 302 hardware procedure](hardware/uds_cia302_test_procedure.md) for diagnostic and NMT acceptance.
