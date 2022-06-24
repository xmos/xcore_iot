// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/swmem_fill.h>
#include <xcore/swmem_evict.h>
#include <xcore/minicache.h>

/* Library headers */
#include "rtos_swmem.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/swmem/swmem_test.h"

static const char* test_name = "read_write_test";

#define local_printf( FMT, ... )    swmem_printf("%s|" FMT, test_name, ##__VA_ARGS__)

#define SWMEM_TEST_BUFFER_SIZE              (1<<4)

__attribute__((section(".SwMem_data")))
uint32_t swmem_buf[SWMEM_TEST_BUFFER_SIZE];

volatile static uint32_t local_buf[SWMEM_TEST_BUFFER_SIZE] = {0};

bool rtos_swmem_read_request_isr(unsigned offset, uint32_t *buf) {
    local_printf("isr fill request for offset 0x%08x", offset);
    if (offset >= (SWMEM_TEST_BUFFER_SIZE >> 3))
    {
        return false;
    }

    for (int i=0; i<SWMEM_FILL_SIZE_WORDS; i++)
    {
        if (((offset>>5) + i) < SWMEM_TEST_BUFFER_SIZE)
        {
            buf[i] = local_buf[(offset>>5) + i];
        } else {
            buf[i] = 0;
        }
    }
    return true;
}

void rtos_swmem_read_request(unsigned offset, uint32_t *buf) {
    local_printf("fill request for offset 0x%08x", offset);
    for (int i=0; i<SWMEM_FILL_SIZE_WORDS; i++)
    {
        if (((offset>>5) + i) < SWMEM_TEST_BUFFER_SIZE)
        {
            buf[i] = local_buf[(offset>>5) + i];
        } else {
            buf[i] = 0;
        }
    }
}

bool rtos_swmem_write_request_isr(unsigned offset, uint32_t dirty_mask, const uint32_t *buf)
{
    local_printf("isr write request for offset 0x%08x dirty mask 0x%x", offset, dirty_mask);
    if (offset >= (SWMEM_TEST_BUFFER_SIZE >> 3))
    {
        return false;
    }

    for (int i=0; i<SWMEM_EVICT_SIZE_WORDS; i++)
    {
        if (((offset>>5) + i) < SWMEM_TEST_BUFFER_SIZE)
        {
            local_buf[(offset>>5) + i] = buf[i];
        }
    }
    return true;
}

void rtos_swmem_write_request(unsigned offset, uint32_t dirty_mask, const uint32_t *buf)
{
    local_printf("write request for offset 0x%08x dirty mask 0x%x", offset, dirty_mask);
    for (int i=0; i<SWMEM_EVICT_SIZE_WORDS; i++)
    {
        if (((offset>>5) + i) < SWMEM_TEST_BUFFER_SIZE)
        {
            local_buf[(offset>>5) + i] = buf[i];
        }
    }
}

SWMEM_MAIN_TEST_ATTR
static int main_test(swmem_test_ctx_t *ctx)
{
    local_printf("Start");
    uint32_t tmp = 0;

    for (int i=0; i<SWMEM_TEST_BUFFER_SIZE; i++)
    {
        tmp = swmem_buf[i];
        local_printf("Read %u", tmp);
        swmem_buf[i] = i;
        local_printf("Wrote %u", i);
        minicache_flush();
        tmp = swmem_buf[i];
        local_printf("Read %u", tmp);

        if (tmp != i)
        {
            local_printf("Failed at index %d. got %u expected %u", i, tmp, i);
            return -1;
        }
    }

    local_printf("Done");
    return 0;
}

void register_swmem_read_write_test(swmem_test_ctx_t *test_ctx)
{
    uint32_t this_test_num = test_ctx->test_cnt;

    local_printf("Register to test num %d", this_test_num);

    test_ctx->name[this_test_num] = (char*)test_name;
    test_ctx->main_test[this_test_num] = main_test;

    test_ctx->test_cnt++;
}

#undef local_printf
