// Copyright (c) 2021, XMOS Ltd, All rights reserved

#ifndef RTOS_OSAL_H_
#define RTOS_OSAL_H_

#include "rtos_osal_port.h"

#define RTOS_OSAL_WAIT_MS(ms)   RTOS_OSAL_PORT_WAIT_MS(ms)
#define RTOS_OSAL_WAIT_FOREVER  RTOS_OSAL_PORT_WAIT_FOREVER
#define RTOS_OSAL_NO_WAIT       RTOS_OSAL_PORT_NO_WAIT

int rtos_osal_critical_enter(void);
void rtos_osal_critical_exit(int state);

void *rtos_osal_malloc(size_t size);
void rtos_osal_free(void *ptr);

typedef enum {
    RTOS_OSAL_SUCCESS,
    RTOS_OSAL_ERROR,
    RTOS_OSAL_TIMEOUT
} rtos_osal_status_t;

typedef void (*rtos_osal_entry_function_t)(void *);
typedef struct rtos_osal_thread_struct rtos_osal_thread_t;
rtos_osal_status_t rtos_osal_thread_create(
        rtos_osal_thread_t *thread,
        char *name,
        rtos_osal_entry_function_t entry_function,
        void *entry_input,
        size_t stack_word_size,
        unsigned int priority);
rtos_osal_status_t rtos_osal_thread_core_exclusion_set(rtos_osal_thread_t *thread, uint32_t core_map);
rtos_osal_status_t rtos_osal_thread_preemption_disable(rtos_osal_thread_t *thread);
rtos_osal_status_t rtos_osal_thread_priority_set(rtos_osal_thread_t *thread, unsigned int priority);
rtos_osal_status_t rtos_osal_thread_priority_get(rtos_osal_thread_t *thread, unsigned int *priority);
rtos_osal_status_t rtos_osal_thread_delete(rtos_osal_thread_t *thread);


typedef struct rtos_osal_mutex_struct rtos_osal_mutex_t;
rtos_osal_status_t rtos_osal_mutex_create(rtos_osal_mutex_t *mutex, char *name, int recursive);
rtos_osal_status_t rtos_osal_mutex_get(rtos_osal_mutex_t *mutex, unsigned timeout);
rtos_osal_status_t rtos_osal_mutex_put(rtos_osal_mutex_t *mutex);
rtos_osal_status_t rtos_osal_mutex_delete(rtos_osal_mutex_t *mutex);

#endif /* RTOS_OSAL_H_ */
