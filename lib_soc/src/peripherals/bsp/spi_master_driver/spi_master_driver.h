// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef SPI_MASTER_DRIVER_H_
#define SPI_MASTER_DRIVER_H_

#include <stdint.h>
#include "soc.h"
#include "spi_master_dev_ctrl.h"


/* Initialize driver*/
soc_peripheral_t spi_master_driver_init(
        int device_id,
        int isr_core);

/* SPI master isr */
void spi_master_isr(soc_peripheral_t device);

/* Initialize device */
void spi_master_device_init(
        soc_peripheral_t dev,
        unsigned cs_port_bit,
        unsigned cpol,
        unsigned cpha,
        unsigned clock_divide,
        unsigned cs_to_data_delay_ns,
        unsigned byte_setup_ns);

/** Perform a SPI transaction
 *
 *  \param dev             the SPI device
 *  \param rx_buf          the buffer to receive response in.  The
 *                         buffer must be large enough to hold the
 *                         specified rx_len.  Parameter should be
 *                         NULL if no response is requested.
 *  \param rx_len          the number of bytes to recieve.  Value is
 *                         ignored if rx_buf is NULL.
 *  \param tx_buf          the buffer to transmit.  Parameter can
 *                         be NULL, to only receive
 *  \param tx_len          the number of bytes to send.  Value is
 *                         ignored if tx_buf is NULL.
 *
 *  \returns               none
 */
void spi_transaction(
        soc_peripheral_t dev,
        uint8_t* rx_buf,
        size_t rx_len,
        uint8_t* tx_buf,
        size_t tx_len);

#endif /* SPI_MASTER_DRIVER_H_ */
