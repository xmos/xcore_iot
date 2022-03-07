// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_OSAL_H_
#define RTOS_OSAL_H_

#include "rtos_osal_port.h"

/*
 * Macros
 */
#define RTOS_OSAL_WAIT_MS(ms)   RTOS_OSAL_PORT_WAIT_MS(ms)
#define RTOS_OSAL_WAIT_FOREVER  RTOS_OSAL_PORT_WAIT_FOREVER
#define RTOS_OSAL_NO_WAIT       RTOS_OSAL_PORT_NO_WAIT

#define RTOS_OSAL_HIGHEST_PRIORITY RTOS_OSAL_PORT_HIGHEST_PRIORITY

#define RTOS_OSAL_OR         RTOS_OSAL_PORT_OR
#define RTOS_OSAL_OR_CLEAR   RTOS_OSAL_PORT_OR_CLEAR
#define RTOS_OSAL_AND        RTOS_OSAL_PORT_AND
#define RTOS_OSAL_AND_CLEAR  RTOS_OSAL_PORT_AND_CLEAR

#define RTOS_OSAL_NOT_RECURSIVE 0
#define RTOS_OSAL_RECURSIVE     1

/*
 * Return statuses
 */
typedef enum {
    RTOS_OSAL_SUCCESS,
    RTOS_OSAL_ERROR,
    RTOS_OSAL_TIMEOUT
} rtos_osal_status_t;

/*
 * Critical sections
 */
int rtos_osal_critical_enter(void);
void rtos_osal_critical_exit(int state);

/*
 * Memory management
 */
void *rtos_osal_malloc(size_t size);
void rtos_osal_free(void *ptr);

/*
 * Time
 */
rtos_osal_tick_t rtos_osal_tick_get(void);
void rtos_osal_delay(unsigned ticks);

/*
 * Thread management
 */
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
rtos_osal_status_t rtos_osal_thread_core_exclusion_get(rtos_osal_thread_t *thread, uint32_t *core_map);
rtos_osal_status_t rtos_osal_thread_preemption_disable(rtos_osal_thread_t *thread);
rtos_osal_status_t rtos_osal_thread_priority_set(rtos_osal_thread_t *thread, unsigned int priority);
rtos_osal_status_t rtos_osal_thread_priority_get(rtos_osal_thread_t *thread, unsigned int *priority);
rtos_osal_status_t rtos_osal_thread_delete(rtos_osal_thread_t *thread);
rtos_osal_status_t rtos_osal_task_yield(void);

/*
 * Mutexes
 */
typedef struct rtos_osal_mutex_struct rtos_osal_mutex_t;
rtos_osal_status_t rtos_osal_mutex_create(rtos_osal_mutex_t *mutex, char *name, int recursive);
rtos_osal_status_t rtos_osal_mutex_put(rtos_osal_mutex_t *mutex);
rtos_osal_status_t rtos_osal_mutex_get(rtos_osal_mutex_t *mutex, unsigned timeout);
rtos_osal_status_t rtos_osal_mutex_delete(rtos_osal_mutex_t *mutex);

/*
 * Semaphores
 */
typedef struct rtos_osal_semaphore_struct rtos_osal_semaphore_t;
rtos_osal_status_t rtos_osal_semaphore_create(rtos_osal_semaphore_t *semaphore, char *name, unsigned max_count, unsigned initial_count);
rtos_osal_status_t rtos_osal_semaphore_put(rtos_osal_semaphore_t *semaphore);
rtos_osal_status_t rtos_osal_semaphore_get(rtos_osal_semaphore_t *semaphore, unsigned timeout);
rtos_osal_status_t rtos_osal_semaphore_delete(rtos_osal_semaphore_t *semaphore);

/*
 * Queues
 */
typedef struct rtos_osal_queue_struct rtos_osal_queue_t;
rtos_osal_status_t rtos_osal_queue_create(rtos_osal_queue_t *queue, char *name, size_t queue_length, size_t item_size);
rtos_osal_status_t rtos_osal_queue_send(rtos_osal_queue_t *queue, const void *item, unsigned timeout);
rtos_osal_status_t rtos_osal_queue_receive(rtos_osal_queue_t *queue, void *item, unsigned timeout);
rtos_osal_status_t rtos_osal_queue_delete(rtos_osal_queue_t *queue);


/*
 * Event groups
 */
typedef struct rtos_osal_event_group_struct rtos_osal_event_group_t;
rtos_osal_status_t rtos_osal_event_group_create(rtos_osal_event_group_t *group, char *name);
rtos_osal_status_t rtos_osal_event_group_set_bits(
        rtos_osal_event_group_t *group,
        uint32_t flags_to_set);
rtos_osal_status_t rtos_osal_event_group_clear_bits(
        rtos_osal_event_group_t *group,
        uint32_t flags_to_clear);
rtos_osal_status_t rtos_osal_event_group_get_bits(
        rtos_osal_event_group_t *group,
        uint32_t requested_flags,
        unsigned get_option,
        uint32_t *actual_flags_ptr,
        unsigned timeout);
rtos_osal_status_t rtos_osal_event_group_delete(rtos_osal_event_group_t *group);

#endif /* RTOS_OSAL_H_ */
