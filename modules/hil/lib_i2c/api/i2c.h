// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _i2c_c_h_
#define _i2c_c_h_

#include <sys/types.h>
#include <xcore/port.h>
#include <xcore/clock.h>
#include <xcore/hwtimer.h>

/**
 * \addtogroup hil_i2c_master hil_i2c_master
 *
 * The public API for using the RTOS I2C slave driver.
 * @{
 */

/**
 * Status codes for I2C master operations
 */
typedef enum {
  I2C_NACK,        /**< The slave has NACKed the last byte. */
  I2C_ACK,         /**< The slave has ACKed the last byte. */
  I2C_STARTED,     /**< The requested I2C transaction has started. */
  I2C_NOT_STARTED  /**< The requested I2C transaction could not start. */
} i2c_res_t;

/**
 * Type representing an I2C master context
 */
typedef struct i2c_master_struct i2c_master_t;

/**
 * Struct to hold an I2C master context.
 *
 * The members in this struct should not be accessed directly.
 */
struct i2c_master_struct {
    port_t p_scl;
    uint32_t scl_mask;
    port_t p_sda;
    uint32_t sda_mask;

    uint32_t scl_high;
    uint32_t sda_high;
    uint32_t scl_low;
    uint32_t sda_low;

    uint16_t bit_time;
    uint16_t p_setup_ticks;
    uint16_t sr_setup_ticks;
    uint16_t s_hold_ticks;
    uint16_t low_period_ticks;
    uint16_t high_period_ticks;

    int interrupt_state;
    int stopped;
};

/**
 * Writes data to an I2C bus as a master.
 *
 * \param ctx             A pointer to the I2C master context to use.
 * \param device_addr     The address of the device to write to.
 * \param buf             The buffer containing data to write.
 * \param n               The number of bytes to write.
 * \param num_bytes_sent  The function will set this value to the
 *                        number of bytes actually sent. On success, this
 *                        will be equal to ``n`` but it will be less if the
 *                        slave sends an early NACK on the bus and the
 *                        transaction fails.
 * \param send_stop_bit   If this is non-zero then a stop bit
 *                        will be sent on the bus after the transaction.
 *                        This is usually required for normal operation. If
 *                        this parameter is zero then no stop bit will
 *                        be omitted. In this case, no other task can use
 *                        the component until a stop bit has been sent.
 *
 * \returns               #I2C_ACK if the write was acknowledged by the device, #I2C_NACK otherwise.
 */
i2c_res_t i2c_master_write(
        i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        size_t *num_bytes_sent,
        int send_stop_bit);

/**
 * Reads data from an I2C bus as a master.
 *
 * \param ctx             A pointer to the I2C master context to use.
 * \param device_addr     The address of the device to read from.
 * \param buf             The buffer to fill with data.
 * \param n               The number of bytes to read.
 * \param send_stop_bit   If this is non-zero then a stop bit.
 *                        will be sent on the bus after the transaction.
 *                        This is usually required for normal operation. If
 *                        this parameter is zero then no stop bit will
 *                        be omitted. In this case, no other task can use
 *                        the component until a stop bit has been sent.
 *
 * \returns               #I2C_ACK if the read was acknowledged by the device, #I2C_NACK otherwise.
 */
i2c_res_t i2c_master_read(
        i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        int send_stop_bit);

/**
 * Send a stop bit to an I2C bus as a master.
 *
 * This function will cause a stop bit to be sent on the bus. It should
 * be used to complete/abort a transaction if the ``send_stop_bit`` argument
 * was not set when calling the i2c_master_read() or i2c_master_write()
 * functions.
 *
 * \param ctx             A pointer to the I2C master context to use.
 */
void i2c_master_stop_bit_send(
        i2c_master_t *ctx);


/**
 * Implements an I2C master device on one or two single or multi-bit ports.
 *
 * \param ctx                 A pointer to the I2C master context to initialize.
 * \param p_scl               The port containing SCL. This may be either the same as
 *                            or different than \p p_sda.
 * \param scl_bit_position    The bit number of the SCL line on the port \p p_scl.
 * \param scl_other_bits_mask A value that is ORed into the port value driven to \p p_scl
 *                            both when SCL is high and low. The bit representing SCL (as
 *                            well as SDA if they share the same port) must be set to 0.
 * \param p_sda               The port containing SDA. This may be either the same as
 *                            or different than \p p_scl.
 * \param sda_bit_position    The bit number of the SDA line on the port \p p_sda.
 * \param sda_other_bits_mask A value that is ORed into the port value driven to \p p_sda
 *                            both when SDA is high and low. The bit representing SDA (as
 *                            well as SCL if they share the same port) must be set to 0.
 * \param kbits_per_second    The speed of the I2C bus. The maximum value allowed is 400.
 */
void i2c_master_init(
        i2c_master_t *ctx,
        const port_t p_scl,
        const uint32_t scl_bit_position,
        const uint32_t scl_other_bits_mask,
        const port_t p_sda,
        const uint32_t sda_bit_position,
        const uint32_t sda_other_bits_mask,
        const unsigned kbits_per_second);


/**
 * Shuts down the I2C master device.
 *
 * \param ctx  A pointer to the I2C master context to shut down.
 *
 * This function disables the ports associated with the I2C master
 * and deallocates its timer if it was not provided by the application.
 *
 * If subsequent reads or writes need to be performed, then i2c_master_init()
 * must be called again first.
 */
void i2c_master_shutdown(i2c_master_t *ctx);

/**@}*/ // END: addtogroup hil_i2c_master

