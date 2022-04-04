// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef DRIVER_INSTANCES_H_
#define DRIVER_INSTANCES_H_

#include "rtos_gpio.h"
#include "rtos_i2c_master.h"
#include "rtos_i2c_slave.h"
#include "rtos_intertile.h"
#include "rtos_i2s.h"
#include "rtos_mic_array.h"
#include "rtos_qspi_flash.h"
#include "rtos_spi_slave.h"

/* Tile specifiers */
#define FLASH_TILE_NO      0
#define I2C_TILE_NO        0
#define I2C_CTRL_TILE_NO   1
#define SPI_OUTPUT_TILE_NO 0
#define MICARRAY_TILE_NO   1
#define I2S_TILE_NO        1

/** TILE 0 Clock Blocks */
#define FLASH_CLKBLK  XS1_CLKBLK_1
// #define UNUSED_CLKBLK XS1_CLKBLK_2
#define SPI_CLKBLK    XS1_CLKBLK_3
#define XUD_CLKBLK_1  XS1_CLKBLK_4 /* Reserved for lib_xud */
#define XUD_CLKBLK_2  XS1_CLKBLK_5 /* Reserved for lib_xud */

/** TILE 1 Clock Blocks */
#define PDM_CLKBLK_1  XS1_CLKBLK_1
#define PDM_CLKBLK_2  XS1_CLKBLK_2
#define I2S_CLKBLK    XS1_CLKBLK_3
#define MCLK_CLKBLK   XS1_CLKBLK_4
// #define UNUSED_CLKBLK XS1_CLKBLK_5

/* Port definitions */
#define PORT_MCLK           PORT_MCLK_IN_OUT
#define PORT_SQI_CS         PORT_SQI_CS_0
#define PORT_SQI_SCLK       PORT_SQI_SCLK_0
#define PORT_SQI_SIO        PORT_SQI_SIO_0
#define PORT_I2S_DAC_DATA   I2S_DATA_IN
#define PORT_I2S_ADC_DATA   I2S_MIC_DATA
#define PORT_I2C_SLAVE_SCL  PORT_I2C_SCL
#define PORT_I2C_SLAVE_SDA  PORT_I2C_SDA
#define PORT_SPI_CS         PORT_SSB
#define PORT_SPI_SCLK       PORT_SQI_SCLK_0

extern rtos_intertile_t *intertile_ctx;
extern rtos_qspi_flash_t *qspi_flash_ctx;
extern rtos_gpio_t *gpio_ctx_t0;
extern rtos_gpio_t *gpio_ctx_t1;
extern rtos_mic_array_t *mic_array_ctx;
extern rtos_i2c_master_t *i2c_master_ctx;
extern rtos_i2c_slave_t *i2c_slave_ctx;
extern rtos_spi_slave_t *spi_slave_ctx;
extern rtos_i2s_t *i2s_ctx;

#endif /* DRIVER_INSTANCES_H_ */
