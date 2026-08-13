/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TEST_FAKE_STM32F7XX_HAL_H
#define TEST_FAKE_STM32F7XX_HAL_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t unused;
} CAN_HandleTypeDef;

typedef struct {
    uint32_t StdId;
    uint32_t ExtId;
    uint32_t IDE;
    uint32_t RTR;
    uint32_t DLC;
    uint32_t TransmitGlobalTime;
} CAN_TxHeaderTypeDef;

typedef enum {
    HAL_OK = 0,
    HAL_ERROR = 1
} HAL_StatusTypeDef;

#define CAN_IT_RX_FIFO0_MSG_PENDING (1U << 0)
#define CAN_IT_RX_FIFO1_MSG_PENDING (1U << 1)
#define CAN_IT_TX_MAILBOX_EMPTY (1U << 2)
#define CAN_IT_ERROR (1U << 3)
#define CAN_ID_STD 0U
#define CAN_RTR_DATA 0U
#define DISABLE 0U

static inline HAL_StatusTypeDef
HAL_CAN_Start(CAN_HandleTypeDef *hcan) {
    (void)hcan;
    return HAL_OK;
}

static inline HAL_StatusTypeDef
HAL_CAN_Stop(CAN_HandleTypeDef *hcan) {
    (void)hcan;
    return HAL_OK;
}

static inline HAL_StatusTypeDef
HAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t notifications) {
    (void)hcan;
    (void)notifications;
    return HAL_OK;
}

static inline HAL_StatusTypeDef
HAL_CAN_DeactivateNotification(CAN_HandleTypeDef *hcan, uint32_t notifications) {
    (void)hcan;
    (void)notifications;
    return HAL_OK;
}

static inline uint32_t
HAL_CAN_GetTxMailboxesFreeLevel(CAN_HandleTypeDef *hcan) {
    (void)hcan;
    return 1U;
}

static inline HAL_StatusTypeDef
HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan,
                     CAN_TxHeaderTypeDef *header,
                     uint8_t *data,
                     uint32_t *mailbox) {
    (void)hcan;
    (void)header;
    (void)data;
    if (mailbox != NULL) {
        *mailbox = 0U;
    }
    return HAL_OK;
}

#endif /* TEST_FAKE_STM32F7XX_HAL_H */
