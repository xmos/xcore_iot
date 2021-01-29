// Copyright (c) 2021, XMOS Ltd, All rights reserved

/**
 * This is the RTOS OS abstraction layer for ThreadX
 */

#include "rtos_osal.h"

int rtos_osal_critical_enter(void)
{
    TX_INTERRUPT_SAVE_AREA;

    TX_DISABLE;

    return interrupt_save;
}

void rtos_osal_critical_exit(int interrupt_save)
{
    TX_RESTORE;
}

rtos_osal_status_t rtos_osal_mutex_create(rtos_osal_mutex_t *mutex, int recursive)
{
    UINT status;

    (void) recursive;

    status = tx_mutex_create(&mutex->mutex, name, TX_INHERIT);

    return status == TX_SUCCESS ? RTOS_OSAL_SUCCESS : RTOS_OSAL_ERROR;
}

rtos_osal_status_t rtos_osal_mutex_get(rtos_osal_mutex_t *mutex, unsigned timeout)
{
    UINT status;

    status = tx_mutex_get(&mutex->mutex, timeout);

    return status == TX_SUCCESS ? RTOS_OSAL_SUCCESS : (status == TX_NOT_AVAILABLE ? RTOS_OSAL_TIMEOUT : RTOS_OSAL_ERROR);
}

rtos_osal_status_t rtos_osal_mutex_put(rtos_osal_mutex_t *mutex)
{
    BaseType_t status;

    status = tx_mutex_put(&mutex->mutex);

    return status == TX_SUCCESS ? RTOS_OSAL_SUCCESS : RTOS_OSAL_ERROR;
}

rtos_osal_status_t rtos_osal_mutex_delete(rtos_osal_mutex_t *mutex)
{
    BaseType_t status;

    status = tx_mutex_delete(&mutex->mutex);

    return status == TX_SUCCESS ? RTOS_OSAL_SUCCESS : RTOS_OSAL_ERROR;
}
