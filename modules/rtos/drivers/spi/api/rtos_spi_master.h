// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_SPI_MASTER_H_
#define RTOS_SPI_MASTER_H_

/**
 * \addtogroup rtos_spi_master_driver rtos_spi_master_driver
 *
 * The public API for using the RTOS SPI master driver.
 * @{
 */

#include "spi.h"

#include "rtos_osal.h"
#include "rtos_driver_rpc.h"

/**
 * Typedef to the RTOS SPI master driver instance struct.
 */
typedef struct rtos_spi_master_struct rtos_spi_master_t;

/**
 * Typedef to the RTOS SPI device instance struct.
 */
typedef struct rtos_spi_master_device_struct rtos_spi_master_device_t;

/**
 * Struct representing an RTOS SPI master driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
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

    unsigned op_task_priority;
    rtos_osal_thread_t op_task;
    rtos_osal_queue_t xfer_req_queue;
    rtos_osal_semaphore_t data_ready;
    rtos_osal_mutex_t lock;
};

/**
 * Struct representing an RTOS SPI device instance.
 *
 * The members in this struct should not be accessed directly.
 */
struct rtos_spi_master_device_struct {
    rtos_spi_master_device_t *host_dev_ctx_ptr; /* Only used by RPC clients */

    rtos_spi_master_t *bus_ctx;
    spi_master_device_t dev_ctx;
};

#include "rtos_spi_master_rpc.h"

/**
 * \addtogroup rtos_spi_master_driver_core rtos_spi_master_driver_core
 *
 * The core functions for using an RTOS SPI master driver instance after
 * it has been initialized and started. These functions may be used
 * by both the host and any client tiles that RPC has been enabled for.
 * @{
 */

/**
 * Starts a transaction with the specified SPI device on a SPI bus.
 * This leaves chip select asserted.
 *
 * \param ctx A pointer to the SPI device instance.
 */
inline void rtos_spi_master_transaction_start(
        rtos_spi_master_device_t *ctx)
{
    ctx->bus_ctx->transaction_start(ctx);
}

/**
 * Transfers data to and from the specified SPI device on a SPI bus.
 * The transaction must already have been started by calling
 * rtos_spi_master_transaction_start() on the same device instance.
 * This may be called multiple times during a single transaction.
 *
 * This function may return before the transfer is complete when data_in
 * is NULL, as the actual transfer operation is queued and executed by a
 * thread created by the driver.
 *
 * \param ctx      A pointer to the SPI device instance.
 * \param data_out Pointer to the data to transfer to the device.
 *                 This may be NULL if there is no data to send.
 * \param data_in  Pointer to the buffer to save the received data to.
 *                 This may be NULL if the received data is not needed.
 * \param len      The number of bytes to transfer in each direction.
 *                 This number of bytes must be available in both the
 *                 \p data_out and \p data_in buffers if they are not NULL.
 */
inline void rtos_spi_master_transfer(
        rtos_spi_master_device_t *ctx,
        uint8_t *data_out,
        uint8_t *data_in,
        size_t len)
{
    ctx->bus_ctx->transfer(ctx, data_out, data_in, len);
}

/**
 * If there is a minimum amount of idle time that is required by
 * the device between transfers within a single transaction, then
 * this may be called between each transfer where a delay is required.
 *
 * This function will return immediately. If the call for the next
 * transfer happens before the minimum time specified has elapsed, the
 * delay will occur then before the transfer begins.
 *
 * \note This must be called during a transaction, otherwise the behavior
 * is unspecified.
 *
 * \note Technically the next transfer will occur no earlier than
 * \p delay_ticks after this function is called, so this should be
 * called immediately following a transfer, rather than immediately
 * before the next.
 *
 * \param ctx         A pointer to the SPI device instance.
 * \param delay_ticks The number of reference clock ticks to delay.
 */
inline void rtos_spi_master_delay_before_next_transfer(
        rtos_spi_master_device_t *ctx,
        uint32_t delay_ticks)
{
    ctx->bus_ctx->delay_before_next_transfer(ctx, delay_ticks);
}

