// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <string.h>

#include <xcore/assert.h>
#include <xcore/interrupt.h>

#include "drivers/rtos/spi/FreeRTOS/rtos_spi_master.h"

typedef struct {
    rtos_spi_master_device_t *ctx;
    uint8_t *data_out;
    uint8_t *data_in;
    size_t len;
    TaskHandle_t req_task;
} spi_xfer_req_t;

static void spi_xfer_thread(rtos_spi_master_t *ctx)
{
    spi_xfer_req_t req;

    for (;;) {
        xQueueReceive(ctx->xfer_req_queue, &req, portMAX_DELAY);

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
            xTaskNotify(req.req_task, 0, eNoAction);
        } else {
            vPortFree(req.data_out);
        }
    }
}

__attribute__((fptrgroup("rtos_spi_master_transaction_start_fptr_grp")))
static void spi_master_local_transaction_start(
        rtos_spi_master_device_t *ctx)
{
    xSemaphoreTakeRecursive(ctx->bus_ctx->lock, portMAX_DELAY);

    spi_master_start_transaction(&ctx->dev_ctx);
}

__attribute__((fptrgroup("rtos_spi_master_transfer_fptr_grp")))
static void spi_master_local_transfer(
        rtos_spi_master_device_t *ctx,
        uint8_t *data_out,
        uint8_t *data_in,
        size_t len)
{
#if 1
    interrupt_mask_all();

    spi_master_transfer(&ctx->dev_ctx,
            data_out,
            data_in,
            len);

    interrupt_unmask_all();
#else

    spi_xfer_req_t req;

    req.ctx = ctx;
    req.data_in = data_in;
    req.len = len;

    if (data_in != NULL) {
        req.data_out = data_out;
        req.req_task = xTaskGetCurrentTaskHandle();
    } else {
        /*
         * TODO: Consider a zero copy option? Caller would
         * be required to malloc data_out. Also a no-free
         * option where the caller knows that data_out will
         * still be in scope by the time the xfer is done,
         * for example if this tx only call is followed by
         * an rx.
         */
        req.data_out = pvPortMalloc(len);
        memcpy(req.data_out, data_out, len);
        req.req_task = NULL;
    }

    xQueueSend(ctx->bus_ctx->xfer_req_queue, &req, portMAX_DELAY);

    if (data_in != NULL) {
        xTaskNotifyWait(
                0x00000000UL,    /* Don't clear notification bits on entry */
                0xFFFFFFFFUL,    /* Reset full notification value on exit */
                NULL,          /* Pass out notification value into value */
                portMAX_DELAY ); /* Wait indefinitely until next notification */
    }
#endif
}

__attribute__((fptrgroup("rtos_spi_master_delay_before_next_transfer_fptr_grp")))
static void spi_master_local_delay_before_next_transfer(
        rtos_spi_master_device_t *ctx,
        uint32_t delay_ticks)
{
    spi_master_delay_before_next_transfer(&ctx->dev_ctx, delay_ticks);
}

__attribute__((fptrgroup("rtos_spi_master_transaction_end_fptr_grp")))
static void spi_master_local_transaction_end(
        rtos_spi_master_device_t *ctx)
{
    spi_master_end_transaction(&ctx->dev_ctx);

    xSemaphoreGiveRecursive(ctx->bus_ctx->lock);
}

void rtos_spi_master_start(
        rtos_spi_master_t *spi_master_ctx,
        unsigned priority)
{
    spi_master_ctx->lock = xSemaphoreCreateRecursiveMutex();

    spi_master_ctx->xfer_req_queue = xQueueCreate(2, sizeof(spi_xfer_req_t));

    xTaskCreate(
                (TaskFunction_t) spi_xfer_thread,
                "spi_xfer_thread",
                RTOS_THREAD_STACK_SIZE(spi_xfer_thread),
                spi_master_ctx,
                priority,
                NULL);

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
