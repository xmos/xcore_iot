// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_SPI_SLAVE_H_
#define RTOS_SPI_SLAVE_H_

/**
 * \defgroup rtos_spi_slave_driver
 *
 * The public API for using the RTOS SPI slave driver.
 * @{
 */

#include <xcore/channel_streaming.h>
#include "spi.h"

#include "rtos/osal/api/rtos_osal.h"
#include "rtos/drivers/rpc/api/rtos_driver_rpc.h"

///**
// * The maximum number of bytes that a the RTOS SPI slave driver can receive from a master
// * in a single write transaction.
// */
//#ifndef RTOS_SPI_SLAVE_BUF_LEN
//#define RTOS_SPI_SLAVE_BUF_LEN 256
//#endif

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
 * These callback functions are optionally called by an SPI slave driver's thread when it is first
 * started. This gives the application a chance to perform startup initialization from within the
 * driver's thread.
 *
 * \param ctx           A pointer to the associated SPI slave driver instance.
 * \param app_data      A pointer to application specific data provided
 *                      by the application. Used to share data between
 *                      this callback function and the application.
 */
typedef void (*rtos_spi_slave_start_cb_t)(rtos_spi_slave_t *ctx, void *app_data);

/**
 * Function pointer type for application provided RTOS SPI slave transmit done callback functions.
 *
 * These callback functions are optionally called when an SPI slave driver instance is done transmitting data to
 * a master device. A buffer to the data sent and the actual number of bytes sent are provided to the callback.
 *
 * The application may want to use this, for example, if the buffer that was sent was malloc'd. This callback
 * can be used to free the buffer.
 *
 * \param ctx           A pointer to the associated SPI slave driver instance.
 * \param app_data      A pointer to application specific data provided
 *                      by the application. Used to share data between
 *                      this callback function and the application.
 * \param data          A pointer to the data transmitted to the master.
 * \param len           The number of bytes transmitted to the master from \p data.
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



void spi_slave_xfer_prepare(rtos_spi_slave_t *ctx, void *rx_buf, size_t rx_buf_len, void *tx_buf, size_t tx_buf_len);
int spi_slave_xfer_complete(rtos_spi_slave_t *ctx, void **rx_buf, size_t *rx_len, void **tx_buf, size_t *tx_len, unsigned timeout);

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
 * \param rx                The callback function to receive data from the bus master.
 * \param tx_start          The callback function to transmit data to the bus master.
 * \param tx_done           The callback function that is notified when transmits are
 *                          complete. This is optional and may be NULL.
 * \param interrupt_core_id The ID of the core on which to enable the SPI interrupt.
 * \param priority          The priority of the task that gets created by the driver to
 *                          call the callback functions.
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
 * \param p_scl         The port containing SCL. This must be a 1-bit port and
 *                      different than \p p_sda.
 * \param p_sda         The port containing SDA. This must be a 1-bit port and
 *                      different than \p p_scl.
 * \param device_addr   The 7-bit address of the slave device.
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
