// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <xs1.h>
#include <xcore/triggerable.h>

#include "rtos_interrupt.h"

#include "drivers/rtos/mic_array/FreeRTOS/rtos_mic_array.h"

static void mic_array_thread(rtos_mic_array_t *ctx)
{
#if MIC_DUAL_NUM_REF_CHANNELS > 0
    chanend_t ref_audio_ends[MIC_DUAL_NUM_REF_CHANNELS];
    for (int i = 0; i < MIC_DUAL_NUM_REF_CHANNELS; i++) {
        ref_audio_ends[i] = ctx->c_ref_audio[i].end_a;
    };
#else
    chanend_t *ref_audio_ends = NULL;
#endif

    rtos_printf("PDM mics on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());
    mic_dual_pdm_rx_decimate(
            ctx->p_pdm_mics,
            ctx->decimation_factor,
            ctx->third_stage_coefs,
            ctx->fir_gain_compensation,
            ctx->c_2x_pdm_mic.end_a,
            ref_audio_ends);
}

DEFINE_RTOS_INTERRUPT_CALLBACK(rtos_mic_array_isr, arg)
{
    rtos_mic_array_t *ctx = arg;
    int32_t *mic_sample_block;
    BaseType_t yield_required = pdFALSE;

    mic_sample_block = (int32_t *) s_chan_in_word(ctx->c_2x_pdm_mic.end_b);

    if (xStreamBufferSpacesAvailable(ctx->audio_stream_buffer) >= MIC_DUAL_FRAME_SIZE * (MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS) * sizeof(int32_t)) {
        size_t len;

        len = xStreamBufferSendFromISR(ctx->audio_stream_buffer,
                                       mic_sample_block,
                                       MIC_DUAL_FRAME_SIZE * (MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS) * sizeof(int32_t),
                                       &yield_required);
    } else {
        rtos_printf("mic samples lost\n");
    }

    portEND_SWITCHING_ISR(yield_required);
}

void rtos_mic_array_interrupt_init(rtos_mic_array_t *mic_array_ctx)
{
    triggerable_setup_interrupt_callback(mic_array_ctx->c_2x_pdm_mic.end_b, mic_array_ctx, RTOS_INTERRUPT_CALLBACK(rtos_mic_array_isr));
    triggerable_enable_trigger(mic_array_ctx->c_2x_pdm_mic.end_b);
}

__attribute__((fptrgroup("rtos_mic_array_rx_fptr_grp")))
static size_t mic_array_local_rx(
        rtos_mic_array_t *mic_array_ctx,
        int32_t sample_buf[][MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS],
        size_t sample_count,
        unsigned timeout)
{
    size_t bytes_recvd;

    bytes_recvd = xStreamBufferReceive(mic_array_ctx->audio_stream_buffer, sample_buf, sample_count * sizeof(sample_buf[0]), timeout);

    return bytes_recvd / sizeof(sample_buf[0]);
}

void rtos_mic_array_start(
        rtos_mic_array_t *mic_array_ctx,
        int decimation_factor,
        mic_dual_third_stage_coef_t *third_stage_coefs,
        int fir_gain_compensation,
        size_t buffer_size,
        unsigned priority)
{
    xassert(buffer_size >= MIC_DUAL_FRAME_SIZE);
    mic_array_ctx->audio_stream_buffer = xStreamBufferCreate(buffer_size * (MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS) * sizeof(int32_t), sizeof(int32_t));

    mic_array_ctx->decimation_factor = decimation_factor;
    mic_array_ctx->third_stage_coefs = third_stage_coefs;
    mic_array_ctx->fir_gain_compensation = fir_gain_compensation;

    xTaskCreate(
                (TaskFunction_t) mic_array_thread,
                "mic_array_thread",
                RTOS_THREAD_STACK_SIZE(mic_array_thread),
                mic_array_ctx,
                priority,
                NULL);

    if (mic_array_ctx->rpc_config != NULL && mic_array_ctx->rpc_config->rpc_host_start != NULL) {
        mic_array_ctx->rpc_config->rpc_host_start(mic_array_ctx->rpc_config);
    }
}

static void mic_array_setup_sdr(
        xclock_t pdmclk,
        port_t p_mclk,
        port_t p_pdm_clk,
        port_t p_pdm_mics,
        int divide)
{
    clock_set_source_port(pdmclk, p_mclk);
    clock_set_divide(pdmclk, divide/2);

    port_set_clock(p_pdm_clk, pdmclk);
    port_set_out_clock(p_pdm_clk);

    port_set_clock(p_pdm_mics, pdmclk);
    port_clear_buffer(p_pdm_mics);

    clock_start(pdmclk);
}

static void mic_array_setup_ddr(
        xclock_t pdmclk,
        xclock_t pdmclk6,
        port_t p_mclk,
        port_t p_pdm_clk,
        port_t p_pdm_mics,
        int divide)
{
    uint32_t tmp;

    clock_set_source_port(pdmclk, p_mclk);
    clock_set_divide(pdmclk, divide/2);

    clock_set_source_port(pdmclk6, p_mclk);
    clock_set_divide(pdmclk6, divide/4);

    port_set_clock(p_pdm_clk, pdmclk);
    port_set_out_clock(p_pdm_clk);

    port_set_clock(p_pdm_mics, pdmclk6);
    port_clear_buffer(p_pdm_mics);

    /* start the faster capture clock */
    clock_start(pdmclk6);

    /* wait for a rising edge on the capture clock */
    //port_clear_trigger_in(p_pdm_mics);
    asm volatile("inpw %0, res[%1], 4" : "=r"(tmp) : "r" (p_pdm_mics));

    /* start the slower output clock */
    clock_start(pdmclk);
}

void rtos_mic_array_init(
        rtos_mic_array_t *mic_array_ctx,
        const xclock_t pdmclk,
        const xclock_t pdmclk2,
        const unsigned pdm_clock_divider,
        const port_t p_mclk,
        const port_t p_pdm_clk,
        const port_t p_pdm_mics)
{
    mic_array_ctx->p_pdm_mics = p_pdm_mics;
    mic_array_ctx->c_2x_pdm_mic = s_chan_alloc();
#if MICARRAYCONF_DUAL_NUM_REF_CHANNELS > 0
    for (int i = 0; i < MICARRAYCONF_DUAL_NUM_REF_CHANNELS; i++) {
        mic_array_ctx->c_ref_audio[i] = s_chan_alloc();
    }
#endif

    xassert(pdmclk != 0);

    if (pdmclk2 == 0)
    {
        mic_array_setup_sdr(pdmclk, p_mclk, p_pdm_clk, p_pdm_mics, pdm_clock_divider);
    }
    else
    {
        mic_array_setup_ddr(pdmclk, pdmclk2, p_mclk, p_pdm_clk, p_pdm_mics, pdm_clock_divider);
    }

    mic_array_ctx->rpc_config = NULL;
    mic_array_ctx->rx = mic_array_local_rx;
}
