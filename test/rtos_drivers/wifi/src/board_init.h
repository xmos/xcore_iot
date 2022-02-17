// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef BOARD_INIT_H_
#define BOARD_INIT_H_

#include "rtos_intertile.h"
#include "rtos_spi_master.h"
#include "rtos_qspi_flash.h"
#include "rtos_gpio.h"

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile_ctx,
        rtos_spi_master_t *spi_master_ctx,
        rtos_qspi_flash_t *qspi_flash_ctx,
        rtos_gpio_t *gpio_ctx,
        rtos_spi_master_device_t *wifi_device_ctx
    );

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile_ctx
    );

#endif /* BOARD_INIT_H_ */
