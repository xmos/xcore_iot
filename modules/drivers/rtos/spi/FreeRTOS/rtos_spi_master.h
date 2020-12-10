// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef RTOS_SPI_MASTER_H_
#define RTOS_SPI_MASTER_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "spi.h"

#include "drivers/rtos/rpc/api/rtos_driver_rpc.h"

typedef struct rtos_spi_master_struct rtos_spi_master_t;
typedef struct rtos_spi_master_device_struct rtos_spi_master_device_t;

struct rtos_spi_master_struct {
    rtos_driver_rpc_t *rpc_config;

    __attribute__((fptrgroup("rtos_spi_master_transaction_start_fptr_grp")))
    void (*transaction_start)(rtos_spi_master_device_t *);

    __attribute__((fptrgroup("rtos_spi_master_transfer_fptr_grp")))
    void (*transfer)(rtos_spi_master_device_t *, uint8_t *, uint8_t *, size_t);

    __attribute__((fptrgroup("rtos_spi_master_delay_before_next_transfer_fptr_grp")))
    void (*delay_before_next_transfer)(rtos_spi_master_device_t *, uint32_t);

    __attribute__((fptrgroup("rtos_spi_master_transaction_end_fptr_grp")))
    void (*transaction_end)(rtos_spi_master_device_t *);

    spi_master_t ctx;

    /* BEGIN RTOS SPECIFIC MEMBERS. */
    QueueHandle_t xfer_req_queue;
    SemaphoreHandle_t lock;
};

struct rtos_spi_master_device_struct {
    rtos_spi_master_device_t *host_dev_ctx_ptr; /* Only used by RPC clients */

    rtos_spi_master_t *bus_ctx;
    spi_master_device_t dev_ctx;
};

#include "drivers/rtos/spi/api/rtos_spi_master_rpc.h"

inline void rtos_spi_master_transaction_start(
        rtos_spi_master_device_t *ctx)
{
    ctx->bus_ctx->transaction_start(ctx);
}

inline void rtos_spi_master_transfer(
        rtos_spi_master_device_t *ctx,
        uint8_t *data_out,
        uint8_t *data_in,
        size_t len)
{
    ctx->bus_ctx->transfer(ctx, data_out, data_in, len);
}

inline void rtos_spi_master_delay_before_next_transfer(
        rtos_spi_master_device_t *ctx,
        uint32_t delay_ticks)
{
    ctx->bus_ctx->delay_before_next_transfer(ctx, delay_ticks);
}

inline void rtos_spi_master_transaction_end(
        rtos_spi_master_device_t *ctx)
{
    ctx->bus_ctx->transaction_end(ctx);
}

void rtos_spi_master_start(
        rtos_spi_master_t *spi_master_ctx,
        unsigned priority);

void rtos_spi_master_init(
        rtos_spi_master_t *bus_ctx,
        xclock_t clock_block,
        port_t cs_port,
        port_t sclk_port,
        port_t mosi_port,
        port_t miso_port);

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
        uint32_t cs_to_cs_delay_ticks);


#endif /* RTOS_SPI_MASTER_H_ */
