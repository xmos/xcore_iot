// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <string.h>

#include "soc.h"

#include "xassert.h"

#include <xcore/chanend.h>
#include <xcore/channel.h>
#include <xcore/channel_transaction.h>
#include <xcore/triggerable.h>
#include <xcore/interrupt_wrappers.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define MAX_PERIPHERALS 8

int soc_dma_ring_buf_length_get(
        soc_dma_ring_buf_t *ring_buf);

int soc_dma_ring_buf_get(
        soc_dma_ring_buf_t *ring_buf,
        void **buf,
        int *more);

void soc_dma_ring_buf_release(
        soc_dma_ring_buf_t *ring_buf,
        int rx,
        int length);

/*
 * The channel end used by the peripheral hub to
 * interrupt the RTOS and to receive requests
 * from the RTOS.
 */
static chanend_t rtos_irq_c;

struct soc_peripheral {

    /* This device's ID */
    int id;

    /* Channel used to send data to this device. */
    chanend_t tx_c;

    /* Channel used to receive data from this device. */
    chanend_t rx_c;

    /* Channel used for control of this device. */
    chanend_t control_c;

    /* Channel used for receiving interrupt requests from this device. */
    chanend_t irq_c;

    /* DMA has begun sending data to this device. */
    uint32_t tx_ready;

    /* Application specific data */
    void *app_data;

    /* The core that handles the interrupts */
    int core_id;

    /* The IRQ source ID number for this device. */
    int irq_source_id;

    /* Interrupt status for the RTOS */
    uint32_t interrupt_status;

    soc_dma_ring_buf_t tx_ring_buf;
    soc_dma_ring_buf_t rx_ring_buf;

};

static struct soc_peripheral peripherals[MAX_PERIPHERALS];

static int peripheral_count;

/* To be called by the bitstream */
soc_peripheral_t soc_peripheral_register(
        chanend_t c[SOC_PERIPHERAL_CHANNEL_COUNT])
{
    int device_id;

    xassert(peripheral_count < MAX_PERIPHERALS);

    device_id = peripheral_count++;

    peripherals[device_id].id = device_id;
    peripherals[device_id].tx_c = c[SOC_PERIPHERAL_FROM_DMA_CH];
    peripherals[device_id].rx_c = c[SOC_PERIPHERAL_TO_DMA_CH];
    peripherals[device_id].control_c = c[SOC_PERIPHERAL_CONTROL_CH];
    peripherals[device_id].irq_c = c[SOC_PERIPHERAL_IRQ_CH];
    peripherals[device_id].tx_ready = 0;
    peripherals[device_id].app_data = NULL;
    peripherals[device_id].core_id = -1;
    peripherals[device_id].irq_source_id = -1;
    peripherals[device_id].interrupt_status = 0;

    return &peripherals[device_id];
}

void soc_peripheral_rx_dma_ready(
        chanend_t c)
{
    chanend_check_end_token(c);
}

int soc_peripheral_rx_dma_xfer(
        chanend_t c,
        void *data,
        int max_length)
{
    transacting_chanend_t tc;
    int length;

    tc = chan_init_transaction_master(c);
    t_chan_out_word(&tc, max_length);
    length = t_chan_in_word(&tc);
    xassert(length <= max_length);
    t_chan_in_buf_byte(&tc, data, length);
    
    (void) chan_complete_transaction(tc);

    return length;
}

int soc_peripheral_rx_dma_direct_xfer(
        soc_peripheral_t device,
        void *data,
        int max_length)
{
    void *tx_buf;
    int length;
    int total_length = -1;
    int more;

    /*
     * In this case the device and RTOS are on the same tile, and the
     * device is checking to see if the RTOS has any data for it in
     * DMA buffers. First the device's tx ring buffer descriptor list
     * is checked to see if it exists. If it does, it checks to see if
     * the next descriptor points to a buffer that has been filled with
     * data and handed off to the device. If so, then the transfer can
     * take place and the buffers are then handed back over to the RTOS.
     */
    if (device->tx_ring_buf.desc != NULL) {
        if (soc_dma_ring_buf_get(&device->tx_ring_buf, NULL, NULL) >= 0) {

            total_length = soc_dma_ring_buf_length_get(&device->tx_ring_buf);
            xassert(total_length <= max_length);

            do {
                length = soc_dma_ring_buf_get(&device->tx_ring_buf, &tx_buf, &more);
                xassert(length >= 0);
                memcpy(data, tx_buf, length);
                soc_dma_ring_buf_release(&device->tx_ring_buf, 0, length);
                data += length;
            } while (more);

            rtos_lock_acquire(0);
            device->interrupt_status |= SOC_PERIPHERAL_ISR_DMA_TX_DONE_BM;
            rtos_lock_release(0);

            /* Do not interrupt the RTOS unless it has registered an ISR */
            if (device->core_id >= 0 && device->irq_source_id >= 0) {
                rtos_irq(device->core_id, device->irq_source_id);
            }
        }
    }

    return total_length;
}

