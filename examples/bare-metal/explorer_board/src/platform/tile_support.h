// Copyright (c) 2021-2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef TILE_SUPPORT_H_
#define TILE_SUPPORT_H_

#include <xcore/chanend.h>

#include "i2c.h"
#include "i2s.h"
#include "spi.h"
#include "qspi_flash.h"
#include "mic_array.h"

typedef struct tile0_struct tile0_ctx_t;
struct tile0_struct {
    chanend_t c_from_gpio;
    chanend_t c_to_gpio;
    spi_master_device_t spi_device_ctx;
    spi_master_t spi_ctx;
    i2c_master_t i2c_ctx;
    qspi_flash_ctx_t qspi_flash_ctx;
    qspi_io_ctx_t qspi_io_ctx;
};

typedef struct tile1_struct tile1_ctx_t;
struct tile1_struct {
    chanend_t c_from_gpio;
    chanend_t c_to_gpio;

    i2s_callback_group_t i2s_cb_group;
    port_t p_i2s_dout[1];
    port_t p_bclk;
    port_t p_lrclk;
    port_t p_mclk;
    xclock_t bclk;
    chanend_t c_i2s_to_dac;
};

extern tile0_ctx_t *tile0_ctx;
extern tile1_ctx_t *tile1_ctx;

#endif /* TILE_SUPPORT_H_ */
