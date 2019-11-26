// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <stdlib.h>
#include <stdint.h>

#include "soc_dma_ring_buf.h"

#include "xassert.h"

/*
 * The buffer descriptor status when a DMA receive operation
 * on it has just completed. The application should receive
 * these by calling soc_dma_ring_rx_buf_get().
 */
#define SOC_DMA_BUF_DESC_STATUS_RX_DONE  3

/*
 * The buffer descriptor status when a DMA transmit operation
 * on it has just completed. The application should receive
 * these by calling soc_dma_ring_tx_buf_get().
 */
#define SOC_DMA_BUF_DESC_STATUS_TX_DONE  2

/*
 * The buffer descriptor status when the application has sent
 * it to the DMA and is waiting for the operation to complete.
 */
#define SOC_DMA_BUF_DESC_STATUS_WAITING  1

/*
 * The buffer descriptor status when it is ready to be assigned
 * a new buffer by the application.
 */
#define SOC_DMA_BUF_DESC_STATUS_READY 0

struct soc_dma_buf_desc {
    void *buf;
    uint16_t length;
    volatile uint16_t status  : 2;
    uint16_t tx_last : 1;

};

static int add(
        soc_dma_ring_buf_t *ring_buf,
        int i,
        int n)
{
    i += n;
    if (i >= ring_buf->desc_count) {
        i -= ring_buf->desc_count;
    }

    return i;
}

void soc_dma_ring_buf_init(
        soc_dma_ring_buf_t *ring_buf,
        uint32_t *desc_buf,
        int buf_desc_count)
{
    int i;

    ring_buf->dma_next = 0;
    ring_buf->app_next = 0;
    ring_buf->done_next = 0;
    ring_buf->desc_count = buf_desc_count;
    ring_buf->desc = (soc_dma_buf_desc_t *) desc_buf;

    for (i = 0; i < ring_buf->desc_count; i++) {
        ring_buf->desc[i].buf = NULL;
        ring_buf->desc[i].length = 0;
        ring_buf->desc[i].tx_last = 1;
        ring_buf->desc[i].status = SOC_DMA_BUF_DESC_STATUS_READY;
    }
}

void soc_dma_ring_rx_buf_set(
        soc_dma_ring_buf_t *ring_buf,
        void *buf,
        uint16_t length)
{
    xassert(ring_buf->desc[ring_buf->app_next].status == SOC_DMA_BUF_DESC_STATUS_READY);

    ring_buf->desc[ring_buf->app_next].buf = buf;
    ring_buf->desc[ring_buf->app_next].length = length;
    asm volatile( "" ::: "memory" );
    ring_buf->desc[ring_buf->app_next].status = SOC_DMA_BUF_DESC_STATUS_WAITING;

    ring_buf->app_next = add(ring_buf, ring_buf->app_next, 1);
}

void *soc_dma_ring_rx_buf_get(
        soc_dma_ring_buf_t *ring_buf,
        int *length)
{
    void *buf = NULL;

    if (ring_buf->desc[ring_buf->done_next].status == SOC_DMA_BUF_DESC_STATUS_RX_DONE) {

        if (length != NULL) {
            *length = ring_buf->desc[ring_buf->done_next].length;
        }

        buf = ring_buf->desc[ring_buf->done_next].buf;
        asm volatile( "" ::: "memory" );
        ring_buf->desc[ring_buf->done_next].status = SOC_DMA_BUF_DESC_STATUS_READY;
        ring_buf->done_next = add(ring_buf, ring_buf->done_next, 1);
    }

    return buf;
}

void soc_dma_ring_tx_buf_set(
        soc_dma_ring_buf_t *ring_buf,
        void *buf,
        uint16_t length)
{
    while (ring_buf->desc[ring_buf->app_next].status != SOC_DMA_BUF_DESC_STATUS_READY);

    ring_buf->desc[ring_buf->app_next].buf = buf;
    ring_buf->desc[ring_buf->app_next].length = length;
    ring_buf->desc[ring_buf->app_next].tx_last = 1;
    asm volatile( "" ::: "memory" );
    ring_buf->desc[ring_buf->app_next].status = SOC_DMA_BUF_DESC_STATUS_WAITING;

    ring_buf->app_next = add(ring_buf, ring_buf->app_next, 1);
}

