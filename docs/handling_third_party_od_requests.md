# Handling third-party Object Dictionary requests

This procedure applies when a contributor receives a request to add or revise CANopen Object Dictionary objects through a GitHub issue, spreadsheet, screenshot, PDF, or vendor message. It is designed for repeatable review and implementation, not for granting production approval.

> **Core policy:** do not invent missing vendor data, do not modify the default CiA 401 Object Dictionary for a customer-specific request, and do not claim production conformance from generated artifacts or host tests alone.

## 1. Establish scope before editing

Read the issue, all comments, attachments, `PRODUCT_SCOPE.md`, the relevant product-profile document, and the current generated OD source. Record the requested indices, the intended personality, whether the request is test-only or production-bound, and whether the request conflicts with the frozen v1 scope. A customer or manufacturer-specific request must be isolated behind an explicit opt-in build/profile selector unless product ownership has approved it as part of the default personality.

Create a short evidence note that records the issue number, attachment names, source hashes when available, and any ambiguities. Screenshots are evidence of labels or layout, not authoritative machine-readable metadata unless the value can be unambiguously verified.

## 2. Normalize the source data

Prefer a reviewable CSV or equivalent catalog as the source of truth. For every requested object or sub-index, capture the index, sub-index, name, access, width, CANopen data type, signedness, default, unit, scaling, persistence, callback/side-effect behavior, and PDO policy when those values are explicitly supplied. Preserve sparse sub-indices and distinguish an absent sub-index from a reserved or zero-valued field.

If a workbook contains contradictory values, do not silently choose a vendor-specific interpretation. Record the contradiction in the catalog or an accompanying analysis note, select the least-assumptive test-only interpretation only when necessary to keep the implementation deterministic, and mark the item for product-owner clarification. Never turn a screenshot placeholder into a safety or identity default.

## 3. Select the implementation boundary

Use a dedicated opt-in profile or personality for manufacturer-specific objects. Keep the default `Generated/OD.c`, default CMake selection, and frozen CiA 401 contract unchanged. Add a mutual-exclusion guard when multiple generated OD personalities cannot safely coexist.

Use the narrowest representation justified by the source evidence. A homogeneous raw array is acceptable only when the vendor has not supplied per-sub-index semantics. A structured record is appropriate when the source defines named sparse fields, mixed widths, or explicit access. Do not add callbacks, persistence, PDO mappings, signedness, scaling limits, or write side effects merely because they are plausible; each requires explicit source evidence and review.

## 4. Generate all derived artifacts

Implement or extend a deterministic generator so the catalog produces the C header, C source, EDS/XDD artifact, and any profile metadata. Generated files must not be edited by hand. Ensure sparse record members have unique C identifiers even when source labels repeat, and ensure generated OD lookup rejects undefined gaps.

The validator must check both the source catalog and generated outputs. At minimum, verify object counts, exact indices and sub-indices, data widths/types, access attributes, defaults, EDS sections, absence of unintended gap entries, and profile isolation. If the source contains an unresolved discrepancy, validate that the discrepancy marker remains visible rather than treating the provisional choice as an approved product value.

## 5. Add layered tests

Add or update the following layers in order:

| Layer | Required evidence |
|---|---|
| Static source validator | Catalog shape, sparse indices, access, types, defaults, EDS consistency, and profile isolation |
| Compiled host OD test | `OD_getSub()` behavior, data lengths, read/write permissions, defaults, writable readback, and undefined-sub-index rejection |
| In-process protocol smoke test | NMT startup, heartbeat, SDO upload/download and aborts, identity behavior, PDO communication/mapping defaults, and profile-specific edge cases |
| Firmware build | The selected opt-in personality compiles and links for the target MCU |
| Physical/HIL test | Required only for claims about CAN timing, arbitration, bus-off, electrical behavior, persistence interruption, safety response, or production conformance |

The in-process runner is useful when `vcan0` or SocketCAN capabilities are unavailable. It records simulated TX/RX frames and checks protocol-level behavior, but it is not evidence for a physical CAN interface, bit timing, arbitration, error frames, bus-off recovery, EMC, or HIL behavior.

## 6. Update the public record

Update the relevant profile document, `PRODUCT_SCOPE.md`, `CONTRIBUTING.md` or the applicable procedure, and the documentation map. State explicitly what is implemented, what remains provisional, and what is out of scope. Post a concise issue comment containing the commit, validation summary, unresolved approvals, and whether the issue should remain open. Close the issue only when the requested scope is implemented or the remaining items are explicitly accepted as follow-up work.

## 7. Review checklist

Before committing, confirm the following:

- The request is isolated behind an opt-in profile when it is manufacturer-specific or test-only.
- The default CiA 401 OD and default build selection are unchanged.
- Every generated artifact was regenerated from a checked-in source catalog.
- No hardware values, identity strings, scaling, side effects, or safety behavior were invented.
- Sparse and undefined sub-indices have tests.
- Read-only and read/write behavior has both OD-level and protocol-level coverage.
- Static validation, compiled host tests, mock protocol tests, and the selected firmware build pass.
- `git diff --check` passes and generated files are included in the review.
- Documentation is linked from `docs/README.md` and the public issue reflects the actual implementation boundary.
