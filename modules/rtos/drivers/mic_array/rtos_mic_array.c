// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#include <string.h>

#include <xcore/assert.h>
#include <xcore/triggerable.h>

#include "rtos_interrupt.h"

#include "rtos/drivers/mic_array/api/rtos_mic_array.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

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

    /* Ensure the mic array thread is never preempted */
    rtos_osal_thread_preemption_disable(NULL);
    /* And exclude it from core 0 where the system tick interrupt runs */
    rtos_osal_thread_core_exclusion_set(NULL, (1 << 0));

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
    size_t words_remaining = MIC_DUAL_FRAME_SIZE * (MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS);
    size_t words_available = ctx->recv_buffer.total_written - ctx->recv_buffer.total_read;
    size_t words_free = ctx->recv_buffer.buf_size - words_available;
    int32_t *mic_sample_block = (int32_t *) s_chan_in_word(ctx->c_2x_pdm_mic.end_b);

    if (words_remaining <= words_free) {
        while (words_remaining) {
            size_t words_to_copy = MIN(words_remaining, ctx->recv_buffer.buf_size - ctx->recv_buffer.write_index);
            memcpy(&ctx->recv_buffer.buf[ctx->recv_buffer.write_index], mic_sample_block, words_to_copy * sizeof(int32_t));
            ctx->recv_buffer.write_index += words_to_copy;

            mic_sample_block += words_to_copy;
            words_remaining -= words_to_copy;

            if (ctx->recv_buffer.write_index >= ctx->recv_buffer.buf_size) {
                ctx->recv_buffer.write_index = 0;
            }
        }

        RTOS_MEMORY_BARRIER();
        ctx->recv_buffer.total_written += MIC_DUAL_FRAME_SIZE * (MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS);
    }

    if (ctx->recv_buffer.required_available_count > 0) {
        words_available = ctx->recv_buffer.total_written - ctx->recv_buffer.total_read;

        if (words_available >= ctx->recv_buffer.required_available_count) {
            ctx->recv_buffer.required_available_count = 0;
            rtos_osal_semaphore_put(&ctx->recv_sem);
        }
    }
}

void rtos_mic_array_interrupt_init(rtos_mic_array_t *mic_array_ctx)
{
    triggerable_setup_interrupt_callback(mic_array_ctx->c_2x_pdm_mic.end_b, mic_array_ctx, RTOS_INTERRUPT_CALLBACK(rtos_mic_array_isr));
    triggerable_enable_trigger(mic_array_ctx->c_2x_pdm_mic.end_b);
}

__attribute__((fptrgroup("rtos_mic_array_rx_fptr_grp")))
static size_t mic_array_local_rx(
        rtos_mic_array_t *ctx,
        int32_t sample_buf[][MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS],
        size_t frame_count,
        unsigned timeout)
{
    size_t frames_recvd = 0;
    size_t words_remaining = frame_count * (MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS);
    int32_t *sample_buf_ptr = (int32_t *) sample_buf;

    xassert(words_remaining <= ctx->recv_buffer.buf_size);
    if (words_remaining > ctx->recv_buffer.buf_size) {
        return frames_recvd;
    }

    if (!ctx->recv_blocked) {
        size_t words_available = ctx->recv_buffer.total_written - ctx->recv_buffer.total_read;
        if (words_remaining > words_available) {
            ctx->recv_buffer.required_available_count = words_remaining;
            ctx->recv_blocked = 1;
        }
    }

    if (ctx->recv_blocked) {
        if (rtos_osal_semaphore_get(&ctx->recv_sem, timeout) == RTOS_OSAL_SUCCESS) {
            ctx->recv_blocked = 0;
        }
    }

    if (!ctx->recv_blocked) {
        while (words_remaining) {
            size_t words_to_copy = MIN(words_remaining, ctx->recv_buffer.buf_size - ctx->recv_buffer.read_index);
            memcpy(sample_buf_ptr, &ctx->recv_buffer.buf[ctx->recv_buffer.read_index], words_to_copy * sizeof(int32_t));
            ctx->recv_buffer.read_index += words_to_copy;

            sample_buf_ptr += words_to_copy;
            words_remaining -= words_to_copy;

            if (ctx->recv_buffer.read_index >= ctx->recv_buffer.buf_size) {
                ctx->recv_buffer.read_index = 0;
            }
        }

        RTOS_MEMORY_BARRIER();
        ctx->recv_buffer.total_read += frame_count * (MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS);

        frames_recvd = frame_count;
    }

    return frames_recvd;
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
    memset(&mic_array_ctx->recv_buffer, 0, sizeof(mic_array_ctx->recv_buffer));
    mic_array_ctx->recv_buffer.buf_size = buffer_size * (MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS);
    mic_array_ctx->recv_buffer.buf = rtos_osal_malloc(mic_array_ctx->recv_buffer.buf_size * sizeof(int32_t));
    rtos_osal_semaphore_create(&mic_array_ctx->recv_sem, "mic_recv_sem", 1, 0);

    mic_array_ctx->decimation_factor = decimation_factor;
    mic_array_ctx->third_stage_coefs = third_stage_coefs;
    mic_array_ctx->fir_gain_compensation = fir_gain_compensation;

    rtos_osal_thread_create(
            NULL,
            "mic_array_thread",
            (rtos_osal_entry_function_t) mic_array_thread,
            mic_array_ctx,
            RTOS_THREAD_STACK_SIZE(mic_array_thread),
            priority);

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
