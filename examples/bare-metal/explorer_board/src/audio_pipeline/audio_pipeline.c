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
#include "xcore_utils.h"
#include "mic_array.h"

/* App headers */
#include "app_conf.h"
#include "audio_pipeline.h"
#include "mic_support.h"

#define FRAMES_TO_BUFFER_PER_STAGE  2
#define STAGE_B_INITIAL_GAIN        10
#define LED_FRAME_POWER_THRESHOLD   10000

void ap_stage_a(chanend_t c_input, chanend_t c_output) {
    int32_t *mic_sample_block = NULL;
    int32_t output[FRAMES_TO_BUFFER_PER_STAGE][MIC_DUAL_FRAME_SIZE][MIC_DUAL_NUM_CHANNELS];
    int buf_ndx = 0;

    triggerable_disable_all();

    TRIGGERABLE_SETUP_EVENT_VECTOR(c_input, input_frames);

    triggerable_enable_trigger(c_input);

    while(1)
    {
        TRIGGERABLE_WAIT_EVENT(input_frames);
        {
            input_frames:
            {
                mic_sample_block = (int32_t *) s_chan_in_word(c_input);
                memcpy(output[buf_ndx], mic_sample_block, MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS * sizeof(int32_t));
                s_chan_out_buf_word(c_output, (uint32_t*)output[buf_ndx], MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS);
                buf_ndx = (buf_ndx+1 >= FRAMES_TO_BUFFER_PER_STAGE) ? buf_ndx+1 : 0;
            }
            continue;
        }
    }
}

void ap_stage_b(chanend_t c_input, chanend_t c_output, chanend_t c_from_gpio) {
    int32_t output[MIC_DUAL_FRAME_SIZE][MIC_DUAL_NUM_CHANNELS];
    uint32_t stage_b_gain = STAGE_B_INITIAL_GAIN;

    triggerable_disable_all();

    TRIGGERABLE_SETUP_EVENT_VECTOR(c_input, input_frames);
    TRIGGERABLE_SETUP_EVENT_VECTOR(c_from_gpio, gpio_request);

    triggerable_enable_trigger(c_input);
    triggerable_enable_trigger(c_from_gpio);

    while(1)
    {
        TRIGGERABLE_WAIT_EVENT(input_frames, gpio_request);
        {
            input_frames:
            {
                s_chan_in_buf_word(c_input, (uint32_t*)output, MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS);
                for(int i=0; i<MIC_DUAL_FRAME_SIZE; i++)
                {
                    output[i][0] *= stage_b_gain;
                    output[i][1] *= stage_b_gain;
                }
                s_chan_out_buf_word(c_output, (uint32_t*)output, MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS);
            }
            continue;
        }
        {
            gpio_request:
            {
                char msg = chanend_in_byte(c_from_gpio);
                switch(msg)
                {
                default:
                    break;
                case 0x01:  /* Btn A */
                    stage_b_gain = (stage_b_gain == 0xFFFFFFFF) ? stage_b_gain : stage_b_gain + 1;
                    break;
                case 0x02:  /* Btn B */
                    stage_b_gain = (stage_b_gain == 0) ? stage_b_gain : stage_b_gain - 1;
                    break;
                }
                debug_printf("Gain set to %u\n", stage_b_gain);
            }
            continue;
        }
    }
}

void ap_stage_c(chanend_t c_input, chanend_t c_output, chanend_t c_to_gpio) {
    int32_t output[MIC_DUAL_FRAME_SIZE][MIC_DUAL_NUM_CHANNELS];

    triggerable_disable_all();

    TRIGGERABLE_SETUP_EVENT_VECTOR(c_input, input_frames);

    triggerable_enable_trigger(c_input);

    while(1)
    {
        TRIGGERABLE_WAIT_EVENT(input_frames);
        {
            input_frames:
            {
                s_chan_in_buf_word(c_input, (uint32_t*)output, MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS);
                uint64_t frame_power0 = 0;
                uint64_t frame_power1 = 0;

                for (int i = 0; i < MIC_DUAL_FRAME_SIZE; ++i) {
                    int64_t smp = output[i][0];
                    frame_power0 += (smp * smp) >> 31;
                    smp = output[i][1];
                    frame_power1 += (smp * smp) >> 31;
                }

                frame_power0 >>= 8;
                frame_power1 >>= 8;

                uint8_t led_byte = 0;
                if((frame_power0 > LED_FRAME_POWER_THRESHOLD) || (frame_power1 > LED_FRAME_POWER_THRESHOLD)) {
                    led_byte = 1;
                }
                chanend_out_byte(c_to_gpio, led_byte);

                s_chan_out_buf_word(c_output, (uint32_t*)output, MIC_DUAL_FRAME_SIZE * MIC_DUAL_NUM_CHANNELS);
            }
            continue;
        }
    }
}
