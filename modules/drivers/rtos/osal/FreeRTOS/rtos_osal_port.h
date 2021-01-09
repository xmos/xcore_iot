// Copyright (c) 2021, XMOS Ltd, All rights reserved

/**
 * This is the RTOS OS abstraction layer for FreeRTOS
 */

#ifndef RTOS_OSAL_PORT_H_
#define RTOS_OSAL_PORT_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "event_groups.h"
#include "timers.h"

#define RTOS_OSAL_PORT_WAIT_MS(ms)   pdMS_TO_TICKS(ms)
#define RTOS_OSAL_PORT_WAIT_FOREVER  portMAX_DELAY
#define RTOS_OSAL_PORT_NO_WAIT       0

struct rtos_osal_thread_struct {
    TaskHandle_t thread;
};

struct rtos_osal_mutex_struct {
    SemaphoreHandle_t mutex;
    int recursive;
};

#endif /* RTOS_OSAL_PORT_H_ */
