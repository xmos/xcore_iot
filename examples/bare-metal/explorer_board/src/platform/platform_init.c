// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* System headers */
#include <platform.h>
#include <xcore/chanend.h>
#include <xcore/channel_streaming.h>

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

static void tile0_setup_mclk(void);
static void tile0_init_i2c(void);
static void tile0_init_spi(void);
static void tile0_init_spi_device(spi_master_t *spi_ctx);

static void tile1_setup_dac(void);
static void tile1_i2s_init(void);
static void tile1_mic_init(void);

void platform_init_tile_0(chanend_t c_other_tile)
{
    memset(tile0_ctx, 0, sizeof(tile0_ctx_t));

    tile0_ctx->c_gpio = soc_channel_establish(c_other_tile, soc_channel_inout);

    tile0_setup_mclk();

    /* Wait for CODEC reset to be complete */
    (void)chanend_in_byte(c_other_tile);

    tile0_init_i2c();

    /* Configure DAC */
    if (aic3204_init(&(tile0_ctx->i2c_ctx)) != 0) {
        debug_printf("DAC Failed\n");
        chanend_out_byte(c_other_tile, 0x01);
    } else {
        debug_printf("DAC Initialized\n");
        chanend_out_byte(c_other_tile, 0x00);
    }

    tile0_init_spi();
    tile0_init_spi_device(&tile0_ctx->spi_ctx);
}

void platform_init_tile_1(chanend_t c_other_tile)
{
    memset(tile1_ctx, 0, sizeof(tile1_ctx_t));

    tile1_ctx->c_gpio = soc_channel_establish(c_other_tile, soc_channel_inout);

    /* Reset CODEC */
    tile1_setup_dac();

    /* Signal codec reset complete */
    chanend_out_byte(c_other_tile, 0x00);

    /* Wait for DAC initialization to be complete */
    char ret_char = chanend_in_byte(c_other_tile);
    if (ret_char != 0) {
        debug_printf("DAC init failed on other tile\n");
    }

    app_pll_init();

    tile1_mic_init();
    tile1_i2s_init();
}

static void tile0_init_spi(void)
{
    spi_master_init(&tile0_ctx->spi_ctx,
        SPI_CLKBLK,
        WIFI_CS_N,
        WIFI_CLK,
        WIFI_MOSI,
        WIFI_MISO);
}

static void tile0_init_spi_device(spi_master_t *spi_ctx)
{
    spi_master_device_init(&tile0_ctx->spi_device_ctx,
        spi_ctx,
        1, /* WiFi CS pin is on bit 1 of the CS port */
        SPI_MODE_0,
        spi_master_source_clock_ref,
        0, /* 50 MHz */
        spi_master_sample_delay_0,
        0, 1 ,0 ,0 );
}

static void tile0_setup_mclk(void)
{
    port_enable(PORT_MCLK_IN);
    clock_enable(MCLK_CLKBLK);
    clock_set_source_port(MCLK_CLKBLK, PORT_MCLK_IN);
    port_set_clock(PORT_MCLK_IN, MCLK_CLKBLK);
    clock_start(MCLK_CLKBLK);
}

static void tile0_init_i2c(void)
{
    i2c_master_init(
            &(tile0_ctx->i2c_ctx),
            PORT_I2C_SCL, 0, 0,
            PORT_I2C_SDA, 0, 0,
            100); /* kbps */
}

static void tile1_setup_dac(void)
{
    const port_t codec_rst_port = PORT_CODEC_RST_N;
    port_enable(codec_rst_port);
    port_out(codec_rst_port, 0xF);
}

static int i2s_mclk_bclk_ratio(
        const unsigned audio_clock_frequency,
        const unsigned sample_rate)
{
    return audio_clock_frequency / (sample_rate * (8 * sizeof(int32_t)) * I2S_CHANS_PER_FRAME);
}

I2S_CALLBACK_ATTR
static void i2s_init(chanend_t *input_c, i2s_config_t *i2s_config)
{
    i2s_config->mode = I2S_MODE_I2S;
    i2s_config->mclk_bclk_ratio =  i2s_mclk_bclk_ratio(appconfAUDIO_CLOCK_FREQUENCY, appconfPIPELINE_AUDIO_SAMPLE_RATE);
}

I2S_CALLBACK_ATTR
static i2s_restart_t i2s_restart_check(chanend_t *input_c)
{
    return I2S_NO_RESTART;
}

I2S_CALLBACK_ATTR
static void i2s_receive(chanend_t *input_c, size_t num_in, const int32_t *i2s_sample_buf)
{
    return;
}

I2S_CALLBACK_ATTR
static void i2s_send(chanend_t *input_c, size_t num_out, int32_t *i2s_sample_buf)
{
    s_chan_in_buf_word(*input_c, (uint32_t*)i2s_sample_buf, MIC_DUAL_NUM_CHANNELS);
}

static void tile1_i2s_init(void)
{
    tile1_ctx->i2s_cb_group.init = (i2s_init_t) i2s_init;
    tile1_ctx->i2s_cb_group.restart_check = (i2s_restart_check_t) i2s_restart_check;
    tile1_ctx->i2s_cb_group.receive = (i2s_receive_t) i2s_receive;
    tile1_ctx->i2s_cb_group.send = (i2s_send_t) i2s_send;
    tile1_ctx->i2s_cb_group.app_data = &tile1_ctx->c_i2s_to_dac;

    tile1_ctx->p_i2s_dout[0] = PORT_I2S_DAC_DATA;
    tile1_ctx->p_bclk = PORT_I2S_BCLK;
    tile1_ctx->p_lrclk = PORT_I2S_LRCLK;
    tile1_ctx->p_mclk = PORT_MCLK_IN;
    tile1_ctx->bclk = XS1_CLKBLK_3;
}

static void tile1_mic_init(void)
{
    tile1_ctx->p_pdm_mic = PORT_PDM_DATA;
    tile1_ctx->pdm_decimation_factor = mic_array_decimation_factor(
            appconfPDM_CLOCK_FREQUENCY,
            appconfPIPELINE_AUDIO_SAMPLE_RATE);

    port_enable(tile1_ctx->p_pdm_mic);
    mic_array_setup_ddr(PDM_CLKBLK_1,
                        PDM_CLKBLK_2,
                        PORT_MCLK_IN,
                        PORT_PDM_CLK,
                        PORT_PDM_DATA,
                        appconfAUDIO_CLOCK_FREQUENCY / appconfPDM_CLOCK_FREQUENCY);
}
