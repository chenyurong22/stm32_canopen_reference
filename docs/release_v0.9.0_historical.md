# v0.9.0 Historical Candidate

The `v0.9.0` tag is retained as an **immutable historical hardware-validation candidate** at source SHA `9c04ef22c93b13f88df37245d47e52926b516d5c`. It is not the current release baseline and must not be used as the reference for new HIL or physical-qualification evidence.

For current review work, the superseding software lineage is [`v0.9.0-rc2`](https://github.com/mahdi-benhassen/stm32_canopen_reference/releases/tag/v0.9.0-rc2). `v0.9.0-rc2` remains a release candidate with physical, HIL, laboratory, security, manufacturing, and applicable conformance gates pending; it is not a hardware-validated or production-ready release.

Evidence must always identify the exact firmware SHA, build manifest, Object Dictionary/EDS/XDD hashes, board revision and serial, instrument identifiers, raw traces, and review approvals. Do not attach evidence to a moving branch or infer hardware qualification from host-only tests. The release sequence and gate order are defined in [`docs/v1_release_readiness_gate.md`](v1_release_readiness_gate.md), and the current product boundary is the CiA 401 path described in [`PRODUCT_SCOPE.md`](../PRODUCT_SCOPE.md).

The repository’s v1 claim remains limited to a software-validated CANopen reference integration until the external evidence package is complete and approved. CiA 418, embedded UDS/ISO-TP, NMEA 2000, complete CiA 302 configuration management, CAN-FD, secure field update, and formal conformance remain outside that frozen claim.
