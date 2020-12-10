// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef RTOS_MIC_ARRAY_H_
#define RTOS_MIC_ARRAY_H_

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

#include <xcore/channel.h>
#include <xcore/clock.h>
#include <xcore/port.h>
#include "mic_array.h"

#include "drivers/rtos/rpc/api/rtos_driver_rpc.h"

#define MIC_DUAL_NUM_CHANNELS 2

typedef struct rtos_mic_array_struct rtos_mic_array_t;
struct rtos_mic_array_struct {
    rtos_driver_rpc_t *rpc_config;

    __attribute__((fptrgroup("rtos_mic_array_rx_fptr_grp")))
    size_t (*rx)(rtos_mic_array_t *, int32_t sample_buf[][MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS], size_t, unsigned);

    port_t p_pdm_mics;
    streaming_channel_t c_2x_pdm_mic;
#if MIC_DUAL_NUM_REF_CHANNELS > 0
    streaming_channel_t c_ref_audio[MIC_DUAL_NUM_REF_CHANNELS];
#endif

    int decimation_factor;
    mic_dual_third_stage_coef_t *third_stage_coefs;
    int fir_gain_compensation;

    /* BEGIN RTOS SPECIFIC MEMBERS. */
    StreamBufferHandle_t audio_stream_buffer;
};

#include "drivers/rtos/mic_array/api/rtos_mic_array_rpc.h"

/**
 * Initializes the mic array's interrupt and ISR. Must be called on the core
 * that will process the interrupts, prior to calling rtos_mic_array_start().
 * It is recommended that the core the ISR runs on is not the same as the core that
 * runs the mic array task started by rtos_mic_array_start().
 *
 * \param mic_array_ctx The mic array context for which to initialize interrupts.
 */
void rtos_mic_array_interrupt_init(rtos_mic_array_t *mic_array_ctx);

inline size_t rtos_mic_array_rx(
        rtos_mic_array_t *mic_array_ctx,
        int32_t sample_buf[][MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS],
        size_t sample_count,
        unsigned timeout)
{
    return mic_array_ctx->rx(mic_array_ctx, sample_buf, sample_count, timeout);
}

inline mic_dual_third_stage_coef_t *rtos_mic_array_third_stage_coefs(
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

inline int rtos_mic_array_fir_compensation(
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

inline int rtos_mic_array_decimation_factor(
        const unsigned pdm_clock_frequency,
        const unsigned sample_rate)
{
    return pdm_clock_frequency / (8 * sizeof(int32_t)) / sample_rate;
}

void rtos_mic_array_start(
        rtos_mic_array_t *mic_array_ctx,
        int decimation_factor,
        mic_dual_third_stage_coef_t *third_stage_coefs,
        int fir_gain_compensation,
        size_t buffer_size,
        unsigned priority);

void rtos_mic_array_init(
        rtos_mic_array_t *mic_array_ctx,
        const xclock_t pdmclk,
        const xclock_t pdmclk2,
        const unsigned pdm_clock_divider,
        const port_t p_mclk,
        const port_t p_pdm_clk,
        const port_t p_pdm_mics);

#endif /* RTOS_MIC_ARRAY_H_ */
