// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT RTOS_I2C

#include <xcore/triggerable.h>

#include "rtos_interrupt.h"
#include "rtos_i2c_slave.h"

#define RX_CB_CODE       0
#define TX_START_CB_CODE 1
#define TX_DONE_CB_CODE  2

#define RX_CB_FLAG       (1 << RX_CB_CODE)
#define TX_START_CB_FLAG (1 << TX_START_CB_CODE)
#define TX_DONE_CB_FLAG  (1 << TX_DONE_CB_CODE)

#define ALL_FLAGS (RX_CB_FLAG | TX_START_CB_FLAG | TX_DONE_CB_FLAG)

#define NO_WAIT 0
#define WAIT    1

DEFINE_RTOS_INTERRUPT_CALLBACK(rtos_i2c_slave_isr, arg)
{
    rtos_i2c_slave_t *ctx = arg;
    int isr_action;

    isr_action = s_chan_in_byte(ctx->c.end_b);

    rtos_osal_event_group_set_bits(&ctx->events, 1 << isr_action);
}

static void tx_state_clear(rtos_i2c_slave_t *ctx)
{
    ctx->tx_data = NULL;
    ctx->tx_data_len = 0;
    ctx->tx_data_i = 0;
    ctx->tx_data_sent = 0;
}

static void rx_state_clear(rtos_i2c_slave_t *ctx)
{
    ctx->rx_data_i = 0;
}

static void xfer_complete_check(rtos_i2c_slave_t *ctx, int wait)
{
    int waiting;
    int completed = 0;

    if (ctx->waiting_for_complete_cb) {
        (void) s_chan_in_byte(ctx->c.end_a);
        ctx->waiting_for_complete_cb = 0;
    }

    if (ctx->tx_data_sent > 0) {
        if (ctx->tx_done != NULL) {
            s_chan_out_byte(ctx->c.end_a, TX_DONE_CB_CODE);
            completed = 1;
            waiting = wait;
        } else {
            tx_state_clear(ctx);
        }
    } else if (ctx->rx_data_i > 0) {
        s_chan_out_byte(ctx->c.end_a, RX_CB_CODE);
        completed = 1;
        waiting = wait;
    }

    if (completed) {
        if (waiting) {
            (void) s_chan_in_byte(ctx->c.end_a);
        } else {
            ctx->waiting_for_complete_cb = 1;
        }
    }
}

static i2c_slave_ack_t i2c_ack_read_req(rtos_i2c_slave_t *ctx)
{
    /* could be repeated start */
    xfer_complete_check(ctx, WAIT);

    ctx->tx_data = ctx->data_buf;
    s_chan_out_byte(ctx->c.end_a, TX_START_CB_CODE);
    (void) s_chan_in_byte(ctx->c.end_a);

    ctx->tx_data_i = 0;
    ctx->tx_data_sent = 0;

    if (ctx->tx_data_len > 0 && ctx->tx_data != NULL)
    {
        return I2C_SLAVE_ACK;
    } else {
        return I2C_SLAVE_NACK;
    }
}

static i2c_slave_ack_t i2c_ack_write_req(rtos_i2c_slave_t *ctx)
{
    /* could be repeated start */
    xfer_complete_check(ctx, WAIT);

    ctx->rx_data_i = 0;
    return I2C_SLAVE_ACK;
}

static uint8_t i2c_master_req_data(rtos_i2c_slave_t *ctx)
{
    if (ctx->tx_data_len > 0 && ctx->tx_data != NULL) {
        uint8_t tx_byte = ctx->tx_data[ctx->tx_data_i++];

        if (ctx->tx_data_i == ctx->tx_data_len) {
            ctx->tx_data_i = 0;
        }

        ctx->tx_data_sent++;

        return tx_byte;
    } else {
        return 0xFF;
    }
}

static i2c_slave_ack_t i2c_master_sent_data(rtos_i2c_slave_t *ctx, uint8_t data)
{
    if (ctx->rx_data_i < RTOS_I2C_SLAVE_BUF_LEN) {
        ctx->data_buf[ctx->rx_data_i++] = data;
    }

    if (ctx->rx_data_i < RTOS_I2C_SLAVE_BUF_LEN) {
        return I2C_SLAVE_ACK;
    } else {
        return I2C_SLAVE_NACK;
    }
}

static void i2c_stop_bit(rtos_i2c_slave_t *ctx)
{
    xfer_complete_check(ctx, NO_WAIT);
}

static int i2c_shutdown(rtos_i2c_slave_t *ctx) {
    return 0;
}

