// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <string.h>

#include <xcore/assert.h>

#include "drivers/rtos/i2s/FreeRTOS/rtos_i2s_master.h"

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
    /* The timeout here really need to be zero, or else there are timing problems
     * This also calls vTaskSuspendAll() with the default sbRECEIVE_COMPLETED() macro,
     * which causes timing problems when there is too much activity on the other cores.
     * This probably should use a custom non-RTOS FIFO. */

    if (xStreamBufferBytesAvailable(ctx->audio_stream_buffer) >= num_out * sizeof(int32_t)) {
        (void) xStreamBufferReceive(ctx->audio_stream_buffer, i2s_sample_buf, num_out * sizeof(int32_t), 0);
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
static size_t i2s_master_local_tx(rtos_i2s_master_t *ctx, int32_t *i2s_sample_buf, size_t frame_count)
{
    size_t bytes_sent;

    while (xStreamBufferSpacesAvailable(ctx->audio_stream_buffer) < frame_count * (2 * ctx->num_out) * sizeof(int32_t));

    bytes_sent = xStreamBufferSend(ctx->audio_stream_buffer, i2s_sample_buf, frame_count * (2 * ctx->num_out) * sizeof(int32_t), 0);

    return bytes_sent / (2 * ctx->num_out * sizeof(int32_t));
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
    i2s_master_ctx->audio_stream_buffer = xStreamBufferCreate(buffer_size * (2 * i2s_master_ctx->num_out) * sizeof(int32_t), 1);

    xTaskCreate((TaskFunction_t) i2s_master_thread,
                "i2s_master_thread",
                RTOS_THREAD_STACK_SIZE(i2s_master_thread),
                i2s_master_ctx,
                priority,
                NULL);

    if (i2s_master_ctx->rpc_config != NULL && i2s_master_ctx->rpc_config->rpc_host_start != NULL) {
        i2s_master_ctx->rpc_config->rpc_host_start(i2s_master_ctx->rpc_config);
    }
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

    i2s_master_ctx->rpc_config = NULL;
    i2s_master_ctx->tx = i2s_master_local_tx;
}
