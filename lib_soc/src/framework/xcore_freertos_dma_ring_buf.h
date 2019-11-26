/*
 * xcore_freertos_dma_ring_buf.h
 *
 *  Created on: Sep 30, 2019
 *      Author: mbruno
 */


#ifndef XCORE_FREERTOS_DMA_RING_BUF_H_
#define XCORE_FREERTOS_DMA_RING_BUF_H_

#ifndef __XC__

#include <stdint.h>

/*
 * The buffer descriptor status when a DMA receive operation
 * on it has just completed. The application should receive
 * these by calling xcore_freertos_dma_ring_buf_rx().
 */
#define XCORE_FREERTOS_DMA_RING_BUF_RX_DONE  3

/*
 * The buffer descriptor status when a DMA transmit operation
 * on it has just completed. The application should receive
 * these by calling xcore_freertos_dma_ring_buf_tx_complete().
 */
#define XCORE_FREERTOS_DMA_RING_BUF_TX_DONE  2

/*
 * The buffer descriptor status when the application has sent
 * it to the DMA and is waiting for the operation to complete.
 */
#define XCORE_FREERTOS_DMA_RING_BUF_WAITING  1

/*
 * The buffer descriptor status when it is ready to be assigned
 * a new buffer by the application.
 */
#define XCORE_FREERTOS_DMA_RING_BUF_READY 0

#define XCORE_FREERTOS_DMA_RING_BUF_DESC_WORDSIZE 2

typedef struct xcore_freertos_dma_ring_buf_desc xcore_freertos_dma_ring_buf_desc_t;

typedef struct {
    xcore_freertos_dma_ring_buf_desc_t *desc;
    int desc_count;
    int dma_next;
    int app_next;
    int done_next;
} xcore_freertos_dma_ring_buf_t;

void xcore_freertos_dma_ring_buf_desc_init(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        uint32_t *desc_buf,
        int buf_desc_count);

void xcore_freertos_dma_ring_buf_rx_desc_set_buf(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        void *buf,
        uint16_t length);

void *xcore_freertos_dma_ring_buf_rx(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        int *length);

void xcore_freertos_dma_ring_buf_tx(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        void *buf,
        uint16_t length);

void xcore_freertos_dma_ring_buf_tx_sg(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        int buf_count,
        void *buf[],
        uint16_t lengths[]);

void *xcore_freertos_dma_ring_buf_tx_complete(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        int *length,
        int *more);

/**** These should only be called by the DMA task ****/
void *xcore_freertos_dma_ring_buf_get_next(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        int *length,
        int *more);

void xcore_freertos_dma_ring_buf_mark_done(
        xcore_freertos_dma_ring_buf_t *ring_buf,
        int new_status,
        int length);
/*****************************************************/

#endif // __XC__

#endif /* XCORE_FREERTOS_DMA_RING_BUF_H_ */