void soc_peripheral_tx_dma_xfer(
        chanend_t c,
        void *data,
        int length)
{
    transacting_chanend_t tc;

    tc = chan_init_transaction_master(c);
    t_chan_out_word(&tc, length);
    t_chan_out_buf_byte(&tc, data, length);

    (void) chan_complete_transaction(tc);
}

void soc_peripheral_tx_dma_direct_xfer(
        soc_peripheral_t device,
        void *data,
        int length)
{
    void *rx_buf;
    int max_length;
    int more;

    /*
     * In this case the device and RTOS are on the same tile, and the
     * device has data to send to the RTOS. First the device's rx ring
     * buffer descriptor list is checked to see if it exists. If it does
     * not then the transfer will not take place and the data will be lost.
     * If it does, it waits for the next descriptor to points to a buffer
     * that is ready to be filled with data and has been handed off to the
     * device. At that point the transfer can take place and the buffers
     * are then handed back over to the RTOS.
     */
    if (device->rx_ring_buf.desc != NULL) {
        do {
            max_length = soc_dma_ring_buf_length_get(&device->rx_ring_buf);
        }
        while (max_length < 0);

        xassert(length <= max_length);

        do {
            max_length = soc_dma_ring_buf_get(&device->rx_ring_buf, &rx_buf, &more);
            xassert(max_length >= 0);
            max_length = MIN(length, max_length);
            length -= max_length;
            memcpy(rx_buf, data, max_length);
            soc_dma_ring_buf_release(&device->rx_ring_buf, 1, max_length);
            data += max_length;
        } while (more);

        rtos_lock_acquire(0);
        device->interrupt_status |= SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM;
        rtos_lock_release(0);

        /* Do not interrupt the RTOS unless it has registered an ISR */
        if (device->core_id >= 0 && device->irq_source_id >= 0) {
            rtos_irq(device->core_id, device->irq_source_id);
        }
    }
}

void soc_peripheral_irq_send(
        chanend_t c,
        uint32_t status)
{
    chan_out_word(c, status);
}

void soc_peripheral_irq_direct_send(
        soc_peripheral_t device,
        uint32_t status)
{
    rtos_lock_acquire(0);
    device->interrupt_status |= status;
    rtos_lock_release(0);

    /* Do not interrupt the RTOS unless it has registered an ISR */
    if (device->core_id >= 0 && device->irq_source_id >= 0) {
        rtos_irq(device->core_id, device->irq_source_id);
    }
}

void soc_peripheral_handler_register(
        soc_peripheral_t device,
        int core_id,
        void *app_data,
        rtos_irq_isr_t isr)
{
    device->core_id = core_id;
    device->app_data = app_data;
    device->irq_source_id = rtos_irq_register(isr, device, rtos_irq_c);
}

void *soc_peripheral_app_data(
        soc_peripheral_t device)
{
    return device->app_data;
}

soc_dma_ring_buf_t *soc_peripheral_rx_dma_ring_buf(
        soc_peripheral_t device)
{
    return &device->rx_ring_buf;
}

soc_dma_ring_buf_t *soc_peripheral_tx_dma_ring_buf(
        soc_peripheral_t device)
{
    return &device->tx_ring_buf;
}

chanend soc_peripheral_ctrl_chanend(
        soc_peripheral_t device)
{
    return device->control_c;
}

int soc_peripheral_get_id(
        soc_peripheral_t device)
{
    return device->id;
}

