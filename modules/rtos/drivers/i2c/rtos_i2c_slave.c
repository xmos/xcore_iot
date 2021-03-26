// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "rtos/drivers/i2c/api/rtos_i2c_slave.h"

static void xfer_complete_check(rtos_i2c_slave_t *ctx)
{
    if (ctx->tx_data_sent > 0) {
        if (ctx->tx_done != NULL) {
            ctx->tx_done(ctx, ctx->app_data, ctx->tx_data, ctx->tx_data_sent);
        }
        ctx->tx_data = NULL;
        ctx->tx_data_len = 0;
        ctx->tx_data_i = 0;
        ctx->tx_data_sent = 0;
    } else if (ctx->rx_data_i > 0) {
        ctx->rx(ctx, ctx->app_data, ctx->data_buf, ctx->rx_data_i);
        ctx->rx_data_i = 0;
    }
}

static i2c_slave_ack_t i2c_ack_read_req(rtos_i2c_slave_t *ctx)
{
    /* could be repeated start */
    xfer_complete_check(ctx);

    ctx->tx_data = ctx->data_buf;
    ctx->tx_data_len = ctx->tx_start(ctx, ctx->app_data, &ctx->tx_data);

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
    xfer_complete_check(ctx);

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
    xfer_complete_check(ctx);
}

static int i2c_shutdown(rtos_i2c_slave_t *ctx) {
    return 0;
}

static void i2c_slave_thread(rtos_i2c_slave_t *ctx)
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

    /* Ensure the I2C thread is never preempted */
    rtos_osal_thread_preemption_disable(NULL);
    /* And exclude it from core 0 where the system tick interrupt runs */
    rtos_osal_thread_core_exclusion_set(NULL, (1 << 0));

    rtos_printf("I2C slave on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());
    i2c_slave(&i2c_cbg,
              ctx->p_scl,
              ctx->p_sda,
              ctx->device_addr);
}

void rtos_i2c_slave_start(
        rtos_i2c_slave_t *i2c_slave_ctx,
        void *app_data,
        rtos_i2c_slave_rx_cb_t rx,
        rtos_i2c_slave_tx_start_cb_t tx_start,
        rtos_i2c_slave_tx_done_cb_t tx_done,
        unsigned priority)
{
    i2c_slave_ctx->app_data = app_data;
    i2c_slave_ctx->rx = rx;
    i2c_slave_ctx->tx_start = tx_start;
    i2c_slave_ctx->tx_done = tx_done;

    i2c_slave_ctx->rx_data_i = 0;
    i2c_slave_ctx->tx_data = NULL;
    i2c_slave_ctx->tx_data_len = 0;
    i2c_slave_ctx->tx_data_i = 0;
    i2c_slave_ctx->tx_data_sent = 0;

    rtos_osal_thread_create(
            NULL,
            "i2c_slave_thread",
            (rtos_osal_entry_function_t) i2c_slave_thread,
            i2c_slave_ctx,
            RTOS_THREAD_STACK_SIZE(i2c_slave_thread),
            priority);

}

void rtos_i2c_slave_init(
        rtos_i2c_slave_t *i2c_slave_ctx,
        const port_t p_scl,
        const port_t p_sda,
        uint8_t device_addr)
{
    memset(i2c_slave_ctx, 0, sizeof(rtos_i2c_slave_t));

    i2c_slave_ctx->p_scl = p_scl;
    i2c_slave_ctx->p_sda = p_sda;
    i2c_slave_ctx->device_addr = device_addr;
}
