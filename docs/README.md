# Documentation map

This directory contains the living engineering documentation and historical review evidence for the STM32F767 CANopen reference. Start with the project [README](../README.md) for build and validation entry points, then use this map to select the document appropriate to the task.

## Architecture and implementation

| Purpose | Document |
|---|---|
| System architecture and ownership boundaries | [01_architecture.md](01_architecture.md) |
| Build, CubeMX, and board-porting boundaries | [02_build_and_cubemx.md](02_build_and_cubemx.md) |
| Profile-module structure | [03_profile_modules.md](03_profile_modules.md) |
| Execution model and timing | [execution_model_and_timing_analysis.md](execution_model_and_timing_analysis.md) |
| Public API and integration seams | [API.md](API.md) |
| Feature and scope matrix | [feature_matrix.md](feature_matrix.md) |
| UDS and ISO-TP architecture | [11_uds_iso_tp.md](11_uds_iso_tp.md) |
| UDS SecurityAccess and DID policy | [12_uds_security_and_dids.md](12_uds_security_and_dids.md) |
| UDS download and recovery | [13_uds_download_and_recovery.md](13_uds_download_and_recovery.md) |
| UDS validation and acceptance | [14_uds_validation_and_acceptance.md](14_uds_validation_and_acceptance.md) |

## Object Dictionary and profile procedures

| Purpose | Document |
|---|---|
| Inventus battery test-only OD, including structured D000 and provisional D001 | [inventus_battery_test_profile.md](inventus_battery_test_profile.md) |
| Procedure for handling third-party OD requests from issues, screenshots, or workbooks | [handling_third_party_od_requests.md](handling_third_party_od_requests.md) |
| Reusable in-process CANopen protocol smoke testing | [mock_canopen_protocol_smoke_testing.md](mock_canopen_protocol_smoke_testing.md) |
| Issue #13 protocol acceptance matrix and standard COB-ID checks | [issue13_protocol_acceptance.md](issue13_protocol_acceptance.md) |
| CiA 401 hardware-owner questionnaire | [cia401_hardware_owner_questionnaire.md](cia401_hardware_owner_questionnaire.md) |
| Open-issue and scope-resolution record | [open_issue_resolution.md](open_issue_resolution.md) |

## Qualification and release gates

| Purpose | Document |
|---|---|
| Complete production validation plan | [production_validation_plan.md](production_validation_plan.md) |
| v1 release readiness gate | [v1_release_readiness_gate.md](v1_release_readiness_gate.md) |
| CAN physical-layer qualification | [can_physical_layer_qualification.md](can_physical_layer_qualification.md) |
| CANopen conformance gate | [canopen_conformance_gate.md](canopen_conformance_gate.md) |
| CiA 401 HIL validation | [cia401_hil_validation.md](cia401_hil_validation.md) |
| CiA 302 peer supervision | [cia302_peer_supervision_qualification.md](cia302_peer_supervision_qualification.md) |
| Bus-off recovery | [bus_off_qualification.md](bus_off_qualification.md) |
| Flash interruption and persistence | [flash_qualification.md](flash_qualification.md) |
| Watchdog qualification | [watchdog_qualification.md](watchdog_qualification.md) |
| Stress and soak qualification | [stress_soak_resource_qualification.md](stress_soak_resource_qualification.md) |
| External evidence package | [external_evidence_package.md](external_evidence_package.md) |

## Security, manufacturing, and release records

The security checklist, manufacturing record, production-build profile, environmental qualification, and release notes are living release-support documents:

- [10_product_security_release_checklist.md](10_product_security_release_checklist.md)
- [security_v1_release_gate.md](security_v1_release_gate.md)
- [manufacturing_production_record.md](manufacturing_production_record.md)
- [production_build_profile.md](production_build_profile.md)
- [emc_environmental_qualification.md](emc_environmental_qualification.md)
- [release_v0.9.0_historical.md](release_v0.9.0_historical.md)
- [release_v0.9.0_candidate.md](release_v0.9.0_candidate.md)
- [release_v0.9.0_rc1_baseline.md](release_v0.9.0_rc1_baseline.md)

## Historical reviews and audits

Dated self-review and audit documents are retained in the top-level `docs/` directory for compatibility with existing evidence references and test fixtures. They are historical records, not normative implementation guidance. New dated review artifacts should be placed under `docs/reviews/` and linked here after the review is closed.

| Record type | Existing record |
|---|---|
| Reconciliation and audit | [audit_2026-08_reconciliation.md](audit_2026-08_reconciliation.md), [audit_remediation_2026.md](audit_remediation_2026.md) |
| Final-status reviews | [final_status_review_2026_08_15.md](final_status_review_2026_08_15.md), [final_status_review_findings_2026.md](final_status_review_findings_2026.md), [final_status_remediation_2026.md](final_status_remediation_2026.md) |
| Action register | [review_2026_08_15_action_register.md](review_2026_08_15_action_register.md) |
| Engineering review | [engineering_validation_review.md](engineering_validation_review.md) |

Normative requirements belong in `PRODUCT_SCOPE.md`, `PRODUCT_CIA401.md`, the relevant profile document, or a qualification procedure—not only in a dated review record.

## Documentation maintenance rule

Every new profile, public Object Dictionary change, qualification gate, or external-evidence package should add a link here and, where appropriate, a link from the top-level README. A document that cannot be discovered from this map should be treated as incomplete documentation, even when its contents are technically correct.
