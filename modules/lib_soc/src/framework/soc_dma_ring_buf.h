// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef SOC_DMA_RING_BUF_H_
#define SOC_DMA_RING_BUF_H_

#ifndef __XC__

#include <stdint.h>

#define SOC_DMA_BUF_DESC_WORDSIZE 2

typedef struct soc_dma_buf_desc soc_dma_buf_desc_t;

typedef struct {
    soc_dma_buf_desc_t *desc;
    int desc_count;
    int dma_next;
    int app_next;
    int done_next;
    void *tx_semaphore;
} soc_dma_ring_buf_t;

void soc_dma_ring_buf_init(
        soc_dma_ring_buf_t *ring_buf,
        uint32_t *desc_buf,
        int buf_desc_count);

void soc_dma_ring_rx_buf_set(
        soc_dma_ring_buf_t *ring_buf,
        void *buf,
        uint16_t length);

void soc_dma_ring_rx_buf_sg_set(
        soc_dma_ring_buf_t *ring_buf,
        void *buf,
        uint16_t length,
        int index,
        int buf_count);

void *soc_dma_ring_rx_buf_get(
        soc_dma_ring_buf_t *ring_buf,
        int *length,
        int *more);

void soc_dma_ring_tx_buf_set(
        soc_dma_ring_buf_t *ring_buf,
        void *buf,
        uint16_t length);

void soc_dma_ring_tx_buf_sg_set(
        soc_dma_ring_buf_t *ring_buf,
        void *buf,
        uint16_t length,
        int index,
        int buf_count);

void *soc_dma_ring_tx_buf_get(
        soc_dma_ring_buf_t *ring_buf,
        int *length,
        int *more);

#endif // __XC__

#endif /* SOC_DMA_RING_BUF_H_ */
