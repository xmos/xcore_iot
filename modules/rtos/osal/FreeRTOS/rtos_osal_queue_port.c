// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * This is the RTOS OS abstraction layer for FreeRTOS
 */

#include "rtos_osal.h"

rtos_osal_status_t rtos_osal_queue_create(rtos_osal_queue_t *queue, char *name, size_t queue_length, size_t item_size)
{
    (void) name;

    queue->queue = xQueueCreate(queue_length, item_size);
    if (queue->queue != NULL) {

        #if configQUEUE_REGISTRY_SIZE > 0
            if (name != NULL) {
                taskENTER_CRITICAL();
                vQueueAddToRegistry(queue->queue, name);
                taskEXIT_CRITICAL();
            }
        #endif

        #if defined(TRC_USE_TRACEALYZER_RECORDER) && TRC_USE_TRACEALYZER_RECORDER
            vTraceSetQueueName(queue->queue, name);
        #endif

        return RTOS_OSAL_SUCCESS;
    } else {
        return RTOS_OSAL_ERROR;
    }
}

rtos_osal_status_t rtos_osal_queue_send(rtos_osal_queue_t *queue, const void *item, unsigned timeout)
{
    BaseType_t status;

    if (portCHECK_IF_IN_ISR()) {
        if (timeout == RTOS_OSAL_PORT_NO_WAIT) {
            BaseType_t yield_required = pdFALSE;
            status = xQueueSendFromISR(queue->queue, item, &yield_required);
            portYIELD_FROM_ISR(yield_required);
        } else {
            status = pdFALSE;
        }
    } else {
        status = xQueueSend(queue->queue, item, timeout);
    }

    return status == errQUEUE_FULL ? RTOS_OSAL_TIMEOUT : RTOS_OSAL_SUCCESS;
}

rtos_osal_status_t rtos_osal_queue_receive(rtos_osal_queue_t *queue, void *item, unsigned timeout)
{
    BaseType_t status;

    if (portCHECK_IF_IN_ISR()) {
        if (timeout == RTOS_OSAL_PORT_NO_WAIT) {
            BaseType_t yield_required = pdFALSE;
            status = xQueueReceiveFromISR(queue->queue, item, &yield_required);
            portYIELD_FROM_ISR(yield_required);
        } else {
            status = pdFALSE;
        }
    } else {
        status = xQueueReceive(queue->queue, item, timeout);
    }

    return status == errQUEUE_EMPTY ? RTOS_OSAL_TIMEOUT : RTOS_OSAL_SUCCESS;
}

rtos_osal_status_t rtos_osal_queue_delete(rtos_osal_queue_t *queue)
{
    vQueueDelete(queue->queue);

    return RTOS_OSAL_SUCCESS;
}
