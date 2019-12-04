// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <string.h>

#include "rtos_support.h"
#include "xcore_c.h"

#include "soc_peripheral_control.h"

#include "gpio_dev.h"

static int event_count;
static port port_list[ MAX_GPIO_EVENTS ];

void gpio_dev_c(
        chanend data_to_dma_c,
        chanend data_from_dma_c,
        chanend ctrl_c)
{
    int i;
    int port_id;
    unsigned port_ptr;
    unsigned port_arg;
    uint32_t cmd;
    uint32_t data;

    memset( port_list, 0, sizeof(port)*MAX_GPIO_EVENTS );

    select_disable_trigger_all();
    chanend_setup_select( ctrl_c, MAX_GPIO_EVENTS );
    chanend_enable_trigger( ctrl_c );

    while( !rtos_irq_ready() );

    for( ;; )
    {
        int event_id;

        for( i = 0; i < event_count; i++)
        {
            ;   // TODO: setup select on all ports that were setup for triggering
        }

        event_id = select_wait();

        /* event_id 0-31 are ports, 32 is control channel rx */
        do {
            if( ( event_id >= 0 ) && ( event_id < MAX_GPIO_EVENTS ) )
            {
                ;   // TODO: communicate interrupt on port event_id
            }
            else if( event_id == MAX_GPIO_EVENTS )
            {
                soc_peripheral_function_code_rx(ctrl_c, &cmd);

                switch( cmd )
                {
                case GPIO_DEV_PORT_ALLOC:
                    soc_peripheral_varlist_rx(
                            ctrl_c, 2,
                            sizeof(port_ptr), &port_ptr,
                            sizeof(port_id), &port_id);

                    port_alloc( (port*)&port_ptr, (port_id_t)port_id );

                    soc_peripheral_varlist_tx(
                            ctrl_c, 1,
                            sizeof(port_ptr), &port_ptr);
                    break;

                case GPIO_DEV_PORT_FREE:
                    soc_peripheral_varlist_rx(
                            ctrl_c, 1,
                            sizeof(port_ptr), &port_ptr);

                    port_free( (port*)&port_ptr );
                    break;

                case GPIO_DEV_PORT_PEEK:
                    soc_peripheral_varlist_rx(
                            ctrl_c, 1,
                            sizeof(port_arg), &port_arg);

                    port_peek( (port)port_arg, &data );

                    soc_peripheral_varlist_tx(
                            ctrl_c, 1,
                            sizeof(data), &data);
                    break;

                case GPIO_DEV_PORT_IN:
                    soc_peripheral_varlist_rx(
                            ctrl_c, 1,
                            sizeof(port_arg), &port_arg);

                    port_in( (port)port_arg, &data );

                    soc_peripheral_varlist_tx(
                            ctrl_c, 1,
                            sizeof(data), &data);
                    break;

                case GPIO_DEV_PORT_OUT:
                    soc_peripheral_varlist_rx(
                            ctrl_c, 2,
                            sizeof(port_arg), &port_arg,
                            sizeof(data), &data);

                    port_out( (port)port_arg, data );
                    break;

                default:
                    fail( "Invalid CMD" );
                    break;
                }
            }

            event_id = select_no_wait( -1 );
        } while( event_id != -1 );
    }
}

































///*
// * The channel end used by the gpio hub to
// * interrupt the RTOS and to receive requests
// * from the RTOS.
// */
//static chanend rtos_gpio_irq_c;
//
///* device checks ctrl channel
// * adds port events
// * waits
// * */
//

//void gpio_dev()
//{
//    int i;
//    int isr_gpio_count = 0;
//
//    chanend_alloc(&rtos_gpio_irq_c);
//
//    select_disable_trigger_all();
//
//    chanend_setup_select(rtos_gpio_irq_c, 2);
//    chanend_enable_trigger(rtos_gpio_irq_c);
//
//    /*
//     * Should wait until all RTOS cores have enabled IRQs,
//     * or else rtos_irq() could fail.
//     */
//    while (!rtos_irq_ready());
//
//    for (;;) {
//        int device_id;
//
//        device_id = select_wait();
//
//        do {
//            if (device_id < peripheral_count) {
//                /* Detected that the device is trying to receive */
//                chanend_disable_trigger(peripherals[device_id].tx_c);
//                dma_to_device(&peripherals[device_id]);
//
//                rtos_lock_acquire(0);
//                peripherals[device_id].interrupt_status |= SOC_PERIPHERAL_ISR_DMA_TX_DONE_BM;
//                rtos_lock_release(0);
//
//                rtos_irq(peripherals[device_id].core_id, peripherals[device_id].irq_source_id);
//
//            } else if ((device_id - MAX_PERIPHERALS) < peripheral_count) {
//                /* Got data on the device's RX channel */
//
//                /*
//                 * Select wait returns MAX_DEVICES + device_id when
//                 * data is received on the RX channel, so convert to
//                 * the actual device ID.
//                 */
//                device_id -= MAX_PERIPHERALS;
//
//                chanend_disable_trigger(peripherals[device_id].rx_c);
//
//                device_to_dma(&peripherals[device_id]);
//
//                rtos_lock_acquire(0);
//                peripherals[device_id].interrupt_status |= SOC_PERIPHERAL_ISR_DMA_RX_DONE_BM;
//                rtos_lock_release(0);
//
//                rtos_irq(peripherals[device_id].core_id, peripherals[device_id].irq_source_id);
//
//            } else if (device_id == 2 * MAX_GPIO_EVENTS) {
//                /* request from the RTOS */
//
//                /*
//                 * An RTOS task has added a new DMA buffer so wake up
//                 * here so we go back to the start of the loop to make sure
//                 * we are waiting on the right channels.
//                 */
//                s_chan_check_ct_end(rtos_irq_c);
//            }
//
//            device_id = select_no_wait(-1);
//        } while (device_id != -1);
//    }
//}
