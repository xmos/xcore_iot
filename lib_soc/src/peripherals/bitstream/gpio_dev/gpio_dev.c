// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <string.h>

#include "rtos_support.h"
#include "xcore_c.h"

#include "soc_peripheral_control.h"

#include "gpio_dev.h"

static int event_count;
static port port_list[ MAX_GPIO_EVENTS ];

void gpio_dev(
        chanend data_to_dma_c,
        chanend data_from_dma_c,
        chanend ctrl_c,
        chanend irq_c)
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

