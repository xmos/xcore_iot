// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos_uart_tx.h"
#include "rtos_uart_rx.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/uart/uart_test.h"

#define UART_RX_TILE 1
#define UART_TX_TILE 0

#if ON_TILE(UART_RX_TILE)
RTOS_UART_RX_CALLBACK_ATTR
void rtos_uart_rx_started(rtos_uart_rx_t *ctx)
{
    uart_test_ctx_t *test_ctx = ctx->app_data;
    if (test_ctx->uart_rx_started[test_ctx->cur_test] != NULL)
    {
        UART_RX_STARTED_ATTR uart_rx_started_t fn;
        fn = test_ctx->uart_rx_started[test_ctx->cur_test];
        fn(ctx);
    } else {
        uart_printf("UART RX started error callback on test %d", test_ctx->cur_test);
    }
}

RTOS_UART_RX_CALLBACK_ATTR
void rtos_uart_rx_error(rtos_uart_rx_t *ctx, uint8_t err_flags)
{
    uart_test_ctx_t *test_ctx = ctx->app_data;
    if (test_ctx->uart_rx_error[test_ctx->cur_test] != NULL)
    {
        UART_RX_ERROR_ATTR uart_rx_error_t fn;
        fn = test_ctx->uart_rx_error[test_ctx->cur_test];
        fn(ctx, err_flags);
    } else {
        uart_printf("UART RX missing error callback on test %d", test_ctx->cur_test);
    }
}

RTOS_UART_RX_CALLBACK_ATTR
void rtos_uart_rx_complete(rtos_uart_rx_t *ctx)
{
    uart_test_ctx_t *test_ctx = ctx->app_data;
    if (test_ctx->uart_rx_complete[test_ctx->cur_test] != NULL)
    {
        UART_RX_COMPLETE_ATTR uart_rx_complete_t fn;
        fn = test_ctx->uart_rx_complete[test_ctx->cur_test];
        fn(ctx);
    } else {
        uart_printf("UART RX complete error callback on test %d", test_ctx->cur_test);
    }
}
#endif

static int run_uart_tests(uart_test_ctx_t *test_ctx, chanend_t c)
{
    int retval = 0;

    do
    {
        sync(c);
        if (test_ctx->main_test[test_ctx->cur_test] != NULL)
        {
            UART_MAIN_TEST_ATTR uart_main_test_t fn;
            fn = test_ctx->main_test[test_ctx->cur_test];
            int tmp = fn(test_ctx);
            retval = (retval != -1) ? tmp : retval;
        } else {
#if ON_TILE(UART_RX_TILE)
            uart_printf("RX missing main_test callback on test %d", test_ctx->cur_test);
#endif
#if ON_TILE(UART_TX_TILE)
            uart_printf("TX missing main_test callback on test %d", test_ctx->cur_test);
#endif
            retval = -1;
        }
    } while (++test_ctx->cur_test < test_ctx->test_cnt);

    return retval;
}

static void start_uart_devices(uart_test_ctx_t *test_ctx)
{
#if ON_TILE(UART_RX_TILE)
    uart_printf("RX start");
    rtos_uart_rx_start(
        test_ctx->rtos_uart_rx_ctx,
        test_ctx,
        rtos_uart_rx_started,
        rtos_uart_rx_complete,
        rtos_uart_rx_error,
        (1 << UART_RX_ISR_CORE),
        appconfSTARTUP_TASK_PRIORITY,
        1024 // Big enough to hold tx_buff[] many times over
        );
#endif

#if ON_TILE(UART_TX_TILE)
    uart_printf("TX start");
    rtos_uart_tx_start(test_ctx->rtos_uart_tx_ctx);
#endif

    uart_printf("Devices setup done");
}

static void register_uart_tests(uart_test_ctx_t *test_ctx)
{
    register_local_loopback_test(test_ctx);
}

static void uart_init_tests(uart_test_ctx_t *test_ctx, rtos_uart_tx_t *rtos_uart_tx_ctx, rtos_uart_rx_t *rtos_uart_rx_ctx)
{
    memset(test_ctx, 0, sizeof(uart_test_ctx_t));
    test_ctx->rtos_uart_tx_ctx = rtos_uart_tx_ctx;
    test_ctx->rtos_uart_rx_ctx = rtos_uart_rx_ctx;

    test_ctx->cur_test = 0;
    test_ctx->test_cnt = 0;

    register_uart_tests(test_ctx);
    configASSERT(test_ctx->test_cnt <= UART_MAX_TESTS);
}

int uart_device_tests(rtos_uart_tx_t *rtos_uart_tx_ctx, rtos_uart_rx_t *rtos_uart_rx_ctx, chanend_t c)
{
    uart_test_ctx_t test_ctx;
    int res = 0;

    sync(c);
    uart_printf("Init test context");
    uart_init_tests(&test_ctx, rtos_uart_tx_ctx, rtos_uart_rx_ctx);
    uart_printf("Test context init");

    sync(c);
    uart_printf("Start devices");
    start_uart_devices(&test_ctx);
    uart_printf("Devices started");

    sync(c);
    uart_printf("Start tests");
    res = run_uart_tests(&test_ctx, c);

    sync(c);   // Sync before return
    return res;
}
