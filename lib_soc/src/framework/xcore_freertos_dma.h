/*
 * xcore_freertos_dma.h
 *
 *  Created on: Sep 25, 2019
 *      Author: mbruno
 */


#ifndef XCORE_FREERTOS_DMA_H_
#define XCORE_FREERTOS_DMA_H_

#include <stdint.h>

#define XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT  3
#define XCORE_FREERTOS_DMA_DATA_TO_DEVICE_CH     0
#define XCORE_FREERTOS_DMA_DATA_FROM_DEVICE_CH   1
#define XCORE_FREERTOS_DMA_DEVICE_CONTROL_CH     2

#define XCORE_FREERTOS_DMA_INTERRUPT_STATUS_RX_DONE 0x00000001
#define XCORE_FREERTOS_DMA_INTERRUPT_STATUS_TX_DONE 0x00000002

#ifndef __XC__

#include "FreeRTOS.h"

#include "xcore_freertos_dma_ring_buf.h"


struct xcore_freertos_device;
typedef struct xcore_freertos_device * xcore_freertos_device_t;

xcore_freertos_device_t xcore_freertos_dma_device_register(
        chanend c[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT]);

void xcore_freertos_dma_device_handler_register(
        xcore_freertos_device_t device,
        int core_id,
        void *app_data,
        rtos_irq_isr_t isr);

void *xcore_freertos_dma_device_app_data(
        xcore_freertos_device_t device);

xcore_freertos_dma_ring_buf_t *xcore_freertos_dma_device_rx_ring_buf(
        xcore_freertos_device_t device);

xcore_freertos_dma_ring_buf_t *xcore_freertos_dma_device_tx_ring_buf(
        xcore_freertos_device_t device);

chanend xcore_freertos_dma_device_ctrl_chanend(
        xcore_freertos_device_t device);

void xcore_freertos_dma_request(void);

uint32_t xcore_freertos_dma_interrupt_status(
        xcore_freertos_device_t device);

void xcore_freertos_dma_device_common_init(
        xcore_freertos_device_t device,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr);

#endif // __XC__

#ifdef __XC__
extern "C" {
#endif //__XC__

#ifdef __XC__
#pragma select handler
#endif //__XC__
void xcore_freertos_dma_device_rx_ready(
        chanend c);

uint16_t xcore_freertos_dma_device_rx_data(
        chanend c,
        void *data,
        uint16_t max_length);

void xcore_freertos_dma_device_tx_data(
        chanend c,
        void *data,
        uint16_t length);

void xcore_freertos_dma_task(
        /*const unsigned int device_count,
        ARRAY_OF_SIZE(chanend, device_c, device_count)*/);

#ifdef __XC__
}
#endif //__XC__

#endif /* XCORE_FREERTOS_DMA_H_ */
