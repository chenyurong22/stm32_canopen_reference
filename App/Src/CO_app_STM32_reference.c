/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Project-owned runtime wrapper derived from the CANopenNode STM32 integration
 * model. Compile this file instead of CANopenNode_STM32/CO_app_STM32.c.
 */
#include "CO_app_STM32.h"

#include <stdbool.h>

#include "CANopen.h"
#include "OD.h"
#include "canopen_reference_config.h"
#include "canopen_reference_cia302.h"
#include "canopen_reference_diagnostics.h"
#include "canopen_reference_gateway.h"
#include "canopen_reference_lss.h"
#include "canopen_reference_storage.h"
#include "cia401_reference.h"
#include "cia402_reference.h"

#define CANOPEN_REFERENCE_NMT_CONTROL \
    (CO_NMT_STARTUP_TO_OPERATIONAL | CO_NMT_ERR_ON_ERR_REG | CO_ERR_REG_GENERIC_ERR | CO_ERR_REG_COMMUNICATION)
#define CANOPEN_REFERENCE_FIRST_HB_MS        500U
#define CANOPEN_REFERENCE_SDO_SRV_TIMEOUT_MS 1000U
#define CANOPEN_REFERENCE_SDO_CLI_TIMEOUT_MS 1000U

CANopenNodeSTM32 *canopenNodeSTM32 = NULL;
CO_t *CO = NULL;

static uint32_t canopenReferenceLastTick;
static CANopenReferenceLssState canopenReferenceLssState;

static void
CANopenReference_ApplyIdentity(void) {
    OD_PERSIST_COMM.x1018_identity.vendor_ID = CANOPEN_REFERENCE_VENDOR_ID;
    OD_PERSIST_COMM.x1018_identity.productCode = CANOPEN_REFERENCE_PRODUCT_CODE;
    OD_PERSIST_COMM.x1018_identity.revisionNumber = CANOPEN_REFERENCE_REVISION;
    OD_PERSIST_COMM.x1018_identity.serialNumber = CANOPEN_REFERENCE_SERIAL;
    OD_PERSIST_COMM.x1017_producerHeartbeatTime = CANOPEN_REFERENCE_HEARTBEAT_MS;
}

static void
CANopenReference_ForceSafeApplication(void) {
    Cia401Reference_ForceSafeOutputs();
    Cia402Reference_ForceDisable();
}

int
canopen_app_init(CANopenNodeSTM32 *instance) {
    uint32_t heapMemoryUsed = 0U;

    if (instance == NULL || instance->CANHandle == NULL || instance->timerHandle == NULL
        || instance->HWInitFunction == NULL) {
        return -1;
    }

    canopenNodeSTM32 = instance;
    CANopenReference_ApplyIdentity();
    CANopenReference_ForceSafeApplication();

    CO = CO_new(NULL, &heapMemoryUsed);
    (void)heapMemoryUsed;
    if (CO == NULL) {
        CANopenReference_ForceSafeApplication();
        return -2;
    }
    canopenNodeSTM32->canOpenStack = CO;

    if (canopen_app_resetCommunication() != 0) {
        CANopenReference_ForceSafeApplication();
        CO_delete(CO);
        CO = NULL;
        canopenNodeSTM32->canOpenStack = NULL;
        return -3;
    }
    return 0;
}

int
canopen_app_resetCommunication(void) {
    CO_ReturnError_t error;
    uint32_t errorInfo = 0U;
    CO_LSS_address_t lssAddress;

    if (CO == NULL || canopenNodeSTM32 == NULL) {
        return -1;
    }

    CANopenReference_ForceSafeApplication();
    CANopenReferenceCia302_Deinit();
    CO->CANmodule->CANnormal = false;
    CO_CANsetConfigurationMode((void *)canopenNodeSTM32);
    CO_CANmodule_disable(CO->CANmodule);

    error = CO_CANinit(CO, canopenNodeSTM32, 0U);
    if (error != CO_ERROR_NO) {
        return -2;
    }
#if defined(CAN_IT_ERROR)
    if (HAL_CAN_ActivateNotification(canopenNodeSTM32->CANHandle,
                                     CAN_IT_ERROR | CAN_IT_ERROR_WARNING | CAN_IT_ERROR_PASSIVE | CAN_IT_BUSOFF
                                         | CAN_IT_LAST_ERROR_CODE)
        != HAL_OK) {
        return -2;
    }
#endif
    if (CANopenReferenceStorage_Init(CO) != CO_ERROR_NO) {
        return -2;
    }

    lssAddress.identity.vendorID = OD_PERSIST_COMM.x1018_identity.vendor_ID;
    lssAddress.identity.productCode = OD_PERSIST_COMM.x1018_identity.productCode;
    lssAddress.identity.revisionNumber = OD_PERSIST_COMM.x1018_identity.revisionNumber;
    lssAddress.identity.serialNumber = OD_PERSIST_COMM.x1018_identity.serialNumber;
    error = CO_LSSinit(CO, &lssAddress, &canopenNodeSTM32->desiredNodeID, &canopenNodeSTM32->baudrate);
    if (error != CO_ERROR_NO) {
        return -3;
    }
    CANopenReferenceLss_Init(CO, &canopenReferenceLssState);
    CANopenReferenceCia302_PrepareOd();

    canopenNodeSTM32->activeNodeID = canopenNodeSTM32->desiredNodeID;
    error = CO_CANopenInit(CO, NULL, NULL, OD, NULL, CANOPEN_REFERENCE_NMT_CONTROL,
                           CANOPEN_REFERENCE_FIRST_HB_MS, CANOPEN_REFERENCE_SDO_SRV_TIMEOUT_MS,
                           CANOPEN_REFERENCE_SDO_CLI_TIMEOUT_MS, true, canopenNodeSTM32->activeNodeID, &errorInfo);
    if (error != CO_ERROR_NO && error != CO_ERROR_NODE_ID_UNCONFIGURED_LSS) {
        (void)errorInfo;
        return -4;
    }

    CANopenReferenceGateway_Init(CO);
    CANopenReferenceCia302_Init(CO, canopenNodeSTM32->activeNodeID, HAL_GetTick());

    error = CO_CANopenInitPDO(CO, CO->em, OD, canopenNodeSTM32->activeNodeID, &errorInfo);
    if (error != CO_ERROR_NO && error != CO_ERROR_NODE_ID_UNCONFIGURED_LSS) {
        (void)errorInfo;
        return -5;
    }

    Cia401Reference_Init();
    Cia402Reference_Init();

    if (HAL_TIM_Base_Start_IT(canopenNodeSTM32->timerHandle) != HAL_OK) {
        CANopenReference_ForceSafeApplication();
        return -6;
    }

    CO_CANsetNormalMode(CO->CANmodule);
    canopenReferenceLastTick = HAL_GetTick();
    return 0;
}

