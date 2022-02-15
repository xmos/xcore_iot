// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */
#include "rtos_gpio.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/gpio/gpio_test.h"

static int run_gpio_tests(gpio_test_ctx_t *test_ctx, chanend_t c)
{
    int retval = 0;

    // Enable ports, one via local calls the other via rpc
#if ON_TILE(0)
    const rtos_gpio_port_id_t p_test_input = rtos_gpio_port(INPUT_PORT);
    gpio_printf("Enable input port 0x%x", p_test_input);
    rtos_gpio_port_enable(test_ctx->gpio_ctx, p_test_input);
#endif
#if ON_TILE(1)
    const rtos_gpio_port_id_t p_test_output = rtos_gpio_port(OUTPUT_PORT);
    gpio_printf("Enable output port 0x%x", p_test_output);
    rtos_gpio_port_enable(test_ctx->gpio_ctx, p_test_output);
#endif

    do
    {
        sync(c);
        if (test_ctx->main_test[test_ctx->cur_test] != NULL)
        {
            GPIO_MAIN_TEST_ATTR gpio_main_test_t fn;
            fn = test_ctx->main_test[test_ctx->cur_test];
            int tmp = fn(test_ctx);
            retval = (retval != -1) ? tmp : retval;
        } else {
            gpio_printf("Missing main_test callback on test %d", test_ctx->cur_test);
            retval = -1;
        }
    } while (++test_ctx->cur_test < test_ctx->test_cnt);

    return retval;
}

static void start_gpio_devices(gpio_test_ctx_t *test_ctx)
{
    gpio_printf("Rpc configure");
    rtos_gpio_rpc_config(test_ctx->gpio_ctx, GPIO_RPC_PORT, GPIO_RPC_HOST_TASK_PRIORITY);

#if ON_TILE(1)
    gpio_printf("Device start");
    rtos_gpio_start(test_ctx->gpio_ctx);
#endif

    gpio_printf("Device setup done");
}

static void register_gpio_tests(gpio_test_ctx_t *test_ctx)
{
    register_io_test(test_ctx);

    register_rpc_io_test(test_ctx);
}

static void gpio_init_tests(gpio_test_ctx_t *test_ctx, rtos_gpio_t *gpio_ctx)
{
    memset(test_ctx, 0, sizeof(gpio_test_ctx_t));
    test_ctx->gpio_ctx = gpio_ctx;

    test_ctx->cur_test = 0;
    test_ctx->test_cnt = 0;

    register_gpio_tests(test_ctx);
    configASSERT(test_ctx->test_cnt <= GPIO_MAX_TESTS);
}

int gpio_device_tests(rtos_gpio_t *gpio_ctx, chanend_t c)
{
    gpio_test_ctx_t test_ctx;
    int res = 0;

    sync(c);
    gpio_printf("Init test context");
    gpio_init_tests(&test_ctx, gpio_ctx);
    gpio_printf("Test context init");

    sync(c);
    gpio_printf("Start devices");
    start_gpio_devices(&test_ctx);
    gpio_printf("Devices started");

    sync(c);
    gpio_printf("Start tests");
    res = run_gpio_tests(&test_ctx, c);

    sync(c);   // Sync before return
    return res;
}
