// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

/**
 * This is the RTOS OS abstraction layer for ThreadX
 */

#ifndef RTOS_OSAL_PORT_H_
#define RTOS_OSAL_PORT_H_

#include "tx_api.h"

#define RTOS_OSAL_PORT_WAIT_MS(ms)   ((ULONG) (((ULONG) (ms) * (ULONG) TX_TIMER_TICKS_PER_SECOND) / (ULONG) 1000))
#define RTOS_OSAL_PORT_WAIT_FOREVER  TX_WAIT_FOREVER
#define RTOS_OSAL_PORT_NO_WAIT       TX_NO_WAIT

struct rtos_osal_mutex_struct {
    TX_MUTEX mutex;
};


#endif /* RTOS_OSAL_PORT_H_ */
