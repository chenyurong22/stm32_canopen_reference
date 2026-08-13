#!/usr/bin/env sh
# SPDX-License-Identifier: Apache-2.0
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

if ! git -C "$CUBE_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1 \
    || ! git -C "$CANOPENSTM32_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "Manifest generation requires Git worktrees for STM32CubeF7 and CanOpenSTM32." >&2
    exit 66
fi

mkdir -p "$(dirname -- "$OUTPUT")"
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
