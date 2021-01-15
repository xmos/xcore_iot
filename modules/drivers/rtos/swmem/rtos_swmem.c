// Copyright (c) 2021, XMOS Ltd, All rights reserved

#include <stdbool.h>

#include <xcore/swmem_fill.h>
#include <xcore/swmem_evict.h>
#include <xcore/triggerable.h>

#include "rtos_interrupt.h"
#include "drivers/rtos/swmem/api/rtos_swmem.h"

static swmem_fill_t swmem_fill_res;
static swmem_evict_t swmem_evict_res;

////////////TEMPORARY UNTIL EVENT FLAGS ARE IN THE OSAL/////////////////////////////
static EventGroupHandle_t swmem_event_group;
////////////////////////////////////////////////////////////////////////////////////


DEFINE_RTOS_INTERRUPT_CALLBACK(sw_mem_fill_isr, arg)
{
    BaseType_t yield_required = false;

    triggerable_disable_trigger(swmem_fill_res);

    ////////////////////// CHANGE TO OSAL FUNCTION ////////////////////
    xEventGroupSetBitsFromISR(swmem_event_group, RTOS_SWMEM_READ_FLAG, &yield_required);
    portYIELD_FROM_ISR(yield_required);
}

DEFINE_RTOS_INTERRUPT_CALLBACK(sw_mem_evict_isr, arg)
{
    BaseType_t yield_required = false;

    triggerable_disable_trigger(swmem_evict_res);

    ////////////////////// CHANGE TO OSAL FUNCTION ////////////////////
    xEventGroupSetBitsFromISR(swmem_event_group, RTOS_SWMEM_WRITE_FLAG, &yield_required);
    portYIELD_FROM_ISR(yield_required);
}

__attribute__((weak))
void rtos_swmem_read_request(unsigned offset, uint32_t *buf)
{
    rtos_printf("SwMem fill request for offset 0x%08x\n", offset);
    for (int i = 0; i < SWMEM_FILL_SIZE_WORDS; i++) {
        buf[i] = i;
    }
}

__attribute__((weak))
void rtos_swmem_write_request(unsigned offset, uint32_t dirty_mask, const uint32_t *buf)
{
    uint8_t *byte_buf = (uint8_t *) buf;

    rtos_printf("SwMem write request for offset 0x%08x. dirty_mask: %08x\n", offset, dirty_mask);
    for (int i = 0; i < sizeof(uint32_t) * SWMEM_FILL_SIZE_WORDS; i++) {
        if (dirty_mask & (1 << i)) {
            //rtos_printf("Byte %d dirty: %02x\n", i, byte_buf[i]);
        }
    }
}

static void rtos_swmem_thread(void *arg)
{
    EventBits_t flags;

    for (;;) {
        flags = xEventGroupWaitBits(swmem_event_group,
                                    RTOS_SWMEM_READ_FLAG | RTOS_SWMEM_WRITE_FLAG,
                                    true,
                                    false,
                                    RTOS_OSAL_WAIT_FOREVER);

        if ((flags & (RTOS_SWMEM_READ_FLAG | RTOS_SWMEM_WRITE_FLAG)) == 0) {
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
        vTaskCoreExclusionSet(NULL, ~(1 << rtos_core_id_get()));

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
        vTaskCoreExclusionSet(NULL, 0);
    }
}

void rtos_swmem_start(unsigned priority)
{
    ////////////////////// CHANGE TO OSAL FUNCTION ////////////////////
    swmem_event_group = xEventGroupCreate();

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
}

void rtos_swmem_init(uint32_t init_flags)
{
    if (init_flags & RTOS_SWMEM_READ_FLAG) {
        swmem_fill_res = swmem_fill_get();
    }

    if (init_flags & RTOS_SWMEM_WRITE_FLAG) {
        swmem_evict_res = swmem_evict_get();
    }
}
