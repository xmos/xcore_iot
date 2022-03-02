// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef DRIVER_INSTANCES_H_
#define DRIVER_INSTANCES_H_

#include "rtos_gpio.h"
#include "rtos_i2c_master.h"
#include "rtos_i2c_slave.h"
#include "rtos_spi_slave.h"
#include "rtos_i2s.h"
#include "rtos_intertile.h"
#include "rtos_mic_array.h"
#include "rtos_qspi_flash.h"
#include "rtos_spi_master.h"

#define FLASH_TILE_NO      0
#define I2C_TILE_NO        0
#define I2C_CTRL_TILE_NO   1
#define SPI_OUTPUT_TILE_NO 0
#define AUDIO_HW_TILE_NO   1
#define WW_TILE_NO       0
#define FS_TILE_NO       0

#if (WW_TILE_NO != FS_TILE_NO)
#error  WW and FS must be on the same tile
#endif

extern rtos_intertile_t *intertile_ctx;
extern rtos_gpio_t *gpio_ctx_t0;
extern rtos_gpio_t *gpio_ctx_t1;
extern rtos_qspi_flash_t *qspi_flash_ctx;
extern rtos_i2c_master_t *i2c_master_ctx;
extern rtos_i2c_slave_t *i2c_slave_ctx;
extern rtos_spi_slave_t *spi_slave_ctx;
extern rtos_mic_array_t *mic_array_ctx;
extern rtos_i2s_t *i2s_ctx;

#endif /* DRIVER_INSTANCES_H_ */
