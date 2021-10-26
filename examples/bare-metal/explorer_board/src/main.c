// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>
#include <xcore/assert.h>
#include <xcore/chanend.h>
#include <xcore/channel_streaming.h>
#include <xcore/parallel.h>
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include <xcore/triggerable.h>

/* SDK headers */
#include "soc.h"
#include "mic_array.h"
#include "xcore_utils.h"

/* App headers */
#include "app_conf.h"
#include "burn.h"

#include "audio_pipeline.h"
#include "mic_support.h"

#include "tile_support.h"
#include "platform_init.h"

#include "i2s.h"


void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void)c0;
    (void)c2;
    (void)c3;

    platform_init_tile_0(c1);

    // uint32_t test = 0xDEADBEEF;
    // chanend_t c_msg = soc_channel_establish(c1, soc_channel_inout);
    // chanend_out_word(c_msg, test);
    // chanend_out_control_token(c_msg, XS1_CT_PAUSE);

    PAR_JOBS (
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ())
    );
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
    return;
}

const port_t p_i2s_dout[1] = {
        PORT_I2S_DAC_DATA
};
const port_t p_bclk = PORT_I2S_BCLK;
const port_t p_lrclk = PORT_I2S_LRCLK;
const port_t p_mclk = PORT_MCLK_IN;
const xclock_t bclk = XS1_CLKBLK_3;


void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void)c1;
    (void)c2;
    (void)c3;

    platform_init_tile_1(c0);

    // chanend_t c_msg = soc_channel_establish(c0, soc_channel_inout);
    // uint32_t test = chanend_in_word(c_msg);
    // debug_printf("tile 1 got 0x%x\n", test);

    streaming_channel_t s_chan_input = s_chan_alloc();
    streaming_channel_t s_chan_ab = s_chan_alloc();
    streaming_channel_t s_chan_bc = s_chan_alloc();
    streaming_channel_t s_chan_output = s_chan_alloc();

    const i2s_callback_group_t i2s_cbg = {
            .init = (i2s_init_t) i2s_init,
            .restart_check = (i2s_restart_check_t) i2s_restart_check,
            .receive = (i2s_receive_t) i2s_receive,
            .send = (i2s_send_t) i2s_send,
            .app_data = &s_chan_ab.end_b,
    };

    PAR_JOBS (
        // PJOB(mic_dual_pdm_rx_decimate, (l_tile1_ctx->p_pdm_mic, l_tile1_ctx->pdm_decimation_factor, mic_array_third_stage_coefs(l_tile1_ctx->pdm_decimation_factor), mic_array_fir_compensation(l_tile1_ctx->pdm_decimation_factor), s_chan_input.end_a, NULL)),
        PJOB(ap_stage_a, (s_chan_input.end_b, s_chan_ab.end_a)),
        // PJOB(ap_stage_b, (s_chan_ab.end_b, s_chan_bc.end_a)),
        // PJOB(ap_stage_c, (s_chan_bc.end_b, s_chan_output.end_a)),
        PJOB(i2s_master, (&i2s_cbg, p_i2s_dout, 1, NULL, 0, p_bclk, p_lrclk, p_mclk, bclk)),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ())
    );
}
