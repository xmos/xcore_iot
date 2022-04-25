// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef _i2c_c_reg_h_
#define _i2c_c_reg_h_

#include <string.h>
#include "i2c.h"

/**
 * \addtogroup hil_i2c_register hil_i2c_register
 *
 * The public API for using the RTOS I2C slave driver.
 * @{
 */

/**
 * This type is used by the supplementary I2C register read/write functions to
 * report back on whether the operation was a success or not.
 */
typedef enum {
  I2C_REGOP_SUCCESS,     /**< The operation was successful. */
  I2C_REGOP_DEVICE_NACK, /**< The operation was NACKed when sending the device address, so either the device is missing or busy. */
  I2C_REGOP_INCOMPLETE,  /**< The operation was NACKed halfway through by the slave. */
} i2c_regop_res_t;

/**
 * Read an 8-bit register on a slave device.
 *
 * This function reads from an 8-bit addressed, 8-bit register in
 * an I2C device. The function reads the data by sending the
 * register address followed reading the register data from the
 * device at the specified device address.
 *
 * \note No stop bit is transmitted between the write and the read.
 * The operation is performed as one transaction using a repeated start.
 *
 * \param ctx         A pointer to the I2C master context to use.
 * \param device_addr The address of the device to read from.
 * \param reg         The address of the register to read from.
 * \param result      Indicates whether the read completed successfully. Will
 *                    be set to #I2C_REGOP_DEVICE_NACK if the slave NACKed,
 *                    and #I2C_REGOP_SUCCESS on successful completion of the
 *                    read.
 *
 * \returns           The value of the register.
 */
inline uint8_t read_reg(
        i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t reg,
        i2c_regop_res_t *result)
{
    uint8_t buf[1] = {reg};
    size_t bytes_sent = 0;
    i2c_res_t res;

    res = i2c_master_write(ctx, device_addr, buf, 1, &bytes_sent, 0);
    if (bytes_sent != 1) {
        *result = I2C_REGOP_DEVICE_NACK;
        i2c_master_stop_bit_send(ctx);
        return 0;
    }
    memset(buf, 0x00, 1);
    res = i2c_master_read(ctx, device_addr, buf, 1, 1);
    if (res == I2C_NACK) {
        *result = I2C_REGOP_DEVICE_NACK;
    } else {
        *result = I2C_REGOP_SUCCESS;
    }
    return buf[0];
}

/**
 * Read an 8-bit register on a slave device.
 *
 * This function reads from an 16-bit addressed, 8-bit register in
 * an I2C device. The function reads the data by sending the
 * register address followed reading the register data from the
 * device at the specified device address.
 *
 * \note No stop bit is transmitted between the write and the read.
 * The operation is performed as one transaction using a repeated start.
 *
 * \param ctx         A pointer to the I2C master context to use.
 * \param device_addr The address of the device to read from.
 * \param reg         The address of the register to read from.
 * \param result      Indicates whether the read completed successfully. Will
 *                    be set to #I2C_REGOP_DEVICE_NACK if the slave NACKed,
 *                    and #I2C_REGOP_SUCCESS on successful completion of the
 *                    read.
 *
 * \returns           The value of the register.
 */
inline uint8_t read_reg8_addr16(
        i2c_master_t *ctx,
        uint8_t device_addr,
        uint16_t reg,
        i2c_regop_res_t *result)
{
    uint8_t buf[2] = {(uint8_t)((reg >> 8) & 0xFF), (uint8_t)(reg & 0xFF)};
    size_t bytes_sent = 0;
    i2c_res_t res;

    res = i2c_master_write(ctx, device_addr, buf, 2, &bytes_sent, 0);
    if (bytes_sent != 2) {
        *result = I2C_REGOP_DEVICE_NACK;
        i2c_master_stop_bit_send(ctx);
        return 0;
    }
    memset(buf, 0x00, 2);
    res = i2c_master_read(ctx, device_addr, buf, 1, 1);
    if (res == I2C_NACK) {
        *result = I2C_REGOP_DEVICE_NACK;
    } else {
        *result = I2C_REGOP_SUCCESS;
    }
    return buf[0];
}

/**
 * Read an 16-bit register on a slave device.
 *
 * This function reads from an 8-bit addressed, 16-bit register in
 * an I2C device. The function reads the data by sending the
 * register address followed reading the register data from the
 * device at the specified device address.
 *
 * \note No stop bit is transmitted between the write and the read.
 * The operation is performed as one transaction using a repeated start.
 *
 * \param ctx         A pointer to the I2C master context to use.
 * \param device_addr The address of the device to read from.
 * \param reg         The address of the register to read from.
 * \param result      Indicates whether the read completed successfully. Will
 *                    be set to #I2C_REGOP_DEVICE_NACK if the slave NACKed,
 *                    and #I2C_REGOP_SUCCESS on successful completion of the
 *                    read.
 *
 * \returns           The value of the register.
 */
