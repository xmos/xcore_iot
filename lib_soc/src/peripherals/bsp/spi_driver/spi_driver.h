// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef SPI_DRIVER_H_
#define SPI_DRIVER_H_

#include <stdint.h>
#include "soc.h"
#include "spi_dev_ctrl.h"

typedef enum spi_mode_t {
  SPI_MODE_0, /**< SPI Mode 0 - Polarity = 0, Clock Edge = 1 */
  SPI_MODE_1, /**< SPI Mode 1 - Polarity = 0, Clock Edge = 0 */
  SPI_MODE_2, /**< SPI Mode 2 - Polarity = 1, Clock Edge = 0 */
  SPI_MODE_3, /**< SPI Mode 3 - Polarity = 1, Clock Edge = 1 */
} spi_mode_t;

soc_peripheral_t spi_driver_init(
        int device_id,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr);

void spi_ISR(soc_peripheral_t device);

/* Setup speed and mode */
void spi_driver_setup(
        soc_peripheral_t dev,
        unsigned speed_in_khz,
        spi_mode_t mode);

/* Send data to DMA for SPI device */
void spi_driver_transmit(
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

#endif /* SPI_DRIVER_H_ */
