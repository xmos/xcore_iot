// Copyright (c) 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* App headers */
#include "platform_init.h"

static void tile0_setup_mclk(void);
static void tile0_init_i2c(void);
static void tile0_init_spi(void);
static void tile0_init_spi_device(spi_master_t *spi_ctx);
static void tile0_init_flash(void);

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

    tile0_init_flash();
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

static void tile0_init_flash(void)
{

    fl_QSPIPorts qspi_ports;
    qspi_ports.qspiCS = PORT_SQI_CS;
    qspi_ports.qspiSCLK = PORT_SQI_SCLK;
    qspi_ports.qspiSIO = PORT_SQI_SIO;
    qspi_ports.qspiClkblk = FLASH_CLKBLK;

    fl_QuadDeviceSpec default_spec = FL_QUADDEVICE_DEFAULT;
    fl_connectToDevice(&qspi_ports, &default_spec, 1);
    fl_quadEnable();
}
