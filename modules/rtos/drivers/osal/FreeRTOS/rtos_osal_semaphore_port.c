// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * This is the RTOS OS abstraction layer for FreeRTOS
 */

#include "rtos_osal.h"

rtos_osal_status_t rtos_osal_semaphore_create(rtos_osal_semaphore_t *semaphore, char *name, unsigned max_count, unsigned initial_count)
{
    (void) name;

    semaphore->semaphore = xSemaphoreCreateCounting(max_count, initial_count);
    if (semaphore->semaphore != NULL) {

        #if configQUEUE_REGISTRY_SIZE > 0
            if (name != NULL) {
                taskENTER_CRITICAL();
                vQueueAddToRegistry(semaphore->semaphore, name);
                taskEXIT_CRITICAL();
            }
        #endif

        #if defined(TRC_USE_TRACEALYZER_RECORDER) && TRC_USE_TRACEALYZER_RECORDER
            vTraceSetSemaphoreName(semaphore->semaphore, name);
        #endif

        return RTOS_OSAL_SUCCESS;
    } else {
        return RTOS_OSAL_ERROR;
    }
}

rtos_osal_status_t rtos_osal_semaphore_put(rtos_osal_semaphore_t *semaphore)
{
    BaseType_t status;

    if (portCHECK_IF_IN_ISR()) {
        BaseType_t yield_required = pdFALSE;
        status = xSemaphoreGiveFromISR(semaphore->semaphore, &yield_required);
        portYIELD_FROM_ISR(yield_required);
    } else {
        status = xSemaphoreGive(semaphore->semaphore);
    }

    return status == pdFALSE ? RTOS_OSAL_ERROR : RTOS_OSAL_SUCCESS;
}

rtos_osal_status_t rtos_osal_semaphore_get(rtos_osal_semaphore_t *semaphore, unsigned timeout)
{
    BaseType_t status;

    if (portCHECK_IF_IN_ISR()) {
        if (timeout == RTOS_OSAL_PORT_NO_WAIT) {
            BaseType_t yield_required = pdFALSE;
            status = xSemaphoreTakeFromISR(semaphore->semaphore, &yield_required);
            portYIELD_FROM_ISR(yield_required);
        } else {
            status = pdFALSE;
        }
    } else {
        status = xSemaphoreTake(semaphore->semaphore, timeout);
    }

    return status == pdFALSE ? RTOS_OSAL_TIMEOUT : RTOS_OSAL_SUCCESS;
}

rtos_osal_status_t rtos_osal_semaphore_delete(rtos_osal_semaphore_t *semaphore)
{
    vSemaphoreDelete(semaphore->semaphore);

    return RTOS_OSAL_SUCCESS;
}
