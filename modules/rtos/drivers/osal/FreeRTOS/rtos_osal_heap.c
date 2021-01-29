// Copyright (c) 2021, XMOS Ltd, All rights reserved

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
