# Product Security Release Checklist

**Author:** Manus AI
**Scope:** Product teams deriving a deployable STM32F767 CANopen device from this reference firmware.

## Purpose

This reference keeps the CiA 309 gateway disabled and its authorization hook default-deny. It does not implement a product bootloader, credential store, certificate chain, audit backend, or a board-specific debug-lock policy. A product team must complete and retain the evidence below before enabling gateway access or releasing field firmware. The required secure-update and recovery evidence aligns with platform-firmware resiliency guidance.[1]

> **Release rule:** A checkbox is evidence, not a substitute for design review. Any item marked **not applicable** requires a signed rationale by the product security owner.

## Mandatory release evidence

| Control area | Required product decision | Required evidence | Release owner |
|---|---|---|---|
| Debug lifecycle | Define the SWD/JTAG state for manufacturing, service, and field deployment. Field images must not leave unrestricted debug access unless an approved service threat model requires it. | MCU option-byte record, manufacturing procedure, and production-unit verification log. | Hardware and manufacturing owner |
| Secure boot and update | Define the immutable trust anchor, signed image format, signature algorithm, key custody, version rollback policy, and recovery path. | Reviewed boot/update design, test results for invalid signature and rollback handling, and key-management record. | Firmware security owner |
| Gateway authorization | Keep `CANOPEN_REFERENCE_ENABLE_GATEWAY=0` unless a product-specific authentication and authorization design is approved. Define identity, authorization lifetime, command allowlist, rate limiting, session teardown, and failure response. | Threat model, integration test with the board override, and negative tests proving unauthenticated access is denied. | Product security owner |
| Gateway audit trail | Define which gateway commands, authorization outcomes, faults, and configuration changes are retained, where they are stored, and how timestamps are obtained. | Log schema, retention policy, storage-failure behavior, and evidence from an integration test. | Product and operations owner |
| CAN network exposure | Define trust boundaries for commissioning, LSS, SDO write access, and diagnostic connectors. | Network threat model, commissioning procedure, and physical-access assumptions. | System architect |
| Vulnerability response | Assign owners for third-party dependency monitoring, CVE triage, firmware signing-key rotation, and customer update notices, consistent with a documented secure-development process.[2] | Bill of materials, monitoring process, and incident response contact. | Product security owner |

## Firmware configuration gates

The following configuration checks apply directly to this repository.

| Gate | Requirement |
|---|---|
| Default image | Build with `CANOPEN_REFERENCE_ENABLE_GATEWAY=0`; default weak authorization must remain deny-by-default. |
| Gateway image | Require a board-provided authorization override, transport implementation, and integration test before a gateway image is distributed. |
| Device identity | Replace the reference vendor ID, product code, revision, and serial number with released values synchronized with the EDS/XDD and device label. |
| Build provenance | Retain the CI firmware artifact and its `ci-build-manifest.txt`, including the compiler, CanOpenSTM32, and STM32CubeF7 revisions. |
| Hardware validation | Perform target-Hardware CAN, transceiver enable/standby, bus-off recovery, power-loss, and watchdog tests; the reference’s host tests cannot provide this evidence. |

## Approval record

A release candidate is approved only when the product security owner, firmware owner, hardware owner, and system test owner have reviewed the required evidence and recorded the firmware artifact hash. The evidence package must identify the exact Git commit and CI artifact manifest used for the release.

## References

[1]: https://csrc.nist.gov/pubs/sp/800/193/final "NIST SP 800-193: Platform Firmware Resiliency Guidelines"
[2]: https://pages.nist.gov/ssdf/ "NIST Secure Software Development Framework"
