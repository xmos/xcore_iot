// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <string.h>

#include <xcore/assert.h>
#include <xcore/triggerable.h>

#include "rtos_interrupt.h"

#include "drivers/rtos/i2s/api/rtos_i2s_master.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define ISR_RESUME_SEND 1
#define ISR_RESUME_RECV 2

DEFINE_RTOS_INTERRUPT_CALLBACK(rtos_i2s_master_isr, arg)
{
    rtos_i2s_master_t *ctx = arg;
    uint32_t isr_action;

    isr_action = s_chan_in_word(ctx->c_i2s_isr.end_b);

    if (isr_action == ISR_RESUME_SEND) {
        rtos_osal_semaphore_put(&ctx->send_sem);
    }
}

I2S_CALLBACK_ATTR
static void i2s_init(rtos_i2s_master_t *ctx, i2s_config_t *i2s_config)
{
    i2s_config->mode = ctx->mode;
    i2s_config->mclk_bclk_ratio = ctx->mclk_bclk_ratio;
}

I2S_CALLBACK_ATTR
static i2s_restart_t i2s_restart_check(rtos_i2s_master_t *ctx)
{
    return I2S_NO_RESTART;
}

I2S_CALLBACK_ATTR
static void i2s_receive(rtos_i2s_master_t *ctx, size_t num_in, const int32_t *samples)
{
    (void) num_in;
    (void) samples;
}

I2S_CALLBACK_ATTR
static void i2s_send(rtos_i2s_master_t *ctx, size_t num_out, int32_t *i2s_sample_buf)
{
    size_t words_available = ctx->send_buffer.total_written - ctx->send_buffer.total_read;

    if (words_available >= num_out) {
        memcpy(i2s_sample_buf, &ctx->send_buffer.buf[ctx->send_buffer.read_index], num_out * sizeof(int32_t));
        ctx->send_buffer.read_index += num_out;
        if (ctx->send_buffer.read_index >= ctx->send_buffer.buf_size) {
            ctx->send_buffer.read_index = 0;
        }
        RTOS_MEMORY_BARRIER();
        ctx->send_buffer.total_read += num_out;
    }

    if (ctx->send_buffer.required_free_count > 0) {
        size_t words_free = ctx->send_buffer.buf_size - (words_available - num_out);

        if (words_free >= ctx->send_buffer.required_free_count) {
            ctx->send_buffer.required_free_count = 0;
            s_chan_out_word(ctx->c_i2s_isr.end_a, ISR_RESUME_SEND);
        }
    }
}

