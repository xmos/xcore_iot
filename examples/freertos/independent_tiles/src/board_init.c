// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#include <platform.h>
#include <xs1.h>
#include <stdarg.h>
#include <xcore/hwtimer.h>

#include "board_init.h"

typedef enum {
    PORT_INPUT = 0,
    PORT_OUTPUT = 1,
    PORT_INOUT = 2,
} port_direction_t;

static port_t port_init(port_t p, port_direction_t d, port_type_t t, ...)
{
    if (t == PORT_UNBUFFERED) {
        port_enable(p);
    } else {
        size_t tw;
        va_list ap;
        va_start(ap, t);
        tw = va_arg(ap, size_t);
        va_end(ap);
        port_start_buffered(p, tw);
    }

    if (d == PORT_OUTPUT) {
        /* ensure port is in output mode. */
        port_out(p, 0);
    }

    return p;
}

static xclock_t clock_init(xclock_t c)
{
    clock_enable(c);

    return c;
}

static void set_app_pll(void)
{
    unsigned tileid = get_local_tile_id();

    xassert(AUDIO_CLOCK_FREQUENCY == 24576000);

    // 24MHz in, 24.576MHz out, integer mode
    // Found exact solution:   IN  24000000.0, OUT  24576000.0, VCO 2457600000.0, RD  5, FD  512                       , OD  5, FOD   10
    const unsigned APP_PLL_DISABLE = 0x0201FF04;
    const unsigned APP_PLL_CTL_0   = 0x0A01FF04;
    const unsigned APP_PLL_DIV_0   = 0x80000004;
    const unsigned APP_PLL_FRAC_0  = 0x00000000;

    write_sswitch_reg(tileid, XS1_SSWITCH_SS_APP_PLL_CTL_NUM, APP_PLL_DISABLE);

    hwtimer_t tmr = hwtimer_alloc();
    {
        xassert(tmr != 0);
        hwtimer_delay(tmr, 100000); // 1ms with 100 MHz timer tick
    }
    hwtimer_free(tmr);

    write_sswitch_reg(tileid, XS1_SSWITCH_SS_APP_PLL_CTL_NUM, APP_PLL_CTL_0);
    write_sswitch_reg(tileid, XS1_SSWITCH_SS_APP_PLL_CTL_NUM, APP_PLL_CTL_0);
    write_sswitch_reg(tileid, XS1_SSWITCH_SS_APP_PLL_FRAC_N_DIVIDER_NUM, APP_PLL_FRAC_0);
    write_sswitch_reg(tileid, XS1_SSWITCH_SS_APP_CLK_DIVIDER_NUM, APP_PLL_DIV_0);
}

