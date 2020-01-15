// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <string.h>

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "spi_master_driver.h"
#include "sl_wfx.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "semphr.h"

static int initialized;
static soc_peripheral_t spi_dev;

/**** XCORE Specific Functions Start ****/

void sl_wfx_host_set_spi_device(soc_peripheral_t dev)
{
    if (!initialized) {
        spi_dev = dev;
    }
}

/**** XCORE Specific Functions End ****/


/**** WF200 Driver Required Host Functions Start ****/

sl_status_t sl_wfx_host_init_bus(void)
{
    if (!initialized && spi_dev != NULL) {
        spi_master_device_init(spi_dev,
                               0,    /* this should be defined in the bitstream */
                               0, 0, /* mode 0 */
                               7,    /* 100 MHz / (2*1) / 2 = 25 MHz */
                               10000,  /* really only needs to be 3ns but it is too quick */
                               0);   /* no inter-byte setup delay required */

        initialized = 1;

        return SL_STATUS_OK;
    } else if (initialized) {
        return SL_STATUS_ALREADY_INITIALIZED;
    } else {
        return SL_STATUS_INITIALIZATION;
    }
}

sl_status_t sl_wfx_host_deinit_bus(void)
{
    initialized = 0;
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_enable_platform_interrupt(void)
{
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_disable_platform_interrupt(void)
{
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_spi_cs_assert(void)
{
    /*
     * CS assertion/deassertion is, in fact, handled
     * by sl_wfx_host_spi_transfer_no_cs_assert().
     */
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_spi_cs_deassert(void)
{
    /*
     * CS assertion/deassertion is, in fact, handled
     * by sl_wfx_host_spi_transfer_no_cs_assert().
     */
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_spi_transfer_no_cs_assert(sl_wfx_host_bus_transfer_type_t type,
                                                  uint8_t *header,
                                                  uint16_t header_length,
                                                  uint8_t *buffer,
                                                  uint16_t buffer_length)
{
    SemaphoreHandle_t sem;
    chanend c;
    soc_dma_ring_buf_t *tx_ring_buf;

    if (!initialized) {
        return SL_STATUS_NOT_INITIALIZED;
    }

    sem = (SemaphoreHandle_t) soc_peripheral_app_data(spi_dev);
    c = soc_peripheral_ctrl_chanend(spi_dev);
    tx_ring_buf = soc_peripheral_tx_dma_ring_buf(spi_dev);

    if (type & SL_WFX_BUS_READ) {
        soc_dma_ring_buf_t *rx_ring_buf = soc_peripheral_rx_dma_ring_buf(spi_dev);
        size_t total_length = header_length + buffer_length;

        soc_peripheral_function_code_tx(c, SPI_MASTER_DEV_TRANSACTION);

        soc_peripheral_varlist_tx(
                c, 1,
                sizeof(size_t), &total_length);

        soc_dma_ring_rx_buf_sg_set(
                rx_ring_buf,
                buffer,
                buffer_length,
                1,
                2);

        soc_dma_ring_rx_buf_sg_set(
                rx_ring_buf,
                header,
                header_length,
                0,
                2);
    }

    if (type & SL_WFX_BUS_WRITE) {
        soc_dma_ring_tx_buf_sg_set(
                tx_ring_buf,
                buffer,
                buffer_length,
                1,
                2);

        soc_dma_ring_tx_buf_sg_set(
                tx_ring_buf,
                header,
                header_length,
                0,
                2);
    } else {
        soc_dma_ring_tx_buf_set(tx_ring_buf, header, header_length);
    }

    /* this request will take care of both the RX and TX */
    soc_peripheral_hub_dma_request(spi_dev, SOC_DMA_TX_REQUEST);

    /* TODO: check return value */
    xSemaphoreTake(sem, portMAX_DELAY);

    if (type & SL_WFX_BUS_READ) {
        /* TODO: check return value */
        xSemaphoreTake(sem, portMAX_DELAY);
    }

    return SL_STATUS_OK;
}

/**** WF200 Driver Required Host Functions End ****/
