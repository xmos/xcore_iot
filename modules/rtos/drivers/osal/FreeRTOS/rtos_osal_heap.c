// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * This is the RTOS OS abstraction layer for FreeRTOS
 */

#include "rtos_osal.h"

void *rtos_osal_malloc(size_t size)
{
    return pvPortMalloc(size);
}

void rtos_osal_free(void *ptr)
{
    vPortFree(ptr);
}