void soc_peripheral_hub_dma_request(
        soc_peripheral_t device,
        soc_dma_request_t request)
{
    if (request == SOC_DMA_TX_REQUEST) {
        if (rtos_irq_ready() && device->tx_c != 0) {
            /*
             * If the tx_c channel is not NULL, then this
             * peripheral's DMA is performed by the peripheral
             * hub. The peripheral hub must be sent an IRQ to
             * let it know that there are TX DMA buffers available
             * and that it may send them to the device when it is
             * ready.
             */
            rtos_irq_peripheral(rtos_irq_c);
        } else if (device->control_c != 0) {
            /*
             * Otherwise, the peripheral itself must be told
             * that there are TX DMA buffers available for it
             * to receive.
             */
            soc_peripheral_function_code_tx(device->control_c, SOC_PERIPHERAL_DMA_TX);
        }
    } else if (request == SOC_DMA_RX_REQUEST) {
        if (rtos_irq_ready() && device->rx_c != 0) {
            /*
             * If the rx_c channel is not NULL, then this
             * peripheral's DMA is performed by the peripheral
             * hub. The peripheral hub must be sent an IRQ to
             * let it know that there are RX DMA buffers available
             * and that it may listen for data from the peripheral.
             */
            rtos_irq_peripheral(rtos_irq_c);
        } else {
            /*
             * No need to tell the device directly that the
             * application is ready to receive. When it has
             * data to send it will wait until there is an
             * available buffer.
             */
        }
    }
}

uint32_t soc_peripheral_interrupt_status(
        soc_peripheral_t device)
{
    uint32_t status;

    rtos_lock_acquire(0);
    status = device->interrupt_status;
    device->interrupt_status = 0;
    rtos_lock_release(0);

    return status;
}

static void device_to_dma(soc_peripheral_t device)
{
    void *rx_buf;
    transacting_chanend_t tc;
    int length;
    uint32_t total_length;
    int more;

    /*
     * In this case the device and RTOS are on different tiles, and the
     * device is sending data to the RTOS. The peripheral hub task has
     * already verified that the device's rx ring buffer descriptor list
     * exists and that the next descriptor points to a buffer that is ready
     * to be filled with data and handed off to the device. The data is read
     * from the device over a channel into the DMA buffers. The buffers are
     * then handed back over to the RTOS.
     */

    length = soc_dma_ring_buf_length_get(&device->rx_ring_buf);

    tc = chan_init_transaction_slave(device->rx_c);
    total_length = t_chan_in_word(&tc);
    xassert(total_length <= length);

    do {
        length = soc_dma_ring_buf_get(&device->rx_ring_buf, &rx_buf, &more);
        xassert(length >= 0);

        length = MIN(total_length, length);
        total_length -= length;

        t_chan_in_buf_byte(&tc, rx_buf, length);
        soc_dma_ring_buf_release(&device->rx_ring_buf, 1, length);
    } while (more);

    (void) chan_complete_transaction(tc);

    rtos_lock_acquire(0);
    device->interrupt_status |= SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM;
    rtos_lock_release(0);

    /* Do not interrupt the RTOS unless it has registered an ISR */
    if (device->core_id >= 0 && device->irq_source_id >= 0) {
        rtos_irq(device->core_id, device->irq_source_id);
    }
}

static void dma_to_device_ready(soc_peripheral_t device)
{
    chanend_out_end_token(device->tx_c);
    device->tx_ready = 1;
}

static void dma_to_device(soc_peripheral_t device)
{
    void *tx_buf;
    transacting_chanend_t tc;
    int length;
    int total_length;
    uint32_t max_length;
    int more;

    /*
     * In this case the device and RTOS are on different tiles, and the
     * device is receiving data from the RTOS. The peripheral hub task has
     * already verified that the device's tx ring buffer descriptor list
     * exists and that the next descriptor points to a buffer that has been
     * filled with data and handed off to the device. The data is read out
     * of the DMA buffers and sent to the device over a channel. The buffers
     * are then handed back over to the RTOS.
     */

    total_length = soc_dma_ring_buf_length_get(&device->tx_ring_buf);

    tc = chan_init_transaction_slave(device->tx_c);
    max_length = t_chan_in_word(&tc);
    xassert(total_length <= max_length);
    t_chan_out_word(&tc, total_length);

    do {
        length = soc_dma_ring_buf_get(&device->tx_ring_buf, &tx_buf, &more);
        xassert(length >= 0);
        total_length -= length;

        t_chan_out_buf_byte(&tc, tx_buf, length);
        soc_dma_ring_buf_release(&device->tx_ring_buf, 0, length);
    } while (more);

    xassert(total_length == 0);

    (void) chan_complete_transaction(tc);

    xassert(device->tx_ready);
    device->tx_ready = 0;

    rtos_lock_acquire(0);
    device->interrupt_status |= SOC_PERIPHERAL_ISR_DMA_TX_DONE_BM;
    rtos_lock_release(0);

    /* Do not interrupt the RTOS unless it has registered an ISR */
    if (device->core_id >= 0 && device->irq_source_id >= 0) {
        rtos_irq(device->core_id, device->irq_source_id);
    }
}

