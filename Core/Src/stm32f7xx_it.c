/* SPDX-License-Identifier: Apache-2.0 */
#include "main.h"

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
    HAL_TIM_IRQHandler(&htim7);
}

void
CAN1_TX_IRQHandler(void) {
    HAL_CAN_IRQHandler(&hcan1);
}

void
CAN1_RX0_IRQHandler(void) {
    HAL_CAN_IRQHandler(&hcan1);
}

void
CAN1_RX1_IRQHandler(void) {
    HAL_CAN_IRQHandler(&hcan1);
}

void
CAN1_SCE_IRQHandler(void) {
    HAL_CAN_IRQHandler(&hcan1);
}