static void i2s_master_thread(rtos_i2s_master_t *ctx)
{
    i2s_callback_group_t i2s_cbg = {
            (i2s_init_t) i2s_init,
            (i2s_restart_check_t) i2s_restart_check,
            (i2s_receive_t) i2s_receive,
            (i2s_send_t) i2s_send,
            ctx
    };

    /* Exclude from core 0 */
    rtos_osal_thread_preemption_disable(NULL);
    rtos_osal_thread_core_exclusion_set(NULL, (1 << 0));

    rtos_printf("I2S on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());
    i2s_master(
               &i2s_cbg,
               ctx->p_dout,
               ctx->num_out,
               ctx->p_din,
               ctx->num_in,
               ctx->p_bclk,
               ctx->p_lrclk,
               ctx->p_mclk,
               ctx->bclk);
}

__attribute__((fptrgroup("rtos_i2s_master_tx_fptr_grp")))
static size_t i2s_master_local_tx(rtos_i2s_master_t *ctx, int32_t *i2s_sample_buf, size_t frame_count, unsigned timeout)
{
    size_t frames_sent = 0;
    size_t words_remaining = frame_count * (2 * ctx->num_out);

    xassert(words_remaining <= ctx->send_buffer.buf_size);
    if (words_remaining > ctx->send_buffer.buf_size) {
        return frames_sent;
    }

    if (!ctx->send_blocked) {
        size_t words_free = ctx->send_buffer.buf_size - (ctx->send_buffer.total_written - ctx->send_buffer.total_read);
        if (words_remaining > words_free) {
            ctx->send_buffer.required_free_count = words_remaining;
            ctx->send_blocked = 1;
        }
    }

    if (ctx->send_blocked) {
        if (rtos_osal_semaphore_get(&ctx->send_sem, timeout) == RTOS_OSAL_SUCCESS) {
            ctx->send_blocked = 0;
        }
    }

    if (!ctx->send_blocked) {
        while (words_remaining) {
            size_t words_to_copy = MIN(words_remaining, ctx->send_buffer.buf_size - ctx->send_buffer.write_index);
            memcpy(&ctx->send_buffer.buf[ctx->send_buffer.write_index], i2s_sample_buf, words_to_copy * sizeof(int32_t));
            ctx->send_buffer.write_index += words_to_copy;

            i2s_sample_buf += words_to_copy;
            words_remaining -= words_to_copy;

            if (ctx->send_buffer.write_index >= ctx->send_buffer.buf_size) {
                ctx->send_buffer.write_index = 0;
            }
        }

        RTOS_MEMORY_BARRIER();
        ctx->send_buffer.total_written += frame_count * (2 * ctx->num_out);

        frames_sent = frame_count;
    }

    return frames_sent;
}

void rtos_i2s_master_start(
        rtos_i2s_master_t *i2s_master_ctx,
        unsigned mclk_bclk_ratio,
        i2s_mode_t mode,
        size_t buffer_size,
        unsigned priority)
{
    i2s_master_ctx->mclk_bclk_ratio = mclk_bclk_ratio;
    i2s_master_ctx->mode = mode;

    memset(&i2s_master_ctx->send_buffer, 0, sizeof(i2s_master_ctx->send_buffer));
    i2s_master_ctx->send_buffer.buf_size = buffer_size * (2 * i2s_master_ctx->num_out);
    i2s_master_ctx->send_buffer.buf = rtos_osal_malloc(i2s_master_ctx->send_buffer.buf_size * sizeof(int32_t));

    rtos_osal_semaphore_create(&i2s_master_ctx->send_sem, "i2s_send_sem", 1, 0);
    rtos_osal_thread_create(
            NULL,
            "i2s_master_thread",
            (rtos_osal_entry_function_t) i2s_master_thread,
            i2s_master_ctx,
            RTOS_THREAD_STACK_SIZE(i2s_master_thread),
            priority);

    if (i2s_master_ctx->rpc_config != NULL && i2s_master_ctx->rpc_config->rpc_host_start != NULL) {
        i2s_master_ctx->rpc_config->rpc_host_start(i2s_master_ctx->rpc_config);
    }
}

void rtos_i2s_master_interrupt_init(rtos_i2s_master_t *i2s_master_ctx)
{
    triggerable_setup_interrupt_callback(i2s_master_ctx->c_i2s_isr.end_b, i2s_master_ctx, RTOS_INTERRUPT_CALLBACK(rtos_i2s_master_isr));
    triggerable_enable_trigger(i2s_master_ctx->c_i2s_isr.end_b);
}

void rtos_i2s_master_init(
        rtos_i2s_master_t *i2s_master_ctx,
        port_t p_dout[],
        size_t num_out,
        port_t p_din[],
        size_t num_in,
        port_t p_bclk,
        port_t p_lrclk,
        port_t p_mclk,
        xclock_t bclk)
{
    xassert(num_out <= I2S_MAX_DATALINES);
    xassert(num_in <= I2S_MAX_DATALINES);

    memcpy(i2s_master_ctx->p_dout, p_dout, num_out * sizeof(port_t));
    memcpy(i2s_master_ctx->p_din, p_din, num_in * sizeof(port_t));

    i2s_master_ctx->num_out = num_out;
    i2s_master_ctx->num_in = num_in;

    i2s_master_ctx->p_bclk = p_bclk;
    i2s_master_ctx-> p_lrclk = p_lrclk;
    i2s_master_ctx->p_mclk = p_mclk;
    i2s_master_ctx-> bclk = bclk;

    i2s_master_ctx->c_i2s_isr = s_chan_alloc();

    i2s_master_ctx->rpc_config = NULL;
    i2s_master_ctx->tx = i2s_master_local_tx;
}
