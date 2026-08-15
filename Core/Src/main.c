/* SPDX-License-Identifier: Apache-2.0 */
#include "main.h"

#include "CO_app_STM32.h"
#include "canopen_reference_board.h"
#include "canopen_reference_config.h"
#include "canopen_reference_timing.h"
#include "canopen_reference_watchdog.h"

CAN_HandleTypeDef hcan1;
TIM_HandleTypeDef htim7;
/* APB1 is divided by four from 216 MHz (54 MHz), and STM32F7 timer kernels
 * run at twice PCLK when the APB prescaler is not one. TIM7 therefore receives
 * 108 MHz and must divide by 108000 for the CANopen 1 ms interrupt cadence. */
#define CANOPEN_REFERENCE_TIM7_INPUT_HZ       108000000UL
#define CANOPEN_REFERENCE_TIM7_PRESCALER_DIV  108000UL
#define CANOPEN_REFERENCE_TIM7_PERIOD_TICKS   1UL
_Static_assert(CANOPEN_REFERENCE_TIM7_INPUT_HZ
                   / (CANOPEN_REFERENCE_TIM7_PRESCALER_DIV * CANOPEN_REFERENCE_TIM7_PERIOD_TICKS)
                   == 1000UL,
               "TIM7 must generate the CANopen 1 ms processing interrupt");

int
main(void) {
    CANopenNodeSTM32 canopenInstance;

    HAL_Init();
    SystemClock_Config();
    CANopenReferenceTiming_Init();
    MX_GPIO_Init();
    CANopenReferenceBoard_InitSafe();
    MX_CAN1_Init();
    MX_TIM7_Init();
    CANopenReferenceWatchdog_Init();

    canopenInstance.desiredNodeID = CANOPEN_REFERENCE_DEFAULT_NODE_ID;
    canopenInstance.activeNodeID = 0U;
    canopenInstance.baudrate = CANOPEN_REFERENCE_DEFAULT_BITRATE_KBPS;
    canopenInstance.timerHandle = &htim7;
    canopenInstance.CANHandle = &hcan1;
    canopenInstance.HWInitFunction = MX_CAN1_Init;
    canopenInstance.outStatusLEDGreen = 0U;
    canopenInstance.outStatusLEDRed = 0U;
    canopenInstance.canOpenStack = NULL;

    if (canopen_app_init(&canopenInstance) != 0) {
        Error_Handler();
    }
    CANopenReferenceBoard_OnCanopenReady();

    for (;;) {
        canopen_app_process();
        CANopenReferenceWatchdog_Process();
        /* Do not put blocking I/O here. A product may enter sleep only after it
         * has evaluated CAN reception, timer, latency, and wake-up guarantees. */
    }
}

/* This is deliberately composed with HAL's tick callback rather than replaced
 * by a library-owned handler. CubeMX regeneration must preserve this function
 * in a USER CODE section or an application callback source file. */
void
HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM7) {
        CANopenReferenceWatchdog_TickISR();
        canopen_app_interrupt();
    }
}

void
MX_CAN1_Init(void) {
    hcan1.Instance = CAN1;
    hcan1.Init.Prescaler = 6U;
    hcan1.Init.Mode = CAN_MODE_NORMAL;
    hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
    hcan1.Init.TimeSeg1 = CAN_BS1_14TQ;
    hcan1.Init.TimeSeg2 = CAN_BS2_3TQ;
    hcan1.Init.TimeTriggeredMode = DISABLE;
    hcan1.Init.AutoBusOff = ENABLE;
    hcan1.Init.AutoWakeUp = DISABLE;
    hcan1.Init.AutoRetransmission = ENABLE;
    hcan1.Init.ReceiveFifoLocked = DISABLE;
    hcan1.Init.TransmitFifoPriority = DISABLE;
    if (HAL_CAN_Init(&hcan1) != HAL_OK) {
        Error_Handler();
    }
}

void
MX_TIM7_Init(void) {
    TIM_MasterConfigTypeDef masterConfig = {0};

    htim7.Instance = TIM7;
    htim7.Init.Prescaler = CANOPEN_REFERENCE_TIM7_PRESCALER_DIV - 1U;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = CANOPEN_REFERENCE_TIM7_PERIOD_TICKS - 1U;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim7) != HAL_OK) {
        Error_Handler();
    }
    masterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    masterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &masterConfig) != HAL_OK) {
        Error_Handler();
    }
}

void
MX_GPIO_Init(void) {
    /* GPIO clocks, CAN transceiver enable/standby pins, and indicators belong
     * in the board-specific MSP and hardware adapter sources. */
}

void
Error_Handler(void) {
    CANopenReferenceBoard_ForceSafe();
    __disable_irq();
    for (;;) {
        /* A production design must bring outputs to their independent safe state
         * before or during reset. */
    }
}
