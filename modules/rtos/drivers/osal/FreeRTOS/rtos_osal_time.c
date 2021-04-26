// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * This is the RTOS OS abstraction layer for FreeRTOS
 */

#include "rtos_osal.h"

rtos_osal_tick_t rtos_osal_tick_get(void)
{
    if (portCHECK_IF_IN_ISR()) {
        return xTaskGetTickCountFromISR();
    } else {
        return xTaskGetTickCount();
    }
}

void rtos_osal_delay(unsigned ticks)
{
    vTaskDelay(ticks);
}
