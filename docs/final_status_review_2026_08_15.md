# Final-status review action list — 15 August 2026

The attached review confirms that the major P0 software defects are addressed and identifies the next improvements needed to make the reference platform more explicit, reproducible, and testable. This action list separates fixes that can be implemented in the repository from evidence that requires physical hardware or an external conformance laboratory.

| Priority | Finding | Repository action |
|---|---|---|
| P1 | Filter capacity is an undocumented fixed 20-entry limit. | Name the limit, reject overflow rather than silently dropping IDs, expose diagnostics, and test it. |
| P1 | Flash image assumes a sector-sized payload. | Add a named sector-size constant and compile-time image-size assertion. |
| P1 | Flash writes have no rate policy. | Add configurable minimum store interval, successful-store counter, and document that 0x1010 is not a logger. |
| P1 | Runtime lifecycle is implicit. | Add an explicit lifecycle enum and transition helpers for startup, running, reset-requested, reinitializing, and safe fault. |
| P1 | Watchdog production consequences need stronger wording. | Add mandatory production integration guidance and acceptance criteria. |
| P1 | Feature matrix is referenced but absent. | Add `docs/feature_matrix.md`, including CiA 402 mode-by-mode status and clear host/HIL/conformance columns. |
| P1 | No reusable conformance vector directory. | Add machine-readable core CANopen vectors and a deterministic host runner; explicitly label it as non-official conformance evidence. |
| P1 | Build manifest is text-only and incomplete. | Extend the existing generator to JSON with source, submodule, toolchain, compiler, OD, linker, personality, and dirty-state metadata. |
| P1 | CiA 401 examples and timing policy are not explicit. | Add a profile capability table covering PDO examples, timing, debounce, scaling, error behavior, and board hooks. |
| P1 | Physical HIL and recognized conformance evidence are absent. | Preserve the limitation; add evidence templates and acceptance artifact requirements, without fabricating results. |
| P2 | LSS Fastscan, complete CiA 302 manager, embedded UDS/ISO-TP, CiA 418 application, NMEA 2000, CAN-FD, and bootloader remain incomplete. | Keep these clearly marked as partial or unimplemented; do not claim completion. |

The target milestone is a traceable reference-platform baseline, not an unsupported claim of formal CANopen conformance or production certification.
