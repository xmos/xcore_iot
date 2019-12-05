// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc_peripheral_hub.h"

#include "xassert.h"

#define MAX_PERIPHERALS 8

void *soc_dma_ring_buf_get(
        soc_dma_ring_buf_t *ring_buf,
        int *length,
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
static chanend rtos_irq_c;

typedef struct soc_peripheral {

    /* This device's ID */
    int id;

    /* Channel used to send data to this device. */
    chanend tx_c;

    /* Channel used to receive data from this device. */
    chanend rx_c;

    /* Channel used for control of this device. */
    chanend control_c;

    /* Channel used for receiving interrupt requests from this device. */
    chanend irq_c;

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
        chanend c[SOC_PERIPHERAL_CHANNEL_COUNT])
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
    peripherals[device_id].irq_source_id = -1;
    peripherals[device_id].app_data = NULL;
    peripherals[device_id].interrupt_status = 0;

    return &peripherals[device_id];
}

void soc_peripheral_rx_dma_ready(
        chanend c)
{
    s_chan_check_ct_end(c);
}

uint16_t soc_peripheral_rx_dma_xfer(
        chanend c,
        void *data,
        uint16_t max_length)
{
    transacting_chanend_t tc;
    uint32_t length;

    chan_init_transaction_master(&c, &tc);
    t_chan_out_word(&tc, max_length);
    t_chan_in_word(&tc, &length);
    xassert(length <= max_length);
    t_chan_in_buf_byte(&tc, data, length);
    chan_complete_transaction(&c, &tc);

    return (int) length;
}

void soc_peripheral_tx_dma_xfer(
        chanend c,
        void *data,
        uint16_t length)
{
    transacting_chanend_t tc;

    chan_init_transaction_master(&c, &tc);
    t_chan_out_word(&tc, length);
    t_chan_out_buf_byte(&tc, data, length);
    chan_complete_transaction(&c, &tc);
}

