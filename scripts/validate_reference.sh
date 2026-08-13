#!/usr/bin/env sh
# SPDX-License-Identifier: Apache-2.0
# Reproducible local validation for the STM32F767 CANopen reference.
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
CUBE_DIR=${STM32_CUBE_F7_DIR:-"$ROOT/third_party/STM32CubeF7"}
LINKER_SCRIPT=${STM32_F7_LINKER_SCRIPT:-"$ROOT/linker/STM32F767_2M_512K_FLASH.ld"}
BUILD_DIR=${BUILD_DIR:-"$ROOT/build/firmware"}

command -v python3 >/dev/null
command -v gcc >/dev/null
command -v cmake >/dev/null
command -v arm-none-eabi-gcc >/dev/null
command -v arm-none-eabi-size >/dev/null

python3 "$ROOT/scripts/validate_od.py"

mkdir -p "$ROOT/build/tests"
gcc -std=c11 -Wall -Wextra -Werror \
    -DCANOPEN_REFERENCE_ENABLE_CIA401=1 \
    -DCANOPEN_REFERENCE_ENABLE_CIA402=1 \
    -DCANOPEN_REFERENCE_ALLOW_COMBINED_PROFILES=1 \
    -I"$ROOT/tests/fakes" -I"$ROOT/App/Inc" \
    "$ROOT/tests/test_profiles.c" \
    "$ROOT/App/Src/cia401_reference.c" \
    "$ROOT/App/Src/cia402_reference.c" \
    -o "$ROOT/build/tests/test_profiles"
"$ROOT/build/tests/test_profiles"

cmake -S "$ROOT" -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/arm-none-eabi-gcc.cmake" \
    -DSTM32_CUBE_F7_DIR="$CUBE_DIR" \
    -DSTM32_F7_LINKER_SCRIPT="$LINKER_SCRIPT"
cmake --build "$BUILD_DIR" --parallel 2
arm-none-eabi-size "$BUILD_DIR/stm32f767_canopen_reference"
test -s "$BUILD_DIR/stm32f767_canopen_reference.hex"
test -s "$BUILD_DIR/stm32f767_canopen_reference.bin"
printf '%s\n' 'Reference validation completed successfully.'
