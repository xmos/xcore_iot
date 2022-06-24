// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos_mic_array.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/mic_array/mic_array_test.h"

static int run_mic_array_tests(mic_array_test_ctx_t *test_ctx, chanend_t c)
{
    int retval = 0;

    do
    {
        sync(c);
        if (test_ctx->main_test[test_ctx->cur_test] != NULL)
        {
            MIC_ARRAY_MAIN_TEST_ATTR mic_array_main_test_t fn;
            fn = test_ctx->main_test[test_ctx->cur_test];
            int tmp = fn(test_ctx);
            retval = (retval != -1) ? tmp : retval;
        } else {
            mic_array_printf("Missing main_test callback on test %d", test_ctx->cur_test);
            retval = -1;
        }
    } while (++test_ctx->cur_test < test_ctx->test_cnt);

    return retval;
}

static void start_mic_array_devices(mic_array_test_ctx_t *test_ctx)
{
    mic_array_printf("RPC configure");
    rtos_mic_array_rpc_config(test_ctx->mic_array_ctx, MIC_ARRAY_RPC_PORT, MIC_ARRAY_RPC_HOST_TASK_PRIORITY);

#if ON_TILE(1)
    mic_array_printf("Device start");

    rtos_mic_array_start(
            test_ctx->mic_array_ctx,
            2 * MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME,
            MIC_ARRAY_ISR_CORE);
#endif

    mic_array_printf("Devices setup done");
}

static void register_mic_array_tests(mic_array_test_ctx_t *test_ctx)
{
    register_get_samples_test(test_ctx);

    register_rpc_get_samples_test(test_ctx);
}

static void mic_array_init_tests(mic_array_test_ctx_t *test_ctx, rtos_mic_array_t *mic_array_ctx)
{
    memset(test_ctx, 0, sizeof(mic_array_test_ctx_t));
    test_ctx->mic_array_ctx = mic_array_ctx;
    test_ctx->cur_test = 0;
    test_ctx->test_cnt = 0;

    register_mic_array_tests(test_ctx);
    configASSERT(test_ctx->test_cnt <= MIC_ARRAY_MAX_TESTS);
}

int mic_array_device_tests(rtos_mic_array_t *mic_array_ctx, chanend_t c)
{
    mic_array_test_ctx_t test_ctx;
    int res = 0;

    sync(c);
    mic_array_printf("Init test context");
    mic_array_init_tests(&test_ctx, mic_array_ctx);
    mic_array_printf("Test context init");

    sync(c);
    mic_array_printf("Start devices");
    start_mic_array_devices(&test_ctx);
    mic_array_printf("Devices started");

    sync(c);
    mic_array_printf("Start tests");
    res = run_mic_array_tests(&test_ctx, c);

    sync(c);   // Sync before return
    return res;
}