inline uint16_t read_reg16_addr8(
        i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t reg,
        i2c_regop_res_t *result)
{
    uint8_t buf[2] = {reg, 0x00};
    size_t bytes_sent = 0;
    i2c_res_t res;

    res = i2c_master_write(ctx, device_addr, buf, 1, &bytes_sent, 0);
    if (bytes_sent != 1) {
        *result = I2C_REGOP_DEVICE_NACK;
        i2c_master_stop_bit_send(ctx);
        return 0;
    }
    memset(buf, 0x00, 2);
    res = i2c_master_read(ctx, device_addr, buf, 2, 1);
    if (res == I2C_NACK) {
        *result = I2C_REGOP_DEVICE_NACK;
    } else {
        *result = I2C_REGOP_SUCCESS;
    }
    return (uint16_t)((buf[0] << 8 )| buf[1]);
}

/**
 * Read an 16-bit register on a slave device.
 *
 * This function reads from an 16-bit addressed, 16-bit register in
 * an I2C device. The function reads the data by sending the
 * register address followed reading the register data from the
 * device at the specified device address.
 *
 * \note No stop bit is transmitted between the write and the read.
 * The operation is performed as one transaction using a repeated start.
 *
 * \param ctx         A pointer to the I2C master context to use.
 * \param device_addr The address of the device to read from.
 * \param reg         The address of the register to read from.
 * \param result      Indicates whether the read completed successfully. Will
 *                    be set to #I2C_REGOP_DEVICE_NACK if the slave NACKed,
 *                    and #I2C_REGOP_SUCCESS on successful completion of the
 *                    read.
 *
 * \returns           The value of the register.
 */
inline uint16_t read_reg16(
        i2c_master_t *ctx,
        uint8_t device_addr,
        uint16_t reg,
        i2c_regop_res_t *result)
{
    uint8_t buf[2] = {(uint8_t)((reg >> 8) & 0xFF), (uint8_t)(reg & 0xFF)};
    size_t bytes_sent = 0;
    i2c_res_t res;

    res = i2c_master_write(ctx, device_addr, buf, 2, &bytes_sent, 0);
    if (bytes_sent != 2) {
        *result = I2C_REGOP_DEVICE_NACK;
        i2c_master_stop_bit_send(ctx);
        return 0;
    }
    memset(buf, 0x00, 2);
    res = i2c_master_read(ctx, device_addr, buf, 2, 1);
    if (res == I2C_NACK) {
        *result = I2C_REGOP_DEVICE_NACK;
    } else {
        *result = I2C_REGOP_SUCCESS;
    }
    return (uint16_t)((buf[0] << 8 )| buf[1]);
}

/**
 * Write to an 8-bit register on an I2C device.
 *
 * This function writes to an 8-bit addressed, 8-bit register in
 * an I2C device. The function writes the data by sending the
 * register address followed by the register data to the device
 * at the specified device address.
 *
 * \param ctx          A pointer to the I2C master context to use.
 * \param device_addr  The address of the device to write to.
 * \param reg          The address of the register to write to.
 * \param data         The 8-bit value to write.
 *
 * \returns            #I2C_REGOP_DEVICE_NACK if the address is NACKed.
 * \returns            #I2C_REGOP_INCOMPLETE if not all data was ACKed.
 * \returns            #I2C_REGOP_SUCCESS on successful completion of the write.
 */
inline i2c_regop_res_t write_reg(
        i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t reg,
        uint8_t data)
{
    uint8_t buf[2] = {reg, data};
    size_t bytes_sent = 0;
    i2c_regop_res_t reg_res;

    i2c_master_write(ctx, device_addr, buf, 2, &bytes_sent, 1);
    if (bytes_sent == 0) {
        reg_res = I2C_REGOP_DEVICE_NACK;
    } else if (bytes_sent < 2) {
        reg_res = I2C_REGOP_INCOMPLETE;
    } else {
        reg_res = I2C_REGOP_SUCCESS;
    }
    return reg_res;
}

