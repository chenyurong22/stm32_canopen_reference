cmake_minimum_required(VERSION 3.22)

project(stm32f767_canopen_reference C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(STM32_CUBE_F7_DIR "" CACHE PATH "Path to the STM32CubeF7 firmware package")
set(STM32_F7_LINKER_SCRIPT "" CACHE FILEPATH "Board/package-specific GNU LD script")
option(CANOPEN_REFERENCE_ENABLE_CIA302_MASTER "Build the opt-in CiA 302 NMT-master personality" OFF)
option(CANOPEN_REFERENCE_ENABLE_CIA418 "Build the opt-in CiA 418 live Object Dictionary personality" OFF)
option(CANOPEN_REFERENCE_ENABLE_INVENTUS_BATTERY "Build the isolated Inventus battery test-only Object Dictionary personality" OFF)
option(CANOPEN_REFERENCE_ENABLE_IWDG "Enable dual-rate IWDG supervision" OFF)
option(CANOPEN_REFERENCE_ENABLE_TIMING_INSTRUMENTATION "Enable DWT timing instrumentation" OFF)
option(CANOPEN_REFERENCE_ENABLE_UDS "Enable the opt-in UDS/ISO-TP diagnostic profile" OFF)
set(CANOPEN_REFERENCE_UDS_RX_CAN_ID "0x7E0" CACHE STRING "UDS diagnostic request CAN ID")
set(CANOPEN_REFERENCE_UDS_TX_CAN_ID "0x7E8" CACHE STRING "UDS diagnostic response CAN ID")
option(CANOPEN_REFERENCE_PRODUCTION_PROFILE "Enable production optimization and compiler hardening" OFF)

if(NOT STM32_CUBE_F7_DIR)
    message(FATAL_ERROR "Set STM32_CUBE_F7_DIR to an STM32CubeF7 package. See docs/02_build_and_cubemx.md.")
endif()
if(NOT STM32_F7_LINKER_SCRIPT)
    message(FATAL_ERROR "Set STM32_F7_LINKER_SCRIPT for the exact STM32F767 package and memory map.")
endif()

set(STM32_SYSTEM_SOURCE "${STM32_CUBE_F7_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/system_stm32f7xx.c")
set(STM32_STARTUP_SOURCE "${STM32_CUBE_F7_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f767xx.s")
if(NOT EXISTS "${STM32_SYSTEM_SOURCE}" OR NOT EXISTS "${STM32_STARTUP_SOURCE}")
    # The pinned CubeF7 package stores the STM32F767ZI CMSIS pair under its
    # Nucleo example tree rather than the deprecated Templates directory.
    set(STM32_SYSTEM_SOURCE "${STM32_CUBE_F7_DIR}/Projects/STM32F767ZI-Nucleo/Applications/EEPROM/EEPROM_Emulation/Src/system_stm32f7xx.c")
    set(STM32_STARTUP_SOURCE "${STM32_CUBE_F7_DIR}/Projects/STM32F767ZI-Nucleo/Applications/EEPROM/EEPROM_Emulation/SW4STM32/startup_stm32f767xx.s")
endif()
if(NOT EXISTS "${STM32_SYSTEM_SOURCE}" OR NOT EXISTS "${STM32_STARTUP_SOURCE}")
    message(FATAL_ERROR "STM32CubeF7 package lacks a usable STM32F767 system/startup source pair")
endif()

set(CANOPEN_STM32_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/CanOpenSTM32")
set(CANOPEN_NODE_DIR "${CANOPEN_STM32_DIR}/CANopenNode")

file(GLOB_RECURSE CANOPEN_NODE_SOURCES CONFIGURE_DEPENDS
    "${CANOPEN_NODE_DIR}/301/*.c"
    "${CANOPEN_NODE_DIR}/303/*.c"
    "${CANOPEN_NODE_DIR}/304/*.c"
    "${CANOPEN_NODE_DIR}/305/*.c"
    "${CANOPEN_NODE_DIR}/309/*.c"
    "${CANOPEN_NODE_DIR}/storage/CO_storage.c")

if(CANOPEN_REFERENCE_ENABLE_INVENTUS_BATTERY)
    set(CANOPEN_REFERENCE_OD_SOURCE Generated/inventus_battery_OD.c)
elseif(CANOPEN_REFERENCE_ENABLE_CIA418)
    set(CANOPEN_REFERENCE_OD_SOURCE Generated/cia418_OD.c)
else()
    set(CANOPEN_REFERENCE_OD_SOURCE Generated/OD.c)
endif()

file(GLOB HAL_SOURCES CONFIGURE_DEPENDS
    "${STM32_CUBE_F7_DIR}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal*.c")
# STM32Cube ships alternate timebase implementations as source templates. They
# are not application sources and rely on peripheral modules intentionally
# disabled by this reference configuration.
list(FILTER HAL_SOURCES EXCLUDE REGEX ".*_template\\.c$")

add_executable(${PROJECT_NAME}
    Core/Src/main.c
    Core/Src/stm32f7xx_hal_msp.c
    Core/Src/stm32f7xx_it.c
    Core/Src/system_clock_reference.c
    Core/Src/syscalls.c
    ${STM32_SYSTEM_SOURCE}
    ${STM32_STARTUP_SOURCE}
    App/Src/CO_app_STM32_reference.c
    App/Src/canopen_reference_hw.c
    App/Src/canopen_reference_board.c
    App/Src/canopen_reference_cia302.c
    App/Src/canopen_reference_diagnostics.c
    App/Src/canopen_reference_watchdog.c
    App/Src/canopen_reference_timing.c
    App/Src/canopen_reference_uds.c
    App/Src/canopen_reference_storage.c
    App/Src/canopen_reference_can_recovery.c
    App/Src/canopen_reference_gateway.c
    App/Src/canopen_reference_lss.c
    App/Src/cia401_reference.c
    App/Src/cia418_reference.c
    App/Src/cia402_reference.c
    App/Src/inventus_battery_data.c
    middleware/canopen/core/can_acceptance_filter.c
    middleware/canopen/core/canopen_core.c
    middleware/canopen/core/cia302_nmt_master.c
    middleware/diagnostics/isotp/isotp.c
    middleware/diagnostics/uds/uds.c
    middleware/diagnostics/uds/uds_did.c
    middleware/diagnostics/uds/uds_security_provider.c
    middleware/diagnostics/uds/uds_download.c
    middleware/diagnostics/uds_stm32/uds_stm32.c
    ${CANOPEN_REFERENCE_OD_SOURCE}
    ${CANOPEN_STM32_DIR}/CANopenNode_STM32/CO_driver_STM32.c
    ${CANOPEN_NODE_DIR}/CANopen.c
    ${CANOPEN_NODE_SOURCES}
    ${HAL_SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    STM32F767xx
    USE_HAL_DRIVER
    CO_DRIVER_CUSTOM
    CO_USE_GLOBALS
    CANOPEN_REFERENCE_ENABLE_CIA302_MASTER=$<BOOL:${CANOPEN_REFERENCE_ENABLE_CIA302_MASTER}>
    CANOPEN_REFERENCE_ENABLE_CIA418=$<BOOL:${CANOPEN_REFERENCE_ENABLE_CIA418}>
    CANOPEN_REFERENCE_ENABLE_INVENTUS_BATTERY=$<BOOL:${CANOPEN_REFERENCE_ENABLE_INVENTUS_BATTERY}>
    $<$<BOOL:${CANOPEN_REFERENCE_ENABLE_CIA418}>:CANOPEN_REFERENCE_ENABLE_CIA401=0>
    $<$<BOOL:${CANOPEN_REFERENCE_ENABLE_CIA418}>:CANOPEN_REFERENCE_ENABLE_CIA402=0>
    $<$<BOOL:${CANOPEN_REFERENCE_ENABLE_INVENTUS_BATTERY}>:CANOPEN_REFERENCE_ENABLE_CIA401=0>
    $<$<BOOL:${CANOPEN_REFERENCE_ENABLE_INVENTUS_BATTERY}>:CANOPEN_REFERENCE_ENABLE_CIA402=0>
    CANOPEN_REFERENCE_ENABLE_IWDG=$<BOOL:${CANOPEN_REFERENCE_ENABLE_IWDG}>
    CANOPEN_REFERENCE_ENABLE_TIMING_INSTRUMENTATION=$<BOOL:${CANOPEN_REFERENCE_ENABLE_TIMING_INSTRUMENTATION}>
    CANOPEN_REFERENCE_ENABLE_UDS=$<BOOL:${CANOPEN_REFERENCE_ENABLE_UDS}>
    UDS_RX_CAN_ID=${CANOPEN_REFERENCE_UDS_RX_CAN_ID}
    UDS_TX_CAN_ID=${CANOPEN_REFERENCE_UDS_TX_CAN_ID}
    $<$<BOOL:${CANOPEN_REFERENCE_ENABLE_IWDG}>:HAL_IWDG_MODULE_ENABLED>
    # Newlib exposes the 64-bit printf macros used by CANopenNode's optional
    # ASCII SDO datatype path only when this target-library guard is defined.
    __int64_t_defined=1
    USE_HAL_CAN_REGISTER_CALLBACKS=$<BOOL:${CANOPEN_REFERENCE_ENABLE_UDS}>
    USE_HAL_TIM_REGISTER_CALLBACKS=0
    ART_ACCELERATOR_ENABLE=1)

target_include_directories(${PROJECT_NAME} PRIVATE
    Core/Inc
    App/Inc
    middleware/canopen/core
    middleware/diagnostics/isotp
    middleware/diagnostics/uds
    middleware/diagnostics/uds_stm32
    Generated
    ${CANOPEN_STM32_DIR}/CANopenNode_STM32
    ${CANOPEN_NODE_DIR}
    ${CANOPEN_NODE_DIR}/301
    ${CANOPEN_NODE_DIR}/303
    ${CANOPEN_NODE_DIR}/304
    ${CANOPEN_NODE_DIR}/305
    ${CANOPEN_NODE_DIR}/309
    ${CANOPEN_NODE_DIR}/storage
    ${STM32_CUBE_F7_DIR}/Drivers/STM32F7xx_HAL_Driver/Inc
    ${STM32_CUBE_F7_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Include
    ${STM32_CUBE_F7_DIR}/Drivers/CMSIS/Include)

target_compile_options(${PROJECT_NAME} PRIVATE
    -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16
    -include stdint.h
    -ffunction-sections -fdata-sections
    -Wall -Wextra -Wpedantic)

if(CANOPEN_REFERENCE_PRODUCTION_PROFILE)
    # The pinned vendor sources are built as part of this executable. Keep
    # their warnings visible, but do not let known dependency-only unused
    # parameter/variable diagnostics defeat fail-on-warning for project code.
    # Generated OD sources belong to the same dependency class: they include
    # CANopenNode headers directly and are regenerated, never hand-edited.
    # CANopen.c sits at the stack root outside the 301..309/storage glob, and
    # system_stm32f7xx.c is a vendor template; both are dependency-grade too.
    set_source_files_properties(
        ${CANOPEN_NODE_SOURCES}
        ${HAL_SOURCES}
        "${CANOPEN_NODE_DIR}/CANopen.c"
        "${CANOPEN_REFERENCE_OD_SOURCE}"
        "${STM32_SYSTEM_SOURCE}"
        "${CANOPEN_STM32_DIR}/CANopenNode_STM32/CO_driver_STM32.c"
        PROPERTIES COMPILE_OPTIONS "-Wno-error=conversion;-Wno-error=sign-conversion;-Wno-error=format;-Wno-error=unused-parameter;-Wno-error=unused-variable;-Wno-error=undef")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -O2
        -fstack-protector-strong
        -fno-common
        -Wformat=2
        -Wconversion
        -Wshadow
        -Wundef
        -Werror
        # Project-owned code enforces -Wconversion as a hard error. The known
        # third-party narrowing idioms are contained by the project include
        # boundary (canopen_reference_co.h) and per-file overrides below, so
        # no global escape hatch remains for project sources.
        -Werror=conversion)
    target_link_options(${PROJECT_NAME} PRIVATE
        -Wl,--fatal-warnings)
endif()

target_link_options(${PROJECT_NAME} PRIVATE
    -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16
    -T${STM32_F7_LINKER_SCRIPT}
    -Wl,--gc-sections
    -Wl,-Map=${PROJECT_NAME}.map
    --specs=nano.specs
    --specs=nosys.specs)

# The target CAN-port facade is intentionally an object-only validation target.
# The production CANopenNode STM32 driver remains the exclusive owner of bxCAN
# callbacks and lifecycle in the firmware executable.
add_library(canopen_stm32_port OBJECT middleware/canopen/port/can_port.c)
target_compile_definitions(canopen_stm32_port PRIVATE
    STM32F767xx
    USE_HAL_DRIVER
    CAN_PORT_STM32
    USE_HAL_CAN_REGISTER_CALLBACKS=$<BOOL:${CANOPEN_REFERENCE_ENABLE_UDS}>
    USE_HAL_TIM_REGISTER_CALLBACKS=0
    ART_ACCELERATOR_ENABLE=1
    __int64_t_defined=1)
target_include_directories(canopen_stm32_port PRIVATE
    Core/Inc
    middleware/canopen/port
    ${STM32_CUBE_F7_DIR}/Drivers/STM32F7xx_HAL_Driver/Inc
    ${STM32_CUBE_F7_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Include
    ${STM32_CUBE_F7_DIR}/Drivers/CMSIS/Include)
target_compile_options(canopen_stm32_port PRIVATE
    -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16
    -ffunction-sections -fdata-sections -Wall -Wextra -Wpedantic)

if(CANOPEN_REFERENCE_PRODUCTION_PROFILE)
    target_compile_options(canopen_stm32_port PRIVATE
        -O2
        -fstack-protector-strong
        -fno-common
        -Wformat=2
        -Wconversion
        -Wshadow
        -Wundef
        -Werror
        # Project-owned code enforces -Wconversion as a hard error. The known
        # third-party narrowing idioms are contained by the project include
        # boundary (canopen_reference_co.h) and per-file overrides below, so
        # no global escape hatch remains for project sources.
        -Werror=conversion)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin)