void
canopen_app_process(void) {
    uint32_t now;
    uint32_t elapsedUs;
    CO_NMT_reset_cmd_t resetCommand;

    if (CO == NULL || canopenNodeSTM32 == NULL) {
        return;
    }

    now = HAL_GetTick();
    if (now == canopenReferenceLastTick) {
        return;
    }
    elapsedUs = (now - canopenReferenceLastTick) * 1000U;
    canopenReferenceLastTick = now;

    CANopenReferenceCia302_PreProcess(now);
    resetCommand = CO_process(CO, CANopenReferenceGateway_Authorized(), elapsedUs, NULL);
    CANopenReferenceCia302_Process(now);
    canopenNodeSTM32->outStatusLEDRed = CO_LED_RED(CO->LEDs, CO_LED_CANopen);
    canopenNodeSTM32->outStatusLEDGreen = CO_LED_GREEN(CO->LEDs, CO_LED_CANopen);
    CANopenReferenceDiagnostics_Process(canopenNodeSTM32->activeNodeID, CO->CANmodule->CANerrorStatus,
                                        canopenNodeSTM32->outStatusLEDGreen, canopenNodeSTM32->outStatusLEDRed, now);

    if (resetCommand == CO_RESET_COMM) {
        (void)HAL_TIM_Base_Stop_IT(canopenNodeSTM32->timerHandle);
        CO_CANsetConfigurationMode((void *)canopenNodeSTM32);
        CO_delete(CO);
        CO = NULL;
        canopenNodeSTM32->canOpenStack = NULL;
        (void)canopen_app_init(canopenNodeSTM32);
    } else if (resetCommand == CO_RESET_APP) {
        CANopenReference_ForceSafeApplication();
        HAL_NVIC_SystemReset();
    }
}

void
canopen_app_interrupt(void) {
    bool_t syncWas = false;

    if (CO == NULL || canopenNodeSTM32 == NULL || CO->nodeIdUnconfigured || !CO->CANmodule->CANnormal) {
        return;
    }

    CO_LOCK_OD(CO->CANmodule);
#if (CO_CONFIG_SYNC) & CO_CONFIG_SYNC_ENABLE
    syncWas = CO_process_SYNC(CO, 1000U, NULL);
#endif
#if (CO_CONFIG_PDO) & CO_CONFIG_RPDO_ENABLE
    CO_process_RPDO(CO, syncWas, 1000U, NULL);
#endif

    /* Bounded application work occurs after commands enter the OD and before
     * status/inputs are packed into TPDOs. No blocking driver, flash, or
     * printf operation is permitted in this interrupt context. */
    Cia401Reference_Process1ms();
    Cia402Reference_Process1ms();

#if (CO_CONFIG_PDO) & CO_CONFIG_TPDO_ENABLE
    CO_process_TPDO(CO, syncWas, 1000U, NULL);
#endif
    CO_UNLOCK_OD(CO->CANmodule);
}


void
HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
    uint32_t hal_error;

    if (hcan == NULL || canopenNodeSTM32 == NULL || hcan != canopenNodeSTM32->CANHandle) {
        return;
    }

    hal_error = HAL_CAN_GetError(hcan);
    CANopenReferenceDiagnostics_ReportCanHardwareError(hal_error);
    if (CO != NULL && CO->CANmodule != NULL) {
        if ((hal_error & HAL_CAN_ERROR_BOF) != 0U) {
            CO->CANmodule->CANerrorStatus |= CO_CAN_ERRTX_BUS_OFF;
        }
        if ((hal_error & (HAL_CAN_ERROR_ACK | HAL_CAN_ERROR_TIMEOUT)) != 0U) {
            CO->CANmodule->CANerrorStatus |= CO_CAN_ERRTX_WARNING;
        }
        if ((hal_error & (HAL_CAN_ERROR_RX_FOV0 | HAL_CAN_ERROR_RX_FOV1)) != 0U) {
            CO->CANmodule->CANerrorStatus |= CO_CAN_ERRRX_OVERFLOW;
        }
        if ((hal_error & (HAL_CAN_ERROR_STF | HAL_CAN_ERROR_FOR | HAL_CAN_ERROR_ACK)) != 0U) {
            CO->CANmodule->CANerrorStatus |= CO_CAN_ERRRX_WARNING;
        }
    }
}
