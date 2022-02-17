// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <string.h>

/* wfx headers */
#include "sl_wfx.h"
#include "FreeRTOS/sl_wfx_host.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "semphr.h"

/* driver headers */
#include "rtos_spi_master.h"
#include "rtos_gpio.h"

void sl_wfx_host_task_rx_notify(BaseType_t *xYieldRequired);
void sl_wfx_host_task_stop(void);
void sl_wfx_host_task_start(void);

typedef struct {
    int initialized;
    rtos_spi_master_device_t *spi_dev;
    rtos_gpio_t *gpio_dev;

    rtos_gpio_port_id_t wirq_gpio_port;
    int wirq_bit;
    rtos_gpio_port_id_t wup_gpio_port;
    int wup_bit;
    rtos_gpio_port_id_t reset_gpio_port;
    int reset_bit;
} sl_wfx_host_hif_t;

static sl_wfx_host_hif_t hif_ctx;

/**** XCORE Specific Functions Start ****/

void sl_wfx_host_set_hif(rtos_spi_master_device_t *spi_dev,
                         rtos_gpio_t *gpio_dev,
                         rtos_gpio_port_id_t wirq_gpio_port, int wirq_bit,
                         rtos_gpio_port_id_t wup_gpio_port, int wup_bit,
                         rtos_gpio_port_id_t reset_gpio_port, int reset_bit)
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
    rtos_gpio_port_id_t port;
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

    configASSERT(hif_ctx.gpio_dev != NULL && port != rtos_gpio_port_none);

    uint32_t port_data = rtos_gpio_port_in(hif_ctx.gpio_dev, port);
    if (value == 0) {
        port_data &= ~(1 << bit);
    } else {
        port_data |= (1 << bit);
    }
    rtos_gpio_port_out(hif_ctx.gpio_dev, port, port_data);
}

RTOS_GPIO_ISR_CALLBACK_ATTR
static void sl_wfx_host_wirq_isr(rtos_gpio_t *ctx, void *app_data, rtos_gpio_port_id_t port_id, uint32_t value)
{
    BaseType_t yield_required = pdFALSE;

    configASSERT(ctx == hif_ctx.gpio_dev);
    configASSERT(port_id == hif_ctx.wirq_gpio_port);

    if ((value & (1 << hif_ctx.wirq_bit)) != 0) {
        sl_wfx_host_task_rx_notify(&yield_required);
    }

    portYIELD_FROM_ISR(yield_required);
}

/**** XCORE Specific Functions End ****/


/**** WF200 Driver Required Host Functions Start ****/

sl_status_t sl_wfx_host_init_bus(void)
{
    if (!hif_ctx.initialized && hif_ctx.spi_dev != NULL && hif_ctx.gpio_dev != NULL) {

        if (hif_ctx.wirq_gpio_port > rtos_gpio_port_none && hif_ctx.wirq_gpio_port < RTOS_GPIO_TOTAL_PORT_CNT) {
            rtos_gpio_port_enable(hif_ctx.gpio_dev, hif_ctx.wirq_gpio_port);
            rtos_gpio_isr_callback_set(hif_ctx.gpio_dev, hif_ctx.wirq_gpio_port, sl_wfx_host_wirq_isr, NULL);
        } else {
            return SL_STATUS_INITIALIZATION;
        }

        if (hif_ctx.reset_gpio_port > rtos_gpio_port_none && hif_ctx.reset_gpio_port < RTOS_GPIO_TOTAL_PORT_CNT) {
            rtos_gpio_port_enable(hif_ctx.gpio_dev, hif_ctx.reset_gpio_port);
        } else {
            return SL_STATUS_INITIALIZATION;
        }

        if (hif_ctx.wup_gpio_port > rtos_gpio_port_none && hif_ctx.wup_gpio_port < RTOS_GPIO_TOTAL_PORT_CNT) {
        	if (hif_ctx.wup_gpio_port != hif_ctx.reset_gpio_port) {
        	    rtos_gpio_port_enable(hif_ctx.gpio_dev, hif_ctx.wup_gpio_port);
        	}
        } else {
            hif_ctx.wup_gpio_port = rtos_gpio_port_none;
        }

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
        rtos_gpio_interrupt_enable(hif_ctx.gpio_dev, hif_ctx.wirq_gpio_port);
        return SL_STATUS_OK;
    } else {
        return SL_STATUS_NOT_INITIALIZED;
    }
}

sl_status_t sl_wfx_host_disable_platform_interrupt(void)
{
    if (hif_ctx.initialized) {
        rtos_gpio_interrupt_disable(hif_ctx.gpio_dev, hif_ctx.wirq_gpio_port);
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
    rtos_spi_master_transaction_start(hif_ctx.spi_dev);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_spi_cs_deassert(void)
{
    rtos_spi_master_transaction_end(hif_ctx.spi_dev);
    return SL_STATUS_OK;
}

sl_status_t sl_wfx_host_spi_transfer_no_cs_assert(sl_wfx_host_bus_transfer_type_t type,
                                                  uint8_t *header,
                                                  uint16_t header_length,
                                                  uint8_t *buffer,
                                                  uint16_t buffer_length)
{
    uint8_t *rx_buf = NULL;
    uint8_t *tx_buf = NULL;

    if (type & SL_WFX_BUS_READ) {
        rx_buf = buffer;
    }
    if (type & SL_WFX_BUS_WRITE) {
        tx_buf = buffer;
    }

    rtos_spi_master_transfer(hif_ctx.spi_dev, header, NULL, header_length);
    rtos_spi_master_transfer(hif_ctx.spi_dev, tx_buf, rx_buf, buffer_length);

    return SL_STATUS_OK;
}

/**** WF200 Driver Required Host Functions End ****/
