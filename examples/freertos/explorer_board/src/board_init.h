// Copyright (c) 2021, XMOS Ltd, All rights reserved

#ifndef BOARD_INIT_H_
#define BOARD_INIT_H_

#include "drivers/rtos/intertile/api/rtos_intertile.h"
#include "drivers/rtos/mic_array/FreeRTOS/rtos_mic_array.h"
#include "drivers/rtos/i2c/api/rtos_i2c_master.h"
#include "drivers/rtos/i2s/api/rtos_i2s_master.h"
#include "drivers/rtos/spi/api/rtos_spi_master.h"
#include "drivers/rtos/qspi_flash/api/rtos_qspi_flash.h"
#include "drivers/rtos/gpio/api/rtos_gpio.h"

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile1_ctx,
        rtos_i2c_master_t *i2c_master_ctx,
        rtos_spi_master_t *spi_master_ctx,
        rtos_qspi_flash_t *qspi_flash_ctx,
        rtos_spi_master_device_t *wifi_device_ctx,
        rtos_gpio_t *gpio_ctx);

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile1_ctx,
        rtos_mic_array_t *mic_array_ctx,
        rtos_i2s_master_t *i2s_master_ctx,
        rtos_i2c_master_t *i2c_master_ctx,
        rtos_gpio_t *gpio_ctx);

#endif /* BOARD_INIT_H_ */
