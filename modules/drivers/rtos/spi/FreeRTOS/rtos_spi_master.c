// Copyright (c) 2021, XMOS Ltd, All rights reserved

#include <string.h>

#include <xcore/assert.h>
#include <xcore/interrupt.h>

#include "drivers/rtos/spi/FreeRTOS/rtos_spi_master.h"

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
    TaskHandle_t requesting_task;
} spi_xfer_req_t;

static void spi_xfer_thread(rtos_spi_master_t *ctx)
{
    spi_xfer_req_t req;
    unsigned current_priority = ctx->op_task_priority;

    for (;;) {
        xQueueReceive(ctx->xfer_req_queue, &req, portMAX_DELAY);

        switch (req.op) {
        case SPI_OP_START:
            if (current_priority != req.priority) {
                vTaskPrioritySet(ctx->op_task, req.priority);
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
                xTaskNotify(req.requesting_task, 0, eNoAction);
            } else {
                vPortFree(req.data_out);
            }
            break;

        case SPI_OP_DELAY:
            spi_master_delay_before_next_transfer(&req.ctx->dev_ctx, req.len);
            break;

        case SPI_OP_END:
            spi_master_end_transaction(&req.ctx->dev_ctx);

            if (current_priority != ctx->op_task_priority) {
                vTaskPrioritySet(ctx->op_task, ctx->op_task_priority);
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
    xSemaphoreTakeRecursive(ctx->bus_ctx->lock, portMAX_DELAY);

    spi_xfer_req_t req;
    req.op = SPI_OP_START;
    req.ctx = ctx;
    req.priority = uxTaskPriorityGet(NULL);
    xQueueSend(ctx->bus_ctx->xfer_req_queue, &req, portMAX_DELAY);
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
        req.requesting_task = xTaskGetCurrentTaskHandle();
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
        req.requesting_task = NULL;
    }

    xQueueSend(ctx->bus_ctx->xfer_req_queue, &req, portMAX_DELAY);

    if (data_in != NULL) {
        xTaskNotifyWait(
                0x00000000UL,    /* Don't clear notification bits on entry */
                0xFFFFFFFFUL,    /* Reset full notification value on exit */
                NULL,          /* Pass out notification value into value */
                portMAX_DELAY ); /* Wait indefinitely until next notification */
    }
}

__attribute__((fptrgroup("rtos_spi_master_delay_before_next_transfer_fptr_grp")))
static void spi_master_local_delay_before_next_transfer(
        rtos_spi_master_device_t *ctx,
        uint32_t delay_ticks)
{
    spi_xfer_req_t req;
    req.op = SPI_OP_DELAY;
    req.len = delay_ticks;
    xQueueSend(ctx->bus_ctx->xfer_req_queue, &req, portMAX_DELAY);
}

__attribute__((fptrgroup("rtos_spi_master_transaction_end_fptr_grp")))
static void spi_master_local_transaction_end(
        rtos_spi_master_device_t *ctx)
{
    spi_xfer_req_t req;
    req.op = SPI_OP_END;
    req.ctx = ctx;
    xQueueSend(ctx->bus_ctx->xfer_req_queue, &req, portMAX_DELAY);

    xSemaphoreGiveRecursive(ctx->bus_ctx->lock);
}

void rtos_spi_master_start(
        rtos_spi_master_t *spi_master_ctx,
        unsigned priority)
{
    spi_master_ctx->lock = xSemaphoreCreateRecursiveMutex();

    spi_master_ctx->xfer_req_queue = xQueueCreate(2, sizeof(spi_xfer_req_t));

    spi_master_ctx->op_task_priority = priority;
    xTaskCreate(
                (TaskFunction_t) spi_xfer_thread,
                "spi_xfer_thread",
                RTOS_THREAD_STACK_SIZE(spi_xfer_thread),
                spi_master_ctx,
                priority,
                &spi_master_ctx->op_task);

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
