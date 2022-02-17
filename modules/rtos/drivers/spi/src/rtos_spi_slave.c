// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT RTOS_SPI

#include <string.h>
#include <xcore/triggerable.h>

#include "rtos_interrupt.h"
#include "rtos_spi_slave.h"

#define XFER_DONE_CB_CODE       0

#define XFER_DONE_CB_FLAG  (1 << XFER_DONE_CB_CODE)

#define ALL_FLAGS (XFER_DONE_CB_FLAG)

#define CLRSR(c) asm volatile("clrsr %0" : : "n"(c));

typedef struct {
    uint8_t *out_buf;
    size_t bytes_written;
    uint8_t *in_buf;
    size_t bytes_read;
} xfer_done_queue_item_t;

DEFINE_RTOS_INTERRUPT_CALLBACK(rtos_spi_slave_isr, arg)
{
    rtos_spi_slave_t *ctx = arg;
    int isr_action;
    xfer_done_queue_item_t item;

    item.out_buf = ctx->out_buf;
    item.bytes_written = ctx->bytes_written;
    item.in_buf = ctx->in_buf;
    item.bytes_read = ctx->bytes_read;

    isr_action = s_chan_in_byte(ctx->c.end_b);

    if (rtos_osal_queue_send(&ctx->xfer_done_queue, &item, RTOS_OSAL_NO_WAIT) == RTOS_OSAL_SUCCESS) {
        if (ctx->xfer_done != NULL) {
            /*
             * TODO: FIXME, FreeRTOS specific, not using OSAL here
             */
            xTaskNotifyGive(ctx->app_thread.thread);
        }
    } else {
        rtos_printf("Lost SPI slave transfer\n");
    }

    ctx->waiting_for_isr = 0;
}

void slave_transaction_started(rtos_spi_slave_t *ctx, uint8_t **out_buf, size_t *outbuf_len, uint8_t **in_buf, size_t *inbuf_len)
{
    while (ctx->waiting_for_isr);

    *out_buf = ctx->out_buf;
    *outbuf_len = ctx->outbuf_len;
    *in_buf = ctx->in_buf;
    *inbuf_len = ctx->inbuf_len;
}

void slave_transaction_ended(rtos_spi_slave_t *ctx, uint8_t **out_buf, size_t bytes_written, uint8_t **in_buf, size_t bytes_read, size_t read_bits)
{
    ctx->bytes_written = bytes_written;
    ctx->bytes_read = bytes_read;
    ctx->waiting_for_isr = 1;
    s_chan_out_byte(ctx->c.end_a, XFER_DONE_CB_CODE);
}

