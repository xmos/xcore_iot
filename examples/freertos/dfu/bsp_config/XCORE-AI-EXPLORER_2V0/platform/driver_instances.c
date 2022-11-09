// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include "platform/driver_instances.h"

static rtos_intertile_t intertile_ctx_s;
rtos_intertile_t *intertile_ctx = &intertile_ctx_s;

static rtos_qspi_flash_t qspi_flash_ctx_s;
rtos_qspi_flash_t *qspi_flash_ctx = &qspi_flash_ctx_s;

static rtos_spi_master_t spi_master_ctx_s;
rtos_spi_master_t *spi_master_ctx = &spi_master_ctx_s;

static rtos_spi_master_device_t wifi_device_ctx_s;
rtos_spi_master_device_t *wifi_device_ctx = &wifi_device_ctx_s;

static rtos_gpio_t gpio_ctx_t0_s;
rtos_gpio_t *gpio_ctx_t0 = &gpio_ctx_t0_s;

static rtos_gpio_t gpio_ctx_t1_s;
rtos_gpio_t *gpio_ctx_t1 = &gpio_ctx_t1_s;

static rtos_i2c_master_t i2c_master_ctx_s;
rtos_i2c_master_t *i2c_master_ctx = &i2c_master_ctx_s;

static rtos_uart_tx_t uart_tx_ctx_s;
rtos_uart_tx_t *uart_tx_ctx = &uart_tx_ctx_s;

static rtos_uart_rx_t uart_rx_ctx_s;
rtos_uart_rx_t *uart_rx_ctx = &uart_rx_ctx_s;

static rtos_dfu_image_t dfu_image_ctx_s;
rtos_dfu_image_t *dfu_image_ctx = &dfu_image_ctx_s;
