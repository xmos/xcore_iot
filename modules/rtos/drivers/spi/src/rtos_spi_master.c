// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <string.h>

#include <xcore/assert.h>
#include <xcore/interrupt.h>

#include "rtos_spi_master.h"

#define SPI_OP_START 0
#define SPI_OP_XFER  1
#define SPI_OP_DELAY 2
#define SPI_OP_END   3

typedef struct {
    rtos_spi_master_device_t *ctx;
    int op;
    uint8_t *data_out;
    uint8_t *data_in;
    size_t len;
    unsigned priority;
} spi_xfer_req_t;

static void spi_xfer_thread(rtos_spi_master_t *ctx)
{
    spi_xfer_req_t req;
    unsigned current_priority = ctx->op_task_priority;

    for (;;) {
        rtos_osal_queue_receive(&ctx->xfer_req_queue, &req, RTOS_OSAL_WAIT_FOREVER);

        switch (req.op) {
        case SPI_OP_START:
            if (current_priority != req.priority) {
                rtos_osal_thread_priority_set(&ctx->op_task, req.priority);
                current_priority = req.priority;
            }

            spi_master_start_transaction(&req.ctx->dev_ctx);
            break;

        case SPI_OP_XFER:
            /*
             * It would be nicer if spi_master_transfer() could handle being
             * interrupted. At the moment it doesn't seem possible. This is
             * the safest thing to do.
             */
            interrupt_mask_all();

            spi_master_transfer(&req.ctx->dev_ctx,
                    req.data_out,
                    req.data_in,
                    req.len);

            interrupt_unmask_all();

            if (req.data_in != NULL) {
                rtos_osal_semaphore_put(&ctx->data_ready);
            } else {
                rtos_osal_free(req.data_out);
            }
            break;

        case SPI_OP_DELAY:
            spi_master_delay_before_next_transfer(&req.ctx->dev_ctx, req.len);
            break;

        case SPI_OP_END:
            spi_master_end_transaction(&req.ctx->dev_ctx);

            if (current_priority != ctx->op_task_priority) {
                rtos_osal_thread_priority_set(&ctx->op_task, ctx->op_task_priority);
                current_priority = ctx->op_task_priority;
            }
            break;
        }
    }
}

__attribute__((fptrgroup("rtos_spi_master_transaction_start_fptr_grp")))
static void spi_master_local_transaction_start(
        rtos_spi_master_device_t *ctx)
{
    rtos_osal_mutex_get(&ctx->bus_ctx->lock, RTOS_OSAL_WAIT_FOREVER);

    spi_xfer_req_t req;
    req.op = SPI_OP_START;
    req.ctx = ctx;
    rtos_osal_thread_priority_get(NULL, &req.priority);
    rtos_osal_queue_send(&ctx->bus_ctx->xfer_req_queue, &req, RTOS_OSAL_WAIT_FOREVER);
}

__attribute__((fptrgroup("rtos_spi_master_transfer_fptr_grp")))
static void spi_master_local_transfer(
        rtos_spi_master_device_t *ctx,
        uint8_t *data_out,
        uint8_t *data_in,
        size_t len)
{
    spi_xfer_req_t req;

    req.op = SPI_OP_XFER;
    req.ctx = ctx;
    req.data_in = data_in;
    req.len = len;

    if (data_in != NULL) {
        req.data_out = data_out;
    } else {
        /*
         * TODO: Consider a zero copy option? Caller would
         * be required to malloc data_out. Also a no-free
         * option where the caller knows that data_out will
         * still be in scope by the time the xfer is done,
         * for example if this tx only call is followed by
         * an rx.
         */
        req.data_out = rtos_osal_malloc(len);
        memcpy(req.data_out, data_out, len);
    }

    rtos_osal_queue_send(&ctx->bus_ctx->xfer_req_queue, &req, RTOS_OSAL_WAIT_FOREVER);

    if (data_in != NULL) {
        rtos_osal_semaphore_get(&ctx->bus_ctx->data_ready, RTOS_OSAL_WAIT_FOREVER);
    }
}

