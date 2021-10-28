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

#define FRAMES_TO_BUFFER_PER_STAGE  2

void ap_stage_a(chanend_t c_input, chanend_t c_output) {
    int32_t *mic_sample_block = NULL;
    int32_t output[FRAMES_TO_BUFFER_PER_STAGE][MIC_DUAL_FRAME_SIZE][MIC_DUAL_NUM_CHANNELS];
    int buf_ndx = 0;
    while(1)
    {
        mic_sample_block = (int32_t *) s_chan_in_word(c_input);
        memcpy(output[buf_ndx], mic_sample_block, MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS * sizeof(int32_t));
        s_chan_out_buf_word(c_output, (uint32_t*)output[buf_ndx], MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS);
        buf_ndx = (buf_ndx+1 >= FRAMES_TO_BUFFER_PER_STAGE) ? buf_ndx+1 : 0;
    }
}

void ap_stage_b(chanend_t c_input, chanend_t c_output) {
    int32_t output[FRAMES_TO_BUFFER_PER_STAGE][MIC_DUAL_FRAME_SIZE][MIC_DUAL_NUM_CHANNELS];
    int buf_ndx = 0;
    while(1)
    {
        s_chan_in_buf_word(c_input, (uint32_t*)output[buf_ndx], MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS);
        for(int i=0; i<MIC_DUAL_FRAME_SIZE; i++)
        {
            output[buf_ndx][i][0] *= 42;
            output[buf_ndx][i][1] *= 42;
        }
        s_chan_out_buf_word(c_output, (uint32_t*)output[buf_ndx], MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS);
        buf_ndx = (buf_ndx+1 >= FRAMES_TO_BUFFER_PER_STAGE) ? buf_ndx+1 : 0;
    }
}

void ap_stage_c(chanend_t c_input, chanend_t c_output) {
    int32_t output[FRAMES_TO_BUFFER_PER_STAGE][MIC_DUAL_FRAME_SIZE][MIC_DUAL_NUM_CHANNELS];
    int buf_ndx = 0;
    while(1)
    {
        s_chan_in_buf_word(c_input, (uint32_t*)output[buf_ndx], MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS);
        s_chan_out_buf_word(c_output, (uint32_t*)output[buf_ndx], MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS);
        buf_ndx = (buf_ndx+1 >= FRAMES_TO_BUFFER_PER_STAGE) ? buf_ndx+1 : 0;
    }
}
