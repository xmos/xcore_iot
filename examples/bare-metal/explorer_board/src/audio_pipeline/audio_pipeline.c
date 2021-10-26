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
#include "audio_pipeline.h"
#include "mic_support.h"

void ap_stage_a(chanend_t c_input, chanend_t c_output) {
    int32_t *mic_sample_block = NULL;
    int32_t output[MIC_DUAL_FRAME_SIZE][MIC_DUAL_NUM_CHANNELS];
    while(1)
    {
        mic_sample_block = (int32_t *) s_chan_in_word(c_input);
        memcpy(output, mic_sample_block, MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS * sizeof(int32_t));
        // debug_printf("a sends\n");
        frame_power(output);
        s_chan_out_buf_word(c_output, (uint32_t*)output, MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS);
    }
}

void ap_stage_b(chanend_t c_input, chanend_t c_output) {
    int32_t output[MIC_DUAL_FRAME_SIZE][MIC_DUAL_NUM_CHANNELS];
    while(1)
    {
        s_chan_in_buf_word(c_input, (uint32_t*)output, MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS);
        // for(int i=0; i<MIC_DUAL_FRAME_SIZE; i++)
        // {
        //     output[i][0] *= 2;
        // }
        debug_printf("b sends\n");
        s_chan_out_buf_word(c_output, (uint32_t*)output, MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS);
    }
}

void ap_stage_c(chanend_t c_input, chanend_t c_output) {
    int32_t output[MIC_DUAL_FRAME_SIZE][MIC_DUAL_NUM_CHANNELS];
    while(1)
    {
        s_chan_in_buf_word(c_input, (uint32_t*)output, MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS);

        // for(int i=0; i<MIC_DUAL_FRAME_SIZE; i++)
        // {
        //     output[i][0] *= 2;
        // }
        frame_power(output);
    }
}