static void device_to_hub_irq(soc_peripheral_t device)
{
    uint32_t status;

    status = chan_in_word(device->irq_c);

    rtos_lock_acquire(0);
    device->interrupt_status |= status;
    rtos_lock_release(0);

    /* Do not interrupt the RTOS unless it has registered an ISR */
    if (device->core_id >= 0 && device->irq_source_id >= 0) {
        rtos_irq(device->core_id, device->irq_source_id);
    }
}

void soc_peripheral_hub()
{
    int i;

    rtos_irq_c = chanend_alloc();

    triggerable_disable_all();

    switch (peripheral_count)
    {
    default: xassert(0); break;
#if MAX_PERIPHERALS > 8
    case 9;
        if (peripherals[8].tx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[8].tx_c,  event_peri8_tx ); }
        if (peripherals[8].rx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[8].rx_c,  event_peri8_rx ); }
        if (peripherals[8].irq_c != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[8].irq_c, event_peri8_irq); }
#endif
#if MAX_PERIPHERALS > 7
    case 8:
        if (peripherals[7].tx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[7].tx_c,  event_peri7_tx ); }
        if (peripherals[7].rx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[7].rx_c,  event_peri7_rx ); }
        if (peripherals[7].irq_c != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[7].irq_c, event_peri7_irq); }
#endif
#if MAX_PERIPHERALS > 6
    case 7:
        if (peripherals[6].tx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[6].tx_c,  event_peri6_tx ); }
        if (peripherals[6].rx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[6].rx_c,  event_peri6_rx ); }
        if (peripherals[6].irq_c != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[6].irq_c, event_peri6_irq); }
#endif
#if MAX_PERIPHERALS > 5
    case 6:
        if (peripherals[5].tx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[5].tx_c,  event_peri5_tx ); }
        if (peripherals[5].rx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[5].rx_c,  event_peri5_rx ); }
        if (peripherals[5].irq_c != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[5].irq_c, event_peri5_irq); }
#endif
#if MAX_PERIPHERALS > 4
    case 5:
        if (peripherals[4].tx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[4].tx_c,  event_peri4_tx ); }
        if (peripherals[4].rx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[4].rx_c,  event_peri4_rx ); }
        if (peripherals[4].irq_c != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[4].irq_c, event_peri4_irq); }
#endif
#if MAX_PERIPHERALS > 3
    case 4:
        if (peripherals[3].tx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[3].tx_c,  event_peri3_tx ); }
        if (peripherals[3].rx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[3].rx_c,  event_peri3_rx ); }
        if (peripherals[3].irq_c != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[3].irq_c, event_peri3_irq); }
#endif
#if MAX_PERIPHERALS > 2
    case 3:
        if (peripherals[2].tx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[2].tx_c,  event_peri2_tx ); }
        if (peripherals[2].rx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[2].rx_c,  event_peri2_rx ); }
        if (peripherals[2].irq_c != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[2].irq_c, event_peri2_irq); }
#endif
#if MAX_PERIPHERALS > 1

    case 2:
        if (peripherals[1].tx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[1].tx_c,  event_peri1_tx ); }
        if (peripherals[1].rx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[1].rx_c,  event_peri1_rx ); }
        if (peripherals[1].irq_c != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[1].irq_c, event_peri1_irq); }