/**
 * \addtogroup hil_i2c_slave hil_i2c_slave
 *
 * The public API for using the HIL I2C slave.
 * @{
 */


/**
 * I2C Slave Response
 *
 *  This type is used to describe the I2C slave response.
 */
typedef enum i2c_slave_ack {
    I2C_SLAVE_ACK,  /**< ACK to accept request */
    I2C_SLAVE_NACK, /**< NACK to ignore request */
} i2c_slave_ack_t;

/**
 * The bus master has requested a read.
 *
 * This callback function is called if the bus master requests a
 * read from this slave device.
 *
 * At this point the slave can choose to accept the request (and
 * drive an ACK signal back to the master) or not (and drive a NACK
 * signal).
 *
 * \param app_data A pointer to application specific data provided
 *                 by the application. Used to share data between
 *                 the callback functions and the application.
 *
 * \returns        The callback must return either #I2C_SLAVE_ACK
 *                 or #I2C_SLAVE_NACK.
 */
typedef i2c_slave_ack_t (*ack_read_request_t)(void *app_data);

/**
 * The bus master has requested a write.
 *
 * This callback function is called if the bus master requests a
 * write from this slave device.
 *
 * At this point the slave can choose to accept the request (and
 * drive an ACK signal back to the master) or not (and drive a NACK
 * signal).
 *
 * \param app_data A pointer to application specific data provided
 *                 by the application. Used to share data between
 *                 the callback functions and the application.
 *
 * \returns        The callback must return either #I2C_SLAVE_ACK
 *                 or #I2C_SLAVE_NACK.
 */
typedef i2c_slave_ack_t (*ack_write_request_t)(void *app_data);

/**
 * The bus master requires data.
 *
 * This callback function is called when the bus master requires
 * data from this slave device.
 *
 * \param app_data A pointer to application specific data provided
 *                 by the application. Used to share data between
 *                 the callback functions and the application.
 *
 * \returns a byte of data to send to the master.
 */
typedef uint8_t (*master_requires_data_t)(void *app_data);

/**
 * The bus master has sent some data.
 *
 * This callback function is called when the bus master has transferred
 * a byte of data this slave device.
 *
 * \param app_data A pointer to application specific data provided
 *                 by the application. Used to share data between
 *                 the callback functions and the application.
 * \param data     The byte of data received from the bus master.
 *
 * \returns        The callback must return either #I2C_SLAVE_ACK
 *                 or #I2C_SLAVE_NACK.
 */
typedef i2c_slave_ack_t (*master_sent_data_t)(void *app_data, uint8_t data);

/**
 * The bus master has sent a stop bit.
 *
 * This callback function is called when a stop bit is sent by the
 * bus master.
 *
 * \param app_data A pointer to application specific data provided
 *                 by the application. Used to share data between
 *                 the callback functions and the application.
 */
typedef void (*stop_bit_t)(void *app_data);

/**
 * Shuts down the I2C slave device.
 *
 * This function can be used to stop the I2C slave task. It will disable
 * the SCL and SDA ports and then return.
 *
 * \param app_data A pointer to application specific data provided
 *                 by the application. Used to share data between
 *                 the callback functions and the application.
 *
 * \returns        - Non-zero if the I2C slave task should shut down.
 *                 - Zero if the I2C slave task should continue running.
 */
typedef int (*shutdown_t)(void *app_data);

/**
 * This attribute must be specified on all I2C callback functions
 * provided by the application.
 */
#define I2C_CALLBACK_ATTR __attribute__((fptrgroup("i2c_callback")))

/**
 * Callback group representing callback events that can occur during the
 * operation of the I2C slave task. Must be initialized by the application
 * prior to passing it to one of the I2C tasks.
 */
typedef struct {
    /** Pointer to the application's ack_read_request_t function to be called by the I2C device */
    I2C_CALLBACK_ATTR ack_read_request_t ack_read_request;

    /** Pointer to the application's ack_write_request_t function to be called by the I2C device */
    I2C_CALLBACK_ATTR ack_write_request_t ack_write_request;

    /** Pointer to the application's master_requires_data_t function to be called by the I2C device */
    I2C_CALLBACK_ATTR master_requires_data_t master_requires_data;

    /** Pointer to the application's master_sent_data_t function to be called by the I2C device */
    I2C_CALLBACK_ATTR master_sent_data_t master_sent_data;

    /** Pointer to the application's stop_bit_t function to be called by the I2C device */
    I2C_CALLBACK_ATTR stop_bit_t stop_bit;

    /** Pointer to the application's shutdown_t function to be called by the I2C device */
    I2C_CALLBACK_ATTR shutdown_t shutdown;

    /** Pointer to application specific data which is passed to each callback. */
    void *app_data;
} i2c_callback_group_t;

/**
 * I2C slave task.
 *
 * This function instantiates an I2C slave device.
 *
 * \param i2c_cbg     The I2C callback group pointing to the application's
 *                    functions to use for initialization and getting and
 *                    receiving frames. Also points to application specific
 *                    data which will be shared between the callbacks.
 * \param  p_scl      The SCL port of the I2C bus. This should be a 1 bit port. If not,
 *                    The SCL pin must be at bit 0 and the other bits unused.
 * \param  p_sda      The SDA port of the I2C bus. This should be a 1 bit port. If not,
 *                    The SDA pin must be at bit 0 and the other bits unused.
 * \param device_addr The address of the slave device.
 *
 */
void i2c_slave(const i2c_callback_group_t *const i2c_cbg,
               port_t p_scl,
               port_t p_sda,
               uint8_t device_addr);

/**@}*/ // END: addtogroup hil_i2c_slave

#include "i2c_reg.h"

#endif
