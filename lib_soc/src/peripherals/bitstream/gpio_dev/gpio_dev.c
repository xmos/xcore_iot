// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <string.h>

#include <xcore/port.h>
#include <xcore/triggerable.h>

#include "rtos_support.h"
#include "soc.h"

#include "gpio_dev.h"

static const port gpio_lookup[ GPIO_TOTAL_PORT_CNT ] =
{
    XS1_PORT_1A, XS1_PORT_1B, XS1_PORT_1C, XS1_PORT_1D, XS1_PORT_1E, XS1_PORT_1F, XS1_PORT_1G,
    XS1_PORT_1H, XS1_PORT_1I, XS1_PORT_1J, XS1_PORT_1K, XS1_PORT_1L, XS1_PORT_1M, XS1_PORT_1N,
    XS1_PORT_1O, XS1_PORT_1P,
    XS1_PORT_4A, XS1_PORT_4B, XS1_PORT_4C, //XS1_PORT_4D, XS1_PORT_4E, XS1_PORT_4F,
    XS1_PORT_8A, XS1_PORT_8B, XS1_PORT_8C, XS1_PORT_8D,
    XS1_PORT_16A, XS1_PORT_16B, XS1_PORT_16C, XS1_PORT_16D,
    XS1_PORT_32A, XS1_PORT_32B
};

static uint32_t port_irq_flags;

static port get_port( gpio_id_t gpio_id )
{
    return gpio_lookup[ gpio_id ];
}

