// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "event_groups.h"

/* Library headers */
#include "rtos_osal.h"
#include "rtos_qspi_flash.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/qspi_flash/qspi_flash_test.h"

static const char* test_name = "multiple_user_test";

#define local_printf( FMT, ... )    qspi_flash_printf("%s|" FMT, test_name, ##__VA_ARGS__)

// #define QSPI_FLASH_TILE         0
#define QSPI_FLASH_TEST_ADDR    0
#define MULTI_TEST_THREADS      4

typedef struct test_args {
    qspi_flash_test_ctx_t *ctx;
    int id;
    EventGroupHandle_t grp;
    EventBits_t *sync_bitmask;
} test_args_t;

// #if ON_TILE(QSPI_FLASH_TILE)
static void test_thread(test_args_t *args)
{
    local_printf("Test thread %d", args->id);

    rtos_qspi_flash_t *ctx = args->ctx->qspi_flash_ctx;

    size_t test_len = rtos_qspi_flash_sector_size_get(ctx);
    uint8_t *test_buf = (uint8_t*)rtos_osal_malloc(test_len * sizeof(uint8_t));

    uint32_t addr = 0x4 << args->id;

    rtos_qspi_flash_lock(ctx);
    rtos_qspi_flash_erase(ctx, addr, test_len);
    rtos_qspi_flash_read(ctx, test_buf, addr, test_len);

    for (int i=0; i<test_len; i++)
    {
        if (test_buf[i] != 0xFF)
        {
            local_printf("Failed in thread %d. rx_buf[%d]: Expected 0 got 0x%x", args->id, i, test_buf[i]);
        }
    }

    /* Write sector */
    for (int i=0; i<test_len; i++)
    {
        test_buf[i] = args->id%(sizeof(uint8_t));
    }
    rtos_qspi_flash_write(ctx, test_buf, addr, test_len);
    memset(test_buf, 0, test_len);
    rtos_qspi_flash_read(ctx, test_buf, addr, test_len);

    for (int i=0; i<test_len; i++)
    {
        if (test_buf[i] != (args->id%(sizeof(uint8_t))))
        {
            local_printf("Failed in thread %d. rx_buf[%d]: Expected %d got 0x%x", args->id, i, i%(sizeof(uint8_t)), test_buf[i]);
        }
    }
    rtos_qspi_flash_unlock(ctx);

    local_printf("Test thread %d sync", args->id);
    xEventGroupSync(args->grp, 1 << (args->id), *(args->sync_bitmask), portMAX_DELAY);

    vTaskSuspend(NULL);
    while(1) {;}
}
// #endif

QSPI_FLASH_MAIN_TEST_ATTR
static int main_test(qspi_flash_test_ctx_t *ctx)
{
    local_printf("Start");

    // #if ON_TILE(QSPI_FLASH_TILE)
    {
        TaskHandle_t output_handle[MULTI_TEST_THREADS];
        test_args_t args[MULTI_TEST_THREADS];

        EventGroupHandle_t event_group = xEventGroupCreate();
        EventBits_t sync_bits = 0;

        for (int i=0; i<MULTI_TEST_THREADS; i++)
        {
            sync_bits |= 1 << i;
            args[i].ctx = ctx;
            args[i].id = i;
            args[i].grp = event_group;
            args[i].sync_bitmask = &sync_bits;
        }
        sync_bits |= 1 << MULTI_TEST_THREADS;

        vTaskPreemptionDisable(NULL);
        {
            for (int i=0; i<MULTI_TEST_THREADS; i++)
            {
                xTaskCreate((TaskFunction_t)test_thread,
                            "test_thread",
                            RTOS_THREAD_STACK_SIZE(test_thread),
                            &args[i],
                            configMAX_PRIORITIES-1,
                            &output_handle[i]);
            }
        }
        vTaskPreemptionEnable(NULL);

        local_printf("Wait for sync");
        xEventGroupSync(event_group, 1 << MULTI_TEST_THREADS, sync_bits, portMAX_DELAY);

        local_printf("Delete threads");
        for (int i=0; i<MULTI_TEST_THREADS; i++)
        {
            vTaskDelete(output_handle[i]);
        }
    }
    // #endif

    local_printf("Done");
    return 0;
}

void register_multiple_user_test(qspi_flash_test_ctx_t *test_ctx)
{
    uint32_t this_test_num = test_ctx->test_cnt;

    local_printf("Register to test num %d", this_test_num);

    test_ctx->name[this_test_num] = (char*)test_name;
    test_ctx->main_test[this_test_num] = main_test;

    test_ctx->test_cnt++;
}

#undef local_printf
