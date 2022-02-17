// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xcore/swmem_fill.h>
#include <xcore/swmem_evict.h>
#include <xcore/triggerable.h>

#include "rtos_interrupt.h"
#include "rtos_swmem.h"

static swmem_fill_t swmem_fill_res;
static swmem_evict_t swmem_evict_res;

static bool started;
static rtos_osal_thread_t swmem_thread;
static rtos_osal_event_group_t swmem_event_group;

static uint32_t swmem_core_exclude_map;

static uint32_t fill_buf[SWMEM_FILL_SIZE_WORDS];
static fill_slot_t fill_slot;

static uint32_t evict_buf[SWMEM_EVICT_SIZE_WORDS];
static evict_slot_t evict_slot;
static evict_mask_t dirty_mask;

#define SWMEM_ADDRESS_UNINITIALISED 0xffffffff

// This must be initialized to a value, to prevent it from being memset to zero during
// C runtime startup.  This value may be set by the bootloader
static volatile unsigned int __swmem_address = SWMEM_ADDRESS_UNINITIALISED;

DEFINE_RTOS_INTERRUPT_CALLBACK(sw_mem_fill_isr, arg)
{
    bool handled = false;
    fill_slot = swmem_fill_in_address(swmem_fill_res);

    if (rtos_swmem_read_request_isr) {
        handled = rtos_swmem_read_request_isr(
                (unsigned)(fill_slot - XS1_SWMEM_BASE + __swmem_address),
                fill_buf);
        if (handled) {
            swmem_fill_populate_from_buffer(swmem_fill_res, fill_slot,
                                            fill_buf);
        }
    }

    if (!handled && rtos_swmem_read_request) {
        /*
         * Remember the core that this ISR is running on. The swmem thread must
         * call swmem_fill_populate_from_buffer() on the same core as swmem_fill_in_address().
         */
        swmem_core_exclude_map = ~(1 << rtos_core_id_get());

        triggerable_disable_trigger(swmem_fill_res);
        rtos_osal_event_group_set_bits(&swmem_event_group,
                                       RTOS_SWMEM_READ_FLAG);
        handled = true;
    }

    xassert(handled);
}

DEFINE_RTOS_INTERRUPT_CALLBACK(sw_mem_evict_isr, arg)
{
    bool handled = false;

    evict_slot = swmem_evict_in_address(swmem_evict_res);
    dirty_mask = swmem_evict_get_dirty_mask(swmem_evict_res, evict_slot);
    swmem_evict_to_buffer(swmem_evict_res, evict_slot, evict_buf);

    if (rtos_swmem_write_request_isr) {
        handled = rtos_swmem_write_request_isr(
                (unsigned)(evict_slot - XS1_SWMEM_BASE + __swmem_address),
                dirty_mask, evict_buf);
    }

    if (!handled && rtos_swmem_write_request) {
        triggerable_disable_trigger(swmem_evict_res);
        rtos_osal_event_group_set_bits(&swmem_event_group,
                                       RTOS_SWMEM_WRITE_FLAG);
        handled = true;
    }

    xassert(handled);
}

static void rtos_swmem_thread(void *arg)
{
    uint32_t flags;
    rtos_osal_status_t status;

    for (;;) {
        status = rtos_osal_event_group_get_bits(
                &swmem_event_group,
                RTOS_SWMEM_READ_FLAG | RTOS_SWMEM_WRITE_FLAG,
                RTOS_OSAL_OR_CLEAR, &flags, RTOS_OSAL_WAIT_FOREVER);

        if (status != RTOS_OSAL_SUCCESS) {
            continue;
        }

        if (flags & RTOS_SWMEM_READ_FLAG) {
            rtos_swmem_read_request(
                    (unsigned)(fill_slot - XS1_SWMEM_BASE + __swmem_address),
                    fill_buf);

            /*
             * Ensure that swmem_fill_populate_from_buffer() is called on the same
             * core that swmem_fill_in_address() was called on.
             */
            rtos_osal_thread_core_exclusion_set(&swmem_thread,
                                                swmem_core_exclude_map);
            swmem_fill_populate_from_buffer(swmem_fill_res, fill_slot,
                                            fill_buf);

            /*
             * Allow this thread to run on any core once again
             */
            rtos_osal_thread_core_exclusion_set(&swmem_thread, 0);

            triggerable_enable_trigger(swmem_fill_res);
        }

        if (flags & RTOS_SWMEM_WRITE_FLAG) {
            rtos_swmem_write_request(
                    (unsigned)(evict_slot - XS1_SWMEM_BASE + __swmem_address),
                    dirty_mask, evict_buf);
            triggerable_enable_trigger(swmem_evict_res);
        }
    }
}

void rtos_swmem_start(unsigned priority)
{
    if (!started) {
        if (rtos_swmem_read_request || rtos_swmem_write_request) {
            rtos_osal_event_group_create(&swmem_event_group,
                                         "swmem_event_group");

            rtos_osal_thread_create(
                    &swmem_thread, "rtos_swmem_thread",
                    (rtos_osal_entry_function_t)rtos_swmem_thread, NULL,
                    RTOS_THREAD_STACK_SIZE(rtos_swmem_thread), priority);
        }

        if (swmem_fill_res != 0) {
            triggerable_setup_interrupt_callback(
                    swmem_fill_res, NULL,
                    RTOS_INTERRUPT_CALLBACK(sw_mem_fill_isr));
            triggerable_enable_trigger(swmem_fill_res);
        }
        if (swmem_evict_res != 0) {
            triggerable_setup_interrupt_callback(
                    swmem_evict_res, NULL,
                    RTOS_INTERRUPT_CALLBACK(sw_mem_evict_isr));
            triggerable_enable_trigger(swmem_evict_res);
        }

        started = true;
    }
}

void rtos_swmem_init(uint32_t init_flags)
{
    if (__swmem_address == SWMEM_ADDRESS_UNINITIALISED) {
        __swmem_address = 0;
    }

    if ((init_flags & RTOS_SWMEM_READ_FLAG) && swmem_fill_res == 0) {
        swmem_fill_res = swmem_fill_get();
    }

    if ((init_flags & RTOS_SWMEM_WRITE_FLAG) && swmem_evict_res == 0) {
        swmem_evict_res = swmem_evict_get();
    }
}
