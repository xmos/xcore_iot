// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#include <platform.h>
#include <xcore/swmem_fill.h>
#include <xcore/swmem_evict.h>
#include <xcore/minicache.h>

#include "rtos/drivers/swmem/api/rtos_swmem.h"

#include "swmem_test.h"

#if ON_TILE(SWMEM_TILE)

__attribute__((section(".SwMem_data")))
uint8_t sw_mem_array[32*10];

void rtos_swmem_read_request(unsigned offset, uint32_t *buf)
{
    rtos_printf("SwMem fill request for offset 0x%08x\n", offset);
    for (int i = 0; i < SWMEM_FILL_SIZE_WORDS; i++) {
        buf[i] = i;
    }
}

void rtos_swmem_write_request(unsigned offset, uint32_t dirty_mask, const uint32_t *buf)
{
    uint8_t *byte_buf = (uint8_t *) buf;

    rtos_printf("SwMem write request for offset 0x%08x. dirty_mask: %08x\n", offset, dirty_mask);
    for (int i = 0; i < sizeof(uint32_t) * SWMEM_EVICT_SIZE_WORDS; i++) {
        if (i == 1) {
            xassert(dirty_mask & (1 << i));
            xassert(byte_buf[i] == 4);
        } else if (i == 18) {
            xassert(dirty_mask & (1 << i));
            xassert(byte_buf[i] == 3);
        } else {
            xassert(!(dirty_mask & (1 << i)));
        }
    }
}

void swmem_test_run(void)
{
    for (int i = 0; i < sizeof(sw_mem_array); i++) {
        rtos_printf("%02x ", sw_mem_array[i]);
    }
    rtos_printf("\n");

    for (int i = 0; i < sizeof(sw_mem_array); i+=32) {
        sw_mem_array[i+1] = 4;
        sw_mem_array[i+18] = 3;
    }

    sw_mem_array[40] = 42;
    xassert(sw_mem_array[40] == 42);
    minicache_invalidate();
    xassert(sw_mem_array[40] == 2);
}

void swmem_test_init(void)
{
    rtos_swmem_init(RTOS_SWMEM_READ_FLAG | RTOS_SWMEM_WRITE_FLAG);
    rtos_swmem_start(configMAX_PRIORITIES - 1);
}

#endif
