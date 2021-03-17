// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * This is the RTOS OS abstraction layer for FreeRTOS
 */

#include "rtos_osal.h"

rtos_osal_status_t rtos_osal_event_group_create(rtos_osal_event_group_t *group, char *name)
{
    (void) name;

    group->group = xEventGroupCreate();
    if (group->group != NULL) {

        #if defined(TRC_USE_TRACEALYZER_RECORDER) && TRC_USE_TRACEALYZER_RECORDER
            vTraceSetEventGroupName(group->group, name);
        #endif

        return RTOS_OSAL_SUCCESS;
    } else {
        return RTOS_OSAL_ERROR;
    }
}

rtos_osal_status_t rtos_osal_event_group_set_bits(
        rtos_osal_event_group_t *group,
        uint32_t flags_to_set)
{
    BaseType_t status = pdTRUE;

    if (portCHECK_IF_IN_ISR()) {
        BaseType_t yield_required = pdFALSE;
        status = xEventGroupSetBitsFromISR(group->group, flags_to_set, &yield_required);
        portYIELD_FROM_ISR(yield_required);
    } else {
        (void) xEventGroupSetBits(group->group, flags_to_set);
    }

    return status == pdFALSE ? RTOS_OSAL_ERROR : RTOS_OSAL_SUCCESS;
}

rtos_osal_status_t rtos_osal_event_group_clear_bits(
        rtos_osal_event_group_t *group,
        uint32_t flags_to_clear)
{
    BaseType_t status = pdTRUE;

    if (portCHECK_IF_IN_ISR()) {
        status = xEventGroupClearBitsFromISR(group->group, flags_to_clear);
    } else {
        (void) xEventGroupClearBits(group->group, flags_to_clear);
    }

    return status == pdFALSE ? RTOS_OSAL_ERROR : RTOS_OSAL_SUCCESS;
}

rtos_osal_status_t rtos_osal_event_group_get_bits(
        rtos_osal_event_group_t *group,
        uint32_t requested_flags,
        unsigned get_option,
        uint32_t *actual_flags_ptr,
        unsigned timeout)
{
    EventBits_t actual_flags;
    const unsigned clear = (get_option & RTOS_OSAL_PORT_CLEAR) != 0;
    const unsigned and = (get_option & RTOS_OSAL_PORT_AND) != 0;

    if (portCHECK_IF_IN_ISR()) {

        if (timeout == RTOS_OSAL_PORT_NO_WAIT) {
            actual_flags = xEventGroupGetBitsFromISR(group->group);
            *actual_flags_ptr = actual_flags;
            actual_flags &= requested_flags;
            if (clear && (actual_flags != 0)) {
                if (!and || (actual_flags == requested_flags)) {
                    if (xEventGroupClearBitsFromISR(group->group, actual_flags) == pdFALSE) {
                        return RTOS_OSAL_ERROR;
                    }
                }
            }
        } else {
            return RTOS_OSAL_ERROR;
        }

    } else {
        actual_flags = xEventGroupWaitBits(
                group->group,
                requested_flags,
                clear,
                and,
                timeout);

        *actual_flags_ptr = actual_flags;

        actual_flags &= requested_flags;
    }

    if (requested_flags == 0) {
        return RTOS_OSAL_SUCCESS;
    } else if (and && (actual_flags == requested_flags)) {
        return RTOS_OSAL_SUCCESS;
    } else if (!and && (actual_flags != 0)) {
        return RTOS_OSAL_SUCCESS;
    } else {
        return RTOS_OSAL_TIMEOUT;
    }
}

rtos_osal_status_t rtos_osal_event_group_delete(rtos_osal_event_group_t *group)
{
    vEventGroupDelete(group->group);

    return RTOS_OSAL_SUCCESS;
}
