// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_SPI_SLAVE_H_
#define RTOS_SPI_SLAVE_H_

/**
 * \addtogroup rtos_spi_slave_driver rtos_spi_slave_driver
 *
 * The public API for using the RTOS SPI slave driver.
 * @{
 */

#include <xcore/channel_streaming.h>
#include "spi.h"

#include "rtos_osal.h"
#include "rtos_driver_rpc.h"

/**
 * This attribute must be specified on all RTOS SPI slave callback functions
 * provided by the application.
 */
#define RTOS_SPI_SLAVE_CALLBACK_ATTR __attribute__((fptrgroup("rtos_spi_slave_cb_fptr_grp")))

/**
 * Typedef to the RTOS SPI slave driver instance struct.
 */
typedef struct rtos_spi_slave_struct rtos_spi_slave_t;

/**
 * Function pointer type for application provided RTOS SPI slave start callback functions.
 *
 * These callback functions are optionally called by a SPI slave driver's thread when it is first
 * started. This gives the application a chance to perform startup initialization from within the
 * driver's thread. It is a good place for the first call to spi_slave_xfer_prepare().
 *
 * \param ctx           A pointer to the associated SPI slave driver instance.
 * \param app_data      A pointer to application specific data provided
 *                      by the application. Used to share data between
 *                      this callback function and the application.
 */
typedef void (*rtos_spi_slave_start_cb_t)(rtos_spi_slave_t *ctx, void *app_data);

/**
 * Function pointer type for application provided RTOS SPI slave transfer done callback functions.
 *
 * These callback functions are optionally called when a SPI slave driver instance is done transferring data with
 * a master device.
 *
 * An application can use this to be notified immediately when a transfer has completed. It can then call
 * spi_slave_xfer_complete() with a timeout of 0 from within this callback to get the transfer results.
 *
 * \param ctx           A pointer to the associated SPI slave driver instance.
 * \param app_data      A pointer to application specific data provided
 *                      by the application. Used to share data between
 *                      this callback function and the application.
 */
typedef void (*rtos_spi_slave_xfer_done_cb_t)(rtos_spi_slave_t *ctx, void *app_data);

/**
 * Struct representing an RTOS SPI slave driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
struct rtos_spi_slave_struct {
    xclock_t clock_block;
    int cpol;
    int cpha;
    port_t p_sclk;
    port_t p_mosi;
    port_t p_miso;
    port_t p_cs;

    void *app_data;
    uint8_t *out_buf;
    size_t outbuf_len;
    size_t bytes_written;
    uint8_t *in_buf;
    size_t inbuf_len;
    size_t bytes_read;

    volatile int waiting_for_isr;

    RTOS_SPI_SLAVE_CALLBACK_ATTR rtos_spi_slave_start_cb_t start;
    RTOS_SPI_SLAVE_CALLBACK_ATTR rtos_spi_slave_xfer_done_cb_t xfer_done;

    streaming_channel_t c;
    rtos_osal_queue_t xfer_done_queue;
    rtos_osal_thread_t hil_thread;
    rtos_osal_thread_t app_thread;
};

/**
 * Prepares an RTOS SPI slave driver instance with buffers for subsequent transfers.
 * Before this is called for the first time, any transfers initiated by a master device
 * with result in all received data over MOSI being dropped, and all data sent over
 * MISO being zeros.
 *
 * This only needs to be called when the buffers need to be changed. If all transfers
 * will use the same buffers, then this only needs to be called once during initialization.
 *
 * \param ctx        A pointer to the SPI slave driver instance to use.
 * \param rx_buf     The buffer to receive data into for any subsequent transfers.
 * \param rx_buf_    The length in bytes of \p rx_buf. If the master transfers more than
 *                   this during a single transfer, then the bytes that do not fit within
 *                   \p rx_buf will be lost.
 * \param tx_buf     The buffer to send data from for any subsequent transfers.
 * \param tx_buf_len The length in bytes of \p tx_buf. If the master transfers more than
 *                   this during a single transfer, zeros will be sent following the last
 *                   byte \p tx_buf.
 */
