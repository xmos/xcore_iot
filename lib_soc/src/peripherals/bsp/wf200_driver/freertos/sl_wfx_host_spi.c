// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <string.h>

/* Library headers */
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "spi_master_driver.h"
#include "gpio_driver.h"
#include "sl_wfx.h"
#include "sl_wfx_host.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "semphr.h"

/* FreeRTOS Plus TCP headers */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"

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

portTIMER_CALLBACK_ATTRIBUTE
static void deferred_tx(uint8_t *tx_buf, uint32_t index)
{
	NetworkBufferDescriptor_t *nbuf;
	uint8_t *ebuf;
	static sl_wfx_register_address_t address;

	sl_wfx_send_frame_req_t *send_frame_req = (sl_wfx_send_frame_req_t *) tx_buf;

	if (index == 0) {
		uint16_t header = sl_wfx_unpack_16bit_big_endian(tx_buf);
		if ((header & 0x8000) == 0) {
			address = header >> 12;
		}

		vPortFree(tx_buf);
	} else if (index == 1 && address == SL_WFX_IN_OUT_QUEUE_REG_ID && send_frame_req->header.id == SL_WFX_SEND_FRAME_REQ_ID) {

		ebuf = (uint8_t *) tx_buf + sizeof(sl_wfx_send_frame_req_t);
		nbuf = pxPacketBuffer_to_NetworkBuffer(ebuf);

		vReleaseNetworkBufferAndDescriptor(nbuf);
	} else {
		vPortFree(tx_buf);
	}
}

static SPI_MASTER_ISR_CALLBACK_FUNCTION(spi_isr_cb, buf, len, buf_index, more, status, xYieldRequired)
{
	(void) len;
	(void) more;

	if (status & SOC_PERIPHERAL_ISR_DMA_TX_DONE_BM) {
		xTimerPendFunctionCallFromISR( (PendedFunction_t) deferred_tx, buf, buf_index, xYieldRequired );
	}
}

void sl_wfx_host_set_hif(int spi_dev_id,
		                 int gpio_dev_id,
                         gpio_id_t wirq_gpio_port, int wirq_bit,
                         gpio_id_t wup_gpio_port, int wup_bit,
                         gpio_id_t reset_gpio_port, int reset_bit)
{
    configASSERT(!hif_ctx.initialized);

    if (hif_ctx.spi_dev == NULL) {
        hif_ctx.spi_dev = spi_master_driver_init(
                spi_dev_id,
                ipconfigZERO_COPY_TX_DRIVER ? 3*2 : 2,  /* Uses 2 DMA buffers per transaction for the scatter/gather */
                0,                                      /* This device's interrupts should happen on core 0 */
                ipconfigZERO_COPY_TX_DRIVER ? SPI_MASTER_FLAG_TX_NOBLOCK : 0,
                ipconfigZERO_COPY_TX_DRIVER ? spi_isr_cb : NULL
                );
    }

    hif_ctx.gpio_dev = bitstream_gpio_devices[gpio_dev_id];
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

    configASSERT(hif_ctx.initialized && port != gpio_none);

    gpio_write_pin(hif_ctx.gpio_dev, port, bit, value);
}

static GPIO_ISR_CALLBACK_FUNCTION(sl_wfx_host_wirq_isr, device, source_id)
{
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

        if (hif_ctx.wirq_gpio_port > gpio_none && hif_ctx.wirq_gpio_port < GPIO_TOTAL_PORT_CNT) {
            gpio_init(hif_ctx.gpio_dev, hif_ctx.wirq_gpio_port);
            gpio_irq_setup_callback(hif_ctx.gpio_dev, hif_ctx.wirq_gpio_port, sl_wfx_host_wirq_isr);
        } else {
            return SL_STATUS_INITIALIZATION;
        }

        if (hif_ctx.reset_gpio_port > gpio_none && hif_ctx.reset_gpio_port < GPIO_TOTAL_PORT_CNT) {
            gpio_init(hif_ctx.gpio_dev, hif_ctx.reset_gpio_port);
        } else {
            return SL_STATUS_INITIALIZATION;
        }

        if (hif_ctx.wup_gpio_port > gpio_none && hif_ctx.wup_gpio_port < GPIO_TOTAL_PORT_CNT) {
        	if (hif_ctx.wup_gpio_port != hif_ctx.reset_gpio_port) {
        		gpio_init(hif_ctx.gpio_dev, hif_ctx.wup_gpio_port);
        	}
        } else {
            hif_ctx.wup_gpio_port = gpio_none;
        }

        spi_master_device_init(hif_ctx.spi_dev,
                               0, 0, /* mode 0 */
                               1,    /* 100 MHz / (1*2) / 2 = 25 MHz SPI clock */
                               3,    /* 3 nanosecond cs to data minimum time */
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
    void *tmp_buf;
    uint8_t *rx_buf[2], *tx_buf[2];
    size_t rx_len[2], tx_len[2];
    size_t rx_buf_count = 0;
    size_t tx_buf_count = 1; /* always send at least the header */

    if (!hif_ctx.initialized) {
        return SL_STATUS_NOT_INITIALIZED;
    }

    if (type & SL_WFX_BUS_READ) {
    	rx_buf_count = 2;
    	rx_buf[0] = header; rx_len[0] = header_length;
    	rx_buf[1] = buffer; rx_len[1] = buffer_length;
    }

    /* Always send the header */
    if (ipconfigZERO_COPY_TX_DRIVER) {
    	/*
    	 * In "zero copy" mode we actually have to
    	 * copy the header into a new malloc'd buffer.
    	 * The SPI transaction below will not block.
    	 * Instead the deferred_tx ISR above will
    	 * be called which will free it.
    	 */
		tmp_buf = pvPortMalloc(header_length);
		memcpy(tmp_buf, header, header_length);
		tx_buf[0] = tmp_buf;
    } else {
    	tx_buf[0] = header;
    }
    tx_len[0] = header_length;

    if (type & SL_WFX_BUS_WRITE) {

    	/* Also sending the buffer after the header */
    	tx_buf_count++;

    	if (ipconfigZERO_COPY_TX_DRIVER) {
			void *tmp_buf;

			sl_wfx_send_frame_req_t *send_frame_req = (sl_wfx_send_frame_req_t *) buffer;
			sl_wfx_register_address_t address = sl_wfx_unpack_16bit_big_endian(header) >> 12;

			if (address == SL_WFX_IN_OUT_QUEUE_REG_ID && send_frame_req->header.id == SL_WFX_SEND_FRAME_REQ_ID) {
				/*
				 * Ethernet frames buffers are handed directly over to device.
				 * The deferred TX ISR above will free them when the DMA transaction
				 * is complete.
				 */
				tmp_buf = buffer;
			} else {
				tmp_buf = pvPortMalloc(buffer_length);
				memcpy(tmp_buf, buffer, buffer_length);
			}

			tx_buf[1] = tmp_buf;
    	} else {
			tx_buf[1] = buffer;
    	}

    	tx_len[1] = buffer_length;
    }

    spi_transaction_sg(
    		hif_ctx.spi_dev,
			rx_buf,
			rx_len,
			rx_buf_count,
			tx_buf,
			tx_len,
			tx_buf_count);

    return SL_STATUS_OK;
}

/**** WF200 Driver Required Host Functions End ****/