static void i2c_slave_hil_thread(rtos_i2c_slave_t *ctx)
{
    i2c_callback_group_t i2c_cbg = {
        .ack_read_request = (ack_read_request_t) i2c_ack_read_req,
        .ack_write_request = (ack_write_request_t) i2c_ack_write_req,
        .master_requires_data = (master_requires_data_t) i2c_master_req_data,
        .master_sent_data = (master_sent_data_t) i2c_master_sent_data,
        .stop_bit = (stop_bit_t) i2c_stop_bit,
        .shutdown = (shutdown_t) i2c_shutdown,
        .app_data = ctx,
    };

    (void) s_chan_in_byte(ctx->c.end_a);

    rtos_printf("I2C slave on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());
    i2c_slave(&i2c_cbg,
              ctx->p_scl,
              ctx->p_sda,
              ctx->device_addr);
}

static void i2c_slave_app_thread(rtos_i2c_slave_t *ctx)
{
    uint32_t flags;

    if (ctx->start != NULL) {
        ctx->start(ctx, ctx->app_data);
    }

    s_chan_out_byte(ctx->c.end_b, 0);

    for (;;) {
        rtos_osal_event_group_get_bits(
                &ctx->events,
                ALL_FLAGS,
                RTOS_OSAL_OR_CLEAR,
                &flags,
                RTOS_OSAL_WAIT_FOREVER);

        if (flags & RX_CB_FLAG) {
            ctx->rx(ctx, ctx->app_data, ctx->data_buf, ctx->rx_data_i);
            rx_state_clear(ctx);
            s_chan_out_byte(ctx->c.end_b, 0);
        }

        if (flags & TX_START_CB_FLAG) {
            ctx->tx_data_len = ctx->tx_start(ctx, ctx->app_data, &ctx->tx_data);
            s_chan_out_byte(ctx->c.end_b, 0);
        }

        if (flags & TX_DONE_CB_FLAG) {
            ctx->tx_done(ctx, ctx->app_data, ctx->tx_data, ctx->tx_data_sent);
            tx_state_clear(ctx);
            s_chan_out_byte(ctx->c.end_b, 0);
        }
    }
}

void rtos_i2c_slave_start(
        rtos_i2c_slave_t *i2c_slave_ctx,
        void *app_data,
        rtos_i2c_slave_start_cb_t start,
        rtos_i2c_slave_rx_cb_t rx,
        rtos_i2c_slave_tx_start_cb_t tx_start,
        rtos_i2c_slave_tx_done_cb_t tx_done,
        unsigned interrupt_core_id,
        unsigned priority)
{
    uint32_t core_exclude_map;

    i2c_slave_ctx->app_data = app_data;
    i2c_slave_ctx->start = start;
    i2c_slave_ctx->rx = rx;
    i2c_slave_ctx->tx_start = tx_start;
    i2c_slave_ctx->tx_done = tx_done;

    i2c_slave_ctx->rx_data_i = 0;
    i2c_slave_ctx->tx_data = NULL;
    i2c_slave_ctx->tx_data_len = 0;
    i2c_slave_ctx->tx_data_i = 0;
    i2c_slave_ctx->tx_data_sent = 0;

    rtos_osal_event_group_create(&i2c_slave_ctx->events, "i2c_slave_events");

    /* Ensure that the I2C interrupt is enabled on the requested core */
    rtos_osal_thread_core_exclusion_get(NULL, &core_exclude_map);
    rtos_osal_thread_core_exclusion_set(NULL, ~(1 << interrupt_core_id));

    triggerable_enable_trigger(i2c_slave_ctx->c.end_b);

    /* Restore the core exclusion map for the calling thread */
    rtos_osal_thread_core_exclusion_set(NULL, core_exclude_map);

    rtos_osal_thread_create(
            &i2c_slave_ctx->app_thread,
            "i2c_slave_app_thread",
            (rtos_osal_entry_function_t) i2c_slave_app_thread,
            i2c_slave_ctx,
            RTOS_THREAD_STACK_SIZE(i2c_slave_app_thread),
            priority);
}

void rtos_i2c_slave_init(
        rtos_i2c_slave_t *i2c_slave_ctx,
        uint32_t io_core_mask,
        const port_t p_scl,
        const port_t p_sda,
        uint8_t device_addr)
{
    memset(i2c_slave_ctx, 0, sizeof(rtos_i2c_slave_t));

    i2c_slave_ctx->p_scl = p_scl;
    i2c_slave_ctx->p_sda = p_sda;
    i2c_slave_ctx->device_addr = device_addr;
    i2c_slave_ctx->c = s_chan_alloc();

    triggerable_setup_interrupt_callback(i2c_slave_ctx->c.end_b, i2c_slave_ctx, RTOS_INTERRUPT_CALLBACK(rtos_i2c_slave_isr));

    rtos_osal_thread_create(
            &i2c_slave_ctx->hil_thread,
            "i2c_slave_hil_thread",
            (rtos_osal_entry_function_t) i2c_slave_hil_thread,
            i2c_slave_ctx,
            RTOS_THREAD_STACK_SIZE(i2c_slave_hil_thread),
            RTOS_OSAL_HIGHEST_PRIORITY);

    /* Ensure the I2C thread is never preempted */
    rtos_osal_thread_preemption_disable(&i2c_slave_ctx->hil_thread);
    /* And ensure it only runs on one of the specified cores */
    rtos_osal_thread_core_exclusion_set(&i2c_slave_ctx->hil_thread, ~io_core_mask);
}
