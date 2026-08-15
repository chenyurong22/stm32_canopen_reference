cmake_minimum_required(VERSION 3.22)

project(stm32f767_canopen_reference C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(STM32_CUBE_F7_DIR "" CACHE PATH "Path to the STM32CubeF7 firmware package")
set(STM32_F7_LINKER_SCRIPT "" CACHE FILEPATH "Board/package-specific GNU LD script")

if(NOT STM32_CUBE_F7_DIR)
    message(FATAL_ERROR "Set STM32_CUBE_F7_DIR to an STM32CubeF7 package. See docs/02_build_and_cubemx.md.")
endif()
if(NOT STM32_F7_LINKER_SCRIPT)
    message(FATAL_ERROR "Set STM32_F7_LINKER_SCRIPT for the exact STM32F767 package and memory map.")
endif()

set(CANOPEN_STM32_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/CanOpenSTM32")
set(CANOPEN_NODE_DIR "${CANOPEN_STM32_DIR}/CANopenNode")

file(GLOB_RECURSE CANOPEN_NODE_SOURCES CONFIGURE_DEPENDS
    "${CANOPEN_NODE_DIR}/301/*.c"
    "${CANOPEN_NODE_DIR}/303/*.c"
    "${CANOPEN_NODE_DIR}/304/*.c"
    "${CANOPEN_NODE_DIR}/305/*.c"
    "${CANOPEN_NODE_DIR}/309/*.c"
    "${CANOPEN_NODE_DIR}/storage/*.c")

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
    ${STM32_CUBE_F7_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/system_stm32f7xx.c
    ${STM32_CUBE_F7_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f767xx.s
    App/Src/CO_app_STM32_reference.c
    App/Src/canopen_reference_hw.c
    App/Src/canopen_reference_board.c
    App/Src/canopen_reference_diagnostics.c
    App/Src/canopen_reference_gateway.c
    App/Src/cia401_reference.c
    App/Src/cia402_reference.c
    middleware/canopen/core/canopen_core.c
    middleware/canopen/core/cia302_nmt_master.c
    Generated/OD.c
    ${CANOPEN_STM32_DIR}/CANopenNode_STM32/CO_driver_STM32.c
    ${CANOPEN_NODE_DIR}/CANopen.c
    ${CANOPEN_NODE_SOURCES}
    ${HAL_SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    STM32F767xx
    USE_HAL_DRIVER
    CO_DRIVER_CUSTOM
    CO_USE_GLOBALS
    # Newlib exposes the 64-bit printf macros used by CANopenNode's optional
    # ASCII SDO datatype path only when this target-library guard is defined.
    __int64_t_defined=1)

target_include_directories(${PROJECT_NAME} PRIVATE
    Core/Inc
    App/Inc
    middleware/canopen/core
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
target_compile_definitions(canopen_stm32_port PRIVATE STM32F767xx USE_HAL_DRIVER CAN_PORT_STM32)
target_include_directories(canopen_stm32_port PRIVATE
    Core/Inc
    middleware/canopen/port
    ${STM32_CUBE_F7_DIR}/Drivers/STM32F7xx_HAL_Driver/Inc
    ${STM32_CUBE_F7_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Include
    ${STM32_CUBE_F7_DIR}/Drivers/CMSIS/Include)
target_compile_options(canopen_stm32_port PRIVATE
    -mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16
    -ffunction-sections -fdata-sections -Wall -Wextra -Wpedantic)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin)
