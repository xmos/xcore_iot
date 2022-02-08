// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xcore/triggerable.h>
#include <xcore/assert.h>
#include <xcore/interrupt.h>

#include "rtos_interrupt.h"
#include "rtos_intertile.h"

DEFINE_RTOS_INTERRUPT_CALLBACK(rtos_intertile_isr, arg)
{
    rtos_intertile_t *ctx = arg;
    uint8_t port;

    triggerable_disable_trigger(ctx->c);

    port = s_chan_in_byte(ctx->c);

    /* wake up the task waiting to receive on this port */
    if (rtos_osal_event_group_set_bits(&ctx->event_group, (1 << port)) !=
        RTOS_OSAL_SUCCESS) {
        /* This shouldn't fail */
        xassert(0);
    }
}

void rtos_intertile_tx_len(rtos_intertile_t *ctx, uint8_t port, size_t len)
{
    rtos_osal_mutex_get(&ctx->lock, RTOS_OSAL_PORT_WAIT_FOREVER);

    xassert(ctx->tx_len == 0);

    ctx->tx_len = len;
    s_chan_out_byte(ctx->c, port); //to the ISR
    s_chan_out_word(ctx->c, len);
}

size_t rtos_intertile_tx_data(rtos_intertile_t *ctx, void *data, size_t len)
{
    size_t tx_len = len <= ctx->tx_len ? len : ctx->tx_len;

    s_chan_out_buf_byte(ctx->c, data, tx_len);

    ctx->tx_len -= tx_len;

    if (ctx->tx_len == 0) {
        rtos_osal_mutex_put(&ctx->lock);
    }

    return tx_len;
}

void rtos_intertile_tx(rtos_intertile_t *ctx, uint8_t port, void *msg,
                       size_t len)
{
    rtos_osal_mutex_get(&ctx->lock, RTOS_OSAL_PORT_WAIT_FOREVER);

    s_chan_out_byte(ctx->c, port); //to the ISR
    s_chan_out_word(ctx->c, len);
    s_chan_out_buf_byte(ctx->c, msg, len);

    rtos_osal_mutex_put(&ctx->lock);
}

size_t rtos_intertile_rx_len(rtos_intertile_t *ctx, uint8_t port,
                             unsigned timeout)
{
    uint32_t flags;
    rtos_osal_status_t status;

    status =
            rtos_osal_event_group_get_bits(&ctx->event_group, (1 << port),
                                           RTOS_OSAL_OR_CLEAR, &flags, timeout);

    if (status == RTOS_OSAL_SUCCESS) {
        xassert(ctx->rx_len == 0);
        ctx->rx_len = s_chan_in_word(ctx->c);
    }

    return ctx->rx_len;
}

size_t rtos_intertile_rx_data(rtos_intertile_t *ctx, void *data, size_t len)
{
    size_t rx_len = len <= ctx->rx_len ? len : ctx->rx_len;

    s_chan_in_buf_byte(ctx->c, data, rx_len);

    ctx->rx_len -= rx_len;

    if (ctx->rx_len == 0) {
        triggerable_enable_trigger(ctx->c);
    }

    return rx_len;
}

size_t rtos_intertile_rx(rtos_intertile_t *ctx, uint8_t port, void **msg,
                         unsigned timeout)
{
    uint32_t len = 0;
    uint32_t flags;
    rtos_osal_status_t status;

    *msg = NULL;

    status =
            rtos_osal_event_group_get_bits(&ctx->event_group, (1 << port),
                                           RTOS_OSAL_OR_CLEAR, &flags, timeout);

    if (status == RTOS_OSAL_SUCCESS) {
        len = s_chan_in_word(ctx->c);

        *msg = rtos_osal_malloc(len);
        xassert(*msg != NULL);

        s_chan_in_buf_byte(ctx->c, *msg, len);

        triggerable_enable_trigger(ctx->c);
    }

    return len;
}

void rtos_intertile_start(rtos_intertile_t *intertile_ctx)
{
    triggerable_setup_interrupt_callback(
            intertile_ctx->c, intertile_ctx,
            RTOS_INTERRUPT_CALLBACK(rtos_intertile_isr));
    triggerable_enable_trigger(intertile_ctx->c);
}

static chanend_t channel_establish(chanend_t remote_tile_chanend)
{
    chanend_t remote_c;
    chanend_t local_c = chanend_alloc();
    xassert(local_c != 0);

    chanend_out_word(remote_tile_chanend, local_c);
    chanend_out_end_token(remote_tile_chanend);

    remote_c = (chanend_t)chanend_in_word(remote_tile_chanend);
    chanend_check_end_token(remote_tile_chanend);
    chanend_set_dest(local_c, remote_c);

    /* Open the link */
    chanend_out_control_token(local_c, XS1_CT_START_TRANSACTION);
    chanend_check_control_token(local_c, XS1_CT_START_TRANSACTION);

    return local_c;
}

void rtos_intertile_init(rtos_intertile_t *intertile_ctx, chanend_t c)
{
    intertile_ctx->c = channel_establish(c);
    intertile_ctx->tx_len = 0;
    intertile_ctx->rx_len = 0;
    rtos_osal_mutex_create(&intertile_ctx->lock, "intertile_mutex",
                           RTOS_OSAL_NOT_RECURSIVE);
    rtos_osal_event_group_create(&intertile_ctx->event_group,
                                 "intertile_group");
}
