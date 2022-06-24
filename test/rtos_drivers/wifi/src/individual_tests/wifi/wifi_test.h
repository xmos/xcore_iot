// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef WIFI_TEST_H_
#define WIFI_TEST_H_

#include "rtos_test/rtos_test_utils.h"
#include "rtos_spi_master.h"
#include "rtos_qspi_flash.h"
#include "rtos_gpio.h"

#define wifi_printf( FMT, ... )       module_printf("WIFI", FMT, ##__VA_ARGS__)

#define WIFI_MAX_TESTS   1

#define WIFI_MAIN_TEST_ATTR      __attribute__((fptrgroup("rtos_test_wifi_main_test_fptr_grp")))

typedef struct wifi_test_ctx wifi_test_ctx_t;

struct wifi_test_ctx {
    uint32_t cur_test;
    uint32_t test_cnt;
    char *name[WIFI_MAX_TESTS];

    rtos_spi_master_device_t *wifi_ctx;
    rtos_gpio_t *gpio_ctx;
    rtos_qspi_flash_t *qspi_flash_ctx;
    rtos_spi_master_t *spi_master_ctx;

    WIFI_MAIN_TEST_ATTR int (*main_test[WIFI_MAX_TESTS])(wifi_test_ctx_t *ctx);
};

typedef int (*wifi_main_test_t)(wifi_test_ctx_t *ctx);

int wifi_device_tests(rtos_spi_master_device_t *wifi_device_ctx, rtos_gpio_t *gpio_ctx, rtos_qspi_flash_t *qspi_flash_ctx, rtos_spi_master_t *spi_master_ctx, chanend_t c);

/* Local Tests */
void register_wifi_test(wifi_test_ctx_t *test_ctx);

/* Initalize WiFi */
void wifi_start(rtos_spi_master_device_t *wifi_device_ctx, rtos_gpio_t *gpio_ctx);

#endif /* WIFI_TEST_H_ */