/**
 * Write to an 8-bit register on an I2C device.
 *
 * This function writes to a 16-bit addressed, 8-bit register in
 * an I2C device. The function writes the data by sending the
 * register address followed by the register data to the device
 * at the specified device address.
 *
 * \param ctx          A pointer to the I2C master context to use.
 * \param device_addr  The address of the device to write to.
 * \param reg          The address of the register to write to.
 * \param data         The 8-bit value to write.
 *
 * \returns            #I2C_REGOP_DEVICE_NACK if the address is NACKed.
 * \returns            #I2C_REGOP_INCOMPLETE if not all data was ACKed.
 * \returns            #I2C_REGOP_SUCCESS on successful completion of the write.
*/
inline i2c_regop_res_t write_reg8_addr16(
        i2c_master_t *ctx,
        uint8_t device_addr,
        uint16_t reg,
        uint8_t data)
{
    uint8_t buf[3] = {(uint8_t)((reg >> 8) & 0xFF), (uint8_t)(reg & 0xFF), (uint8_t)(data)};
    size_t bytes_sent = 0;
    i2c_regop_res_t reg_res;

    i2c_master_write(ctx, device_addr, buf, 3, &bytes_sent, 1);
    if (bytes_sent == 0) {
        reg_res = I2C_REGOP_DEVICE_NACK;
    } else if (bytes_sent < 3) {
        reg_res = I2C_REGOP_INCOMPLETE;
    } else {
        reg_res = I2C_REGOP_SUCCESS;
    }
    return reg_res;
}

/**
 * Write to a 16-bit register on an I2C device.
 *
 * This function writes to an 8-bit addressed, 16-bit register in
 * an I2C device. The function writes the data by sending the
 * register address followed by the register data to the device
 * at the specified device address.
 *
 * \param ctx          A pointer to the I2C master context to use.
 * \param device_addr  The address of the device to write to.
 * \param reg          The address of the register to write to.
 * \param data         The 16-bit value to write.
 *
 * \returns            #I2C_REGOP_DEVICE_NACK if the address is NACKed.
 * \returns            #I2C_REGOP_INCOMPLETE if not all data was ACKed.
 * \returns            #I2C_REGOP_SUCCESS on successful completion of the write.
 */
inline i2c_regop_res_t write_reg16_addr8(
        i2c_master_t *ctx,
        uint8_t device_addr,
        uint8_t reg,
        uint16_t data)
{
    uint8_t buf[3] = {(uint8_t)(reg), (uint8_t)((data >> 8) & 0xFF), (uint8_t)(data & 0xFF)};
    size_t bytes_sent = 0;
    i2c_regop_res_t reg_res;

    i2c_master_write(ctx, device_addr, buf, 3, &bytes_sent, 1);
    if (bytes_sent == 0) {
        reg_res = I2C_REGOP_DEVICE_NACK;
    } else if (bytes_sent < 3) {
        reg_res = I2C_REGOP_INCOMPLETE;
    } else {
        reg_res = I2C_REGOP_SUCCESS;
    }
    return reg_res;
}

/**
 * Write to a 16-bit register on an I2C device.
 *
 * This function writes to a 16-bit addressed, 16-bit register in
 * an I2C device. The function writes the data by sending the
 * register address followed by the register data to the device
 * at the specified device address.
 *
 * \param ctx          A pointer to the I2C master context to use.
 * \param device_addr  The address of the device to write to.
 * \param reg          The address of the register to write to.
 * \param data         The 16-bit value to write.
 *
 * \returns            #I2C_REGOP_DEVICE_NACK if the address is NACKed.
 * \returns            #I2C_REGOP_INCOMPLETE if not all data was ACKed.
 * \returns            #I2C_REGOP_SUCCESS on successful completion of the write.
 */
inline i2c_regop_res_t write_reg16(
        i2c_master_t *ctx,
        uint8_t device_addr,
        uint16_t reg,
        uint16_t data)
{
    uint8_t buf[4] = {(uint8_t)((reg >> 8) & 0xFF), (uint8_t)(reg & 0xFF),
                      (uint8_t)((data >> 8) & 0xFF), (uint8_t)(data & 0xFF)};
    size_t bytes_sent = 0;
    i2c_regop_res_t reg_res;

    i2c_master_write(ctx, device_addr, buf, 4, &bytes_sent, 1);
    if (bytes_sent == 0) {
        reg_res = I2C_REGOP_DEVICE_NACK;
    } else if (bytes_sent < 4) {
        reg_res = I2C_REGOP_INCOMPLETE;
    } else {
        reg_res = I2C_REGOP_SUCCESS;
    }
    return reg_res;
}

/**@}*/ // END: addtogroup hil_i2c_register

#endif
