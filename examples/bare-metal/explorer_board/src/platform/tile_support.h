// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef TILE_SUPPORT_H_
#define TILE_SUPPORT_H_

#include <xcore/chanend.h>

#include "i2c.h"
#include "i2s.h"
#include "spi.h"

typedef struct tile0_struct tile0_ctx_t;
struct tile0_struct {
    chanend_t c_gpio;
    spi_master_device_t spi_device_ctx;
    spi_master_t spi_ctx;
    i2c_master_t i2c_ctx;
};

typedef struct tile1_struct tile1_ctx_t;
struct tile1_struct {
    chanend_t c_gpio;

    port_t p_pdm_mic;
    int pdm_decimation_factor;

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
