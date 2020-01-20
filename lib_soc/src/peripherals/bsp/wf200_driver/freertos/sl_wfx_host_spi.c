// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <string.h>

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "spi_master_driver.h"
#include "gpio_driver.h"
#include "sl_wfx.h"
#include "sl_wfx_host.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "semphr.h"

void sl_wfx_host_task_rx_notify(BaseType_t *xYieldRequired);
void sl_wfx_host_task_stop(void);
void sl_wfx_host_task_start(void);

typedef struct {
    int initialized;
    soc_peripheral_t spi_dev;
    soc_peripheral_t gpio_dev;

    gpio_id_t wirq_gpio_port;
    int wirq_bit;
    gpio_id_t wup_gpio_port;
    int wup_bit;
    gpio_id_t reset_gpio_port;
    int reset_bit;
} sl_wfx_host_hif_t;

static sl_wfx_host_hif_t hif_ctx;

/**** XCORE Specific Functions Start ****/

void sl_wfx_host_set_hif(soc_peripheral_t spi_dev,
                         soc_peripheral_t gpio_dev,
                         gpio_id_t wirq_gpio_port, int wirq_bit,
                         gpio_id_t wup_gpio_port, int wup_bit,
                         gpio_id_t reset_gpio_port, int reset_bit)
{
    configASSERT(!hif_ctx.initialized);

    hif_ctx.spi_dev = spi_dev;
    hif_ctx.gpio_dev = gpio_dev;
    hif_ctx.wirq_gpio_port = wirq_gpio_port;
    hif_ctx.wirq_bit = wirq_bit;
    hif_ctx.wup_gpio_port = wup_gpio_port;
    hif_ctx.wup_bit = wup_bit;
    hif_ctx.reset_gpio_port = reset_gpio_port;
    hif_ctx.reset_bit = reset_bit;
}

void sl_wfx_host_gpio(int gpio,
                      int value)
{
    gpio_id_t port;
    int bit;

    switch (gpio) {
    case SL_WFX_HIF_GPIO_WUP:
        port = hif_ctx.wup_gpio_port;
        bit = hif_ctx.wup_bit;
        break;
    case SL_WFX_HIF_GPIO_RESET:
        port = hif_ctx.reset_gpio_port;
        bit = hif_ctx.reset_bit;
        break;
    default:
        return;
    }

    configASSERT(hif_ctx.initialized && port != -1);

    gpio_write_pin(hif_ctx.gpio_dev, port, bit, value);
}

static GPIO_ISR_CALLBACK_FUNCTION(sl_wfx_host_wirq_isr, device, source_id)
{
    uint32_t value;
    BaseType_t xYieldRequired = pdFALSE;

    configASSERT(device == hif_ctx.gpio_dev);
    configASSERT(source_id == hif_ctx.wirq_gpio_port);

    if (gpio_read_pin(device, hif_ctx.wirq_gpio_port, hif_ctx.wirq_bit)) {
        sl_wfx_host_task_rx_notify(&xYieldRequired);
    }

    return xYieldRequired;
}

/**** XCORE Specific Functions End ****/


/**** WF200 Driver Required Host Functions Start ****/

sl_status_t sl_wfx_host_init_bus(void)
{
    if (!hif_ctx.initialized && hif_ctx.spi_dev != NULL && hif_ctx.gpio_dev != NULL) {

        if (hif_ctx.wirq_gpio_port >= 0 && hif_ctx.wirq_gpio_port < GPIO_TOTAL_PORT_CNT) {
            gpio_init(hif_ctx.gpio_dev, hif_ctx.wirq_gpio_port);
            gpio_irq_setup_callback(hif_ctx.gpio_dev, hif_ctx.wirq_gpio_port, sl_wfx_host_wirq_isr);
            //gpio_irq_enable(hif_ctx.gpio_dev, hif_ctx.wirq_gpio_port);//////////TEMPORARY
        } else {
            return SL_STATUS_INITIALIZATION;
        }

        if (hif_ctx.reset_gpio_port >= 0 && hif_ctx.reset_gpio_port < GPIO_TOTAL_PORT_CNT) {
            gpio_init(hif_ctx.gpio_dev, hif_ctx.reset_gpio_port);
        } else {
            return SL_STATUS_INITIALIZATION;
        }

        if (hif_ctx.wup_gpio_port >= 0 && hif_ctx.wup_gpio_port < GPIO_TOTAL_PORT_CNT) {
            gpio_init(hif_ctx.gpio_dev, hif_ctx.wup_gpio_port);
        } else {
            hif_ctx.wup_gpio_port = -1;
        }

        spi_master_device_init(hif_ctx.spi_dev,
                               0, 0, /* mode 0 */
                               2,    /* 100 MHz / (2* 2 ) / 2 = 12.5 MHz */
                               500,  /* really only needs to be 3ns but it is too quick */
                               0,    /* no inter-byte setup delay required */
                               0);   /* no last clock to cs delay required */

        sl_wfx_host_task_start();
        hif_ctx.initialized = 1;

        return SL_STATUS_OK;
    } else if (hif_ctx.initialized) {
        return SL_STATUS_ALREADY_INITIALIZED;
    } else {
        return SL_STATUS_INITIALIZATION;
    }
}

sl_status_t sl_wfx_host_enable_platform_interrupt(void)
{
    if (hif_ctx.initialized) {
        rtos_printf("enable irq\n");
        gpio_irq_enable(hif_ctx.gpio_dev, hif_ctx.wirq_gpio_port);
        return SL_STATUS_OK;
    } else {
        return SL_STATUS_NOT_INITIALIZED;
    }
}

sl_status_t sl_wfx_host_disable_platform_interrupt(void)
{
    if (hif_ctx.initialized) {
        gpio_irq_disable(hif_ctx.gpio_dev, hif_ctx.wirq_gpio_port);
        return SL_STATUS_OK;
    } else {
        return SL_STATUS_NOT_INITIALIZED;
    }
}

sl_status_t sl_wfx_host_deinit_bus(void)
{
    sl_wfx_host_disable_platform_interrupt();
    sl_wfx_host_task_stop();

    hif_ctx.initialized = 0;
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

    if (!hif_ctx.initialized) {
        return SL_STATUS_NOT_INITIALIZED;
    }

    sem = (SemaphoreHandle_t) soc_peripheral_app_data(hif_ctx.spi_dev);
    c = soc_peripheral_ctrl_chanend(hif_ctx.spi_dev);
    tx_ring_buf = soc_peripheral_tx_dma_ring_buf(hif_ctx.spi_dev);

    if (type & SL_WFX_BUS_READ) {
        soc_dma_ring_buf_t *rx_ring_buf = soc_peripheral_rx_dma_ring_buf(hif_ctx.spi_dev);
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
    soc_peripheral_hub_dma_request(hif_ctx.spi_dev, SOC_DMA_TX_REQUEST);

    /* TODO: check return value */
    xSemaphoreTake(sem, portMAX_DELAY);

    if (type & SL_WFX_BUS_READ) {
        /* TODO: check return value */
        xSemaphoreTake(sem, portMAX_DELAY);
    }

    return SL_STATUS_OK;
}

/**** WF200 Driver Required Host Functions End ****/
