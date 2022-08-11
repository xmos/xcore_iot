// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>
#include <stdarg.h>
#include <xcore/hwtimer.h>

#include "app_conf.h"
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

    xassert(appconfAUDIO_CLOCK_FREQUENCY == 24576000);

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

static rtos_driver_rpc_t i2c_master_rpc_config;
static rtos_driver_rpc_t gpio_rpc_config;
static rtos_driver_rpc_t qspi_flash_rpc_config;
static rtos_driver_rpc_t i2s_master_rpc_config;
static rtos_driver_rpc_t mic_array_rpc_config;

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile_ctx,
        rtos_mic_array_t *mic_array_ctx,
        rtos_i2c_master_t *i2c_master_ctx,
        rtos_qspi_flash_t *qspi_flash_ctx,
        rtos_gpio_t *gpio_ctx,
        rtos_i2s_t *i2s_master_ctx,
        rtos_i2s_t *i2s_slave_ctx
    )
{
    rtos_intertile_init(intertile_ctx, tile1);
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};

    rtos_i2c_master_init(
            i2c_master_ctx,
            PORT_I2C_SCL, 0, 0,
            PORT_I2C_SDA, 0, 0,
            0,
            100);

    /* Manual parameters required for Explorer Board 2V0 */
    qspi_flash_ctx->ctx.sfdp_skip = true;
    qspi_flash_ctx->ctx.sfdp_supported = false;
    qspi_flash_ctx->ctx.page_size_bytes = 256;
    qspi_flash_ctx->ctx.page_count = 16384;
    qspi_flash_ctx->ctx.flash_size_kbytes = 4096;
    qspi_flash_ctx->ctx.address_bytes = 3;
    qspi_flash_ctx->ctx.erase_info[0].size_log2 = 12;
    qspi_flash_ctx->ctx.erase_info[0].cmd = 0xEEFEEEEE;
    qspi_flash_ctx->ctx.erase_info[1].size_log2 = 15;
    qspi_flash_ctx->ctx.erase_info[1].cmd = 0xEFEFEEFE;
    qspi_flash_ctx->ctx.erase_info[2].size_log2 = 16;
    qspi_flash_ctx->ctx.erase_info[2].cmd = 0xFFEFFEEE;
    qspi_flash_ctx->ctx.erase_info[3].size_log2 = 0;
    qspi_flash_ctx->ctx.erase_info[3].cmd = 0;
    qspi_flash_ctx->ctx.busy_poll_cmd = 0xEEEEEFEF;
    qspi_flash_ctx->ctx.busy_poll_bit = 0;
    qspi_flash_ctx->ctx.busy_poll_ready_value = 0;
    qspi_flash_ctx->ctx.qe_reg = 2;
    qspi_flash_ctx->ctx.qe_bit = 1;
    qspi_flash_ctx->ctx.sr2_read_cmd = 0xEEFFEFEF;
    qspi_flash_ctx->ctx.sr2_write_cmd = 0xEEEEEEEE;

    rtos_qspi_flash_init(
            qspi_flash_ctx,
            XS1_CLKBLK_2,
            PORT_SQI_CS,
            PORT_SQI_SCLK,
            PORT_SQI_SIO,

            /** Derive QSPI clock from the 600 MHz xcore clock **/
            qspi_io_source_clock_xcore,

            /** Full speed clock configuration **/
            5, // 600 MHz / (2*5) -> 60 MHz,
            1,
            qspi_io_sample_edge_rising,
            0,

            /** SPI read clock configuration **/
            12, // 600 MHz / (2*12) -> 25 MHz
            0,
            qspi_io_sample_edge_falling,
            0,

            qspi_flash_page_program_1_4_4);

    rtos_i2c_master_rpc_host_init(
            i2c_master_ctx,
            &i2c_master_rpc_config,
            client_intertile_ctx,
            1);

    rtos_gpio_rpc_client_init(
            gpio_ctx,
            &gpio_rpc_config,
            intertile_ctx);

    rtos_qspi_flash_rpc_host_init(
            qspi_flash_ctx,
            &qspi_flash_rpc_config,
            client_intertile_ctx,
            1);

    rtos_i2s_rpc_client_init(
            i2s_master_ctx,
            &i2s_master_rpc_config,
            intertile_ctx);

    /* Ports for the I2S. */
    port_t p_i2s_dout[1] = {
            XS1_PORT_1F
    };
    port_t p_i2s_din[1] = {
            XS1_PORT_1E
    };
    port_t p_bclk = XS1_PORT_1G;
    port_t p_lrclk = XS1_PORT_1H;

    /* Clock blocks for I2S */
    xclock_t bclk = XS1_CLKBLK_3;

    rtos_i2s_slave_init(
           i2s_slave_ctx,
           I2S_SLAVE_CORE_MASK,
           p_i2s_dout,
           1,
           p_i2s_din,
           1,
           p_bclk,
           p_lrclk,
           bclk);

    rtos_mic_array_rpc_client_init(
           mic_array_ctx,
           &mic_array_rpc_config,
           intertile_ctx);
}

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile_ctx,
        rtos_mic_array_t *mic_array_ctx,
        rtos_i2c_master_t *i2c_master_ctx,
        rtos_i2c_slave_t *i2c_slave_ctx,
        rtos_qspi_flash_t *qspi_flash_ctx,
        rtos_gpio_t *gpio_ctx,
        rtos_i2s_t *i2s_master_ctx
    )
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
    port_t p_i2s_din[1] = {
            PORT_I2S_ADC_DATA
    };
    port_t p_bclk = PORT_I2S_BCLK;
    port_t p_lrclk = PORT_I2S_LRCLK;

    /* Clock blocks for I2S */
    xclock_t bclk = clock_init(XS1_CLKBLK_3);

    /* Ports for I2C Slave */
    port_t p_scl = PORT_I2C_SLAVE_SCL;
    port_t p_sda = PORT_I2C_SLAVE_SDA;

    set_app_pll();

    rtos_intertile_init(intertile_ctx, tile0);
    rtos_intertile_t *client_intertile_ctx[1] = {intertile_ctx};

    rtos_gpio_init(
            gpio_ctx);

    rtos_mic_array_init(
            mic_array_ctx,
            MIC_ARRAY_CORE_MASK,
            RTOS_MIC_ARRAY_SAMPLE_CHANNEL);

    rtos_i2s_master_init(
            i2s_master_ctx,
            I2S_MASTER_CORE_MASK,
            p_i2s_dout,
            1,
            p_i2s_din,
            1,
            p_bclk,
            p_lrclk,
            p_mclk,
            bclk);

    rtos_i2c_slave_init(
            i2c_slave_ctx,
            I2C_SLAVE_CORE_MASK,
            p_scl,
            p_sda,
            I2C_SLAVE_ADDR);

    rtos_i2c_master_rpc_client_init(
            i2c_master_ctx,
            &i2c_master_rpc_config,
            intertile_ctx);

    rtos_gpio_rpc_host_init(
            gpio_ctx,
            &gpio_rpc_config,
            client_intertile_ctx,
            1);

    rtos_qspi_flash_rpc_client_init(
            qspi_flash_ctx,
            &qspi_flash_rpc_config,
            intertile_ctx);

    rtos_i2s_rpc_host_init(
            i2s_master_ctx,
            &i2s_master_rpc_config,
            client_intertile_ctx,
            1);

    rtos_mic_array_rpc_host_init(
            mic_array_ctx,
            &mic_array_rpc_config,
            client_intertile_ctx,
            1);
}
