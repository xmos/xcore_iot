// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos_spi_master.h"
#include "rtos_spi_slave.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/spi/spi_test.h"

#if ON_TILE(1)
RTOS_SPI_SLAVE_CALLBACK_ATTR
static void spi_slave_start(rtos_spi_slave_t *ctx, void *app_data)
{
    (void) ctx;
    (void) app_data;
    spi_printf("SLAVE start");
}

RTOS_SPI_SLAVE_CALLBACK_ATTR
static void spi_slave_xfer_done(rtos_spi_slave_t *ctx, void *app_data)
{
    spi_test_ctx_t *test_ctx = (spi_test_ctx_t*)ctx->app_data;
    spi_printf("SLAVE xfer done");
    if (test_ctx->slave_xfer_done[test_ctx->cur_test] != NULL)
    {
        SPI_SLAVE_XFER_DONE_ATTR spi_slave_xfer_done_t fn;
        fn = test_ctx->slave_xfer_done[test_ctx->cur_test];
        fn(ctx, app_data);
    } else {
        spi_printf("SLAVE missing slave_xfer_done callback on test %d", test_ctx->cur_test);
    }
}
#endif /* ON_TILE(1) */

static int run_spi_tests(spi_test_ctx_t *test_ctx, chanend_t c)
{
    int retval = 0;

    do
    {
        sync(c);
        if (test_ctx->main_test[test_ctx->cur_test] != NULL)
        {
            SPI_MAIN_TEST_ATTR spi_main_test_t fn;
            fn = test_ctx->main_test[test_ctx->cur_test];
            int tmp = fn(test_ctx);
            retval = (retval != -1) ? tmp : retval;
        } else {
            spi_printf("Missing main_test callback on test %d", test_ctx->cur_test);
            retval = -1;
        }
    } while (++test_ctx->cur_test < test_ctx->test_cnt);

    return retval;
}

static void start_spi_devices(spi_test_ctx_t *test_ctx)
{
    spi_printf("MASTER rpc configure");
    rtos_spi_master_rpc_config(test_ctx->spi_master_ctx, SPI_MASTER_RPC_PORT, SPI_MASTER_RPC_HOST_TASK_PRIORITY);

#if ON_TILE(0)
    spi_printf("MASTER device start");
    rtos_spi_master_start(test_ctx->spi_master_ctx, configMAX_PRIORITIES-1);
#endif

#if ON_TILE(1)
    spi_printf("SLAVE start");
    rtos_spi_slave_start(test_ctx->spi_slave_ctx,
                         test_ctx,
                         spi_slave_start,
                         spi_slave_xfer_done,
                         SPI_SLAVE_ISR_CORE,
                         configMAX_PRIORITIES-1);
#endif

    spi_printf("Devices setup done");
}

static void register_spi_tests(spi_test_ctx_t *test_ctx)
{
    register_single_transaction_test(test_ctx);
    register_multiple_transaction_test(test_ctx);

    register_rpc_single_transaction_test(test_ctx);
    register_rpc_multiple_transaction_test(test_ctx);

    register_slave_default_buffer_test(test_ctx);
}

static void spi_init_tests(spi_test_ctx_t *test_ctx, rtos_spi_master_t *spi_master_ctx, rtos_spi_master_device_t *spi_device_ctx, rtos_spi_slave_t *spi_slave_ctx)
{
    memset(test_ctx, 0, sizeof(spi_test_ctx_t));
    test_ctx->spi_master_ctx = spi_master_ctx;
    test_ctx->spi_device_ctx = spi_device_ctx;
    test_ctx->spi_slave_ctx = spi_slave_ctx;
    test_ctx->cur_test = 0;
    test_ctx->test_cnt = 0;

    register_spi_tests(test_ctx);
    configASSERT(test_ctx->test_cnt <= SPI_MAX_TESTS);
}

int spi_device_tests(rtos_spi_master_t *spi_master_ctx, rtos_spi_master_device_t *spi_device_ctx, rtos_spi_slave_t *spi_slave_ctx, chanend_t c)
{
    spi_test_ctx_t test_ctx;
    int res = 0;

    sync(c);
    spi_printf("Init test context");
    spi_init_tests(&test_ctx, spi_master_ctx, spi_device_ctx, spi_slave_ctx);
    spi_printf("Test context init");

    sync(c);
    spi_printf("Start devices");
    start_spi_devices(&test_ctx);
    spi_printf("Devices started");

    sync(c);
    spi_printf("Start tests");
    res = run_spi_tests(&test_ctx, c);

    sync(c);   // Sync before return
    return res;
}