void soc_peripheral_irq_send(
        chanend c,
        uint32_t status)
{
    chan_out_word(c, status);
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

void soc_peripheral_hub_dma_request(void)
{
    rtos_irq_peripheral(rtos_irq_c);
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
    int max_length;
    uint32_t length;

    rx_buf = soc_dma_ring_buf_get(&device->rx_ring_buf, &max_length, NULL);
    xassert(rx_buf != NULL);

    chan_init_transaction_slave(&device->rx_c, &tc);
    t_chan_in_word(&tc, &length);
    xassert(length <= max_length);
    t_chan_in_buf_byte(&tc, rx_buf, length);
    chan_complete_transaction(&device->rx_c, &tc);

    soc_dma_ring_buf_release(&device->rx_ring_buf, 1, length);
}

static void dma_to_device_ready(soc_peripheral_t device)
{
    s_chan_out_ct_end(device->tx_c);
    device->tx_ready = 1;
}

static void dma_to_device(soc_peripheral_t device)
{
    void *tx_buf;
    transacting_chanend_t tc;
    int length;
    uint32_t max_length;
    int more;

    tx_buf = soc_dma_ring_buf_get(&device->tx_ring_buf, &length, &more);
//    debug_printf("got tx buf 0x%x of length %d\n", tx_buf, length);
    xassert(tx_buf != NULL);

    /****** Temporary *******/
    xassert(more == 0);
    /************************/

    chan_init_transaction_slave(&device->tx_c, &tc);
    t_chan_in_word(&tc, &max_length);
    xassert(length <= max_length);
    t_chan_out_word(&tc, length);
    t_chan_out_buf_byte(&tc, tx_buf, length);
    chan_complete_transaction(&device->tx_c, &tc);

    device->tx_ready = 0;

    soc_dma_ring_buf_release(&device->tx_ring_buf, 0, length);
}

static uint32_t device_to_hub_irq(soc_peripheral_t device)
{
    uint32_t status;

    chan_in_word(device->irq_c, &status);

    return status;
}

void soc_peripheral_hub()
{
    int i;

    chanend_alloc(&rtos_irq_c);

    select_disable_trigger_all();

    for (i = 0; i < peripheral_count; i++) {
        if (peripherals[i].tx_c != 0) {
            chanend_setup_select(peripherals[i].tx_c, 0 * MAX_PERIPHERALS + i);
        }
        if (peripherals[i].rx_c != 0) {
            chanend_setup_select(peripherals[i].rx_c, 1 * MAX_PERIPHERALS + i);
        }
        if (peripherals[i].irq_c != 0) {
            chanend_setup_select(peripherals[i].irq_c, 2 * MAX_PERIPHERALS + i);
        }
    }

    chanend_setup_select(rtos_irq_c, 3 * MAX_PERIPHERALS);
    chanend_enable_trigger(rtos_irq_c);

    /*
     * Should wait until all RTOS cores have enabled IRQs,
     * or else rtos_irq() could fail.
     */
    while (!rtos_irq_ready());

    for (;;) {
        int device_id;

        for (i = 0; i < peripheral_count; i++) {

            if (peripherals[i].tx_c != 0 && peripherals[i].tx_ring_buf.desc != NULL) {
                if (soc_dma_ring_buf_get(&peripherals[i].tx_ring_buf, NULL, NULL) != NULL) {
                    /*
                     * If there is a transmit channel and there is a transmit
                     * buffer available, then we can listen for a data request
                     * from the device.
                     */
//                    debug_printf("DMA to listen for data request from device %d\n", i);
                    chanend_enable_trigger(peripherals[i].tx_c);

                    if (!peripherals[i].tx_ready) {
                        dma_to_device_ready(&peripherals[i]);
                    }
                }
            }

            if (peripherals[i].rx_c != 0 && peripherals[i].rx_ring_buf.desc != NULL) {
                if (soc_dma_ring_buf_get(&peripherals[i].rx_ring_buf, NULL, NULL) != NULL) {
                    /*
                     * If there is a receive channel and there is a receive
                     * buffer available, then we can listen for data from
                     * the device.
                     */
//                    debug_printf("DMA to listen for data from device %d\n", i);
                    chanend_enable_trigger(peripherals[i].rx_c);
                }
            }

            if (peripherals[i].irq_c != 0) {
                chanend_enable_trigger(peripherals[i].irq_c);
            }
        }

        device_id = select_wait();

        do {
            if (device_id < peripheral_count) {
                /* Detected that the device is trying to receive */
                chanend_disable_trigger(peripherals[device_id].tx_c);
                dma_to_device(&peripherals[device_id]);

                rtos_lock_acquire(0);
                peripherals[device_id].interrupt_status |= SOC_PERIPHERAL_ISR_DMA_TX_DONE_BM;
                rtos_lock_release(0);

                rtos_irq(peripherals[device_id].core_id, peripherals[device_id].irq_source_id);

            } else if ((device_id - 1 * MAX_PERIPHERALS) < peripheral_count) {
                /* Got data on the device's RX channel */

                /*
                 * Select wait returns 1 * MAX_DEVICES + device_id when
                 * data is received on the RX channel, so convert to
                 * the actual device ID.
                 */
                device_id -= 1 * MAX_PERIPHERALS;

                chanend_disable_trigger(peripherals[device_id].rx_c);

                device_to_dma(&peripherals[device_id]);

                rtos_lock_acquire(0);
                peripherals[device_id].interrupt_status |= SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM;
                rtos_lock_release(0);

                rtos_irq(peripherals[device_id].core_id, peripherals[device_id].irq_source_id);

            } else if ((device_id - 2 * MAX_PERIPHERALS) < peripheral_count) {
                uint32_t status;

                /* Got data on the device's IRQ channel */

                /*
                 * Select wait returns 2 * MAX_DEVICES + device_id when
                 * data is received on the IRQ channel, so convert to
                 * the actual device ID.
                 */
                device_id -= 2 * MAX_PERIPHERALS;

                status = device_to_hub_irq(&peripherals[device_id]);

                rtos_lock_acquire(0);
                peripherals[device_id].interrupt_status |= status;
                rtos_lock_release(0);

                rtos_irq(peripherals[device_id].core_id, peripherals[device_id].irq_source_id);

            } else if (device_id == 3 * MAX_PERIPHERALS) {
                /* request from the RTOS */

                /*
                 * An RTOS task has added a new DMA buffer so wake up
                 * here so we go back to the start of the loop to make sure
                 * we are waiting on the right channels.
                 */
                s_chan_check_ct_end(rtos_irq_c);
            }

            device_id = select_no_wait(-1);
        } while (device_id != -1);
    }
}
