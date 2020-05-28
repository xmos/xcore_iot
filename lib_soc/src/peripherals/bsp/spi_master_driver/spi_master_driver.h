// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef SPI_MASTER_DRIVER_H_
#define SPI_MASTER_DRIVER_H_

#include "soc.h"

#include "FreeRTOS.h"

/* Attribute for the intertile_isr_callback_map member of the intertile isr.
 * Required by xcc to calculate stack usage.
 */
#define SPI_MASTER_ISR_CALLBACK_ATTR __attribute__((fptrgroup("spi_master_isr_callback")))

/* INTERTILE ISR callback function macros. For xcc this ensures they get added
 * to the intertile isr callback group so that stack usage for certain functions
 * can be calculated.
 */
#define SPI_MASTER_ISR_CALLBACK_FUNCTION_PROTO( xFunction, buf, len, buf_index, more, status, xYieldRequired ) void xFunction( uint8_t *buf, int len, int buf_index, int more, uint32_t status, BaseType_t *xYieldRequired )
#define SPI_MASTER_ISR_CALLBACK_FUNCTION( xFunction, buf, len, buf_index, more, status, xYieldRequired ) SPI_MASTER_ISR_CALLBACK_ATTR void xFunction( uint8_t *buf, int len, int buf_index, int more, uint32_t status, BaseType_t *xYieldRequired )

typedef void (*spi_master_isr_cb_t)(uint8_t *buf, int len, int buf_index, int more, uint32_t status, BaseType_t *xYieldRequired);

/**
 * If this flag is set then spi_transaction() will return
 * immediately after the DMA receive transaction is requested,
 * potentially before the data is actually received. In this
 * case, an application ISR callback must be provided which
 * will be called with the receive buffer once the transaction
 * is complete.
 */
#define SPI_MASTER_FLAG_RX_NOBLOCK 0x00000001

/**
 * If this flag is set then spi_transaction() will return
 * immediately after the DMA transmit transaction is requested,
 * potentially before the data is actually read by the SPI device.
 * In this case, an application ISR callback must be provided which
 * will be called with the transmit buffer once the SPI device no
 * longer needs. Until the ISR is called, the application must not
 * release or reuse the transmit buffer.
 */
#define SPI_MASTER_FLAG_TX_NOBLOCK 0x00000002

/* Initialize driver*/
soc_peripheral_t spi_master_driver_init(
        int device_id,
        int dma_buffer_count,
        int isr_core,
		uint32_t flags,
		spi_master_isr_cb_t isr_cb);

/* Initialize device */
void spi_master_device_init(
        soc_peripheral_t dev,
        unsigned cpol,
        unsigned cpha,
        unsigned clock_divide,
        unsigned cs_to_data_delay_ns,
        unsigned byte_setup_ns,
        unsigned data_to_cs_delay_ns);

void spi_transaction_sg(
        soc_peripheral_t dev,
        uint8_t *rx_buf[],
        size_t rx_len[],
		size_t rx_buf_count,
        uint8_t *tx_buf[],
        size_t tx_len[],
		size_t tx_buf_count);

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
        uint8_t *rx_buf,
        size_t rx_len,
        uint8_t *tx_buf,
        size_t tx_len);

#endif /* SPI_MASTER_DRIVER_H_ */
