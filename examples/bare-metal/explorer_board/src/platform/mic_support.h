// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef MIC_SUPPORT_H_
#define MIC_SUPPORT_H_

#include <xcore/clock.h>
#include <xcore/port.h>

#include "app_conf.h"

#define MAX(X, Y) ((X) >= (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) <= (Y) ? (X) : (Y))

#define MIC_DUAL_NUM_CHANNELS 2

void mic_array_setup_ddr(
        xclock_t pdmclk,
        xclock_t pdmclk6,
        port_t p_mclk,
        port_t p_pdm_clk,
        port_t p_pdm_mics,
        int divide);

void frame_power(int32_t (*audio_frame)[MIC_DUAL_NUM_CHANNELS]);

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