#endif
#if MAX_PERIPHERALS > 0
    case 1:
        if (peripherals[0].tx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[0].tx_c,  event_peri0_tx ); }
        if (peripherals[0].rx_c  != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[0].rx_c,  event_peri0_rx ); }
        if (peripherals[0].irq_c != 0) { TRIGGERABLE_SETUP_EVENT_VECTOR(peripherals[0].irq_c, event_peri0_irq); }
#endif
#if MAX_PERIPHERALS > 9
#error Can't have more than 9 peripherals
#endif
    case 0: break;
    }

    TRIGGERABLE_SETUP_EVENT_VECTOR(rtos_irq_c, event_irq_c);
    triggerable_enable_trigger(rtos_irq_c);

    /*
     * Should wait until all RTOS cores have enabled IRQs,
     * or else rtos_irq() could fail.
     */
    while (!rtos_irq_ready());

    for (;;) {
        int device_id;

        for (i = 0; i < peripheral_count; i++) {

            if (peripherals[i].tx_c != 0 && peripherals[i].tx_ring_buf.desc != NULL) {
                if (soc_dma_ring_buf_get(&peripherals[i].tx_ring_buf, NULL, NULL) >= 0) {
                    /*
                     * If there is a transmit channel and there is a transmit
                     * buffer available, then we can listen for a data request
                     * from the device.
                     */
//                    debug_printf("DMA to listen for data request from device %d\n", i);
                    triggerable_enable_trigger(peripherals[i].tx_c);

                    if (!peripherals[i].tx_ready) {
                        dma_to_device_ready(&peripherals[i]);
                    }
                }
            }

            if (peripherals[i].rx_c != 0 && peripherals[i].rx_ring_buf.desc != NULL) {
                if (soc_dma_ring_buf_get(&peripherals[i].rx_ring_buf, NULL, NULL) >= 0) {
                    /*
                     * If there is a receive channel and there is a receive
                     * buffer available, then we can listen for data from
                     * the device.
                     */
//                    debug_printf("DMA to listen for data from device %d\n", i);
                    triggerable_enable_trigger(peripherals[i].rx_c);
                }
            }

            if (peripherals[i].irq_c != 0) {
                triggerable_enable_trigger(peripherals[i].irq_c);
            }
        }

        TRIGGERABLE_WAIT_EVENT(
            event_peri0_tx, event_peri0_rx, event_peri0_irq,
            event_peri1_tx, event_peri1_rx, event_peri1_irq,
            event_peri2_tx, event_peri2_rx, event_peri2_irq,
            event_peri3_tx, event_peri3_rx, event_peri3_irq,
            event_peri4_tx, event_peri4_rx, event_peri4_irq,
            event_peri5_tx, event_peri5_rx, event_peri5_irq,
            event_peri6_tx, event_peri6_rx, event_peri6_irq,
            event_peri7_tx, event_peri7_rx, event_peri7_irq,
            event_peri8_tx, event_peri8_rx, event_peri8_irq,
            event_irq_c);

        while (0) {
        event_peri0_tx:  device_id = 0 * MAX_PERIPHERALS + 0; break;
        event_peri0_rx:  device_id = 1 * MAX_PERIPHERALS + 0; break;
        event_peri0_irq: device_id = 2 * MAX_PERIPHERALS + 0; break;
        event_peri1_tx:  device_id = 0 * MAX_PERIPHERALS + 1; break;
        event_peri1_rx:  device_id = 1 * MAX_PERIPHERALS + 1; break;
        event_peri1_irq: device_id = 2 * MAX_PERIPHERALS + 1; break;
        event_peri2_tx:  device_id = 0 * MAX_PERIPHERALS + 2; break;
        event_peri2_rx:  device_id = 1 * MAX_PERIPHERALS + 2; break;
        event_peri2_irq: device_id = 2 * MAX_PERIPHERALS + 2; break;
        event_peri3_tx:  device_id = 0 * MAX_PERIPHERALS + 3; break;
        event_peri3_rx:  device_id = 1 * MAX_PERIPHERALS + 3; break;
        event_peri3_irq: device_id = 2 * MAX_PERIPHERALS + 3; break;
        event_peri4_tx:  device_id = 0 * MAX_PERIPHERALS + 4; break;
        event_peri4_rx:  device_id = 1 * MAX_PERIPHERALS + 4; break;
        event_peri4_irq: device_id = 2 * MAX_PERIPHERALS + 4; break;
        event_peri5_tx:  device_id = 0 * MAX_PERIPHERALS + 5; break;
        event_peri5_rx:  device_id = 1 * MAX_PERIPHERALS + 5; break;
        event_peri5_irq: device_id = 2 * MAX_PERIPHERALS + 5; break;
        event_peri6_tx:  device_id = 0 * MAX_PERIPHERALS + 6; break;
        event_peri6_rx:  device_id = 1 * MAX_PERIPHERALS + 6; break;
        event_peri6_irq: device_id = 2 * MAX_PERIPHERALS + 6; break;
        event_peri7_tx:  device_id = 0 * MAX_PERIPHERALS + 7; break;
        event_peri7_rx:  device_id = 1 * MAX_PERIPHERALS + 7; break;
        event_peri7_irq: device_id = 2 * MAX_PERIPHERALS + 7; break;
        event_peri8_tx:  device_id = 0 * MAX_PERIPHERALS + 8; break;
        event_peri8_rx:  device_id = 1 * MAX_PERIPHERALS + 8; break;
        event_peri8_irq: device_id = 2 * MAX_PERIPHERALS + 8; break;
        event_irq_c:     device_id = 3 * MAX_PERIPHERALS;     break;
        }

        do {
            if (device_id < peripheral_count) {
                /* The device is trying to receive data */

                triggerable_disable_trigger(peripherals[device_id].tx_c);
                dma_to_device(&peripherals[device_id]);

            } else if ((device_id - 1 * MAX_PERIPHERALS) < peripheral_count) {
                /* The device is trying to send data */

                /*
                 * Select wait returns 1 * MAX_DEVICES + device_id when
                 * data is received on the RX channel, so convert to
                 * the actual device ID.
                 */
                device_id -= 1 * MAX_PERIPHERALS;

                triggerable_disable_trigger(peripherals[device_id].rx_c);
                device_to_dma(&peripherals[device_id]);

            } else if ((device_id - 2 * MAX_PERIPHERALS) < peripheral_count) {
                /* The device is trying to send an IRQ */

                /*
                 * Select wait returns 2 * MAX_DEVICES + device_id when
                 * data is received on the IRQ channel, so convert to
                 * the actual device ID.
                 */
                device_id -= 2 * MAX_PERIPHERALS;

                triggerable_disable_trigger(peripherals[device_id].irq_c);
                device_to_hub_irq(&peripherals[device_id]);

            } else if (device_id == 3 * MAX_PERIPHERALS) {
                /* request from the RTOS */

                /*
                 * An RTOS task has added a new DMA buffer so wake up
                 * here so we go back to the start of the loop to make sure
                 * we are waiting on the right channels.
                 */

                chanend_check_end_token(rtos_irq_c);
            }

            TRIGGERABLE_TAKE_EVENT(
              event_peri0_tx, event_peri0_rx, event_peri0_irq,
              event_peri1_tx, event_peri1_rx, event_peri1_irq,
              event_peri2_tx, event_peri2_rx, event_peri2_irq,
              event_peri3_tx, event_peri3_rx, event_peri3_irq,
              event_peri4_tx, event_peri4_rx, event_peri4_irq,
              event_peri5_tx, event_peri5_rx, event_peri5_irq,
              event_peri6_tx, event_peri6_rx, event_peri6_irq,
              event_peri7_tx, event_peri7_rx, event_peri7_irq,
              event_peri8_tx, event_peri8_rx, event_peri8_irq,
              event_irq_c);
        } while (0);
    }
}

