#!/usr/bin/env sh
# SPDX-License-Identifier: LicenseRef-STM32-CANopen-Research-Education-Commercial-1.0
# Record the source and toolchain inputs required to reproduce a firmware build.
set -eu

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <output-file> <STM32CubeF7-directory>" >&2
    exit 64
fi

OUTPUT=$1
CUBE_DIR=$2
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
CANOPENSTM32_DIR="$ROOT/third_party/CanOpenSTM32"
LINKER_SCRIPT=${STM32_F7_LINKER_SCRIPT:-$ROOT/linker/STM32F767_2M_512K_FLASH.ld}
OD_FILE="$ROOT/Generated/OD.c"
PERSONALITY=${CANOPEN_REFERENCE_PERSONALITY:-unspecified}
JSON_OUTPUT=${BUILD_MANIFEST_JSON:-${OUTPUT%.txt}.json}

if ! git -C "$CUBE_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1 \
    || ! git -C "$CANOPENSTM32_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "Manifest generation requires Git worktrees for STM32CubeF7 and CanOpenSTM32." >&2
    exit 66
fi

mkdir -p "$(dirname -- "$OUTPUT")"
source_revision=$(git -C "$ROOT" rev-parse HEAD)
source_dirty=$(test -n "$(git -C "$ROOT" status --porcelain)" && printf true || printf false)
canopenstm32_revision=$(git -C "$CANOPENSTM32_DIR" rev-parse HEAD)
stm32cubef7_revision=$(git -C "$CUBE_DIR" rev-parse HEAD)
arm_gcc_version=$(arm-none-eabi-gcc --version | sed -n '1p')
arm_ld_version=$(arm-none-eabi-ld --version | sed -n '1p')
cmake_version=$(cmake --version | sed -n '1p')
od_sha256=$(sha256sum "$OD_FILE" | awk '{print $1}')
linker_sha256=$(sha256sum "$LINKER_SCRIPT" | awk '{print $1}')
{
    printf '%s\n' '# STM32F767 CANopen reference build manifest'
    printf 'source_revision=%s\n' "$(git -C "$ROOT" rev-parse HEAD)"
    printf 'source_dirty=%s\n' "$(git -C "$ROOT" status --porcelain | sed -n '1p' | wc -l | tr -d ' ')"
    printf 'canopenstm32_revision=%s\n' "$(git -C "$CANOPENSTM32_DIR" rev-parse HEAD)"
    printf 'stm32cubef7_revision=%s\n' "$(git -C "$CUBE_DIR" rev-parse HEAD)"
    printf 'arm_none_eabi_gcc=%s\n' "$(arm-none-eabi-gcc --version | sed -n '1p')"
    printf 'arm_none_eabi_ld=%s\n' "$(arm-none-eabi-ld --version | sed -n '1p')"
    printf 'cmake=%s\n' "$(cmake --version | sed -n '1p')"
    printf 'c_flags=%s\n' "${CMAKE_C_FLAGS:-}"
    printf 'toolchain_file=%s\n' "${CMAKE_TOOLCHAIN_FILE:-cmake/arm-none-eabi-gcc.cmake}"
} > "$OUTPUT"
cat > "$JSON_OUTPUT" <<EOF
{
  "schema": "stm32-canopen-build-manifest-v2",
  "source": {
    "revision": "$source_revision",
    "dirty": $source_dirty
  },
  "submodules": {
    "canopenstm32_revision": "$canopenstm32_revision",
    "stm32cubef7_revision": "$stm32cubef7_revision"
  },
  "toolchain": {
    "arm_none_eabi_gcc": "$arm_gcc_version",
    "arm_none_eabi_ld": "$arm_ld_version",
    "cmake": "$cmake_version",
    "toolchain_file": "${CMAKE_TOOLCHAIN_FILE:-cmake/arm-none-eabi-gcc.cmake}"
  },
  "configuration": {
    "personality": "$PERSONALITY",
    "c_flags": "${CMAKE_C_FLAGS:-}",
    "linker_script": "$LINKER_SCRIPT"
  },
  "inputs": {
    "object_dictionary_sha256": "$od_sha256",
    "linker_script_sha256": "$linker_sha256"
  }
}
EOF
printf 'text_manifest=%s\njson_manifest=%s\n' "$OUTPUT" "$JSON_OUTPUT"
