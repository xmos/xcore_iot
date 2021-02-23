// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>
#include <xs1.h>
#include <stdarg.h>
#include <xcore/hwtimer.h>

#include "board_init.h"
#include "app_conf.h"


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

#if OSPREY_BOARD || XCOREAI_EXPLORER
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
#endif

static rtos_driver_rpc_t gpio_rpc_config;
static rtos_driver_rpc_t mic_rpc_config;

void board_tile0_init(
        chanend_t tile1,
        rtos_intertile_t *intertile1_ctx,
        rtos_qspi_flash_t *qspi_flash_ctx,
        rtos_gpio_t *gpio_ctx,
        rtos_mic_array_t *mic_array_ctx)
{
    rtos_intertile_init(intertile1_ctx, tile1);
    rtos_intertile_t *client_intertile_ctx[1] = {intertile1_ctx};

    rtos_gpio_init(
            gpio_ctx);

    rtos_gpio_rpc_host_init(
            gpio_ctx,
            &gpio_rpc_config,
            client_intertile_ctx,
            1);

    rtos_qspi_flash_init(
            qspi_flash_ctx,
            XS1_CLKBLK_1,
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

#if OSPREY_BOARD || XCOREAI_EXPLORER
    rtos_mic_array_rpc_client_init(
            mic_array_ctx,
            &mic_rpc_config,
            intertile1_ctx);
#endif
}

void board_tile1_init(
        chanend_t tile0,
        rtos_intertile_t *intertile1_ctx,
        rtos_gpio_t *gpio_ctx,
        rtos_mic_array_t *mic_array_ctx)
{
#if OSPREY_BOARD || XCOREAI_EXPLORER
    /* Clock blocks for PDM mics */
    xclock_t pdmclk = clock_init(XS1_CLKBLK_1);
    xclock_t pdmclk2 = clock_init(XS1_CLKBLK_2);

    /* Clock port for the PDM mics and I2S */
    port_t p_mclk = port_init(PORT_MCLK_IN, PORT_INPUT, PORT_UNBUFFERED);

    /* Ports for the PDM microphones */
    port_t p_pdm_clk = port_init(PORT_PDM_CLK,   PORT_OUTPUT, PORT_UNBUFFERED);
    port_t p_pdm_mics = port_init(PORT_PDM_DATA, PORT_INPUT,  PORT_BUFFERED, 32);

    set_app_pll();
#endif

    rtos_intertile_init(intertile1_ctx, tile0);
    rtos_intertile_t *client_intertile_ctx[1] = {intertile1_ctx};

#if OSPREY_BOARD || XCOREAI_EXPLORER
    rtos_mic_array_init(
            mic_array_ctx,
            pdmclk,
            pdmclk2,
            AUDIO_CLOCK_FREQUENCY / PDM_CLOCK_FREQUENCY,
            p_mclk,
            p_pdm_clk,
            p_pdm_mics);
#endif

    rtos_gpio_rpc_client_init(
            gpio_ctx,
            &gpio_rpc_config,
            intertile1_ctx);

#if OSPREY_BOARD || XCOREAI_EXPLORER
    rtos_mic_array_rpc_host_init(
            mic_array_ctx,
            &mic_rpc_config,
            client_intertile_ctx,
            1);
#endif
}
