# Review action register — 2026-08-15

## Software/CI actions

| Priority | Finding | Repository action |
|---|---|---|
| P1 | SocketCAN/vcan is warning-only in PR CI | Add a release-validation workflow/job that fails when vcan is unavailable and runs integration tests as a mandatory gate. Keep PR CI portable. |
| P1 | Six vectors are insufficient for core regression | Expand machine-readable vectors across NMT, heartbeat, EMCY, SDO expedited/segmented upload/download/abort, PDO/mapping, SYNC, TIME, LSS, reset, invalid OD, timeout, bus-off, and recovery. |
| P1 | SDO/PDO/NMT/LSS/error-injection coverage is not explicit | Add vector cases and validator categories; report category counts. |
| P2 | No sanitizer/coverage target is documented | Add a host sanitizer/coverage target and CI execution where supported. |
| P2 | Production evidence is described but not executable as a procedure | Add objective HIL, CAN physical, Flash power-loss/endurance, watchdog timing, electrical/EMC, and conformance evidence procedures. |

## Declared scope gaps

The review correctly identifies that complete CiA 401 product behavior, complete CiA 402 drive modes, complete CiA 302 network configuration, LSS Fastscan provisioning, embedded UDS/ISO-TP, CiA 418, embedded NMEA 2000, and bootloader/secure update are not implemented. These cannot be honestly closed by source-contract edits without product requirements, hardware, safety/security architecture, and conformance evidence. The repository must keep these limitations explicit and provide a staged roadmap rather than claim 100% completion.

## External release gates

Physical CAN/HIL, bus-off repetition, power-loss Flash interruption, endurance/temperature derating, measured IWDG/LSI timing, transceiver/electrical/EMC testing, multi-node behavior, and formal CiA 301/302/305/401/402 evidence require the actual board, instrumentation, test equipment, and applicable test specifications. They will be documented as blocking release evidence gates, not represented as completed by software tests.

## Production mechanisms

Bootloader, authenticated firmware update, image rollback, debug-port policy, option-byte/RDP configuration, key management, manufacturing test, and factory-reset policy remain a separately scoped product/security workstream. This review cycle adds requirements and acceptance criteria but does not fabricate a secure boot implementation without a defined trust anchor and manufacturing-key process.