__attribute__((fptrgroup("rtos_spi_master_delay_before_next_transfer_fptr_grp")))
static void spi_master_local_delay_before_next_transfer(
        rtos_spi_master_device_t *ctx,
        uint32_t delay_ticks)
{
    spi_xfer_req_t req;
    req.op = SPI_OP_DELAY;
    req.ctx = ctx;
    req.len = delay_ticks;
    rtos_osal_queue_send(&ctx->bus_ctx->xfer_req_queue, &req, RTOS_OSAL_WAIT_FOREVER);
}

__attribute__((fptrgroup("rtos_spi_master_transaction_end_fptr_grp")))
static void spi_master_local_transaction_end(
        rtos_spi_master_device_t *ctx)
{
    spi_xfer_req_t req;
    req.op = SPI_OP_END;
    req.ctx = ctx;
    rtos_osal_queue_send(&ctx->bus_ctx->xfer_req_queue, &req, RTOS_OSAL_WAIT_FOREVER);

    rtos_osal_mutex_put(&ctx->bus_ctx->lock);
}

void rtos_spi_master_start(
        rtos_spi_master_t *spi_master_ctx,
        unsigned priority)
{
    rtos_osal_mutex_create(&spi_master_ctx->lock, "spi_master_lock", RTOS_OSAL_RECURSIVE);
    rtos_osal_queue_create(&spi_master_ctx->xfer_req_queue, "spi_req_queue", 2, sizeof(spi_xfer_req_t));
    rtos_osal_semaphore_create(&spi_master_ctx->data_ready, "spi_dr_sem", 1, 0);

    spi_master_ctx->op_task_priority = priority;
    rtos_osal_thread_create(
            &spi_master_ctx->op_task,
            "spi_xfer_thread",
            (rtos_osal_entry_function_t) spi_xfer_thread,
            spi_master_ctx,
            RTOS_THREAD_STACK_SIZE(spi_xfer_thread),
            priority);

    if (spi_master_ctx->rpc_config != NULL && spi_master_ctx->rpc_config->rpc_host_start != NULL) {
        spi_master_ctx->rpc_config->rpc_host_start(spi_master_ctx->rpc_config);
    }
}

void rtos_spi_master_init(
        rtos_spi_master_t *bus_ctx,
        xclock_t clock_block,
        port_t cs_port,
        port_t sclk_port,
        port_t mosi_port,
        port_t miso_port)
{
    spi_master_init(
                    &bus_ctx->ctx,
                    clock_block,
                    cs_port,
                    sclk_port, mosi_port, miso_port);

    /*
     * TODO: Setting all of these here results in all these functions
     * getting linked into the binary, regardless of whether or not
     * they are used. Is there a clean way to prevent this?
     */
    bus_ctx->rpc_config = NULL;
    bus_ctx->transaction_start = spi_master_local_transaction_start;
    bus_ctx->transfer = spi_master_local_transfer;
    bus_ctx->delay_before_next_transfer = spi_master_local_delay_before_next_transfer;
    bus_ctx->transaction_end = spi_master_local_transaction_end;
}

void rtos_spi_master_device_init(
        rtos_spi_master_device_t *dev_ctx,
        rtos_spi_master_t *bus_ctx,
        uint32_t cs_pin,
        int cpol,
        int cpha,
        spi_master_source_clock_t source_clock,
        uint32_t clock_divisor,
        spi_master_sample_delay_t miso_sample_delay,
        uint32_t miso_pad_delay,
        uint32_t cs_to_clk_delay_ticks,
        uint32_t clk_to_cs_delay_ticks,
        uint32_t cs_to_cs_delay_ticks)
{
    dev_ctx->bus_ctx = bus_ctx;

    spi_master_device_init(
                           &dev_ctx->dev_ctx,
                           &bus_ctx->ctx,
                           cs_pin,
                           cpol, cpha,
                           source_clock,
                           clock_divisor,
                           miso_sample_delay,
                           miso_pad_delay,
                           cs_to_clk_delay_ticks,
                           clk_to_cs_delay_ticks, cs_to_cs_delay_ticks);
}