/**
 * Ends a transaction with the specified SPI device on a SPI bus.
 * This leaves chip select de-asserted.
 *
 * \param ctx A pointer to the SPI device instance.
 */
inline void rtos_spi_master_transaction_end(
        rtos_spi_master_device_t *ctx)
{
    ctx->bus_ctx->transaction_end(ctx);
}

/**@}*/


/**
 * Starts an RTOS SPI master driver instance. This must only be called by the tile that
 * owns the driver instance. It may be called either before or after starting
 * the RTOS, but must be called before any of the core SPI master driver functions are
 * called with this instance.
 *
 * rtos_spi_master_init() must be called on this SPI master driver instance prior to calling this.
 *
 * \param spi_master_ctx  A pointer to the SPI master driver instance to start.
 * \param priority        The priority of the task that gets created by the driver to
 *                        handle the SPI master interface.
 */
void rtos_spi_master_start(
        rtos_spi_master_t *spi_master_ctx,
        unsigned priority);

/**
 * Initializes an RTOS SPI master driver instance.
 * This must only be called by the tile that owns the driver instance. It may be
 * called either before or after starting the RTOS, but must be called before calling
 * rtos_spi_master_start() or any of the core SPI master driver functions with this instance.
 *
 * \param bus_ctx     A pointer to the SPI master driver instance to initialize.
 * \param clock_block The clock block to use for the SPI master interface.
 * \param cs_port     The SPI interface's chip select port. This may be a multi-bit port.
 * \param sclk_port   The SPI interface's SCLK port. Must be a 1-bit port.
 * \param mosi_port   The SPI interface's MOSI port. Must be a 1-bit port.
 * \param miso_port   The SPI interface's MISO port. Must be a 1-bit port.
 */
void rtos_spi_master_init(
        rtos_spi_master_t *bus_ctx,
        xclock_t clock_block,
        port_t cs_port,
        port_t sclk_port,
        port_t mosi_port,
        port_t miso_port);

/**
 * Initialize a SPI device. Multiple SPI devices may be initialized per RTOS SPI master
 * driver instance. Each must be on a unique pin of the interface's chip select port.
 * This must only be called by the tile that owns the driver instance. It may be
 * called either before or after starting the RTOS, but must be called before calling
 * rtos_spi_master_start() or any of the core SPI master driver functions with this instance.
 *
 * \param dev_ctx               A pointer to the SPI device instance to initialize.
 * \param bus_ctx               A pointer to the SPI master driver instance to attach the device to.
 * \param cs_pin                The bit number of the chip select port that is connected to the device's chip select pin.
 * \param cpol                  The clock polarity required by the device.
 * \param cpha                  The clock phase required by the device.
 * \param source_clock          The source clock to derive SCLK from. See spi_master_source_clock_t.
 * \param clock_divisor         The value to divide the source clock by.
 *                              The frequency of SCLK will be set to:
 *                               - (F_src) / (4 * clock_divisor) when clock_divisor > 0
 *                               - (F_src) / (2)                 when clock_divisor = 0
 *                              Where F_src is the frequency of the source clock.
 * \param miso_sample_delay     When to sample MISO. See spi_master_sample_delay_t.
 * \param miso_pad_delay        The number of core clock cycles to delay sampling the MISO pad during
 *                              a transaction. This allows for more fine grained adjustment
 *                              of sampling time. The value may be between 0 and 5.
 * \param cs_to_clk_delay_ticks The minimum number of reference clock ticks between assertion of chip select
 *                              and the first clock edge.
 * \param clk_to_cs_delay_ticks The minimum number of reference clock ticks between the last clock edge and
 *                              de-assertion of chip select.
 * \param cs_to_cs_delay_ticks  The minimum number of reference clock ticks between transactions, which is between
 *                              de-assertion of chip select and the end of one transaction, and its re-assertion at
 *                              the beginning of the next.
 */
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

/**@}*/

#endif /* RTOS_SPI_MASTER_H_ */