void gpio_dev(
        soc_peripheral_t peripheral,
        chanend_t data_to_dma_c,
        chanend_t data_from_dma_c,
        chanend_t ctrl_c,
        chanend_t irq_c)
{
    gpio_id_t gpio_id;
    int retval_int;
    port_t port_res;
    uint32_t cmd;
    uint32_t data;
    uint32_t mask;


    TRIGGERABLE_SETUP_EVENT_VECTOR(ctrl_c, event_ctrl);

    triggerable_disable_all();
    triggerable_enable_trigger( ctrl_c );

    for( ;; )
    {
        TRIGGERABLE_WAIT_EVENT(
            event_with_id_0,  event_with_id_1,  event_with_id_2,  event_with_id_3,  event_with_id_4,
            event_with_id_5,  event_with_id_6,  event_with_id_7,  event_with_id_8,  event_with_id_9,
            event_with_id_10, event_with_id_11, event_with_id_12, event_with_id_13, event_with_id_14,
            event_with_id_15, event_with_id_16, event_with_id_17, event_with_id_18, event_with_id_19, 
            event_with_id_20, event_with_id_21, event_with_id_22, event_with_id_23, event_with_id_24,
            event_with_id_25, event_with_id_26, event_with_id_27, event_with_id_28,
            \\ event_with_id_, event_with_id_, event_with_id_, 
            event_ctrl);
        
        {
            int event_id;
            while (0) { // This loop is just a hacky way to let us use break to go to the generic bit...
            event_with_id_0:  event_id = 0;  break; // XS1_PORT_1A
            event_with_id_1:  event_id = 1;  break; // XS1_PORT_1B
            event_with_id_2:  event_id = 2;  break; // XS1_PORT_1C
            event_with_id_3:  event_id = 3;  break; // XS1_PORT_1D
            event_with_id_4:  event_id = 4;  break; // XS1_PORT_1E
            event_with_id_5:  event_id = 5;  break; // XS1_PORT_1F
            event_with_id_6:  event_id = 6;  break; // XS1_PORT_1G
            event_with_id_7:  event_id = 7;  break; // XS1_PORT_1H
            event_with_id_8:  event_id = 8;  break; // XS1_PORT_1I
            event_with_id_9:  event_id = 9;  break; // XS1_PORT_1J
            event_with_id_10: event_id = 10; break; // XS1_PORT_1K
            event_with_id_11: event_id = 11; break; // XS1_PORT_1L
            event_with_id_12: event_id = 12; break; // XS1_PORT_1M
            event_with_id_13: event_id = 13; break; // XS1_PORT_1N
            event_with_id_14: event_id = 14; break; // XS1_PORT_1O
            event_with_id_15: event_id = 15; break; // XS1_PORT_1P
            event_with_id_16: event_id = 16; break; // XS1_PORT_4A
            event_with_id_17: event_id = 17; break; // XS1_PORT_4B
            event_with_id_18: event_id = 18; break; // XS1_PORT_4C
          //event_with_id_: event_id = ; break; // XS1_PORT_4D
          //event_with_id_: event_id = ; break; // XS1_PORT_4E
          //event_with_id_: event_id = ; break; // XS1_PORT_4F
            event_with_id_19: event_id = 19; break; // XS1_PORT_8A
            event_with_id_20: event_id = 20; break; // XS1_PORT_8B
            event_with_id_21: event_id = 21; break; // XS1_PORT_8C
            event_with_id_22: event_id = 22; break; // XS1_PORT_8D
            event_with_id_23: event_id = 23; break; // XS1_PORT_16A
            event_with_id_24: event_id = 24; break; // XS1_PORT_16B
            event_with_id_25: event_id = 25; break; // XS1_PORT_16C
            event_with_id_26: event_id = 26; break; // XS1_PORT_16D
            event_with_id_27: event_id = 27; break; // XS1_PORT_32A
            event_with_id_28: event_id = 28; break; // XS1_PORT_32B
            } 
            xassert(( event_id >= 0 ) && ( event_id < GPIO_TOTAL_PORT_CNT ));
            mask = ( 0x1 << event_id );
            triggerable_disable_trigger( get_port(event_id) );
            port_irq_flags |= mask;
            if ( irq_c != 0 )
            {
                soc_peripheral_irq_send( irq_c, mask );
            }
            else if ( peripheral != NULL )
            {
                soc_peripheral_irq_direct_send( peripheral, mask );
            }
            continue;
        }

        {
        event_ctrl:
            soc_peripheral_function_code_rx(ctrl_c, &cmd);

            switch( cmd )
            {
            case GPIO_DEV_PORT_ALLOC:
                soc_peripheral_varlist_rx(
                        ctrl_c, 1,
                        sizeof(gpio_id), &gpio_id);

                port_res = get_port( gpio_id );
                port_enable( port_res );
                retval_int = 0;

                soc_peripheral_varlist_tx(
                        ctrl_c, 1,
                        sizeof(retval_int), &retval_int);
                break;

            case GPIO_DEV_PORT_FREE:
                soc_peripheral_varlist_rx(
                        ctrl_c, 1,
                        sizeof(gpio_id), &gpio_id);

                port_res = get_port( gpio_id );
                port_disable( port_res );
                retval_int = 0;

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
                    data = port_in( port_res );
                    port_set_trigger_in_not_equal( port_res, data );
                    triggerable_enable_trigger( port_res );
                }
                else
                {
                    data = port_peek( port_res );
                }
                retval_int = 0;

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

                port_out( port_res, data );
                retval_int = 0;

                soc_peripheral_varlist_tx(
                        ctrl_c, 1,
                        sizeof(retval_int), &retval_int);
                break;

            case GPIO_DEV_PORT_PEEK:
                soc_peripheral_varlist_rx(
                        ctrl_c, 1,
                        sizeof(gpio_id), &gpio_id);

                port_res = get_port( gpio_id );

                data = port_peek( port_res);
                retval_int = 0;

                soc_peripheral_varlist_tx(
                        ctrl_c, 2,
                        sizeof(data), &data,
                        sizeof(retval_int), &retval_int);
                break;

            case GPIO_DEV_PORT_IRQ_SETUP:
                soc_peripheral_varlist_rx(
                        ctrl_c, 1,
                        sizeof(gpio_id), &gpio_id);

                // port_res = get_port( gpio_id );

                switch (gpio_id)
                {
                case 0:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(0),  event_with_id_0 ); break;
                case 1:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(1),  event_with_id_1 ); break;
                case 2:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(2),  event_with_id_2 ); break;
                case 3:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(3),  event_with_id_3 ); break;
                case 4:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(4),  event_with_id_4 ); break;
                case 5:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(5),  event_with_id_5 ); break;
                case 6:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(6),  event_with_id_6 ); break;
                case 7:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(7),  event_with_id_7 ); break;
                case 8:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(8),  event_with_id_8 ); break;
                case 9:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(9),  event_with_id_9 ); break;
                case 10: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(10), event_with_id_10); break;
                case 11: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(11), event_with_id_11); break;
                case 12: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(12), event_with_id_12); break;
                case 13: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(13), event_with_id_13); break;
                case 14: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(14), event_with_id_14); break;
                case 15: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(15), event_with_id_15); break;
                case 16: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(16), event_with_id_16); break;
                case 17: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(17), event_with_id_17); break;
                case 18: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(18), event_with_id_18); break;
                case 19: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(19), event_with_id_19); break;
                case 20: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(20), event_with_id_20); break;
                case 21: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(21), event_with_id_21); break;
                case 22: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(22), event_with_id_22); break;
                case 23: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(23), event_with_id_23); break;
                case 24: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(24), event_with_id_24); break;
                case 25: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(25), event_with_id_25); break;
                case 26: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(26), event_with_id_26); break;
                case 27: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(27), event_with_id_27); break;
                case 28: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(28), event_with_id_28); break;
                // case 29: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(29), event_with_id_29); break;
                // case 30: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(30), event_with_id_30); break;
                // case 31: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(31), event_with_id_31); break;
                default: xassert(0); break;
                }
                // retval_int = port_setup_select( port_res, gpio_id );

                retval_int = 0;

                soc_peripheral_varlist_tx(
                        ctrl_c, 1,
                        sizeof(retval_int), &retval_int);
                break;

            case GPIO_DEV_PORT_IRQ_ENABLE:
                soc_peripheral_varlist_rx(
                        ctrl_c, 1,
                        sizeof(gpio_id), &gpio_id);

                port_res = get_port( gpio_id );

                data = port_peek( port_res );
                port_set_trigger_in_not_equal( port_res, data );
                triggerable_enable_trigger( port_res );
                retval_int = 0;

                soc_peripheral_varlist_tx(
                        ctrl_c, 1,
                        sizeof(retval_int), &retval_int);
                break;

            case GPIO_DEV_PORT_IRQ_DISABLE:
                soc_peripheral_varlist_rx(
                        ctrl_c, 1,
                        sizeof(gpio_id), &gpio_id);

                port_res = get_port( gpio_id );

                triggerable_disable_trigger( port_res );
                retval_int = 0;

                /* In case IRQ disable is called between an interrupt
                 * firing and the ISR reading the port, this prevents
                 * the read from re-enabling the interrupt. */
                mask = ( 0x1 << gpio_id );
                port_irq_flags &= ~mask;

                soc_peripheral_varlist_tx(
                        ctrl_c, 1,
                        sizeof(retval_int), &retval_int);
                break;

            default:
                fail( "Invalid CMD" );
                break;
            }
            continue;
        }
    }
}

