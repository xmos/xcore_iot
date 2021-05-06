// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

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

#define RTOS_OSAL_PORT_HIGHEST_PRIORITY (configMAX_PRIORITIES - 1)

#define RTOS_OSAL_PORT_CLEAR      1
#define RTOS_OSAL_PORT_OR         0
#define RTOS_OSAL_PORT_OR_CLEAR   (RTOS_OSAL_PORT_OR | RTOS_OSAL_PORT_CLEAR)
#define RTOS_OSAL_PORT_AND        2
#define RTOS_OSAL_PORT_AND_CLEAR  (RTOS_OSAL_PORT_AND | RTOS_OSAL_PORT_CLEAR)

typedef TickType_t rtos_osal_tick_t;

struct rtos_osal_thread_struct {
    TaskHandle_t thread;
};

struct rtos_osal_mutex_struct {
    SemaphoreHandle_t mutex;
    int recursive;
};

struct rtos_osal_semaphore_struct {
    SemaphoreHandle_t semaphore;
};

struct rtos_osal_queue_struct {
    QueueHandle_t queue;
};

struct rtos_osal_event_group_struct {
    EventGroupHandle_t group;
};

#endif /* RTOS_OSAL_PORT_H_ */
