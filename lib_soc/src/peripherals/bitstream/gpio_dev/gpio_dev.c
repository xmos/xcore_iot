// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <string.h>

#include "rtos_support.h"
#include "xcore_c.h"
#include "soc.h"

#include "gpio_dev.h"

static const port gpio_lookup[ GPIO_TOTAL_PORT_CNT ] =
{
    port_1A, port_1B, port_1C, port_1D, port_1E, port_1F, port_1G,
    port_1H, port_1I, port_1J, port_1K, port_1L, port_1M, port_1N,
    port_1O, port_1P,
    port_4A, port_4B, port_4C, port_4D, port_4E, port_4F,
    port_8A, port_8B, port_8C, port_8D,
    port_16A, port_16B, port_16C, port_16D,
    port_32A, port_32B
};

static uint32_t port_irq_flags;

static port get_port( gpio_id_t gpio_id )
{
    return gpio_lookup[ gpio_id ];
}

void gpio_dev(
        soc_peripheral_t peripheral,
        chanend data_to_dma_c,
        chanend data_from_dma_c,
        chanend ctrl_c,
        chanend irq_c)
{
    gpio_id_t gpio_id;
    int retval_int;
    port port_res;
    uint32_t cmd;
    uint32_t data;
    uint32_t mask;

    select_disable_trigger_all();
    chanend_setup_select( ctrl_c, GPIO_TOTAL_PORT_CNT );
    chanend_enable_trigger( ctrl_c );

    //while( !rtos_irq_ready() );

    for( ;; )
    {
        int event_id;

        event_id = select_wait();

        /* event_id 0-31 are ports, 32 is control channel rx */
        do {
            if( ( event_id >= 0 ) && ( event_id < GPIO_TOTAL_PORT_CNT ) )
            {
                mask = ( 0x1 << event_id );
                port_disable_trigger( get_port(event_id) );
                port_irq_flags |= mask;
                if ( irq_c != 0 )
                {
                    soc_peripheral_irq_send( irq_c, mask );
                }
                else if ( peripheral != NULL )
                {
                    soc_peripheral_irq_direct_send( peripheral, mask );
                }
            }
            else if( event_id == GPIO_TOTAL_PORT_CNT )
            {
                soc_peripheral_function_code_rx(ctrl_c, &cmd);

                switch( cmd )
                {
                case GPIO_DEV_PORT_ALLOC:
                    soc_peripheral_varlist_rx(
                            ctrl_c, 1,
                            sizeof(gpio_id), &gpio_id);

                    port_res = get_port( gpio_id );
                    retval_int = port_alloc( &port_res, (port_id_t)port_res );

                    soc_peripheral_varlist_tx(
                            ctrl_c, 1,
                            sizeof(retval_int), &retval_int);
                    break;

                case GPIO_DEV_PORT_FREE:
                    soc_peripheral_varlist_rx(
                            ctrl_c, 1,
                            sizeof(gpio_id), &gpio_id);

                    port_res = get_port( gpio_id );
                    retval_int = port_free( &port_res );

                    soc_peripheral_varlist_tx(
                            ctrl_c, 1,
                            sizeof(retval_int), &retval_int);
                    break;

                case GPIO_DEV_PORT_IN:
                    soc_peripheral_varlist_rx(
                            ctrl_c, 1,
                            sizeof(gpio_id), &gpio_id);

                    port_res = get_port( gpio_id );
                    mask = ( 0x1 << gpio_id );

                    if( ( port_irq_flags & mask ) != 0 )
                    {
                        port_irq_flags &= ~mask;
                        retval_int = port_in( port_res, &data );
                        port_set_trigger_in_not_equal( port_res, data );
                        port_enable_trigger( port_res );
                    }
                    else
                    {
                        retval_int = port_peek( port_res, &data );
                    }

                    soc_peripheral_varlist_tx(
                            ctrl_c, 2,
                            sizeof(data), &data,
                            sizeof(retval_int), &retval_int);
                    break;

                case GPIO_DEV_PORT_OUT:
                    soc_peripheral_varlist_rx(
                            ctrl_c, 2,
                            sizeof(gpio_id), &gpio_id,
                            sizeof(data), &data);

                    port_res = get_port( gpio_id );

                    retval_int = port_out( port_res, data );

                    soc_peripheral_varlist_tx(
                            ctrl_c, 1,
                            sizeof(retval_int), &retval_int);
                    break;

                case GPIO_DEV_PORT_PEEK:
                    soc_peripheral_varlist_rx(
                            ctrl_c, 1,
                            sizeof(gpio_id), &gpio_id);

                    port_res = get_port( gpio_id );

                    retval_int = port_peek( port_res, &data );

                    soc_peripheral_varlist_tx(
                            ctrl_c, 2,
                            sizeof(data), &data,
                            sizeof(retval_int), &retval_int);
                    break;

                case GPIO_DEV_PORT_IRQ_SETUP:
                    soc_peripheral_varlist_rx(
                            ctrl_c, 1,
                            sizeof(gpio_id), &gpio_id);

                    port_res = get_port( gpio_id );

                    retval_int = port_setup_select( port_res, gpio_id );

                    soc_peripheral_varlist_tx(
                            ctrl_c, 1,
                            sizeof(retval_int), &retval_int);
                    break;

                case GPIO_DEV_PORT_IRQ_ENABLE:
                    soc_peripheral_varlist_rx(
                            ctrl_c, 1,
                            sizeof(gpio_id), &gpio_id);

                    port_res = get_port( gpio_id );

                    port_peek( port_res, &data );
                    port_set_trigger_in_not_equal( port_res, data );
                    retval_int = port_enable_trigger( port_res );

                    soc_peripheral_varlist_tx(
                            ctrl_c, 1,
                            sizeof(retval_int), &retval_int);
                    break;

                case GPIO_DEV_PORT_IRQ_DISABLE:
                    soc_peripheral_varlist_rx(
                            ctrl_c, 1,
                            sizeof(gpio_id), &gpio_id);

                    port_res = get_port( gpio_id );

                    retval_int = port_disable_trigger( port_res );

                    soc_peripheral_varlist_tx(
                            ctrl_c, 1,
                            sizeof(retval_int), &retval_int);
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