#if I2C_RPC_ENABLED
static rtos_driver_rpc_t i2c_rpc_config;
#endif
#if MIC_ARRAY_RPC_ENABLED
static rtos_driver_rpc_t mic_rpc_config;
#endif
#if I2S_RPC_ENABLED
static rtos_driver_rpc_t i2s_rpc_config;
#endif
#if GPIO_RPC_ENABLED
static rtos_driver_rpc_t gpio_rpc_config;
#endif
#if SPI_RPC_ENABLED
static rtos_driver_rpc_t spi_rpc_config;
#endif
#if QSPI_FLASH_RPC_ENABLED
static rtos_driver_rpc_t qspi_flash_rpc_config;
#endif

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile1_ctx,
        rtos_intertile_t *intertile2_ctx,
        rtos_mic_array_t *mic_array_ctx,
        rtos_i2s_master_t *i2s_master_ctx,
        rtos_i2c_master_t *i2c_master_ctx,
        rtos_spi_master_t *spi_master_ctx,
        rtos_qspi_flash_t *qspi_flash_ctx,
        rtos_spi_master_device_t *wifi_device_ctx,
        rtos_gpio_t *gpio_ctx)
{
    rtos_intertile_init(intertile1_ctx, tile1);
    rtos_intertile_init(intertile2_ctx, tile1);
    rtos_intertile_t *client_intertile_ctx[1] = {intertile1_ctx};

    rtos_i2c_master_init(
            i2c_master_ctx,
            PORT_I2C_SCL, 0, 0,
            PORT_I2C_SDA, 0, 0,
            0,
            100);

    rtos_spi_master_init(
            spi_master_ctx,
            XS1_CLKBLK_1,
            WIFI_CS_N,
            WIFI_CLK,
            WIFI_MOSI,
            WIFI_MISO);

    rtos_spi_master_device_init(
            wifi_device_ctx,
            spi_master_ctx,
            1, /* WiFi CS pin is on bit 1 of the CS port */
            SPI_MODE_0,
            spi_master_source_clock_ref,
            0, /* 50 MHz */
            spi_master_sample_delay_2, /* what should this be? 2? 3? 4? */
            0, /* should this be > 0 if the above is 3-4 ? */
            1,
            0,
            0);

    rtos_qspi_flash_init(
            qspi_flash_ctx,
            XS1_CLKBLK_2,
            PORT_SQI_CS,
            PORT_SQI_SCLK,
            PORT_SQI_SIO,

            /** Derive QSPI clock from the 700 MHz xcore clock **/
            qspi_io_source_clock_xcore,

            /** Full speed clock configuration **/
            5, // 700 MHz / (2*5) -> 70 MHz,
            1,
            qspi_io_sample_edge_rising,
            0,
            /** SPI read clock configuration **/
            12, // 700 MHz / (2*12) -> ~29 MHz
            0,
            qspi_io_sample_edge_falling,
            0,

            1, /* Enable quad page programming */
            QSPI_IO_BYTE_TO_MOSI(0x38), /* The quad page program command */

            QSPI_IO_BYTE_TO_MOSI(0x05),  /* The quad enable register read command */
            QSPI_IO_BYTE_TO_MOSI(0x01),  /* The quad enable register write command */
            0x40,                        /* quad_enable_bitmask */

            256, /* page size is 256 bytes */
            16384); /* the flash has 16384 pages */

    rtos_gpio_init(
            gpio_ctx);

#if GPIO_RPC_ENABLED
    rtos_gpio_rpc_host_init(
            gpio_ctx,
            &gpio_rpc_config,
            client_intertile_ctx,
            1);
#endif
#if I2C_RPC_ENABLED
    rtos_i2c_master_rpc_host_init(
            i2c_master_ctx,
            &i2c_rpc_config,
            client_intertile_ctx,
            1);
#endif
#if SPI_RPC_ENABLED
    rtos_spi_master_rpc_host_init(
            spi_master_ctx,
            &wifi_device_ctx, 1,
            &spi_rpc_config,
            client_intertile_ctx,
            1);
#endif
#if QSPI_FLASH_RPC_ENABLED
    rtos_qspi_flash_rpc_host_init(
            qspi_flash_ctx,
            &qspi_flash_rpc_config,
            client_intertile_ctx,
            1);
#endif
#if MIC_ARRAY_RPC_ENABLED
    rtos_mic_array_rpc_client_init(
            mic_array_ctx,
            &mic_rpc_config,
            intertile1_ctx);
#endif
#if I2S_RPC_ENABLED
    rtos_i2s_master_rpc_client_init(
            i2s_master_ctx,
            &i2s_rpc_config,
            intertile1_ctx);
#endif
}

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile1_ctx,
        rtos_intertile_t *intertile2_ctx,
        rtos_mic_array_t *mic_array_ctx,
        rtos_i2s_master_t *i2s_master_ctx,
        rtos_i2c_master_t *i2c_master_ctx,
        rtos_spi_master_t *spi_master_ctx,
        rtos_qspi_flash_t *qspi_flash_ctx,
        rtos_spi_master_device_t *wifi_device_ctx,
        rtos_gpio_t *gpio_ctx)
{
    port_t p_rst_shared = port_init(PORT_CODEC_RST_N, PORT_OUTPUT, PORT_UNBUFFERED);
    port_out(p_rst_shared, 0xF);

    /* Clock blocks for PDM mics */
    xclock_t pdmclk = clock_init(XS1_CLKBLK_1);
    xclock_t pdmclk2 = clock_init(XS1_CLKBLK_2);

    /* Clock port for the PDM mics and I2S */
    port_t p_mclk = port_init(PORT_MCLK_IN, PORT_INPUT, PORT_UNBUFFERED);

    /* Ports for the PDM microphones */
    port_t p_pdm_clk = port_init(PORT_PDM_CLK,   PORT_OUTPUT, PORT_UNBUFFERED);
    port_t p_pdm_mics = port_init(PORT_PDM_DATA, PORT_INPUT,  PORT_BUFFERED, 32);

    /* Ports for the I2S. */
    port_t p_i2s_dout[1] = {
            PORT_I2S_DAC_DATA
    };
    port_t p_bclk = PORT_I2S_BCLK;
    port_t p_lrclk = PORT_I2S_LRCLK;

    /* Clock blocks for I2S */
    xclock_t bclk = clock_init(XS1_CLKBLK_3);

    set_app_pll();

    rtos_intertile_init(intertile1_ctx, tile0);
    rtos_intertile_init(intertile2_ctx, tile0);
    rtos_intertile_t *client_intertile_ctx[1] = {intertile1_ctx};

    rtos_mic_array_init(
            mic_array_ctx,
            pdmclk,
            pdmclk2,
            AUDIO_CLOCK_FREQUENCY / PDM_CLOCK_FREQUENCY,
            p_mclk,
            p_pdm_clk,
            p_pdm_mics);

    rtos_i2s_master_init(
            i2s_master_ctx,
            p_i2s_dout,
            1,
            NULL,
            0,
            p_bclk,
            p_lrclk,
            p_mclk,
            bclk);

#if GPIO_RPC_ENABLED
    rtos_gpio_rpc_client_init(
            gpio_ctx,
            &gpio_rpc_config,
            intertile1_ctx);
#endif
#if I2C_RPC_ENABLED
    rtos_i2c_master_rpc_client_init(
            i2c_master_ctx,
            &i2c_rpc_config,
            intertile1_ctx);
#endif
#if SPI_RPC_ENABLED
    rtos_spi_master_rpc_client_init(
            spi_master_ctx,
            &wifi_device_ctx, 1,
            &spi_rpc_config,
            intertile1_ctx);
#endif
#if QSPI_FLASH_RPC_ENABLED
    rtos_qspi_flash_rpc_client_init(
            qspi_flash_ctx,
            &qspi_flash_rpc_config,
            intertile1_ctx);
#endif
#if MIC_ARRAY_RPC_ENABLED
    rtos_mic_array_rpc_host_init(
            mic_array_ctx,
            &mic_rpc_config,
            client_intertile_ctx,
            1);
#endif
#if I2S_RPC_ENABLED
    rtos_i2s_master_rpc_host_init(
            i2s_master_ctx,
            &i2s_rpc_config,
            client_intertile_ctx,
            1);
#endif
}
