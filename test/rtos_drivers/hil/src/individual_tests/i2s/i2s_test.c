// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos_i2s.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/i2s/i2s_test.h"

static int run_i2s_tests(i2s_test_ctx_t *test_ctx, chanend_t c)
{
    int retval = 0;

    do
    {
        sync(c);
        if (test_ctx->main_test[test_ctx->cur_test] != NULL)
        {
            I2S_MAIN_TEST_ATTR i2s_main_test_t fn;
            fn = test_ctx->main_test[test_ctx->cur_test];
            int tmp = fn(test_ctx);
            retval = (retval != -1) ? tmp : retval;
        } else {
#if ON_TILE(0)
            i2s_printf("MASTER missing main_test callback on test %d", test_ctx->cur_test);
#endif
#if ON_TILE(1)
            i2s_printf("SLAVE missing main_test callback on test %d", test_ctx->cur_test);
#endif
            retval = -1;
        }
    } while (++test_ctx->cur_test < test_ctx->test_cnt);

    return retval;
}

static void start_i2s_devices(i2s_test_ctx_t *test_ctx)
{
    i2s_printf("MASTER rpc configure");
    rtos_i2s_rpc_config(test_ctx->i2s_master_ctx, I2S_MASTER_RPC_PORT, I2S_MASTER_RPC_HOST_TASK_PRIORITY);

#if ON_TILE(1)
    i2s_printf("MASTER device start");
    rtos_i2s_start(
            test_ctx->i2s_master_ctx,
            rtos_i2s_mclk_bclk_ratio(I2S_TEST_AUDIO_CLOCK_FREQUENCY, I2S_TEST_AUDIO_SAMPLE_RATE),
            I2S_MODE_I2S,
            I2S_MASTER_RECV_BUF_SIZE,
            I2S_MASTER_SEND_BUF_SIZE,
            I2S_MASTER_ISR_CORE);
#endif

#if ON_TILE(0)
    i2s_printf("SLAVE device start");
    rtos_i2s_start(
            test_ctx->i2s_slave_ctx,
            rtos_i2s_mclk_bclk_ratio(I2S_TEST_AUDIO_CLOCK_FREQUENCY, I2S_TEST_AUDIO_SAMPLE_RATE),
            I2S_MODE_I2S,
            I2S_SLAVE_RECV_BUF_SIZE,
            I2S_SLAVE_SEND_BUF_SIZE,
            I2S_SLAVE_ISR_CORE);
#endif

    i2s_printf("Devices setup done");
}

static void register_i2s_tests(i2s_test_ctx_t *test_ctx)
{
    register_master_to_slave_test(test_ctx);
    register_slave_to_master_test(test_ctx);

    register_rpc_master_to_slave_test(test_ctx);
    register_rpc_slave_to_master_test(test_ctx);
}

static void i2s_init_tests(i2s_test_ctx_t *test_ctx, rtos_i2s_t *i2s_master_ctx, rtos_i2s_t *i2s_slave_ctx)
{
    memset(test_ctx, 0, sizeof(i2s_test_ctx_t));
    test_ctx->i2s_master_ctx = i2s_master_ctx;
    test_ctx->i2s_slave_ctx = i2s_slave_ctx;
    test_ctx->cur_test = 0;
    test_ctx->test_cnt = 0;

    register_i2s_tests(test_ctx);
    configASSERT(test_ctx->test_cnt <= I2S_MAX_TESTS);
}

int i2s_device_tests(rtos_i2s_t *i2s_master_ctx, rtos_i2s_t *i2s_slave_ctx, chanend_t c)
{
    i2s_test_ctx_t test_ctx;
    int res = 0;

    sync(c);
    i2s_printf("Init test context");
    i2s_init_tests(&test_ctx, i2s_master_ctx, i2s_slave_ctx);
    i2s_printf("Test context init");

    sync(c);
    i2s_printf("Start devices");
    start_i2s_devices(&test_ctx);
    i2s_printf("Devices started");

    sync(c);
    i2s_printf("Start tests");
    res = run_i2s_tests(&test_ctx, c);

    sync(c);   // Sync before return
    return res;
}
