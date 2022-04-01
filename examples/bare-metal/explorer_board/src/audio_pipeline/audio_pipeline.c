// Copyright 2021-2022 XMOS LIMITED.
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
#include "xcore_utils.h"
#include "mic_array.h"

/* App headers */
#include "app_conf.h"
#include "audio_pipeline.h"

#define FRAMES_TO_BUFFER_PER_STAGE  2
#define STAGE_B_INITIAL_GAIN        256
#define LED_FRAME_POWER_THRESHOLD   10000

#include <hwtimer.h>
void ap_stage_a(chanend_t c_input, chanend_t c_output) {
    int32_t output[FRAMES_TO_BUFFER_PER_STAGE][MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME][MIC_ARRAY_CONFIG_MIC_COUNT];
    int buf_ndx = 0;

    while(1)
    {
        ma_frame_rx_transpose((int32_t *)output[buf_ndx], c_input, MIC_ARRAY_CONFIG_MIC_COUNT, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME);

        s_chan_out_buf_word(c_output, (uint32_t*)output[buf_ndx], MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * MIC_ARRAY_CONFIG_MIC_COUNT);
        buf_ndx = (buf_ndx+1 >= FRAMES_TO_BUFFER_PER_STAGE) ? buf_ndx+1 : 0;
    }
}

void ap_stage_b(chanend_t c_input, chanend_t c_output, chanend_t c_from_gpio) {
    int32_t output[MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME][MIC_ARRAY_CONFIG_MIC_COUNT];
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
                s_chan_in_buf_word(c_input, (uint32_t*)output, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * MIC_ARRAY_CONFIG_MIC_COUNT);
                for(int i=0; i<MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME; i++)
                {
                    output[i][0] *= stage_b_gain;
                    output[i][1] *= stage_b_gain;
                }
                s_chan_out_buf_word(c_output, (uint32_t*)output, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * MIC_ARRAY_CONFIG_MIC_COUNT);
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
    int32_t output[MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME][MIC_ARRAY_CONFIG_MIC_COUNT];

    triggerable_disable_all();

    TRIGGERABLE_SETUP_EVENT_VECTOR(c_input, input_frames);

    triggerable_enable_trigger(c_input);

    while(1)
    {
        TRIGGERABLE_WAIT_EVENT(input_frames);
        {
            input_frames:
            {
                s_chan_in_buf_word(c_input, (uint32_t*)output, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * MIC_ARRAY_CONFIG_MIC_COUNT);
                uint64_t frame_power0 = 0;
                uint64_t frame_power1 = 0;

                for (int i = 0; i < MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME; ++i) {
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

                s_chan_out_buf_word(c_output, (uint32_t*)output, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * MIC_ARRAY_CONFIG_MIC_COUNT);
            }
            continue;
        }
    }
}
