// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos_swmem.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/swmem/swmem_test.h"

static int run_swmem_tests(swmem_test_ctx_t *test_ctx, chanend_t c)
{
    int retval = 0;

    do
    {
        sync(c);
        if (test_ctx->main_test[test_ctx->cur_test] != NULL)
        {
            SWMEM_MAIN_TEST_ATTR swmem_main_test_t fn;
            fn = test_ctx->main_test[test_ctx->cur_test];
            int tmp = fn(test_ctx);
            retval = (retval != -1) ? tmp : retval;
        } else {
            swmem_printf("Missing main_test callback on test %d", test_ctx->cur_test);
            retval = -1;
        }
    } while (++test_ctx->cur_test < test_ctx->test_cnt);

    return retval;
}

static void start_swmem_devices(swmem_test_ctx_t *test_ctx)
{
    swmem_printf("Device start");
    rtos_swmem_init(RTOS_SWMEM_READ_FLAG|RTOS_SWMEM_WRITE_FLAG);
    rtos_swmem_start(configMAX_PRIORITIES-1);
    swmem_printf("Device setup done");
}

static void register_swmem_tests(swmem_test_ctx_t *test_ctx)
{
    register_swmem_read_write_test(test_ctx);
}

static void swmem_init_tests(swmem_test_ctx_t *test_ctx)
{
    memset(test_ctx, 0, sizeof(swmem_test_ctx_t));

    test_ctx->cur_test = 0;
    test_ctx->test_cnt = 0;

    register_swmem_tests(test_ctx);
    configASSERT(test_ctx->test_cnt <= SWMEM_MAX_TESTS);
}

int swmem_device_tests(chanend_t c)
{
    swmem_test_ctx_t test_ctx;
    int res = 0;

    sync(c);
    swmem_printf("Init test context");
    swmem_init_tests(&test_ctx);
    swmem_printf("Test context init");

    sync(c);
    swmem_printf("Start devices");
    start_swmem_devices(&test_ctx);
    swmem_printf("Devices started");

    sync(c);
    swmem_printf("Start tests");
    res = run_swmem_tests(&test_ctx, c);

    sync(c);   // Sync before return
    return res;
}
