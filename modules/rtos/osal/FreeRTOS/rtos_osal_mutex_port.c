// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * This is the RTOS OS abstraction layer for FreeRTOS
 */

#include "rtos_osal.h"

rtos_osal_status_t rtos_osal_mutex_create(rtos_osal_mutex_t *mutex, char *name, int recursive)
{
    (void) name;

    mutex->recursive = recursive;

    if (recursive) {
        mutex->mutex = xSemaphoreCreateRecursiveMutex();
    } else {
        mutex->mutex = xSemaphoreCreateMutex();
    }

    if (mutex->mutex != NULL) {

        #if configQUEUE_REGISTRY_SIZE > 0
            if (name != NULL) {
                taskENTER_CRITICAL();
                vQueueAddToRegistry(mutex->mutex, name);
                taskEXIT_CRITICAL();
            }
        #endif

        #if defined(TRC_USE_TRACEALYZER_RECORDER) && TRC_USE_TRACEALYZER_RECORDER
            vTraceSetMutexName(mutex->mutex, name);
        #endif

        return RTOS_OSAL_SUCCESS;
    } else {
        return RTOS_OSAL_ERROR;
    }
}

rtos_osal_status_t rtos_osal_mutex_put(rtos_osal_mutex_t *mutex)
{
    BaseType_t status;

    if (mutex->recursive) {
        status = xSemaphoreGiveRecursive(mutex->mutex);
    } else {
        status = xSemaphoreGive(mutex->mutex);
    }

    return status == pdFALSE ? RTOS_OSAL_ERROR : RTOS_OSAL_SUCCESS;
}

rtos_osal_status_t rtos_osal_mutex_get(rtos_osal_mutex_t *mutex, unsigned timeout)
{
    BaseType_t status;

    if (mutex->recursive) {
        status = xSemaphoreTakeRecursive(mutex->mutex, timeout);
    } else {
        status = xSemaphoreTake(mutex->mutex, timeout);
    }

    return status == pdFALSE ? RTOS_OSAL_TIMEOUT : RTOS_OSAL_SUCCESS;
}

rtos_osal_status_t rtos_osal_mutex_delete(rtos_osal_mutex_t *mutex)
{
    vSemaphoreDelete(mutex->mutex);

    return RTOS_OSAL_SUCCESS;
}
