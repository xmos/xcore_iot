// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* System headers */
#include <platform.h>

/* SDK headers */
#include "soc.h"
#include "mic_array.h"
#include "xcore_utils.h"

#include "app_conf.h"
#include "platform_init.h"
#include "tile_support.h"
#include "resource_map.h"

#include "app_pll_ctrl.h"
#include "aic3204.h"
#include "mic_support.h"

#include "i2c.h"
#include "i2s.h"

static void platform_init_common(tile_ctx_t* ctx, chanend_t c_other_tile)
{
    memset(ctx, 0, sizeof(tile_ctx_t));
    ctx->c_other_tile = c_other_tile;
}

void platform_init_tile_0(chanend_t c_other_tile)
{
    platform_init_common(tile0_ctx, c_other_tile);
    tile0_ctx->local_ctx = l_tile0_ctx;
    memset(l_tile0_ctx, 0, sizeof(tile0_ctx_t));

    port_enable(PORT_MCLK_IN);
    clock_enable(MCLK_CLKBLK);
    clock_set_source_port(MCLK_CLKBLK, PORT_MCLK_IN);
    port_set_clock(PORT_MCLK_IN, MCLK_CLKBLK);
    clock_start(MCLK_CLKBLK);

    /* Wait for CODEC reset to be complete */
    (void)chanend_in_word(c_other_tile);

    /* Init I2C */
    i2c_master_t* i2c_ctx_ptr = &(l_tile0_ctx->i2c_ctx);
    i2c_master_init(
            i2c_ctx_ptr,
            PORT_I2C_SCL, 0, 0,
            PORT_I2C_SDA, 0, 0,
            100); /* kbps */

    /* Configure DAC */
    if (aic3204_init(i2c_ctx_ptr) != 0) {
        debug_printf("DAC Failed\n");
    } else {
        debug_printf("DAC Initialized\n");
        chanend_out_word(c_other_tile, 0xDEADBEEF);
    }
}

void platform_init_tile_1(chanend_t c_other_tile)
{
    platform_init_common(tile1_ctx, c_other_tile);
    tile1_ctx->local_ctx = l_tile1_ctx;
    memset(l_tile1_ctx, 0, sizeof(tile1_ctx_t));

    /* Reset CODEC */
    port_t codec_rst_port = PORT_CODEC_RST_N;
    port_enable(codec_rst_port);
    port_out(codec_rst_port, 0xF);
    chanend_out_word(c_other_tile, 0xA5A5A5A5);

    /* Wait for DAC initialization to be complete */
    (void)chanend_in_word(c_other_tile);

    l_tile1_ctx->p_pdm_mic = PORT_PDM_DATA;
    port_enable(l_tile1_ctx->p_pdm_mic);
    l_tile1_ctx->pdm_decimation_factor = mic_array_decimation_factor(
            appconfPDM_CLOCK_FREQUENCY,
            appconfPIPELINE_AUDIO_SAMPLE_RATE);

    app_pll_init();

    mic_array_setup_ddr(PDM_CLKBLK_1,
                        PDM_CLKBLK_2,
                        PORT_MCLK_IN,
                        PORT_PDM_CLK,
                        PORT_PDM_DATA,
                        appconfAUDIO_CLOCK_FREQUENCY / appconfPDM_CLOCK_FREQUENCY);
}
