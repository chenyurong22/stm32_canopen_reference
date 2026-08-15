# Remediation Completion Record

**Date:** 13 August 2026  
**Scope:** Completion record for the remediation sequence defined in `04_specification_audit.md`. The requested `feat/canopen` branch and five-commit history are explicitly excluded at the user’s direction.

## 1. Outcome

The STM32F767 CANopen reference now contains a project-owned middleware facade, guarded Object Dictionary import workflow, SocketCAN/vcan transport, CANopen wire-contract regression suite, GitHub Actions CI, safe board-integration hooks, optional CiA 309 gateway personality, and expanded profile/RTOS/HIL engineering guidance. The core production personality remains a single-owner CANopenNode device runtime; no incompatible second CANopen stack or duplicate bxCAN interrupt owner was introduced.

> This record confirms source, build, and host-test evidence only. It does not claim final CiA profile conformance, CiA 304 functional safety, field interoperability, physical EMC behavior, or hardware-in-the-loop completion.

## 2. Remediation traceability

| Audit item | Remediation status | Delivered evidence |
|---|---|---|
| P0 — reconcile generic API with CANopenNode lifecycle | **Complete** | `docs/05_architecture_decision.md`; `middleware/canopen/core/canopen_core.[ch]`. The facade delegates to the native runtime and preserves CANopenNode OD ownership. |
| P0 — `feat/canopen` branch and five incremental commits | **Excluded by user direction** | No branch/history rewrite was performed. The source archive and working tree remain the delivery mechanism. |
| P0 — stable CAN-port interface | **Complete** | `middleware/canopen/port/can_port.[ch]` for STM32 object-only validation and `vcan_port.c` for SocketCAN. The production STM32 CANopenNode driver remains the sole bxCAN callback owner. |
| P0 — objdictgen import/validation workflow | **Complete** | `tools/import_objdict.sh`, enhanced `scripts/validate_od.py`, generated C/H/EDS validation, guarded staging, and explicit `--replace` activation requirement. |
| P0 — Linux host structure and `test_sdo.py` identity read | **Complete, CI runtime pending** | `tests/host/test_sdo.py`, `can_socket.py`, `canopen_vcan_device.py`, `setup_vcan.sh`, and `Makefile`. The suite reads `0x1018` through an SDO upload in its isolated wire-level device harness. A privileged vcan interface is unavailable in this sandbox; CI provisions and runs it. |
| P1 — CiA 301 protocol regression coverage | **Complete at wire-contract level** | The vcan suite covers expedited and segmented SDO, NMT, heartbeat, RPDO/TPDO, PDO mapping contract, LSS Fastscan response, and EMCY. It does not replace target/hardware execution or official conformance tests. |
| P1 — repeatable embedded build and artifacts | **Complete** | `CMakeLists.txt`, ARM toolchain file, linker script, GitHub workflow, firmware HEX/BIN/MAP output, and artifact retention. The CI build validates default and optional gateway personalities. |
| P1 — transceiver, fatal-state, and diagnostic boundaries | **Complete** | `canopen_reference_board.[ch]`, guarded board hooks in `main.c`, and rate-limited disabled-by-default diagnostics in `canopen_reference_diagnostics.[ch]`. |
| P2 — gateway design | **Complete as controlled optional personality** | `canopen_reference_gateway.[ch]`, `CANOPEN_REFERENCE_ENABLE_GATEWAY`, SDO/NMT gateway dependencies, runtime authorization hook, and a separate successful Cortex-M7 build. |
| P2 — CiA 401/402, CiA 304/405, FreeRTOS, and HIL roadmap | **Complete as engineering definition** | `docs/06_board_integration_and_hil.md` and `docs/07_profile_gateway_rtos_roadmap.md`. Product-specific implementation/conformance remains required. |

## 3. Validation evidence

| Validation | Result | Evidence |
|---|---|---|
| Generated OD and EDS synchronization | Passed | `scripts/validate_od.py` reports the expected ordered generated OD and EDS/profile synchronization. |
| Profile behavior | Passed | `tests/test_profiles.c` is run by `scripts/validate_reference.sh`. |
| Default STM32F767 firmware | Passed | ARM Cortex-M7 firmware build: text 53,472 B; data 1,216 B; BSS 9,624 B; total 64,312 B. |
| Native SocketCAN-port compilation | Passed | `make -C tests/host all`. |
| Optional CiA 309 gateway firmware | Passed | Separate Cortex-M7 configuration with `CANOPEN_REFERENCE_ENABLE_GATEWAY=1`: text 105,356 B; data 1,672 B; BSS 11,328 B; total 118,356 B. |
| vcan runtime protocol suite | **Not locally runnable** | The sandbox lacks network-administration capability to create `vcan0`. `.github/workflows/ci.yml` provisions `vcan0` with `sudo modprobe vcan` and runs `make -C tests/host test` on GitHub-hosted Ubuntu. |

## 4. How to reproduce

The default, target, profile, and OD checks are run with:

```sh
./scripts/validate_reference.sh
make -C tests/host all
```

On a Linux host or CI runner with the required network-administration capability, run the full SocketCAN suite with:

```sh
sudo ./scripts/setup_vcan.sh vcan0
CAN_PORT_IFACE=vcan0 make -C tests/host test
```

The optional gateway personality is built separately to avoid accidentally exposing gateway behavior in a device build:

```sh
cmake -S . -B build/gateway \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake \
  -DSTM32_CUBE_F7_DIR=/path/to/STM32CubeF7 \
  -DSTM32_F7_LINKER_SCRIPT="$PWD/linker/STM32F767_2M_512K_FLASH.ld" \
  -DCMAKE_C_FLAGS=-DCANOPEN_REFERENCE_ENABLE_GATEWAY=1
cmake --build build/gateway --parallel 2
```

## 5. Residual release gates

The remaining work is external-product evidence rather than missing generic source structure. It includes board-pin ownership, transceiver electrical enable/standby validation, physical CAN interoperability, characterization under bus load, production flash/boot/field-update policy, selected CiA 401 or CiA 402 profile completion, security design for any gateway transport, and a formal safety lifecycle before any CiA 304 claim. The mandatory owner, test method, acceptance result, and evidence location should be recorded in the product’s verification plan.
