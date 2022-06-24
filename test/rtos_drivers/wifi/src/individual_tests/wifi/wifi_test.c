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
#include "fs_support.h"

/* App headers */
#include "app_conf.h"
#include "individual_tests/wifi/wifi_test.h"

static int run_wifi_tests(wifi_test_ctx_t *test_ctx, chanend_t c)
{
    int retval = 0;

    do
    {
        sync(c);
        if (test_ctx->main_test[test_ctx->cur_test] != NULL)
        {
            WIFI_MAIN_TEST_ATTR wifi_main_test_t fn;
            fn = test_ctx->main_test[test_ctx->cur_test];
            int tmp = fn(test_ctx);
            retval = (retval != -1) ? tmp : retval;
        } else {
            wifi_printf("Missing main_test callback on test %d", test_ctx->cur_test);
            retval = -1;
        }
    } while (++test_ctx->cur_test < test_ctx->test_cnt);

    return retval;
}

static void start_wifi_devices(wifi_test_ctx_t *test_ctx)
{
    wifi_printf("Device start");
#if ON_TILE(0)
    rtos_gpio_start(test_ctx->gpio_ctx);
    rtos_spi_master_start(test_ctx->spi_master_ctx, configMAX_PRIORITIES-1);
    rtos_qspi_flash_start(test_ctx->qspi_flash_ctx, configMAX_PRIORITIES-1);
    rtos_fatfs_init(test_ctx->qspi_flash_ctx);
    wifi_start(test_ctx->wifi_ctx, test_ctx->gpio_ctx);
#endif
    wifi_printf("Device setup done");
}

static void register_wifi_tests(wifi_test_ctx_t *test_ctx)
{
    register_wifi_test(test_ctx);
}

static void wifi_init_tests(wifi_test_ctx_t *test_ctx, rtos_spi_master_device_t *wifi_device_ctx, rtos_gpio_t *gpio_ctx, rtos_qspi_flash_t *qspi_flash_ctx, rtos_spi_master_t *spi_master_ctx)
{
    memset(test_ctx, 0, sizeof(wifi_test_ctx_t));
    test_ctx->wifi_ctx = wifi_device_ctx;
    test_ctx->gpio_ctx = gpio_ctx;
    test_ctx->qspi_flash_ctx = qspi_flash_ctx;
    test_ctx->spi_master_ctx = spi_master_ctx;

    test_ctx->cur_test = 0;
    test_ctx->test_cnt = 0;

    register_wifi_tests(test_ctx);
    configASSERT(test_ctx->test_cnt <= WIFI_MAX_TESTS);
}

int wifi_device_tests(rtos_spi_master_device_t *wifi_device_ctx, rtos_gpio_t *gpio_ctx, rtos_qspi_flash_t *qspi_flash_ctx, rtos_spi_master_t *spi_master_ctx, chanend_t c)
{
    wifi_test_ctx_t test_ctx;
    int res = 0;

    sync(c);
    wifi_printf("Init test context");
    wifi_init_tests(&test_ctx, wifi_device_ctx, gpio_ctx, qspi_flash_ctx, spi_master_ctx);
    wifi_printf("Test context init");

    sync(c);
    wifi_printf("Start devices");
    start_wifi_devices(&test_ctx);
    wifi_printf("Devices started");

    sync(c);
    wifi_printf("Start tests");
    res = run_wifi_tests(&test_ctx, c);

    sync(c);   // Sync before return
    return res;
}
