// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_I2C_SLAVE_H_
#define RTOS_I2C_SLAVE_H_

/**
 * \addtogroup rtos_i2c_slave_driver rtos_i2c_slave_driver
 *
 * The public API for using the RTOS I2C slave driver.
 * @{
 */

#include <xcore/channel_streaming.h>
#include "i2c.h"

#include "rtos_osal.h"
#include "rtos_driver_rpc.h"

/**
 * The maximum number of bytes that a the RTOS I2C slave driver can receive from a master
 * in a single write transaction.
 */
#ifndef RTOS_I2C_SLAVE_BUF_LEN
#define RTOS_I2C_SLAVE_BUF_LEN 256
#endif

/**
 * This attribute must be specified on all RTOS I2C slave callback functions
 * provided by the application.
 */
#define RTOS_I2C_SLAVE_CALLBACK_ATTR __attribute__((fptrgroup("rtos_i2c_slave_cb_fptr_grp")))

/**
 * Typedef to the RTOS I2C slave driver instance struct.
 */
typedef struct rtos_i2c_slave_struct rtos_i2c_slave_t;

/**
 * Function pointer type for application provided RTOS I2C slave start callback functions.
 *
 * These callback functions are optionally called by an I2C slave driver's thread when it is first
 * started. This gives the application a chance to perform startup initialization from within the
 * driver's thread.
 *
 * \param ctx           A pointer to the associated I2C slave driver instance.
 * \param app_data      A pointer to application specific data provided
 *                      by the application. Used to share data between
 *                      this callback function and the application.
 */
typedef void (*rtos_i2c_slave_start_cb_t)(rtos_i2c_slave_t *ctx, void *app_data);

/**
 * Function pointer type for application provided RTOS I2C slave receive callback functions.
 *
 * These callback functions are called when an I2C slave driver instance has received data from
 * a master device.
 *
 * \param ctx           A pointer to the associated I2C slave driver instance.
 * \param app_data      A pointer to application specific data provided
 *                      by the application. Used to share data between
 *                      this callback function and the application.
 * \param data          A pointer to the data received from the master.
 * \param len           The number of valid bytes in \p data.
 */
typedef void (*rtos_i2c_slave_rx_cb_t)(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len);

/**
 * Function pointer type for application provided RTOS I2C slave transmit start callback functions.
 *
 * These callback functions are called when an I2C slave driver instance needs to transmit data to
 * a master device. This callback must provide the data to transmit and the length.
 *
 * \param ctx           A pointer to the associated I2C slave driver instance.
 * \param app_data      A pointer to application specific data provided
 *                      by the application. Used to share data between
 *                      this callback function and the application.
 * \param data          A pointer to the data buffer to transmit to the master. The driver sets this
 *                      to its internal data buffer, which has a size of RTOS_I2C_SLAVE_BUF_LEN, prior
 *                      to calling this callback. This may be set to a different buffer by the callback.
 *                      The callback must fill this buffer with the data to send to the master.
 *
 * \return              The number of bytes to transmit to the master from \p data. If the master
 *                      reads more bytes than this, the driver will wrap around to the start of the
 *                      buffer and send it again.
 */
typedef size_t (*rtos_i2c_slave_tx_start_cb_t)(rtos_i2c_slave_t *ctx, void *app_data, uint8_t **data);

/**
 * Function pointer type for application provided RTOS I2C slave transmit done callback functions.
 *
 * These callback functions are optionally called when an I2C slave driver instance is done transmitting data to
 * a master device. A buffer to the data sent and the actual number of bytes sent are provided to the callback.
 *
 * The application may want to use this, for example, if the buffer that was sent was malloc'd. This callback
 * can be used to free the buffer.
 *
 * \param ctx           A pointer to the associated I2C slave driver instance.
 * \param app_data      A pointer to application specific data provided
 *                      by the application. Used to share data between
 *                      this callback function and the application.
 * \param data          A pointer to the data transmitted to the master.
 * \param len           The number of bytes transmitted to the master from \p data.
 */
