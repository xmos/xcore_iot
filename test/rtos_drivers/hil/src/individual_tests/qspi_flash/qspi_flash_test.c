// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos_qspi_flash.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/qspi_flash/qspi_flash_test.h"

static int run_qspi_flash_tests(qspi_flash_test_ctx_t *test_ctx, chanend_t c)
{
    int retval = 0;

    do
    {
        sync(c);
        if (test_ctx->main_test[test_ctx->cur_test] != NULL)
        {
            QSPI_FLASH_MAIN_TEST_ATTR qspi_flash_main_test_t fn;
            fn = test_ctx->main_test[test_ctx->cur_test];
            int tmp = fn(test_ctx);
            retval = (retval != -1) ? tmp : retval;
        } else {
            qspi_flash_printf("Missing main_test callback on test %d", test_ctx->cur_test);
            retval = -1;
        }
    } while (++test_ctx->cur_test < test_ctx->test_cnt);

    return retval;
}

static void start_qspi_flash_devices(qspi_flash_test_ctx_t *test_ctx)
{
    qspi_flash_printf("rpc configure");
    rtos_qspi_flash_rpc_config(test_ctx->qspi_flash_ctx, QSPI_FLASH_RPC_PORT, QSPI_FLASH_RPC_HOST_TASK_PRIORITY);

#if ON_TILE(0)
    qspi_flash_printf("Device start");
    rtos_qspi_flash_start(test_ctx->qspi_flash_ctx, configMAX_PRIORITIES-1);
#endif

    qspi_flash_printf("Devices setup done");
}

static void register_qspi_flash_tests(qspi_flash_test_ctx_t *test_ctx)
{
    register_check_params_test(test_ctx);

    register_read_write_read_test(test_ctx);

    register_rpc_read_write_read_test(test_ctx);

    register_multiple_user_test(test_ctx);
}

static void qspi_flash_init_tests(qspi_flash_test_ctx_t *test_ctx, rtos_qspi_flash_t *qspi_flash_ctx)
{
    memset(test_ctx, 0, sizeof(qspi_flash_test_ctx_t));
    test_ctx->qspi_flash_ctx = qspi_flash_ctx;
    test_ctx->cur_test = 0;
    test_ctx->test_cnt = 0;

    register_qspi_flash_tests(test_ctx);
    configASSERT(test_ctx->test_cnt <= QSPI_FLASH_MAX_TESTS);
}

int qspi_flash_device_tests(rtos_qspi_flash_t *qspi_flash_ctx, chanend_t c)
{
    qspi_flash_test_ctx_t test_ctx;
    int res = 0;

    sync(c);
    qspi_flash_printf("Init test context");
    qspi_flash_init_tests(&test_ctx, qspi_flash_ctx);
    qspi_flash_printf("Test context init");

    sync(c);
    qspi_flash_printf("Start devices");
    start_qspi_flash_devices(&test_ctx);
    qspi_flash_printf("Devices started");

    sync(c);
    qspi_flash_printf("Start tests");
    res = run_qspi_flash_tests(&test_ctx, c);

    sync(c);   // Sync before return
    return res;
}
