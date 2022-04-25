// Copyright 2020-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT RTOS_MIC_ARRAY

#include <string.h>

#include <xcore/assert.h>
#include <xcore/triggerable.h>

#include "rtos_interrupt.h"
#include "rtos_mic_array.h"

#undef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define CLRSR(c) asm volatile("clrsr %0" : : "n"(c));

static void mic_array_thread(rtos_mic_array_t *ctx)
{
    (void) s_chan_in_byte(ctx->c_pdm_mic.end_a);

    rtos_printf("PDM mics on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());

    rtos_interrupt_mask_all();

    /*
     * ma_basic_task() itself uses interrupts, and does re-enable them. However,
     * it appears to assume that KEDI is not set, therefore it is cleared here.
     */
    CLRSR(XS1_SR_KEDI_MASK);

    ma_vanilla_task(ctx->c_pdm_mic.end_a);
}

DEFINE_RTOS_INTERRUPT_CALLBACK(rtos_mic_array_isr, arg)
{
    rtos_mic_array_t *ctx = arg;
    size_t words_remaining = MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * MIC_ARRAY_CONFIG_MIC_COUNT;
    size_t words_available = ctx->recv_buffer.total_written - ctx->recv_buffer.total_read;
    size_t words_free = ctx->recv_buffer.buf_size - words_available;

    int32_t mic_samples[MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * MIC_ARRAY_CONFIG_MIC_COUNT];

    if (ctx->format == RTOS_MIC_ARRAY_CHANNEL_SAMPLE) {
        ma_frame_rx(mic_samples, ctx->c_pdm_mic.end_b, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME, MIC_ARRAY_CONFIG_MIC_COUNT);
    } else if (ctx->format == RTOS_MIC_ARRAY_SAMPLE_CHANNEL) {
        ma_frame_rx_transpose(mic_samples, ctx->c_pdm_mic.end_b, MIC_ARRAY_CONFIG_MIC_COUNT, MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME);
    } else {
        xassert(0); /* Invalid format */
    }
    int32_t *mic_sample_block = (int32_t *)&mic_samples;

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
        ctx->recv_buffer.total_written += MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME * MIC_ARRAY_CONFIG_MIC_COUNT;
    } else {
        rtos_printf("mic rx overrun\n");
    }

    if (ctx->recv_buffer.required_available_count > 0) {
        words_available = ctx->recv_buffer.total_written - ctx->recv_buffer.total_read;

        if (words_available >= ctx->recv_buffer.required_available_count) {
            ctx->recv_buffer.required_available_count = 0;
            rtos_osal_semaphore_put(&ctx->recv_sem);
        }
    }
}

__attribute__((fptrgroup("rtos_mic_array_rx_fptr_grp")))
static size_t mic_array_local_rx(
        rtos_mic_array_t *ctx,
        int32_t **sample_buf,
        size_t frame_count,
        unsigned timeout)
{
    size_t frames_recvd = 0;
    size_t words_remaining = frame_count * MIC_ARRAY_CONFIG_MIC_COUNT;
    int32_t *sample_buf_ptr = (int32_t *) sample_buf;

    if(ctx->format == RTOS_MIC_ARRAY_CHANNEL_SAMPLE) {
        xassert(frame_count == MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME);
    }

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
        ctx->recv_buffer.total_read += frame_count * MIC_ARRAY_CONFIG_MIC_COUNT;

        frames_recvd = frame_count;
    }

    return frames_recvd;
}

void rtos_mic_array_start(
        rtos_mic_array_t *mic_array_ctx,
        size_t buffer_size,
        unsigned interrupt_core_id)
{
    uint32_t core_exclude_map;

    xassert(buffer_size >= MIC_ARRAY_CONFIG_SAMPLES_PER_FRAME);
    memset(&mic_array_ctx->recv_buffer, 0, sizeof(mic_array_ctx->recv_buffer));
    mic_array_ctx->recv_buffer.buf_size = buffer_size * MIC_ARRAY_CONFIG_MIC_COUNT;
    mic_array_ctx->recv_buffer.buf = rtos_osal_malloc(mic_array_ctx->recv_buffer.buf_size * sizeof(int32_t));
    rtos_osal_semaphore_create(&mic_array_ctx->recv_sem, "mic_recv_sem", 1, 0);

    /* Ensure that the mic array interrupt is enabled on the requested core */
    rtos_osal_thread_core_exclusion_get(NULL, &core_exclude_map);
    rtos_osal_thread_core_exclusion_set(NULL, ~(1 << interrupt_core_id));

    triggerable_enable_trigger(mic_array_ctx->c_pdm_mic.end_b);

    /* Tells the task running the decimator to start */
    s_chan_out_byte(mic_array_ctx->c_pdm_mic.end_b, 0);

    /* Restore the core exclusion map for the calling thread */
    rtos_osal_thread_core_exclusion_set(NULL, core_exclude_map);

    if (mic_array_ctx->rpc_config != NULL && mic_array_ctx->rpc_config->rpc_host_start != NULL) {
        mic_array_ctx->rpc_config->rpc_host_start(mic_array_ctx->rpc_config);
    }
}

void rtos_mic_array_init(
        rtos_mic_array_t *mic_array_ctx,
        uint32_t io_core_mask,
        rtos_mic_array_format_t format)
{
    mic_array_ctx->c_pdm_mic = s_chan_alloc();
    mic_array_ctx->format = format;

    xassert(format < RTOS_MIC_ARRAY_FORMAT_COUNT);

    ma_vanilla_init();

    mic_array_ctx->rpc_config = NULL;
    mic_array_ctx->rx = mic_array_local_rx;

    triggerable_setup_interrupt_callback(mic_array_ctx->c_pdm_mic.end_b, mic_array_ctx, RTOS_INTERRUPT_CALLBACK(rtos_mic_array_isr));

    rtos_osal_thread_create(
            &mic_array_ctx->hil_thread,
            "mic_array_thread",
            (rtos_osal_entry_function_t) mic_array_thread,
            mic_array_ctx,
            RTOS_THREAD_STACK_SIZE(mic_array_thread),
            RTOS_OSAL_HIGHEST_PRIORITY);

    /* Ensure the mic array thread is never preempted */
    rtos_osal_thread_preemption_disable(&mic_array_ctx->hil_thread);
    /* And ensure it only runs on one of the specified cores */
    rtos_osal_thread_core_exclusion_set(&mic_array_ctx->hil_thread, ~io_core_mask);
}

#undef MIN