void spi_slave_xfer_prepare(
        rtos_spi_slave_t *ctx,
        void *rx_buf,
        size_t rx_buf_len,
        void *tx_buf,
        size_t tx_buf_len);

/**
 * Waits for a SPI transfer to complete. Returns either when the timeout is reached, or
 * when a transfer completes, whichever comes first. If a transfer does complete, then
 * the buffers and the number of bytes read from or written to them are returned via
 * the parameters.
 *
 * \param ctx     A pointer to the SPI slave driver instance to use.
 * \param rx_buf  The receive buffer used for the completed transfer. This is set by
 *                the function upon completion of a transfer.
 * \param rx_len  The number of bytes written to rx_buf. This is set by the function
 *                upon completion of a transfer.
 * \param tx_buf  The transmit buffer used for the completed transfer. This is set by
 *                the function upon completion of a transfer.
 * \param tx_len  The number of bytes sent from tx_buf. This is set by the function
 *                upon completion of a transfer.
 * \param timeout The number of RTOS ticks to wait before the next transfer is complete.
 *                When called from within the "xfer_done" callback, this should be 0.
 *
 * \retval  0 if a transfer completed. All buffers and lengths are set in this case.
 * \retval -1 if no transfer completed before the timeout expired. No buffers or lengths
 *            are returned in this case.
 */
int spi_slave_xfer_complete(
        rtos_spi_slave_t *ctx,
        void **rx_buf,
        size_t *rx_len,
        void **tx_buf,
        size_t *tx_len,
        unsigned timeout);

/**
 * Starts an RTOS SPI slave driver instance. This must only be called by the tile that
 * owns the driver instance. It must be called after starting the RTOS from an RTOS thread.
 *
 * rtos_spi_slave_init() must be called on this SPI slave driver instance prior to calling this.
 *
 * \param spi_slave_ctx     A pointer to the SPI slave driver instance to start.
 * \param app_data          A pointer to application specific data to pass to
 *                          the callback functions.
 * \param start             The callback function that is called when the driver's
 *                          thread starts. This is optional and may be NULL.
 * \param xfer_done         The callback function that is notified when transfers are
 *                          complete. This is optional and may be NULL.
 * \param interrupt_core_id The ID of the core on which to enable the SPI interrupt.
 * \param priority          The priority of the task that gets created by the driver to
 *                          call the callback functions. If both callback functions are
 *                          NULL, then this is unused.
 */
void rtos_spi_slave_start(
        rtos_spi_slave_t *spi_slave_ctx,
        void *app_data,
        rtos_spi_slave_start_cb_t start,
        rtos_spi_slave_xfer_done_cb_t xfer_done,
        unsigned interrupt_core_id,
        unsigned priority);

/**
 * Initializes an RTOS SPI slave driver instance.
 * This must only be called by the tile that owns the driver instance. It should be
 * called before starting the RTOS, and must be called before calling rtos_spi_slave_start().
 *
 * \param spi_slave_ctx A pointer to the SPI slave driver instance to initialize.
 * \param io_core_mask  A bitmask representing the cores on which the low level SPI I/O thread
 *                      created by the driver is allowed to run. Bit 0 is core 0, bit 1 is core 1,
 *                      etc.
 * \param clock_block   The clock block to use for the SPI slave.
 * \param cpol          The clock polarity to use.
 * \param cpha          The clock phase to use.
 * \param p_sclk        The SPI slave's SCLK port. Must be a 1-bit port.
 * \param p_mosi        The SPI slave's MOSI port. Must be a 1-bit port.
 * \param p_miso        The SPI slave's MISO port. Must be a 1-bit port.
 * \param p_cs          The SPI slave's CS port. Must be a 1-bit port.
 */
void rtos_spi_slave_init(
        rtos_spi_slave_t *spi_slave_ctx,
        uint32_t io_core_mask,
        xclock_t clock_block,
        int cpol,
        int cpha,
        port_t p_sclk,
        port_t p_mosi,
        port_t p_miso,
        port_t p_cs);

/**@}*/

#endif /* RTOS_SPI_SLAVE_H_ */
