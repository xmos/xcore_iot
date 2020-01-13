// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef SPI_MASTER_DRIVER_H_
#define SPI_MASTER_DRIVER_H_

#include <stdint.h>
#include "soc.h"
#include "spi_master_dev_ctrl.h"

soc_peripheral_t spi_master_driver_init(
        int device_id,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr);

void spi_master_ISR(soc_peripheral_t device);

/* Initialize device */
void spi_master_device_init(
        soc_peripheral_t dev,
        unsigned cs_port_bit,
        unsigned cpol,
        unsigned cpha,
        unsigned clock_divide,
        unsigned cs_to_data_delay_ns,
        unsigned byte_setup_ns);

/* Send data to DMA for SPI device
 * This function only transmits, and ignores response */
void spi_transmit(
        soc_peripheral_t dev,
        uint8_t* tx_buf,
        size_t len);

/* Create request buffer DMA, IRQ is fired when response is received */
void spi_request(
        soc_peripheral_t dev,
        uint8_t* rx_buf,
        size_t len);

/* Send data, block until transaction is complete */
void spi_transmit_blocking(
        soc_peripheral_t dev,
        uint8_t* tx_buf,
        size_t len);

/* Receive buffer, block until transaction is complete
 * no interrupt on receive */
void spi_request_blocking(
        soc_peripheral_t dev,
        uint8_t* rx_buf,
        size_t len);

/* Send data to DMA for SPI device and fire IRQ when response
 * is received */
void spi_transaction(
        soc_peripheral_t dev,
        uint8_t* rx_buf,
        uint8_t* tx_buf,
        size_t len);

/* Send data and receive data, block until transaction is complete */
void spi_transaction_blocking(
        soc_peripheral_t dev,
        uint8_t* rx_buf,
        uint8_t* tx_buf,
        size_t len);

#endif /* SPI_MASTER_DRIVER_H_ */
