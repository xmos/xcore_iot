// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdbool.h>

#include <xcore/swmem_fill.h>
#include <xcore/swmem_evict.h>
#include <xcore/triggerable.h>

#include "rtos_interrupt.h"
#include "rtos/drivers/swmem/api/rtos_swmem.h"

static swmem_fill_t swmem_fill_res;
static swmem_evict_t swmem_evict_res;

static bool started;
static rtos_osal_event_group_t swmem_event_group;


DEFINE_RTOS_INTERRUPT_CALLBACK(sw_mem_fill_isr, arg)
{
    triggerable_disable_trigger(swmem_fill_res);
    rtos_osal_event_group_set_bits(&swmem_event_group, RTOS_SWMEM_READ_FLAG);
}

DEFINE_RTOS_INTERRUPT_CALLBACK(sw_mem_evict_isr, arg)
{
    triggerable_disable_trigger(swmem_evict_res);
    rtos_osal_event_group_set_bits(&swmem_event_group, RTOS_SWMEM_WRITE_FLAG);
}

__attribute__((weak))
void rtos_swmem_read_request(unsigned offset, uint32_t *buf)
{
    (void) offset;
    (void) buf;
}

__attribute__((weak))
void rtos_swmem_write_request(unsigned offset, uint32_t dirty_mask, const uint32_t *buf)
{
    (void) offset;
    (void) dirty_mask;
    (void) buf;
}

static void rtos_swmem_thread(void *arg)
{
    uint32_t flags;
    rtos_osal_status_t status;

    for (;;) {
        status = rtos_osal_event_group_get_bits(
                &swmem_event_group,
                RTOS_SWMEM_READ_FLAG | RTOS_SWMEM_WRITE_FLAG,
                RTOS_OSAL_OR_CLEAR,
                &flags,
                RTOS_OSAL_WAIT_FOREVER);

        if (status != RTOS_OSAL_SUCCESS) {
            continue;
        }

        /*
         * Disabling interrupts here guarantees that there will not be a context switch
         * resulting in this task moving to another core between calling rtos_core_id_get()
         * and vTaskCoreExclusionSet().
         * vTaskCoreExclusionSet() will reenable interrupts before it returns.
         *
         * This locks this thread to the core it is currently on, because it is required that
         * swmem_*_in_address() and swmem_*_buffer() be called from the same core.
         */
        rtos_interrupt_mask_all();
        rtos_osal_thread_core_exclusion_set(NULL, ~(1 << rtos_core_id_get()));

        if (flags & RTOS_SWMEM_READ_FLAG) {
            uint32_t buf[SWMEM_FILL_SIZE_WORDS];
            fill_slot_t fill_slot = swmem_fill_in_address(swmem_fill_res);
            rtos_swmem_read_request((unsigned)(fill_slot - XS1_SWMEM_BASE), buf);
            swmem_fill_populate_from_buffer(swmem_fill_res, fill_slot, buf);
            triggerable_enable_trigger(swmem_fill_res);
        }

        if (flags & RTOS_SWMEM_WRITE_FLAG) {
            uint32_t buf[SWMEM_EVICT_SIZE_WORDS];
            evict_slot_t evict_slot = swmem_evict_in_address(swmem_evict_res);
            evict_mask_t dirty_mask = swmem_evict_get_dirty_mask(swmem_evict_res, evict_slot);
            swmem_evict_to_buffer(swmem_evict_res, evict_slot, buf);
            rtos_swmem_write_request((unsigned)(evict_slot - XS1_SWMEM_BASE), dirty_mask, buf);
            triggerable_enable_trigger(swmem_evict_res);
        }

        /*
         * Allow this thread to run on any core once again
         */
        rtos_osal_thread_core_exclusion_set(NULL, 0);
    }
}

void rtos_swmem_start(unsigned priority)
{
    if (!started) {
        rtos_osal_event_group_create(&swmem_event_group, "swmem_event_group");;

        rtos_osal_thread_create(
                NULL,
                "rtos_swmem_thread",
                (rtos_osal_entry_function_t) rtos_swmem_thread,
                NULL,
                RTOS_THREAD_STACK_SIZE(rtos_swmem_thread),
                priority);

        if (swmem_fill_res != 0) {
            triggerable_setup_interrupt_callback(swmem_fill_res, NULL, RTOS_INTERRUPT_CALLBACK(sw_mem_fill_isr));
            triggerable_enable_trigger(swmem_fill_res);
        }
        if (swmem_evict_res != 0) {
            triggerable_setup_interrupt_callback(swmem_evict_res, NULL, RTOS_INTERRUPT_CALLBACK(sw_mem_evict_isr));
            triggerable_enable_trigger(swmem_evict_res);
        }

        started = true;
    }
}

void rtos_swmem_init(uint32_t init_flags)
{
    if ((init_flags & RTOS_SWMEM_READ_FLAG) && swmem_fill_res == 0) {
        swmem_fill_res = swmem_fill_get();
    }

    if ((init_flags & RTOS_SWMEM_WRITE_FLAG) && swmem_evict_res == 0) {
        swmem_evict_res = swmem_evict_get();
    }
}
