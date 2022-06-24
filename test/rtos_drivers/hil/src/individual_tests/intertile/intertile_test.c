// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>

/* Library headers */
#include "rtos_intertile.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/intertile/intertile_test.h"

static int run_intertile_tests(intertile_test_ctx_t *test_ctx, chanend_t c)
{
    int retval = 0;

    do
    {
        sync(c);
        if (test_ctx->main_test[test_ctx->cur_test] != NULL)
        {
            INTERTILE_MAIN_TEST_ATTR intertile_main_test_t fn;
            fn = test_ctx->main_test[test_ctx->cur_test];
            int tmp = fn(test_ctx);
            retval = (retval != -1) ? tmp : retval;
        } else {
            intertile_printf("Missing main_test callback on test %d", test_ctx->cur_test);
            retval = -1;
        }
    } while (++test_ctx->cur_test < test_ctx->test_cnt);

    return retval;
}

static void start_intertile_devices(intertile_test_ctx_t *test_ctx)
{
    intertile_printf("Device start");
    rtos_intertile_start(test_ctx->intertile_ctx);
    intertile_printf("Device setup done");
}

static void register_intertile_tests(intertile_test_ctx_t *test_ctx)
{
    register_fixed_len_tx_test(test_ctx);
    register_var_len_tx_test(test_ctx);
}

static void intertile_init_tests(intertile_test_ctx_t *test_ctx, rtos_intertile_t *intertile_ctx)
{
    memset(test_ctx, 0, sizeof(intertile_test_ctx_t));
    test_ctx->intertile_ctx = intertile_ctx;

    test_ctx->cur_test = 0;
    test_ctx->test_cnt = 0;

    register_intertile_tests(test_ctx);
    configASSERT(test_ctx->test_cnt <= INTERTILE_MAX_TESTS);
}

int intertile_device_tests(rtos_intertile_t *intertile_ctx, chanend_t c)
{
    intertile_test_ctx_t test_ctx;
    int res = 0;

    sync(c);
    intertile_printf("Init test context");
    intertile_init_tests(&test_ctx, intertile_ctx);
    intertile_printf("Test context init");

    sync(c);
    intertile_printf("Start devices");
    start_intertile_devices(&test_ctx);
    intertile_printf("Devices started");

    sync(c);
    intertile_printf("Start tests");
    res = run_intertile_tests(&test_ctx, c);

    sync(c);   // Sync before return
    return res;
}
