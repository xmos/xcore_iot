// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef BOARD_INIT_H_
#define BOARD_INIT_H_

#include "drivers/rtos/intertile/FreeRTOS/rtos_intertile.h"
#include "drivers/rtos/mic_array/FreeRTOS/rtos_mic_array.h"
#include "drivers/rtos/i2c/FreeRTOS/rtos_i2c_master.h"
#include "drivers/rtos/i2s/FreeRTOS/rtos_i2s_master.h"
#include "drivers/rtos/spi/FreeRTOS/rtos_spi_master.h"
#include "drivers/rtos/qspi_flash/FreeRTOS/rtos_qspi_flash.h"
#include "drivers/rtos/gpio/api/rtos_gpio.h"

#define AUDIO_CLOCK_FREQUENCY 24576000
#define PDM_CLOCK_FREQUENCY    3072000

#define I2C_RPC_ENABLED 1
#define MIC_ARRAY_RPC_ENABLED 1
#define I2S_RPC_ENABLED 1
#define GPIO_RPC_ENABLED 1
#define SPI_RPC_ENABLED 1
#define QSPI_FLASH_RPC_ENABLED 0

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile1_ctx,
        rtos_intertile_t *intertile2_ctx,
        rtos_mic_array_t *mic_array_ctx,
        rtos_i2s_master_t *i2s_master_ctx,
        rtos_i2c_master_t *i2c_master_ctx,
        rtos_spi_master_t *spi_master_ctx,
        rtos_qspi_flash_t *qspi_flash_ctx,
        rtos_spi_master_device_t *wifi_device_ctx,
        rtos_gpio_t *gpio_ctx);

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile1_ctx,
        rtos_intertile_t *intertile2_ctx,
        rtos_mic_array_t *mic_array_ctx,
        rtos_i2s_master_t *i2s_master_ctx,
        rtos_i2c_master_t *i2c_master_ctx,
        rtos_spi_master_t *spi_master_ctx,
        rtos_qspi_flash_t *qspi_flash_ctx,
        rtos_spi_master_device_t *wifi_device_ctx,
        rtos_gpio_t *gpio_ctx);

#endif /* BOARD_INIT_H_ */
