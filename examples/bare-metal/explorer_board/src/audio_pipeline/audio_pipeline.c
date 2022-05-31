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
#include "bfp_math.h"

/* App headers */
#include "app_conf.h"
#include "audio_pipeline.h"

#define NORM_EXP -31
#define FRAMES_TO_BUFFER_PER_STAGE  2
#define INITIAL_GAIN (float)20.0
#define LED_FRAME_POWER_THRESHOLD   (float)0.00001

#include <hwtimer.h>
void ap_stage_a(chanend_t c_input, chanend_t c_output) {
    // initialise the array which will hold the data
    int32_t output[FRAMES_TO_BUFFER_PER_STAGE][MIC_ARRAY_CONFIG_MIC_COUNT][MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME];
    int buf_ndx = 0;

    while(1)
    {
        ma_frame_rx_transpose((int32_t *)output[buf_ndx], c_input, MIC_ARRAY_CONFIG_MIC_COUNT, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME);

        s_chan_out_buf_word(c_output, (uint32_t*)output[buf_ndx], MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * MIC_ARRAY_CONFIG_MIC_COUNT);
        buf_ndx = (buf_ndx + 1 >= FRAMES_TO_BUFFER_PER_STAGE) ? buf_ndx + 1 : 0;
    }
}

void ap_stage_b(chanend_t c_input, chanend_t c_output, chanend_t c_from_gpio) {
    // initialise the array which will hold the data
    int32_t output[MIC_ARRAY_CONFIG_MIC_COUNT][MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME];
    // initialise block floating point structures for both channels
    bfp_s32_t ch1, ch2;
    bfp_s32_init(&ch1, output[0], NORM_EXP, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME, 0);
    bfp_s32_init(&ch2, output[1], NORM_EXP, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME, 0);

    float gain_db = INITIAL_GAIN;

    triggerable_disable_all();
    // initialise events
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
                // recieve frame over the channel
                s_chan_in_buf_word(c_input, (uint32_t*)output, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * MIC_ARRAY_CONFIG_MIC_COUNT);
                // calculate the headroom of the new frames
                bfp_s32_headroom(&ch1);
                bfp_s32_headroom(&ch2);
                // update the gain
                float power = gain_db / 20.0;
                float gain_fl = powf(10.0, power);
                float_s32_t gain = float_to_float_s32(gain_fl);
                // scale both channels 
                bfp_s32_scale(&ch1, &ch1, gain);
                bfp_s32_scale(&ch2, &ch2, gain);
                // normalise exponent
                bfp_s32_use_exponent(&ch1, NORM_EXP);
                bfp_s32_use_exponent(&ch2, NORM_EXP);
                // send frame over the channel
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
                    gain_db = (gain_db == 60.0) ? gain_db : gain_db + 4.0;
                    break;
                case 0x02:  /* Btn B */
                    gain_db = (gain_db == 0.0) ? gain_db : gain_db - 4.0;
                    break;
                }
                debug_printf("Gain set to %f\n", gain_db);
            }
            continue;
        }
    }
}

void ap_stage_c(chanend_t c_input, chanend_t c_output, chanend_t c_to_gpio) {

    int32_t output[MIC_ARRAY_CONFIG_MIC_COUNT][MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME];
    // initialise block floating point structures for both channels
    bfp_s32_t ch1, ch2;
    bfp_s32_init(&ch1, output[0], NORM_EXP, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME, 0);
    bfp_s32_init(&ch2, output[1], NORM_EXP, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME, 0);

    float th = LED_FRAME_POWER_THRESHOLD;

    triggerable_disable_all();
    // initialise event
    TRIGGERABLE_SETUP_EVENT_VECTOR(c_input, input_frames);

    triggerable_enable_trigger(c_input);

    while(1)
    {
        TRIGGERABLE_WAIT_EVENT(input_frames);
        {
            input_frames:
            {
                uint8_t led_byte = 0;
                // recieve frame over the channel
                s_chan_in_buf_word(c_input, (uint32_t*)output, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * MIC_ARRAY_CONFIG_MIC_COUNT);
                // calculate the headroom of the new frames
                bfp_s32_headroom(&ch1);
                bfp_s32_headroom(&ch2);
                // calculate the frame energy
                float_s32_t frame_energy_ch1 = float_s64_to_float_s32(bfp_s32_energy(&ch1));
                float_s32_t frame_energy_ch2 = float_s64_to_float_s32(bfp_s32_energy(&ch2));
                // calculate the frame power
                float frame_pow1 = float_s32_to_float(frame_energy_ch1) / (float)MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME;
                float frame_pow2 = float_s32_to_float(frame_energy_ch2) / (float)MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME;
                if((frame_pow1 > th) || (frame_pow2 > th)){
                    led_byte = 1;
                }
                // send led value to gpio
                chanend_out_byte(c_to_gpio, led_byte);
                // send frame over the channel
                s_chan_out_buf_word(c_output, (uint32_t*)output, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * MIC_ARRAY_CONFIG_MIC_COUNT);
            }
            continue;
        }
    }
}
