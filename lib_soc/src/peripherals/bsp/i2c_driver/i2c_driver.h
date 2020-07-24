// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved

#ifndef I2C_DRIVER_H_
#define I2C_DRIVER_H_

#include "soc.h"
#include "i2c_dev_ctrl.h"

typedef enum {
  I2C_NACK,    ///< the slave has NACKed the last byte
  I2C_ACK,     ///< the slave has ACKed the last byte
} i2c_res_t;

typedef enum {
  I2C_REGOP_SUCCESS,     ///< the operation was successful
  I2C_REGOP_DEVICE_NACK, ///< the operation was NACKed when sending the device address, so either the device is missing or busy
  I2C_REGOP_INCOMPLETE   ///< the operation was NACKed halfway through by the slave
} i2c_regop_res_t;

/**
 * Write the i2c device
 *
 * \param[in]     device_id      ID of i2c device to use
 * \param[in]     device_addr    Address of device to write to
 * \param[in]     buf    		 Buffer to write
 * \param[in]     n    		     Number of bytes to send
 * \param[in/out] num_bytes_sent Pointer to number of bytes sent
 * \param[in]     send_stop_bit  Send stop bit
 *
 * \returns       I2C_ACK on success
 * 				  I2C_NACK otherwise
 */
i2c_res_t i2c_driver_write(
        soc_peripheral_t dev,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        size_t *num_bytes_sent,
        int send_stop_bit);

/**
 * Read from the i2c device
 *
 * \param[in]     device_id      ID of i2c device to use
 * \param[in]     device_addr    Address of device to read from
 * \param[in]     buf    		 Buffer to read into
 * \param[in]     n    		     Number of bytes to read
 * \param[in]     send_stop_bit  Send stop bit
 *
 * \returns       I2C_ACK on success
 * 				  I2C_NACK otherwise
 */
i2c_res_t i2c_driver_read(
        soc_peripheral_t dev,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        int send_stop_bit);

/**
 * Write to i2c device register
 *
 * \param[in]     device_id      ID of i2c device to use
 * \param[in]     device_addr    Address of device to write to
 * \param[in]     reg    		 Register to write to
 * \param[in]     data    		 Data to be written
 *
 * \returns       I2C_REGOP_SUCCESS on success
 * 				  I2C_REGOP_DEVICE_NACK when device is busy or missing
 * 				  I2C_REGOP_INCOMPLETE when operation was NACKed part way through by the slave
 */
i2c_regop_res_t i2c_driver_write_reg(
        soc_peripheral_t dev,
        uint8_t device_addr,
        uint8_t reg,
        uint8_t data);

/**
 * Read from i2c device register
 *
 * \param[in]     device_id      ID of i2c device to use
 * \param[in]     device_addr    Address of device to write to
 * \param[in]     reg    		 Register to write to
 * \param[in/out] result    	 Pointer to the operation result code
 *
 * \returns       Byte read
 */
uint8_t i2c_driver_read_reg(
        soc_peripheral_t dev,
        uint8_t device_addr,
        uint8_t reg,
        i2c_regop_res_t *result);

/**
 * Initialize the i2c device
 *
 * \param[in]     device_id      ID of i2c device to use
 *
 * \returns       Initialized i2c device
 */
soc_peripheral_t i2c_driver_init(
        int device_id);

#endif /* I2C_DRIVER_H_ */