#if 0
/* Scatter gather not ready yet */

static void set_tx_desc_buf(
        soc_dma_ring_buf_t *ring_buf,
        int n,
        void *buf,
        uint16_t length,
        int last)
{
    n = add(ring_buf, ring_buf->app_next, n);

    while (ring_buf->desc[n].status != SOC_DMA_BUF_DESC_STATUS_READY);

    ring_buf->desc[n].buf = buf;
    ring_buf->desc[n].length = length;
    ring_buf->desc[n].tx_last = last;
    asm volatile( "" ::: "memory" );
    ring_buf->desc[n].status = SOC_DMA_BUF_DESC_STATUS_WAITING;
}

void soc_dma_ring_tx_buf_set_sg(
        soc_dma_ring_buf_t *ring_buf,
        int buf_count,
        void *buf[],
        uint16_t lengths[])
{
    int i;

    xassert(buf_count <= ring_buf->desc_count);

    for (i = 1; i < buf_count; i++) {
        set_tx_desc_buf(ring_buf, i, buf[i], lengths[i], i == (buf_count - 1));
    }

    while (ring_buf->desc[ring_buf->app_next].status != SOC_DMA_BUF_DESC_STATUS_READY);

    ring_buf->desc[ring_buf->app_next].buf = buf[0];
    ring_buf->desc[ring_buf->app_next].length = lengths[0];
    ring_buf->desc[ring_buf->app_next].tx_last = (buf_count == 1);
    asm volatile( "" ::: "memory" );
    ring_buf->desc[ring_buf->app_next].status = SOC_DMA_BUF_DESC_STATUS_WAITING;

    ring_buf->app_next = add(ring_buf, ring_buf->app_next, buf_count);
}
#endif

void *soc_dma_ring_tx_buf_get(
        soc_dma_ring_buf_t *ring_buf,
        int *length,
        int *more)
{
    void *buf = NULL;

    if (ring_buf->desc[ring_buf->done_next].status == SOC_DMA_BUF_DESC_STATUS_TX_DONE) {

        if (length != NULL) {
            *length = ring_buf->desc[ring_buf->done_next].length;
        }
        if (more != NULL) {
            *more = !ring_buf->desc[ring_buf->done_next].tx_last;
        }

        buf = ring_buf->desc[ring_buf->done_next].buf;
        asm volatile( "" ::: "memory" );
        ring_buf->desc[ring_buf->done_next].status = SOC_DMA_BUF_DESC_STATUS_READY;
        ring_buf->done_next = add(ring_buf, ring_buf->done_next, 1);
    }

    return buf;
}

/*
 * To be called only by the peripheral hub
 */
void *soc_dma_ring_buf_get(
        soc_dma_ring_buf_t *ring_buf,
        int *length,
        int *more)
{
    if (ring_buf->desc[ring_buf->dma_next].status == SOC_DMA_BUF_DESC_STATUS_WAITING) {
        if (length != NULL) {
            *length = ring_buf->desc[ring_buf->dma_next].length;
        }
        if (more != NULL) {
            *more = !ring_buf->desc[ring_buf->dma_next].tx_last;
        }
        return ring_buf->desc[ring_buf->dma_next].buf;
    } else {
        return NULL;
    }
}

/*
 * To be called only by the peripheral hub
 */
void soc_dma_ring_buf_release(
        soc_dma_ring_buf_t *ring_buf,
        int rx,
        int length)
{
    xassert(ring_buf->desc[ring_buf->dma_next].status == SOC_DMA_BUF_DESC_STATUS_WAITING);
    ring_buf->desc[ring_buf->dma_next].length = length;
    asm volatile( "" ::: "memory" );
    ring_buf->desc[ring_buf->dma_next].status = rx ? SOC_DMA_BUF_DESC_STATUS_RX_DONE : SOC_DMA_BUF_DESC_STATUS_TX_DONE;

    ring_buf->dma_next = add(ring_buf, ring_buf->dma_next, 1);
}
