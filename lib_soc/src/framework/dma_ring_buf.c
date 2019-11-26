/*
 * dma_ring_buf.c
 *
 *  Created on: Sep 30, 2019
 *      Author: mbruno
 */

#include <stdlib.h>
#include <stdint.h>

#include "xcore_freertos_dma_ring_buf.h"

#include "xassert.h"

struct xcore_freertos_dma_ring_buf_desc {
    void *buf;
    uint16_t length;
    volatile uint16_t status  : 2;
    uint16_t tx_last : 1;

};

static int add(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        int i,
        int n)
{
    i += n;
    if (i >= ring_buf->desc_count) {
        i -= ring_buf->desc_count;
    }

    return i;
}

void xcore_freertos_dma_ring_buf_desc_init(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        uint32_t *desc_buf,
        int buf_desc_count)
{
    int i;

    ring_buf->dma_next = 0;
    ring_buf->app_next = 0;
    ring_buf->done_next = 0;
    ring_buf->desc_count = buf_desc_count;
    ring_buf->desc = (xcore_freertos_dma_ring_buf_desc_t *) desc_buf;

    for (i = 0; i < ring_buf->desc_count; i++) {
        ring_buf->desc[i].buf = NULL;
        ring_buf->desc[i].length = 0;
        ring_buf->desc[i].tx_last = 1;
        ring_buf->desc[i].status = XCORE_FREERTOS_DMA_RING_BUF_READY;
    }
}

void xcore_freertos_dma_ring_buf_rx_desc_set_buf(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        void *buf,
        uint16_t length)
{
    xassert(ring_buf->desc[ring_buf->app_next].status == XCORE_FREERTOS_DMA_RING_BUF_READY);

    ring_buf->desc[ring_buf->app_next].buf = buf;
    ring_buf->desc[ring_buf->app_next].length = length;
    asm volatile( "" ::: "memory" );
    ring_buf->desc[ring_buf->app_next].status = XCORE_FREERTOS_DMA_RING_BUF_WAITING;

    ring_buf->app_next = add(ring_buf, ring_buf->app_next, 1);
}

void *xcore_freertos_dma_ring_buf_rx(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        int *length)
{
    void *buf = NULL;

    if (ring_buf->desc[ring_buf->done_next].status == XCORE_FREERTOS_DMA_RING_BUF_RX_DONE) {

        if (length != NULL) {
            *length = ring_buf->desc[ring_buf->done_next].length;
        }

        buf = ring_buf->desc[ring_buf->done_next].buf;
        asm volatile( "" ::: "memory" );
        ring_buf->desc[ring_buf->done_next].status = XCORE_FREERTOS_DMA_RING_BUF_READY;
        ring_buf->done_next = add(ring_buf, ring_buf->done_next, 1);
    }

    return buf;
}

static void set_tx_desc_buf(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        int n,
        void *buf,
        uint16_t length,
        int last)
{
    n = add(ring_buf, ring_buf->app_next, n);

    while (ring_buf->desc[n].status != XCORE_FREERTOS_DMA_RING_BUF_READY);

    ring_buf->desc[n].buf = buf;
    ring_buf->desc[n].length = length;
    ring_buf->desc[n].tx_last = last;
    asm volatile( "" ::: "memory" );
    ring_buf->desc[n].status = XCORE_FREERTOS_DMA_RING_BUF_WAITING;
}

void xcore_freertos_dma_ring_buf_tx(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        void *buf,
        uint16_t length)
{
    while (ring_buf->desc[ring_buf->app_next].status != XCORE_FREERTOS_DMA_RING_BUF_READY);

    ring_buf->desc[ring_buf->app_next].buf = buf;
    ring_buf->desc[ring_buf->app_next].length = length;
    ring_buf->desc[ring_buf->app_next].tx_last = 1;
    asm volatile( "" ::: "memory" );
    ring_buf->desc[ring_buf->app_next].status = XCORE_FREERTOS_DMA_RING_BUF_WAITING;

    ring_buf->app_next = add(ring_buf, ring_buf->app_next, 1);
}

void xcore_freertos_dma_ring_buf_tx_sg(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        int buf_count,
        void *buf[],
        uint16_t lengths[])
{
    int i;

    xassert(buf_count <= ring_buf->desc_count);

    for (i = 1; i < buf_count; i++) {
        set_tx_desc_buf(ring_buf, i, buf[i], lengths[i], i == (buf_count - 1));
    }

    while (ring_buf->desc[ring_buf->app_next].status != XCORE_FREERTOS_DMA_RING_BUF_READY);

    ring_buf->desc[ring_buf->app_next].buf = buf[0];
    ring_buf->desc[ring_buf->app_next].length = lengths[0];
    ring_buf->desc[ring_buf->app_next].tx_last = (buf_count == 1);
    asm volatile( "" ::: "memory" );
    ring_buf->desc[ring_buf->app_next].status = XCORE_FREERTOS_DMA_RING_BUF_WAITING;

    ring_buf->app_next = add(ring_buf, ring_buf->app_next, buf_count);
}

void *xcore_freertos_dma_ring_buf_tx_complete(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        int *length,
        int *more)
{
    void *buf = NULL;

    if (ring_buf->desc[ring_buf->done_next].status == XCORE_FREERTOS_DMA_RING_BUF_TX_DONE) {

        if (length != NULL) {
            *length = ring_buf->desc[ring_buf->done_next].length;
        }
        if (more != NULL) {
            *more = !ring_buf->desc[ring_buf->done_next].tx_last;
        }

        buf = ring_buf->desc[ring_buf->done_next].buf;
        asm volatile( "" ::: "memory" );
        ring_buf->desc[ring_buf->done_next].status = XCORE_FREERTOS_DMA_RING_BUF_READY;
        ring_buf->done_next = add(ring_buf, ring_buf->done_next, 1);
    }

    return buf;
}

/*
 * To be called by the DMA task.
 */
void *xcore_freertos_dma_ring_buf_get_next(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        int *length,
        int *more)
{
    if (ring_buf->desc[ring_buf->dma_next].status == XCORE_FREERTOS_DMA_RING_BUF_WAITING) {
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
 * To be called by the DMA task.
 */
void xcore_freertos_dma_ring_buf_mark_done(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        int new_status,
        int length)
{
    xassert(ring_buf->desc[ring_buf->dma_next].status == XCORE_FREERTOS_DMA_RING_BUF_WAITING);
    ring_buf->desc[ring_buf->dma_next].length = length;
    asm volatile( "" ::: "memory" );
    ring_buf->desc[ring_buf->dma_next].status = new_status;

    ring_buf->dma_next = add(ring_buf, ring_buf->dma_next, 1);
}
