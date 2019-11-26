/*
 * dma_task.xc
 *
 *  Created on: Sep 24, 2019
 *      Author: mbruno
 */

#include "FreeRTOS.h"
#include "task.h"

#include "xcore_freertos_dma.h"

#include "xassert.h"

#define MAX_DEVICES 8

/*
 * The channel end used by the DMA controller to
 * interrupt FreeRTOS and to receive requests
 * from FreeRTOS.
 */
static chanend freertos_ici_c;

typedef struct xcore_freertos_device {

    /* This device's ID */
    int id;

    /* Channel used to send data to this device. */
    chanend tx_c;

    /* Channel used to receive data from this device. */
    chanend rx_c;

    /* Channel used for control of this device. */
    chanend control_c;

    /* DMA has begun sending data to this device. */
    uint32_t tx_ready;

    /* Application specific data */
    void *app_data;

    /* The core that handles the interrupts */
    int core_id;

    /* The FreeRTOS intercore interrupt source ID number for this device. */
    int ici_source_id;

    /* Interrupt status for FreeRTOS */
    uint32_t interrupt_status;

    xcore_freertos_dma_ring_buf_t tx_ring_buf;
    xcore_freertos_dma_ring_buf_t rx_ring_buf;

} dev_info_t;

static dev_info_t device_info[MAX_DEVICES];

static int device_count;

/* To be called by the bitstream */
xcore_freertos_device_t xcore_freertos_dma_device_register(
        chanend c[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT])
{
    int device_id;

    xassert(device_count < MAX_DEVICES);

    device_id = device_count++;

    device_info[device_id].id = device_id;
    device_info[device_id].tx_c = c[XCORE_FREERTOS_DMA_DATA_TO_DEVICE_CH];
    device_info[device_id].rx_c = c[XCORE_FREERTOS_DMA_DATA_FROM_DEVICE_CH];
    device_info[device_id].control_c = c[XCORE_FREERTOS_DMA_DEVICE_CONTROL_CH];
    device_info[device_id].tx_ready = 0;
    device_info[device_id].ici_source_id = -1;
    device_info[device_id].app_data = NULL;
    device_info[device_id].interrupt_status = 0;

    return &device_info[device_id];
}

void xcore_freertos_dma_device_rx_ready(
        chanend c)
{
    s_chan_check_ct_end(c);
}

