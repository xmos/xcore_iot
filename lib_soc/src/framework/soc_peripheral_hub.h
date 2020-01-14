// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef SOC_PERIPHERAL_HUB_H_
#define SOC_PERIPHERAL_HUB_H_

#include <stdint.h>

#define SOC_PERIPHERAL_CHANNEL_COUNT 4
#define SOC_PERIPHERAL_FROM_DMA_CH   0
#define SOC_PERIPHERAL_TO_DMA_CH     1
#define SOC_PERIPHERAL_CONTROL_CH    2
#define SOC_PERIPHERAL_IRQ_CH        3

#define SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM 0x00000001
#define SOC_PERIPHERAL_ISR_DMA_TX_DONE_BM 0x00000002

typedef enum {
    SOC_DMA_TX_REQUEST,
    SOC_DMA_RX_REQUEST
} soc_dma_request_t;

#define SOC_PERIPHERAL_DMA_TX 0x00

struct soc_peripheral;

#ifndef __XC__
typedef struct soc_peripheral * soc_peripheral_t;
#else
typedef struct soc_peripheral * unsafe soc_peripheral_t;
#endif

#ifndef __XC__

#include "soc_dma_ring_buf.h"
#include "rtos_support.h"

soc_peripheral_t soc_peripheral_register(
        chanend c[SOC_PERIPHERAL_CHANNEL_COUNT]);

void soc_peripheral_handler_register(
        soc_peripheral_t device,
        int core_id,
        void *app_data,
        rtos_irq_isr_t isr);

void *soc_peripheral_app_data(
        soc_peripheral_t device);

soc_dma_ring_buf_t *soc_peripheral_rx_dma_ring_buf(
        soc_peripheral_t device);

soc_dma_ring_buf_t *soc_peripheral_tx_dma_ring_buf(
        soc_peripheral_t device);

chanend soc_peripheral_ctrl_chanend(
        soc_peripheral_t device);

void soc_peripheral_hub_dma_request(
        soc_peripheral_t device,
        soc_dma_request_t request);

uint32_t soc_peripheral_interrupt_status(
        soc_peripheral_t device);

#endif // __XC__

#ifdef __XC__
extern "C" {
#endif //__XC__

#ifdef __XC__
#pragma select handler
#endif //__XC__
/**
 *
 */
void soc_peripheral_rx_dma_ready(
        chanend c);

int soc_peripheral_rx_dma_xfer(
        chanend c,
        void *data,
        int max_length);

int soc_peripheral_rx_dma_direct_xfer(
        soc_peripheral_t device,
        void *data,
        int max_length);

void soc_peripheral_tx_dma_xfer(
        chanend c,
        void *data,
        int length);

void soc_peripheral_tx_dma_direct_xfer(
        soc_peripheral_t device,
        void *data,
        int length);

void soc_peripheral_irq_send(
        chanend c,
        uint32_t status);

void soc_peripheral_irq_direct_send(
        soc_peripheral_t device,
        uint32_t status);

void soc_peripheral_hub();

#ifdef __XC__
}
#endif //__XC__

#endif /* SOC_PERIPHERAL_HUB_H_ */
