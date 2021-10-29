// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* System headers */
#include <platform.h>
#include <xcore/chanend.h>
#include <xcore/channel_streaming.h>

/* SDK headers */
#include "soc.h"
#include "xcore_utils.h"
#include "i2c.h"
#include "spi.h"
#include "qspi_flash.h"

/* App headers */
#include "app_conf.h"
#include "platform_init.h"
#include "tile_support.h"
#include "app_pll_ctrl.h"
#include "aic3204.h"

static void tile0_setup_mclk(void);
static void tile0_init_i2c(void);
static void tile0_init_spi(void);
static void tile0_init_spi_device(spi_master_t *spi_ctx);
static void tile0_init_flash(qspi_flash_ctx_t *qspi_flash_ctx);

void platform_init_tile_0(chanend_t c_other_tile)
{
    memset(tile0_ctx, 0, sizeof(tile0_ctx_t));

    tile0_ctx->c_from_gpio = soc_channel_establish(c_other_tile, soc_channel_output);
    tile0_ctx->c_to_gpio = soc_channel_establish(c_other_tile, soc_channel_input);

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

    tile0_init_flash(&tile0_ctx->qspi_flash_ctx);
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

static void tile0_init_flash(qspi_flash_ctx_t *qspi_flash_ctx)
{
    qspi_io_ctx_t *qspi_io_ctx = &qspi_flash_ctx->qspi_io_ctx;

    qspi_flash_ctx->custom_clock_setup = 1;
    qspi_flash_ctx->quad_page_program_cmd = qspi_flash_page_program_1_4_4;
    qspi_flash_ctx->source_clock = qspi_io_source_clock_xcore;

    qspi_io_ctx->clock_block = FLASH_CLKBLK;
    qspi_io_ctx->cs_port = PORT_SQI_CS;
    qspi_io_ctx->sclk_port = PORT_SQI_SCLK;
    qspi_io_ctx->sio_port = PORT_SQI_SIO;

    /** Full speed clock configuration **/
    qspi_io_ctx->full_speed_clk_divisor = 5; // 600 MHz / (2*5) -> 60 MHz
    qspi_io_ctx->full_speed_sclk_sample_delay = 1;
    qspi_io_ctx->full_speed_sclk_sample_edge = qspi_io_sample_edge_rising;

    /** SPI read clock configuration **/
    qspi_io_ctx->spi_read_clk_divisor = 12;  // 600 MHz / (2*12) -> 25 MHz

    qspi_io_ctx->spi_read_sclk_sample_delay = 0;
    qspi_io_ctx->spi_read_sclk_sample_edge = qspi_io_sample_edge_falling;
    qspi_io_ctx->full_speed_sio_pad_delay = 0;
    qspi_io_ctx->spi_read_sio_pad_delay = 0;

    qspi_flash_init(qspi_flash_ctx);
}
