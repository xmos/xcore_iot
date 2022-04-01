// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <string.h>
#include <xcore/assert.h>
#include <xcore/chanend.h>
#include <xcore/channel.h>
#include <xcore/channel_streaming.h>
#include <xcore/parallel.h>
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include <xcore/triggerable.h>

/* SDK headers */
#include "soc.h"
#include "mic_array.h"
#include "mic_array_vanilla.h"
#include "xcore_utils.h"
#include "i2s.h"

/* App headers */
#include "app_conf.h"
#include "app_demos.h"
#include "burn.h"
#include "audio_pipeline.h"
#include "tile_support.h"
#include "platform_init.h"

void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void)c0;
    (void)c2;
    (void)c3;

    platform_init_tile_0(c1);

    PAR_JOBS (
        PJOB(spi_demo, (&tile0_ctx->spi_device_ctx)),
        PJOB(gpio_server, (tile0_ctx->c_from_gpio, tile0_ctx->c_to_gpio)),
        PJOB(flash_demo, (&tile0_ctx->qspi_flash_ctx)),
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

    platform_init_tile_1(c0);

    streaming_channel_t s_chan_ab = s_chan_alloc();
    streaming_channel_t s_chan_bc = s_chan_alloc();
    streaming_channel_t s_chan_output = s_chan_alloc();
    channel_t chan_decoupler = chan_alloc();

    tile1_ctx->c_i2s_to_dac = s_chan_output.end_b;

    PAR_JOBS (
        PJOB(ma_vanilla_task, (chan_decoupler.end_a)),
        PJOB(ap_stage_a, (chan_decoupler.end_b, s_chan_ab.end_a)),
        PJOB(ap_stage_b, (s_chan_ab.end_b, s_chan_bc.end_a, tile1_ctx->c_from_gpio)),
        PJOB(ap_stage_c, (s_chan_bc.end_b, s_chan_output.end_a, tile1_ctx->c_to_gpio)),
        PJOB(i2s_master, (&tile1_ctx->i2s_cb_group, tile1_ctx->p_i2s_dout, 1, NULL, 0, tile1_ctx->p_bclk, tile1_ctx->p_lrclk, tile1_ctx->p_mclk, tile1_ctx->bclk)),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ())
    );
}