DEFINE_INTERRUPT_CALLBACK(rtos_isr, soc_peripheral_hub_isr, id)
{
    int i;

//    for (;;) {
        int device_id = (int) id;

		if (device_id < peripheral_count) {
			/* The device is trying to receive data */

			triggerable_disable_trigger(peripherals[device_id].tx_c);
			dma_to_device(&peripherals[device_id]);

		} else if ((device_id - 1 * MAX_PERIPHERALS) < peripheral_count) {
			/* The device is trying to send data */

			/*
			 * Select wait returns 1 * MAX_DEVICES + device_id when
			 * data is received on the RX channel, so convert to
			 * the actual device ID.
			 */
			device_id -= 1 * MAX_PERIPHERALS;

			triggerable_disable_trigger(peripherals[device_id].rx_c);
			device_to_dma(&peripherals[device_id]);

		} else if ((device_id - 2 * MAX_PERIPHERALS) < peripheral_count) {
			/* The device is trying to send an IRQ */

			/*
			 * Select wait returns 2 * MAX_DEVICES + device_id when
			 * data is received on the IRQ channel, so convert to
			 * the actual device ID.
			 */
			device_id -= 2 * MAX_PERIPHERALS;

			triggerable_disable_trigger(peripherals[device_id].irq_c);
			device_to_hub_irq(&peripherals[device_id]);

		} else if (device_id == 3 * MAX_PERIPHERALS) {
			/* request from the RTOS */

			/*
			 * An RTOS task has added a new DMA buffer so wake up
			 * here so we go back to the start of the loop to make sure
			 * we are waiting on the right channels.
			 */

			chanend_check_end_token(rtos_irq_c);
		}

		/*
		 * TODO: ROUND ROBIN IS PROBABLY BROKEN.
		 * TRY TO FIGURE OUT HOW TO ACHEIVE IT WITH INTERRUPTS.
		 */

        for (i = 0; i < peripheral_count; i++) {

            if (peripherals[i].tx_c != 0 && peripherals[i].tx_ring_buf.desc != NULL) {
                if (soc_dma_ring_buf_get(&peripherals[i].tx_ring_buf, NULL, NULL) >= 0) {
                    /*
                     * If there is a transmit channel and there is a transmit
                     * buffer available, then we can listen for a data request
                     * from the device.
                     */
//                    debug_printf("DMA to listen for data request from device %d\n", i);
                    triggerable_enable_trigger(peripherals[i].tx_c);

                    if (!peripherals[i].tx_ready) {
                        dma_to_device_ready(&peripherals[i]);
                    }
                }
            }

            if (peripherals[i].rx_c != 0 && peripherals[i].rx_ring_buf.desc != NULL) {
                if (soc_dma_ring_buf_get(&peripherals[i].rx_ring_buf, NULL, NULL) >= 0) {
                    /*
                     * If there is a receive channel and there is a receive
                     * buffer available, then we can listen for data from
                     * the device.
                     */
//                    debug_printf("DMA to listen for data from device %d\n", i);
                    triggerable_enable_trigger(peripherals[i].rx_c);
                }
            }

            if (peripherals[i].irq_c != 0) {
                triggerable_enable_trigger(peripherals[i].irq_c);
            }
        }
//    }
}

