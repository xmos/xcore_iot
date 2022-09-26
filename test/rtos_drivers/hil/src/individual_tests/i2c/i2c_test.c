// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos_i2c_master.h"
#include "rtos_i2c_slave.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/i2c/i2c_test.h"

#if ON_TILE(1)
RTOS_I2C_SLAVE_CALLBACK_ATTR
static void i2c_slave_start(rtos_i2c_slave_t *ctx, void *app_data)
{
    i2c_printf("SLAVE started");
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
static void i2c_slave_rx(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    i2c_test_ctx_t *test_ctx = (i2c_test_ctx_t*)ctx->app_data;
    if (test_ctx->slave_rx[test_ctx->cur_test] != NULL)
    {
        I2C_SLAVE_RX_ATTR i2c_slave_rx_t fn;
        fn = test_ctx->slave_rx[test_ctx->cur_test];
        fn(ctx, app_data, data, len);
    } else {
        i2c_printf("SLAVE missing slave_rx callback on test %d", test_ctx->cur_test);
    }
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
static size_t i2c_slave_tx_start(rtos_i2c_slave_t *ctx, void *app_data, uint8_t **data)
{
    i2c_test_ctx_t *test_ctx = (i2c_test_ctx_t*)ctx->app_data;
    size_t len = 0;

    if (test_ctx->slave_tx_start[test_ctx->cur_test] != NULL)
    {
        I2C_SLAVE_RX_ATTR i2c_slave_tx_start_t fn;
        fn = test_ctx->slave_tx_start[test_ctx->cur_test];
        len = fn(ctx, app_data, data);
    } else {
        i2c_printf("SLAVE missing i2c_slave_tx_start callback on test %d", test_ctx->cur_test);
    }

    return len;
}

RTOS_I2C_SLAVE_CALLBACK_ATTR
static void i2c_slave_tx_done(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len)
{
    i2c_test_ctx_t *test_ctx = (i2c_test_ctx_t*)ctx->app_data;
    if (test_ctx->slave_tx_done[test_ctx->cur_test] != NULL)
    {
        I2C_SLAVE_RX_ATTR i2c_slave_tx_done_t fn;
        fn = test_ctx->slave_tx_done[test_ctx->cur_test];
        fn(ctx, app_data, data, len);
    } else {
        i2c_printf("SLAVE missing i2c_slave_tx_done callback on test %d", test_ctx->cur_test);
    }
}

RTOS_I2C_SLAVE_RX_BYTE_CHECK_CALLBACK_ATTR
void i2c_slave_rx_byte_check(rtos_i2c_slave_t *ctx, void *app_data, uint8_t data, i2c_slave_ack_t *cur_status)
{
    i2c_test_ctx_t *test_ctx = (i2c_test_ctx_t*)ctx->app_data;
    if (test_ctx->slave_rx_check_byte[test_ctx->cur_test] != NULL)
    {
        I2C_SLAVE_RX_BYTE_CHECK_ATTR i2c_slave_rx_byte_check_cb_t fn;
        fn = test_ctx->slave_rx_check_byte[test_ctx->cur_test];
        fn(ctx, app_data, data, cur_status);
    } else {
        i2c_printf("SLAVE missing slave_rx_check_byte callback on test %d", test_ctx->cur_test);
    }
}

RTOS_I2C_SLAVE_WRITE_ADDR_REQUEST_CALLBACK_ATTR
void i2c_slave_write_addr_req(rtos_i2c_slave_t *ctx, void *app_data, i2c_slave_ack_t *cur_status)
{
    i2c_test_ctx_t *test_ctx = (i2c_test_ctx_t*)ctx->app_data;
    if (test_ctx->slave_write_addr_req[test_ctx->cur_test] != NULL)
    {
        I2C_SLAVE_WRITE_ADDR_REQ_ATTR i2c_slave_write_addr_req_cb_t fn;
        fn = test_ctx->slave_write_addr_req[test_ctx->cur_test];
        fn(ctx, app_data, cur_status);
    } else {
        i2c_printf("SLAVE missing slave_write_addr_req callback on test %d", test_ctx->cur_test);
    }
}

#endif /* ON_TILE(1) */

static int run_i2c_tests(i2c_test_ctx_t *test_ctx, chanend_t c)
{
    int retval = 0;
    do
    {
        sync(c);
        if (test_ctx->main_test[test_ctx->cur_test] != NULL)
        {
            I2C_MAIN_TEST_ATTR i2c_main_test_t fn;
            fn = test_ctx->main_test[test_ctx->cur_test];
            int tmp = fn(test_ctx);
            retval = (retval != -1) ? tmp : retval;
        } else {
#if ON_TILE(0)
            i2c_printf("MASTER missing main_test callback on test %d", test_ctx->cur_test);
#endif
#if ON_TILE(1)
            i2c_printf("SLAVE missing main_test callback on test %d", test_ctx->cur_test);
#endif
            retval = -1;
        }
    } while (++test_ctx->cur_test < test_ctx->test_cnt);

    return retval;
}

static void start_i2c_devices(i2c_test_ctx_t *test_ctx)
{
    i2c_printf("MASTER rpc configure");
    rtos_i2c_master_rpc_config(test_ctx->i2c_master_ctx, I2C_MASTER_RPC_PORT, I2C_MASTER_RPC_HOST_TASK_PRIORITY);

#if ON_TILE(0)
    i2c_printf("MASTER device start");
    rtos_i2c_master_start(test_ctx->i2c_master_ctx);
#endif

#if ON_TILE(1)
    i2c_printf("SLAVE start");
    rtos_i2c_slave_start(test_ctx->i2c_slave_ctx,
                         test_ctx,
                         i2c_slave_start,
                         i2c_slave_rx,
                         i2c_slave_tx_start,
                         i2c_slave_tx_done,
                         i2c_slave_rx_byte_check,
                         i2c_slave_write_addr_req,
                         I2C_SLAVE_ISR_CORE,
                         configMAX_PRIORITIES-1);
#endif

    i2c_printf("Devices setup done");
}

static void register_i2c_tests(i2c_test_ctx_t *test_ctx)
{
    register_master_reg_write_test(test_ctx);
    register_master_reg_read_test(test_ctx);
    register_master_write_test(test_ctx);
    register_master_write_multiple_test(test_ctx);
    register_master_read_test(test_ctx);
    register_master_read_multiple_test(test_ctx);

    register_rpc_master_reg_write_test(test_ctx);
    register_rpc_master_reg_read_test(test_ctx);
    register_rpc_master_write_test(test_ctx);
    register_rpc_master_write_multiple_test(test_ctx);
    register_rpc_master_read_test(test_ctx);
    register_rpc_master_read_multiple_test(test_ctx);
}

static void i2c_init_tests(i2c_test_ctx_t *test_ctx, rtos_i2c_master_t *i2c_master_ctx, rtos_i2c_slave_t *i2c_slave_ctx)
{
    memset(test_ctx, 0, sizeof(i2c_test_ctx_t));
    test_ctx->i2c_master_ctx = i2c_master_ctx;
    test_ctx->i2c_slave_ctx = i2c_slave_ctx;
    test_ctx->cur_test = 0;
    test_ctx->test_cnt = 0;

    register_i2c_tests(test_ctx);
    configASSERT(test_ctx->test_cnt <= I2C_MAX_TESTS);
}

int i2c_device_tests(rtos_i2c_master_t *i2c_master_ctx, rtos_i2c_slave_t *i2c_slave_ctx, chanend_t c)
{
    i2c_test_ctx_t test_ctx;
    int res = 0;

    sync(c);
    i2c_printf("Init test context");
    i2c_init_tests(&test_ctx, i2c_master_ctx, i2c_slave_ctx);
    i2c_printf("Test context init");

    sync(c);
    i2c_printf("Start devices");
    start_i2c_devices(&test_ctx);
    i2c_printf("Devices started");

    sync(c);
    i2c_printf("Start tests");
    res = run_i2c_tests(&test_ctx, c);

    sync(c);   // Sync before return
    return res;
}
