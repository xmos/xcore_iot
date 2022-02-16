// Copyright (c) 2021-2022 XMOS LIMITED. This Software is subject to the terms of the
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

static rtos_mic_array_t mic_array_ctx_s;
rtos_mic_array_t *mic_array_ctx = &mic_array_ctx_s;

static rtos_i2c_master_t i2c_master_ctx_s;
rtos_i2c_master_t *i2c_master_ctx = &i2c_master_ctx_s;

static rtos_i2s_t i2s_ctx_s;
rtos_i2s_t *i2s_ctx = &i2s_ctx_s;