void soc_peripheral_hub_isr_setup(void)
//DEFINE_INTERRUPT_PERMITTED(rtos_isr, void, soc_peripheral_hub_isr_setup)
{
    int i;

    rtos_irq_c = chanend_alloc();

    for (i = 0; i < peripheral_count; i++) {
        if (peripherals[i].tx_c  != 0) {
            triggerable_setup_interrupt_callback(peripherals[i].tx_c,  0 * MAX_PERIPHERALS + i, INTERRUPT_CALLBACK(soc_peripheral_hub_isr));
        }
        if (peripherals[i].rx_c  != 0) {
            triggerable_setup_interrupt_callback(peripherals[i].rx_c,  1 * MAX_PERIPHERALS + i, INTERRUPT_CALLBACK(soc_peripheral_hub_isr));
        }
        if (peripherals[i].irq_c != 0) {
            triggerable_setup_interrupt_callback(peripherals[i].irq_c, 2 * MAX_PERIPHERALS + i, INTERRUPT_CALLBACK(soc_peripheral_hub_isr));
        }
    }

    triggerable_setup_interrupt_callback(rtos_irq_c, 3 * MAX_PERIPHERALS, INTERRUPT_CALLBACK(soc_peripheral_hub_isr));
    triggerable_enable_trigger(rtos_irq_c);

    /*
     * Should wait until all RTOS cores have enabled IRQs,
     * or else rtos_irq() could fail.
     */
//    while (!rtos_irq_ready());

    rtos_printf("ENABLING RTOS IRQ INTERRUPT\n");

//    rtos_interrupt_unmask_all();
//    for (;;);
}

