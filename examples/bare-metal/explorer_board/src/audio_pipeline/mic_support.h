// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef MIC_SUPPORT_H_
#define MIC_SUPPORT_H_

#include <xcore/clock.h>
#include <xcore/port.h>

#include "app_conf.h"
#include "dsp.h"

#define MIN(X, Y) ((X) <= (Y) ? (X) : (Y))

#define MIC_DUAL_NUM_CHANNELS 2

void mic_array_setup_ddr(
        xclock_t pdmclk,
        xclock_t pdmclk6,
        port_t p_mclk,
        port_t p_pdm_clk,
        port_t p_pdm_mics,
        int divide);

inline void frame_power(int32_t (*audio_frame)[MIC_DUAL_NUM_CHANNELS])
{
    uint64_t frame_power0 = 0;
    uint64_t frame_power1 = 0;

    for (int i = 0; i < MIC_DUAL_FRAME_SIZE; ++i) {
        int64_t smp = audio_frame[i][0];
        frame_power0 += (smp * smp) >> 31;
        smp = audio_frame[i][1];
        frame_power1 += (smp * smp) >> 31;
    }

    /* divide by appconfMIC_FRAME_LENGTH (2^8) */
    frame_power0 >>= 8;
    frame_power1 >>= 8;

    debug_printf("Frame power:\nch0: %d\nch1: %d\n",
                 MIN(frame_power0, (uint64_t) Q31(F31(INT32_MAX))),
                 MIN(frame_power0, (uint64_t) Q31(F31(INT32_MAX))));
}

inline int mic_array_decimation_factor(
        const unsigned pdm_clock_frequency,
        const unsigned sample_rate)
{
    return pdm_clock_frequency / (8 * sizeof(int32_t)) / sample_rate;
}

inline mic_dual_third_stage_coef_t *mic_array_third_stage_coefs(
        const unsigned decimation_factor)
{
    mic_dual_third_stage_coef_t * const fir_coefs[7] = {
            NULL,
            g_third_stage_div_2_fir_dual,
            g_third_stage_div_4_fir_dual,
            g_third_stage_div_6_fir_dual,
            g_third_stage_div_8_fir_dual,
            NULL,
            g_third_stage_div_12_fir_dual
    };

    return fir_coefs[decimation_factor/2];
}

inline int mic_array_fir_compensation(
        const unsigned decimation_factor)
{
    const int fir_gain_compen[7] = {
            0,
            FIR_COMPENSATOR_DIV_2,
            FIR_COMPENSATOR_DIV_4,
            FIR_COMPENSATOR_DIV_6,
            FIR_COMPENSATOR_DIV_8,
            0,
            FIR_COMPENSATOR_DIV_12
    };

    return fir_gain_compen[decimation_factor/2];
}

#endif /* MIC_SUPPORT_H_ */
