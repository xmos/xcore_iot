// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/assert.h>
#include <xcore/chanend.h>
#include <xcore/channel_streaming.h>
#include <xcore/parallel.h>
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include <xcore/triggerable.h>

/* SDK headers */
#include "soc.h"
#include "i2c.h"
#include "mic_array.h"
#include "xcore_utils.h"

/* App headers */
#include "app_conf.h"
#include "platform/app_pll_ctrl.h"
#include "burn.h"

#include "audio_pipeline.h"
#include "mic_support.h"

#include "aic3204.h"


// DECLARE_JOB(configure_dac, (chanend_t c));



// static void tile_common_init(chanend_t c)
// {
//     chanend_free(c);
// }

/** TILE 0 Clock Blocks */
#define MCLK_CLKBLK   XS1_CLKBLK_4

/** TILE 1 Clock Blocks */
#define PDM_CLKBLK_1  XS1_CLKBLK_1
#define PDM_CLKBLK_2  XS1_CLKBLK_2
#define I2S_CLKBLK    XS1_CLKBLK_3

DECLARE_JOB(i2c_task, (i2c_master_t*));

void i2c_task(i2c_master_t* ctx)
{
    port_t codec_rst_port = PORT_CODEC_RST_N;
    port_enable(codec_rst_port);
    port_out(codec_rst_port, 0xF);

    aic3204_init(ctx);

    debug_printf("DAC Initialized\n");
}

void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void)c0;
    (void)c2;
    (void)c3;

    // tile_common_init(c1);
    uint32_t test = 0xDEADBEEF;

    chanend_t c_msg = soc_channel_establish(c1, soc_channel_inout);

    chanend_out_word(c_msg, test);
    chanend_out_control_token(c_msg, XS1_CT_PAUSE);

    port_enable(PORT_MCLK_IN);
    clock_enable(MCLK_CLKBLK);
    clock_set_source_port(MCLK_CLKBLK, PORT_MCLK_IN);
    port_set_clock(PORT_MCLK_IN, MCLK_CLKBLK);
    clock_start(MCLK_CLKBLK);


    i2c_master_t i2c_ctx;
    i2c_master_t* i2c_ctx_ptr = &i2c_ctx;

    i2c_master_init(
            i2c_ctx_ptr,
            XS1_PORT_1N, 0, 0,
            XS1_PORT_1O, 0, 0,
            100); /* kbps */

    PAR_JOBS (
        PJOB(i2c_task, (i2c_ctx_ptr)),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ())
    );
}

void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void)c1;
    (void)c2;
    (void)c3;

    chanend_t c_msg = soc_channel_establish(c0, soc_channel_inout);

    uint32_t test = chanend_in_word(c_msg);

    debug_printf("tile 1 got 0x%x\n", test);

    port_t p_pdm_mic = PORT_PDM_DATA;
    port_enable(p_pdm_mic);
    // port_t p_pdm_clk = XS1_PORT_1G;     // PORT_PDM_CLK


    const int pdm_decimation_factor = mic_array_decimation_factor(
            appconfPDM_CLOCK_FREQUENCY,
            appconfPIPELINE_AUDIO_SAMPLE_RATE);

    app_pll_init();

    mic_array_setup_ddr(PDM_CLKBLK_1,
                        PDM_CLKBLK_2,
                        PORT_MCLK_IN,
                        PORT_PDM_CLK,
                        PORT_PDM_DATA,
                        appconfAUDIO_CLOCK_FREQUENCY / appconfPDM_CLOCK_FREQUENCY);

    streaming_channel_t s_chan_input = s_chan_alloc();

    streaming_channel_t s_chan_ab = s_chan_alloc();
    streaming_channel_t s_chan_bc = s_chan_alloc();
    streaming_channel_t s_chan_output = s_chan_alloc();

    PAR_JOBS (
        PJOB(mic_dual_pdm_rx_decimate, (p_pdm_mic, pdm_decimation_factor, mic_array_third_stage_coefs(pdm_decimation_factor), mic_array_fir_compensation(pdm_decimation_factor), s_chan_input.end_a, NULL)),
        PJOB(ap_stage_a, (s_chan_input.end_b, s_chan_ab.end_a)),
        PJOB(ap_stage_b, (s_chan_ab.end_b, s_chan_bc.end_a)),
        PJOB(ap_stage_c, (s_chan_bc.end_b, s_chan_output.end_a)),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ())
    );
}
