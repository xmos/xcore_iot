// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_I2C_MASTER_H_
#define RTOS_I2C_MASTER_H_

/**
 * \addtogroup rtos_i2c_master_driver rtos_i2c_master_driver
 *
 * The public API for using the RTOS I2C master driver.
 * @{
 */

#include "i2c.h"

#include "rtos_osal.h"
#include "rtos_driver_rpc.h"

/**
 * Typedef to the RTOS I2C master driver instance struct.
 */
typedef struct rtos_i2c_master_struct rtos_i2c_master_t;

/**
 * Struct representing an RTOS I2C master driver instance.
 *
 * The members in this struct should not be accessed directly.
 */
struct rtos_i2c_master_struct {
    rtos_driver_rpc_t *rpc_config;

    __attribute__((fptrgroup("rtos_i2c_master_write_fptr_grp")))
    i2c_res_t (*write)(rtos_i2c_master_t *, uint8_t, uint8_t buf[], size_t, size_t *, int);

    __attribute__((fptrgroup("rtos_i2c_master_read_fptr_grp")))
    i2c_res_t (*read)(rtos_i2c_master_t *, uint8_t, uint8_t buf[], size_t, int);

    __attribute__((fptrgroup("rtos_i2c_master_stop_bit_send_fptr_grp")))
    void (*stop_bit_send)(rtos_i2c_master_t *);

    __attribute__((fptrgroup("rtos_i2c_master_reg_write_fptr_grp")))
    i2c_regop_res_t (*reg_write)(rtos_i2c_master_t *, uint8_t, uint8_t, uint8_t);

    __attribute__((fptrgroup("rtos_i2c_master_reg_read_fptr_grp")))
    i2c_regop_res_t (*reg_read)(rtos_i2c_master_t *, uint8_t, uint8_t, uint8_t *);

    i2c_master_t ctx;

    rtos_osal_mutex_t lock;
};

#include "rtos_i2c_master_rpc.h"

/**
 * \addtogroup rtos_i2c_master_driver_core rtos_i2c_master_driver_core
 *
 * The core functions for using an RTOS I2C master driver instance after
 * it has been initialized and started. These functions may be used
 * by both the host and any client tiles that RPC has been enabled for.
 * @{
 */

/**
 * Writes data to an I2C bus as a master.
 *
 * \param ctx             A pointer to the I2C master driver instance to use.
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
 * \retval               ``I2C_ACK`` if the write was acknowledged by the device.
 * \retval               ``I2C_NACK``otherwise.
 */
inline i2c_res_t rtos_i2c_master_write(
        rtos_i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        size_t *num_bytes_sent,
        int send_stop_bit)
{
    return ctx->write(ctx, device_addr, buf, n, num_bytes_sent, send_stop_bit);
}

/**
 * Reads data from an I2C bus as a master.
 *
 * \param ctx             A pointer to the I2C master driver instance to use.
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
 * \retval               ``I2C_ACK`` if the read was acknowledged by the device.
 * \retval               ``I2C_NACK``otherwise.
 */
inline i2c_res_t rtos_i2c_master_read(
        rtos_i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        int send_stop_bit)
{
    return ctx->read(ctx, device_addr, buf, n, send_stop_bit);
}


/**
 * Send a stop bit to an I2C bus as a master.
 *
 * This function will cause a stop bit to be sent on the bus. It should
 * be used to complete/abort a transaction if the ``send_stop_bit`` argument
 * was not set when calling the rtos_i2c_master_read() or rtos_i2c_master_write()
 * functions.
 *
 * \param ctx             A pointer to the I2C master driver instance to use.
 */
inline void rtos_i2c_master_stop_bit_send(
        rtos_i2c_master_t *ctx)
{
    ctx->stop_bit_send(ctx);
}

/**
 * Write to an 8-bit register on an I2C device.
 *
 * This function writes to an 8-bit addressed, 8-bit register in
 * an I2C device. The function writes the data by sending the
 * register address followed by the register data to the device
 * at the specified device address.
 *
 * \param ctx          A pointer to the I2C master driver instance to use.
 * \param device_addr  The address of the device to write to.
 * \param reg_addr     The address of the register to write to.
 * \param data         The 8-bit value to write.
 *
 * \retval             ``I2C_REGOP_DEVICE_NACK`` if the address is NACKed.
 * \retval             ``I2C_REGOP_INCOMPLETE`` if not all data was ACKed.
 * \retval             ``I2C_REGOP_SUCCESS`` on successful completion of the write.
 */
inline i2c_regop_res_t  rtos_i2c_master_reg_write(
        rtos_i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t reg_addr,
        uint8_t data)
{
    return ctx->reg_write(ctx, device_addr, reg_addr, data);
}

/**
 * Reads from an 8-bit register on an I2C device.
 *
 * This function reads from an 8-bit addressed, 8-bit register in
 * an I2C device. The function reads the data by sending the
 * register address followed reading the register data from the
 * device at the specified device address.
 *
 * Note that no stop bit is transmitted between the write and the read.
 * The operation is performed as one transaction using a repeated start.
 *
 * \param ctx          A pointer to the I2C master driver instance to use.
 * \param device_addr  The address of the device to read from.
 * \param reg_addr     The address of the register to read from.
 * \param data         A pointer to the byte to fill with data read from the register.
 *
 * \retval             ``I2C_REGOP_DEVICE_NACK`` if the device NACKed.
 * \retval             ``I2C_REGOP_SUCCESS`` on successful completion of the read.
 */
inline i2c_regop_res_t  rtos_i2c_master_reg_read(
        rtos_i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t reg_addr,
        uint8_t *data)
{
    return ctx->reg_read(ctx, device_addr, reg_addr, data);
}

/**@}*/

/**
 * Starts an RTOS I2C master driver instance. This must only be called by the tile that
 * owns the driver instance. It may be called either before or after starting
 * the RTOS, but must be called before any of the core I2C master driver functions are
 * called with this instance.
 *
 * rtos_i2c_master_init() must be called on this I2C master driver instance prior to calling this.
 *
 * \param i2c_master_ctx A pointer to the I2C master driver instance to start.
 */
void rtos_i2c_master_start(
        rtos_i2c_master_t *i2c_master_ctx);

/**
 * Initializes an RTOS I2C master driver instance.
 * This must only be called by the tile that owns the driver instance. It may be
 * called either before or after starting the RTOS, but must be called before calling
 * rtos_i2c_master_start() or any of the core I2C master driver functions with this instance.
 *
 * \param i2c_master_ctx      A pointer to the I2C master driver instance to initialize.
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
 * \param tmr                 This is unused and should be set to 0. This will be removed.
 * \param kbits_per_second    The speed of the I2C bus. The maximum value allowed is 400.
 */
void rtos_i2c_master_init(
        rtos_i2c_master_t *i2c_master_ctx,
        const port_t p_scl,
        const uint32_t scl_bit_position,
        const uint32_t scl_other_bits_mask,
        const port_t p_sda,
        const uint32_t sda_bit_position,
        const uint32_t sda_other_bits_mask,
        hwtimer_t tmr,
        const unsigned kbits_per_second);

/**@}*/

#endif /* RTOS_I2C_MASTER_H_ */