uint16_t xcore_freertos_dma_device_rx_data(
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

void xcore_freertos_dma_device_tx_data(
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

/* To be called by FreeRTOS */
void xcore_freertos_dma_device_handler_register(
        xcore_freertos_device_t device,
        int core_id,
        void *app_data,
        rtos_irq_isr_t isr)
{
    device->core_id = core_id;
    device->app_data = app_data;
    device->ici_source_id = rtos_irq_register(isr, device, freertos_ici_c);
}

void *xcore_freertos_dma_device_app_data(
        xcore_freertos_device_t device)
{
    return device->app_data;
}

xcore_freertos_dma_ring_buf_t *xcore_freertos_dma_device_rx_ring_buf(
        xcore_freertos_device_t device)
{
    return &device->rx_ring_buf;
}

xcore_freertos_dma_ring_buf_t *xcore_freertos_dma_device_tx_ring_buf(
        xcore_freertos_device_t device)
{
    return &device->tx_ring_buf;
}

chanend xcore_freertos_dma_device_ctrl_chanend(
        xcore_freertos_device_t device)
{
    return device->control_c;
}

/* To be called by FreeRTOS */
void xcore_freertos_dma_request(void)
{
    rtos_irq_peripheral(freertos_ici_c);
}

/* To be called by FreeRTOS */
uint32_t xcore_freertos_dma_interrupt_status(
        xcore_freertos_device_t device)
{
    uint32_t status;

    rtos_lock_acquire(0);
    status = device->interrupt_status;
    device->interrupt_status = 0;
    rtos_lock_release(0);

    return status;
}

static void device_to_dma(xcore_freertos_device_t device)
{
    void *rx_buf;
    transacting_chanend_t tc;
    int max_length;
    uint32_t length;

    rx_buf = xcore_freertos_dma_ring_buf_get_next(&device->rx_ring_buf, &max_length, NULL);
    xassert(rx_buf != NULL);

    chan_init_transaction_slave(&device->rx_c, &tc);
    t_chan_in_word(&tc, &length);
    xassert(length <= max_length);
    t_chan_in_buf_byte(&tc, rx_buf, length);
    chan_complete_transaction(&device->rx_c, &tc);

    xcore_freertos_dma_ring_buf_mark_done(&device->rx_ring_buf, XCORE_FREERTOS_DMA_RING_BUF_RX_DONE, length);
}

static void dma_to_device_ready(xcore_freertos_device_t device)
{
    s_chan_out_ct_end(device->tx_c);
    device->tx_ready = 1;
}

static void dma_to_device(xcore_freertos_device_t device)
{
    void *tx_buf;
    transacting_chanend_t tc;
    int length;
    uint32_t max_length;
    int more;

    tx_buf = xcore_freertos_dma_ring_buf_get_next(&device->tx_ring_buf, &length, &more);
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

    xcore_freertos_dma_ring_buf_mark_done(&device->tx_ring_buf, XCORE_FREERTOS_DMA_RING_BUF_TX_DONE, length);
}

/* To be called by FreeRTOS */
void xcore_freertos_dma_device_common_init(
        xcore_freertos_device_t device,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr)
{
    xcore_freertos_dma_ring_buf_t *ring_buf;
    uint32_t *buf_desc;
    int i;

#ifdef taskVALID_CORE_ID
    xassert(taskVALID_CORE_ID(isr_core));
#endif

    if (rx_desc_count > 0) {
        buf_desc = pvPortMalloc(rx_desc_count * XCORE_FREERTOS_DMA_RING_BUF_DESC_WORDSIZE * sizeof(uint32_t));
        xassert(buf_desc != NULL);

        ring_buf = xcore_freertos_dma_device_rx_ring_buf(device);
        xcore_freertos_dma_ring_buf_desc_init(ring_buf, buf_desc, rx_desc_count);

        if (rx_buf_size > 0) {
            for (i = 0; i < rx_desc_count; i++) {
                void *buf = pvPortMalloc(rx_buf_size);
                xassert(buf != NULL);
                xcore_freertos_dma_ring_buf_rx_desc_set_buf(ring_buf, buf, rx_buf_size);
            }
        }
    }

    if (tx_desc_count > 0) {
        buf_desc = pvPortMalloc(tx_desc_count * XCORE_FREERTOS_DMA_RING_BUF_DESC_WORDSIZE * sizeof(uint32_t));
        xassert(buf_desc != NULL);
        ring_buf = xcore_freertos_dma_device_tx_ring_buf(device);
        xcore_freertos_dma_ring_buf_desc_init(ring_buf, buf_desc, tx_desc_count);
    }

    xcore_freertos_dma_device_handler_register(device, isr_core, app_data, isr);
}

/*
 * Must be run on a tile with FreeRTOS
 */
void xcore_freertos_dma_task()
{
    int i;

    chanend_alloc(&freertos_ici_c);

    select_disable_trigger_all();

    for (i = 0; i < device_count; i++) {
        if (device_info[i].tx_c != 0) {
            chanend_setup_select(device_info[i].tx_c, 0 + i);
        }
        if (device_info[i].rx_c != 0) {
            chanend_setup_select(device_info[i].rx_c, MAX_DEVICES + i);
        }
    }

    chanend_setup_select(freertos_ici_c, 2 * MAX_DEVICES);
    chanend_enable_trigger(freertos_ici_c);

    /*
     * Should wait until all RTOS cores have enabled IRQs,
     * or else rtos_irq() could fail.
     */
    while (!rtos_irq_ready());

    for (;;) {
        int device_id;

        for (i = 0; i < device_count; i++) {

            if (device_info[i].tx_c != 0 && device_info[i].tx_ring_buf.desc != NULL) {
                if (xcore_freertos_dma_ring_buf_get_next(&device_info[i].tx_ring_buf, NULL, NULL) != NULL) {
                    /*
                     * If there is a transmit channel and there is a transmit
                     * buffer available, then we can listen for a data request
                     * from the device.
                     */
//                    debug_printf("DMA to listen for data request from device %d\n", i);
                    chanend_enable_trigger(device_info[i].tx_c);

                    if (!device_info[i].tx_ready) {
                        dma_to_device_ready(&device_info[i]);
                    }
                }
            }

            if (device_info[i].rx_c != 0 && device_info[i].rx_ring_buf.desc != NULL) {
                if (xcore_freertos_dma_ring_buf_get_next(&device_info[i].rx_ring_buf, NULL, NULL) != NULL) {
                    /*
                     * If there is a receive channel and there is a receive
                     * buffer available, then we can listen for data from
                     * the device.
                     */
//                    debug_printf("DMA to listen for data from device %d\n", i);
                    chanend_enable_trigger(device_info[i].rx_c);
                }
            }
        }

        device_id = select_wait();

        do {
            if (device_id < device_count) {
                /* Detected that the device is trying to receive */
                chanend_disable_trigger(device_info[device_id].tx_c);
                dma_to_device(&device_info[device_id]);

                rtos_lock_acquire(0);
                device_info[device_id].interrupt_status |= XCORE_FREERTOS_DMA_INTERRUPT_STATUS_TX_DONE;
                rtos_lock_release(0);

                rtos_irq(device_info[device_id].core_id, device_info[device_id].ici_source_id);

            } else if ((device_id - MAX_DEVICES) < device_count) {
                /* Got data on the device's RX channel */

                /*
                 * Select wait returns MAX_DEVICES + device_id when
                 * data is received on the RX channel, so convert to
                 * the actual device ID.
                 */
                device_id -= MAX_DEVICES;

                chanend_disable_trigger(device_info[device_id].rx_c);

                device_to_dma(&device_info[device_id]);

                rtos_lock_acquire(0);
                device_info[device_id].interrupt_status |= XCORE_FREERTOS_DMA_INTERRUPT_STATUS_RX_DONE;
                rtos_lock_release(0);

                rtos_irq(device_info[device_id].core_id, device_info[device_id].ici_source_id);

            } else if (device_id == 2 * MAX_DEVICES) {
                /* request from freertos */

                /*
                 * A FreeRTOS task has added a new DMA buffer so wake up
                 * here so we go back to the start of the loop to make sure
                 * we are waiting on the right channels.
                 */
                s_chan_check_ct_end(freertos_ici_c);
            }

            device_id = select_no_wait(-1);
        } while (device_id != -1);
    }
}