typedef void (*rtos_i2c_slave_tx_done_cb_t)(rtos_i2c_slave_t *ctx, void *app_data, uint8_t *data, size_t len);

/**
 * Struct representing an RTOS I2C slave driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
struct rtos_i2c_slave_struct {
    port_t p_scl;
    port_t p_sda;
    uint8_t device_addr;

    void *app_data;
    uint8_t data_buf[RTOS_I2C_SLAVE_BUF_LEN];
    size_t rx_data_i;

    uint8_t *tx_data;
    size_t tx_data_len;
    size_t tx_data_i;
    size_t tx_data_sent;

    int waiting_for_complete_cb;

    RTOS_I2C_SLAVE_CALLBACK_ATTR rtos_i2c_slave_start_cb_t start;
    RTOS_I2C_SLAVE_CALLBACK_ATTR rtos_i2c_slave_rx_cb_t rx;
    RTOS_I2C_SLAVE_CALLBACK_ATTR rtos_i2c_slave_tx_start_cb_t tx_start;
    RTOS_I2C_SLAVE_CALLBACK_ATTR rtos_i2c_slave_tx_done_cb_t tx_done;

    streaming_channel_t c;
    rtos_osal_event_group_t events;
    rtos_osal_thread_t hil_thread;
    rtos_osal_thread_t app_thread;
};

/**
 * Starts an RTOS I2C slave driver instance. This must only be called by the tile that
 * owns the driver instance. It must be called after starting the RTOS from an RTOS thread.
 *
 * rtos_i2c_slave_init() must be called on this I2C slave driver instance prior to calling this.
 *
 * \param i2c_slave_ctx     A pointer to the I2C slave driver instance to start.
 * \param app_data          A pointer to application specific data to pass to
 *                          the callback functions.
 * \param start             The callback function that is called when the driver's
 *                          thread starts. This is optional and may be NULL.
 * \param rx                The callback function to receive data from the bus master.
 * \param tx_start          The callback function to transmit data to the bus master.
 * \param tx_done           The callback function that is notified when transmits are
 *                          complete. This is optional and may be NULL.
 * \param interrupt_core_id The ID of the core on which to enable the I2C interrupt.
 * \param priority          The priority of the task that gets created by the driver to
 *                          call the callback functions.
 */
void rtos_i2c_slave_start(
        rtos_i2c_slave_t *i2c_slave_ctx,
        void *app_data,
        rtos_i2c_slave_start_cb_t start,
        rtos_i2c_slave_rx_cb_t rx,
        rtos_i2c_slave_tx_start_cb_t tx_start,
        rtos_i2c_slave_tx_done_cb_t tx_done,
        unsigned interrupt_core_id,
        unsigned priority);

/**
 * Initializes an RTOS I2C slave driver instance.
 * This must only be called by the tile that owns the driver instance. It should be
 * called before starting the RTOS, and must be called before calling rtos_i2c_slave_start().
 *
 * \param i2c_slave_ctx A pointer to the I2C slave driver instance to initialize.
 * \param io_core_mask  A bitmask representing the cores on which the low level I2C I/O thread
 *                      created by the driver is allowed to run. Bit 0 is core 0, bit 1 is core 1,
 *                      etc.
 * \param p_scl         The port containing SCL. This must be a 1-bit port and
 *                      different than \p p_sda.
 * \param p_sda         The port containing SDA. This must be a 1-bit port and
 *                      different than \p p_scl.
 * \param device_addr   The 7-bit address of the slave device.
 */
void rtos_i2c_slave_init(
        rtos_i2c_slave_t *i2c_slave_ctx,
        uint32_t io_core_mask,
        const port_t p_scl,
        const port_t p_sda,
        uint8_t device_addr);

/**@}*/

#endif /* RTOS_I2C_SLAVE_H_ */
