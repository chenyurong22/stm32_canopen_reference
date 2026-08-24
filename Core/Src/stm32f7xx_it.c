/* SPDX-License-Identifier: LicenseRef-STM32-CANopen-Research-Education-Commercial-1.0 */
#include "main.h"
#include "canopen_reference_timing.h"

void
NMI_Handler(void) {
    while (1) {
    }
}

void
HardFault_Handler(void) {
    while (1) {
    }
}

void
MemManage_Handler(void) {
    while (1) {
    }
}

void
BusFault_Handler(void) {
    while (1) {
    }
}

void
UsageFault_Handler(void) {
    while (1) {
    }
}

void
SVC_Handler(void) {
}

void
DebugMon_Handler(void) {
}

void
PendSV_Handler(void) {
}

void
SysTick_Handler(void) {
    HAL_IncTick();
}

void
TIM7_IRQHandler(void) {
    uint32_t timing_start = CANopenReferenceTiming_Tim7Enter();
    HAL_TIM_IRQHandler(&htim7);
    CANopenReferenceTiming_Tim7Exit(timing_start);
}

void
CAN1_TX_IRQHandler(void) {
    uint32_t timing_start = CANopenReferenceTiming_CanEnter(CANOPEN_REFERENCE_TIMING_CAN_TX);
    HAL_CAN_IRQHandler(&hcan1);
    CANopenReferenceTiming_CanExit(CANOPEN_REFERENCE_TIMING_CAN_TX, timing_start);
}

void
CAN1_RX0_IRQHandler(void) {
    uint32_t timing_start = CANopenReferenceTiming_CanEnter(CANOPEN_REFERENCE_TIMING_CAN_RX0);
    HAL_CAN_IRQHandler(&hcan1);
    CANopenReferenceTiming_CanExit(CANOPEN_REFERENCE_TIMING_CAN_RX0, timing_start);
}

void
CAN1_RX1_IRQHandler(void) {
    uint32_t timing_start = CANopenReferenceTiming_CanEnter(CANOPEN_REFERENCE_TIMING_CAN_RX1);
    HAL_CAN_IRQHandler(&hcan1);
    CANopenReferenceTiming_CanExit(CANOPEN_REFERENCE_TIMING_CAN_RX1, timing_start);
}

void
CAN1_SCE_IRQHandler(void) {
    uint32_t timing_start = CANopenReferenceTiming_CanEnter(CANOPEN_REFERENCE_TIMING_CAN_ERROR);
    HAL_CAN_IRQHandler(&hcan1);
    CANopenReferenceTiming_CanExit(CANOPEN_REFERENCE_TIMING_CAN_ERROR, timing_start);
}