static void spi_slave_hil_thread(rtos_spi_slave_t *ctx)
{
    spi_slave_callback_group_t spi_cbg = {
        .slave_transaction_started = (slave_transaction_started_t) slave_transaction_started,
        .slave_transaction_ended = (slave_transaction_ended_t) slave_transaction_ended,
        .app_data = ctx,
    };

    if (ctx->start != NULL) {
        (void) s_chan_in_byte(ctx->c.end_a);
    }

    rtos_printf("SPI slave on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());

    /*
     * spi_slave() will re-enable.
     */
    rtos_interrupt_mask_all();

    /*
     * spi_slave() itself uses interrupts, and does re-enable them. However,
     * it assumes that KEDI is not set, therefore it is cleared here.
     */
    CLRSR(XS1_SR_KEDI_MASK);

    spi_slave(
            &spi_cbg,
            ctx->p_sclk,
            ctx->p_mosi,
            ctx->p_miso,
            ctx->p_cs,
            ctx->clock_block,
            ctx->cpol,
            ctx->cpha);
}

static void spi_slave_app_thread(rtos_spi_slave_t *ctx)
{
    if (ctx->start != NULL) {
        ctx->start(ctx, ctx->app_data);
        s_chan_out_byte(ctx->c.end_b, 0);
    }

    for (;;) {
        /*
         * TODO: FIXME, FreeRTOS specific, not using OSAL here
         */
        ulTaskNotifyTake(pdTRUE, RTOS_OSAL_WAIT_FOREVER);

        if (ctx->xfer_done != NULL) {
            ctx->xfer_done(ctx, ctx->app_data);
        }
    }
}

void spi_slave_xfer_prepare(rtos_spi_slave_t *ctx, void *rx_buf, size_t rx_buf_len, void *tx_buf, size_t tx_buf_len)
{
    ctx->in_buf = rx_buf;
    ctx->inbuf_len = rx_buf_len;
    ctx->out_buf = tx_buf;
    ctx->outbuf_len = tx_buf_len;
}

int spi_slave_xfer_complete(rtos_spi_slave_t *ctx, void **rx_buf, size_t *rx_len, void **tx_buf, size_t *tx_len, unsigned timeout)
{
    xfer_done_queue_item_t item;

    if (rtos_osal_queue_receive(&ctx->xfer_done_queue, &item, timeout) == RTOS_OSAL_SUCCESS) {
        *rx_buf = item.in_buf;
        *rx_len = item.bytes_read;
        *tx_buf = item.out_buf;
        *tx_len = item.bytes_written;

        return 0;
    } else {
        return -1;
    }
}

void rtos_spi_slave_start(
        rtos_spi_slave_t *spi_slave_ctx,
        void *app_data,
        rtos_spi_slave_start_cb_t start,
        rtos_spi_slave_xfer_done_cb_t xfer_done,
        unsigned interrupt_core_id,
        unsigned priority)
{
    uint32_t core_exclude_map;

    spi_slave_ctx->app_data = app_data;
    spi_slave_ctx->start = start;
    spi_slave_ctx->xfer_done = xfer_done;

    rtos_osal_queue_create(&spi_slave_ctx->xfer_done_queue, "spi_slave_queue", 2, sizeof(xfer_done_queue_item_t));

    /* Ensure that the SPI interrupt is enabled on the requested core */
    rtos_osal_thread_core_exclusion_get(NULL, &core_exclude_map);
    rtos_osal_thread_core_exclusion_set(NULL, ~(1 << interrupt_core_id));

    triggerable_enable_trigger(spi_slave_ctx->c.end_b);

    /* Restore the core exclusion map for the calling thread */
    rtos_osal_thread_core_exclusion_set(NULL, core_exclude_map);

    if (start != NULL || xfer_done != NULL) {
        rtos_osal_thread_create(
                &spi_slave_ctx->app_thread,
                "spi_slave_app_thread",
                (rtos_osal_entry_function_t) spi_slave_app_thread,
                spi_slave_ctx,
                RTOS_THREAD_STACK_SIZE(spi_slave_app_thread),
                priority);
    }
}

void rtos_spi_slave_init(
        rtos_spi_slave_t *spi_slave_ctx,
        uint32_t io_core_mask,
        xclock_t clock_block,
        int cpol,
        int cpha,
        port_t p_sclk,
        port_t p_mosi,
        port_t p_miso,
        port_t p_cs)
{
    memset(spi_slave_ctx, 0, sizeof(rtos_spi_slave_t));

    spi_slave_ctx->clock_block = clock_block;
    spi_slave_ctx->cpol = cpol;
    spi_slave_ctx->cpha = cpha;
    spi_slave_ctx->p_sclk = p_sclk;
    spi_slave_ctx->p_mosi = p_mosi;
    spi_slave_ctx->p_miso = p_miso;
    spi_slave_ctx->p_cs = p_cs;
    spi_slave_ctx->c = s_chan_alloc();

    triggerable_setup_interrupt_callback(spi_slave_ctx->c.end_b, spi_slave_ctx, RTOS_INTERRUPT_CALLBACK(rtos_spi_slave_isr));

    rtos_osal_thread_create(
            &spi_slave_ctx->hil_thread,
            "spi_slave_hil_thread",
            (rtos_osal_entry_function_t) spi_slave_hil_thread,
            spi_slave_ctx,
            RTOS_THREAD_STACK_SIZE(spi_slave_hil_thread),
            RTOS_OSAL_HIGHEST_PRIORITY);

    /* Ensure the SPI thread is never preempted */
    rtos_osal_thread_preemption_disable(&spi_slave_ctx->hil_thread);
    /* And ensure it only runs on one of the specified cores */
    rtos_osal_thread_core_exclusion_set(&spi_slave_ctx->hil_thread, ~io_core_mask);
}
