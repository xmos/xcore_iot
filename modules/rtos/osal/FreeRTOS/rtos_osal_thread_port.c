// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * This is the RTOS OS abstraction layer for FreeRTOS
 */

#include "rtos_osal.h"

int rtos_osal_critical_enter(void)
{
    int state;

    if (portCHECK_IF_IN_ISR()) {
        state = taskENTER_CRITICAL_FROM_ISR();
    } else {
        taskENTER_CRITICAL();
        state = 0;
    }

    return state;
}

void rtos_osal_critical_exit(int state)
{
    if (portCHECK_IF_IN_ISR()) {
        taskEXIT_CRITICAL_FROM_ISR(state);
    } else {
        taskEXIT_CRITICAL();
    }
}

#define thread_handle(thread) (thread != NULL ? thread->thread : NULL)
#define thread_handle_ptr(thread) (thread != NULL ? &thread->thread : NULL)

rtos_osal_status_t rtos_osal_thread_create(
        rtos_osal_thread_t *thread,
        char *name,
        rtos_osal_entry_function_t entry_function,
        void *entry_input,
        size_t stack_word_size,
        unsigned int priority)
{
    BaseType_t status;

    status = xTaskCreate(entry_function, name, stack_word_size, entry_input, priority, thread_handle_ptr(thread));

    return status == pdPASS ? RTOS_OSAL_SUCCESS : RTOS_OSAL_TIMEOUT;
}

rtos_osal_status_t rtos_osal_thread_core_exclusion_set(rtos_osal_thread_t *thread, uint32_t core_map)
{
    vTaskCoreAffinitySet(thread_handle(thread), ~core_map);

    return RTOS_OSAL_SUCCESS;
}

rtos_osal_status_t rtos_osal_thread_core_exclusion_get(rtos_osal_thread_t *thread, uint32_t *core_map)
{
    uint32_t affinity_core_map;

    affinity_core_map = vTaskCoreAffinityGet(thread_handle(thread));
    *core_map = ~affinity_core_map;

    return RTOS_OSAL_SUCCESS;
}

rtos_osal_status_t rtos_osal_thread_preemption_disable(rtos_osal_thread_t *thread)
{
    vTaskPreemptionDisable(thread_handle(thread));

    return RTOS_OSAL_SUCCESS;
}

rtos_osal_status_t rtos_osal_thread_preemption_enable(rtos_osal_thread_t *thread)
{
    vTaskPreemptionEnable(thread_handle(thread));

    return RTOS_OSAL_SUCCESS;
}

rtos_osal_status_t rtos_osal_thread_priority_set(rtos_osal_thread_t *thread, unsigned int priority)
{
    vTaskPrioritySet(thread_handle(thread), priority);

    return RTOS_OSAL_SUCCESS;
}

rtos_osal_status_t rtos_osal_thread_priority_get(rtos_osal_thread_t *thread, unsigned int *priority)
{
    *priority = uxTaskPriorityGet(thread_handle(thread));

    return RTOS_OSAL_SUCCESS;
}
rtos_osal_status_t rtos_osal_thread_delete(rtos_osal_thread_t *thread)
{
    vTaskDelete(thread_handle(thread));

    return RTOS_OSAL_SUCCESS;
}

rtos_osal_status_t rtos_osal_task_yield(void)
{
    taskYIELD();

    return RTOS_OSAL_SUCCESS;
}
