// Copyright (c) 2021-2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef DRIVER_INSTANCES_H_
#define DRIVER_INSTANCES_H_

#include "rtos_gpio.h"
#include "rtos_intertile.h"
#include "rtos_qspi_flash.h"
#include "rtos_spi_master.h"

#define FLASH_TILE_NO      0

extern rtos_intertile_t *intertile_ctx;
extern rtos_qspi_flash_t *qspi_flash_ctx;
extern rtos_spi_master_t *spi_master_ctx;
extern rtos_spi_master_device_t *wifi_device_ctx;
extern rtos_gpio_t *gpio_ctx;

#endif /* DRIVER_INSTANCES_H_ */
