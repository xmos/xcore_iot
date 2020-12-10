// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <xcore/triggerable.h>
#include <xcore/assert.h>
#include <xcore/interrupt.h>

#include "rtos_interrupt.h"

#include "drivers/rtos/intertile/FreeRTOS/rtos_intertile.h"

DEFINE_RTOS_INTERRUPT_CALLBACK(rtos_intertile_isr, arg)
{
    rtos_intertile_t *ctx = arg;
    BaseType_t yield_required = pdFALSE;
    uint8_t port;

    triggerable_disable_trigger(ctx->c);

    port = s_chan_in_byte(ctx->c);

    /* wake up the task waiting to receive on this port */
    if (xEventGroupSetBitsFromISR(ctx->event_group, (1 << port), &yield_required) == pdFAIL) {
        /* This shouldn't fail */
        xassert(0);
    }

    portEND_SWITCHING_ISR(yield_required);
}

void rtos_intertile_tx(
        rtos_intertile_t *ctx,
        uint8_t port,
        void *msg,
        uint32_t len)
{
    transacting_chanend_t tc;

    xSemaphoreTake(ctx->lock, portMAX_DELAY);

    s_chan_out_byte(ctx->c, port); //to the ISR
    s_chan_out_word(ctx->c, len);
    s_chan_out_buf_byte(ctx->c, msg, len);

    xSemaphoreGive(ctx->lock);
}

uint32_t rtos_intertile_rx(
        rtos_intertile_t *ctx,
        uint8_t port,
        void **msg,
        unsigned timeout)
{
    uint32_t len;

    xEventGroupWaitBits(ctx->event_group,
                        (1 << port),
                        pdTRUE,
                        pdTRUE,
                        timeout);

    len = s_chan_in_word(ctx->c);

    *msg = pvPortMalloc(len);
    xassert(*msg != NULL);

    s_chan_in_buf_byte(ctx->c, *msg, len);

    triggerable_enable_trigger(ctx->c);

    return len;
}

void rtos_intertile_start(
        rtos_intertile_t *intertile_ctx)
{
    intertile_ctx->lock = xSemaphoreCreateMutex();
    intertile_ctx->event_group = xEventGroupCreate();

    triggerable_setup_interrupt_callback(intertile_ctx->c, intertile_ctx, RTOS_INTERRUPT_CALLBACK(rtos_intertile_isr));
    triggerable_enable_trigger(intertile_ctx->c);
}

static chanend_t channel_establish(
        chanend_t remote_tile_chanend)
{
    chanend_t remote_c;
    chanend_t local_c = chanend_alloc();
    xassert(local_c != 0);

    chanend_out_word(remote_tile_chanend, local_c);
    chanend_out_end_token(remote_tile_chanend);

    remote_c = (chanend_t) chanend_in_word(remote_tile_chanend);
    chanend_check_end_token(remote_tile_chanend);
    chanend_set_dest(local_c, remote_c);

    /* Open the link */
    chanend_out_control_token(local_c, XS1_CT_START_TRANSACTION);
    chanend_check_control_token(local_c, XS1_CT_START_TRANSACTION);

    return local_c;
}

void rtos_intertile_init(
        rtos_intertile_t *intertile_ctx,
        chanend_t c)
{
    intertile_ctx->c = channel_establish(c);
}
