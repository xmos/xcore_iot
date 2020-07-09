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
    XS1_PORT_4A, XS1_PORT_4B, XS1_PORT_4C, XS1_PORT_4D, XS1_PORT_4E, XS1_PORT_4F,
    XS1_PORT_8A, XS1_PORT_8B, XS1_PORT_8C, XS1_PORT_8D,
    XS1_PORT_16A, XS1_PORT_16B, XS1_PORT_16C, //XS1_PORT_16D,
    //XS1_PORT_32A, XS1_PORT_32B
};

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

    uint32_t gpio_port_trigger_value[ GPIO_TOTAL_PORT_CNT ];
    uint32_t port_irq_flags = 0;


    TRIGGERABLE_SETUP_EVENT_VECTOR(ctrl_c, event_ctrl);

    triggerable_disable_all();
    triggerable_enable_trigger( ctrl_c );

    for( ;; )
    {
        TRIGGERABLE_WAIT_EVENT(
        	event_gpio_1A,  event_gpio_1B,  event_gpio_1C,  event_gpio_1D,  event_gpio_1E,
			event_gpio_1F,  event_gpio_1G,  event_gpio_1H,  event_gpio_1I,  event_gpio_1J,
			event_gpio_1K,  event_gpio_1L,  event_gpio_1M,  event_gpio_1N,  event_gpio_1O,
			event_gpio_1P,  event_gpio_4A,  event_gpio_4B,  event_gpio_4C,  event_gpio_4D,
			event_gpio_4E,  event_gpio_4F,  event_gpio_8A,  event_gpio_8B,  event_gpio_8C,
			event_gpio_8D,  event_gpio_16A, event_gpio_16B, event_gpio_16C,
         // event_gpio_16D, event_gpio_32A, event_gpio_32B,
            event_ctrl);
        {
            while (0) { /* This loop is just a hacky way to let us use break to go to the generic bit... */
            event_gpio_1A:  gpio_id = gpio_1A;  break;
            event_gpio_1B:  gpio_id = gpio_1B;  break;
            event_gpio_1C:  gpio_id = gpio_1C;  break;
            event_gpio_1D:  gpio_id = gpio_1D;  break;
            event_gpio_1E:  gpio_id = gpio_1E;  break;
            event_gpio_1F:  gpio_id = gpio_1F;  break;
            event_gpio_1G:  gpio_id = gpio_1G;  break;
            event_gpio_1H:  gpio_id = gpio_1H;  break;
            event_gpio_1I:  gpio_id = gpio_1I;  break;
            event_gpio_1J:  gpio_id = gpio_1J;  break;
            event_gpio_1K:  gpio_id = gpio_1K;  break;
            event_gpio_1L:  gpio_id = gpio_1L;  break;
            event_gpio_1M:  gpio_id = gpio_1M;  break;
            event_gpio_1N:  gpio_id = gpio_1N;  break;
            event_gpio_1O:  gpio_id = gpio_1O;  break;
            event_gpio_1P:  gpio_id = gpio_1P;  break;
            event_gpio_4A:  gpio_id = gpio_4A;  break;
            event_gpio_4B:  gpio_id = gpio_4B;  break;
            event_gpio_4C:  gpio_id = gpio_4C;  break;
            event_gpio_4D:  gpio_id = gpio_4D;  break;
            event_gpio_4E:  gpio_id = gpio_4E;  break;
            event_gpio_4F:  gpio_id = gpio_4F;  break;
            event_gpio_8A:  gpio_id = gpio_8A;  break;
            event_gpio_8B:  gpio_id = gpio_8B;  break;
            event_gpio_8C:  gpio_id = gpio_8C;  break;
            event_gpio_8D:  gpio_id = gpio_8D;  break;
            event_gpio_16A: gpio_id = gpio_16A; break;
            event_gpio_16B: gpio_id = gpio_16B; break;
            event_gpio_16C: gpio_id = gpio_16C; break;
          //event_gpio_16D: gpio_id = gpio_16D; break;
          //event_gpio_32A: gpio_id = gpio_32A; break;
          //event_gpio_32B: gpio_id = gpio_32B; break;
            }
            xassert(( gpio_id >= 0 ) && ( gpio_id < GPIO_TOTAL_PORT_CNT ));
            mask = ( 0x1 << gpio_id );

            port_res = get_port( gpio_id );

            gpio_port_trigger_value[ gpio_id ] = port_in( port_res );
            triggerable_disable_trigger( port_res );
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
                    data = gpio_port_trigger_value[ gpio_id ];
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
                case gpio_1A:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1A ); break;
                case gpio_1B:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1B ); break;
                case gpio_1C:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1C ); break;
                case gpio_1D:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1D ); break;
                case gpio_1E:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1E ); break;
                case gpio_1F:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1F ); break;
                case gpio_1G:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1G ); break;
                case gpio_1H:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1H ); break;
                case gpio_1I:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1I ); break;
                case gpio_1J:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1J ); break;
                case gpio_1K:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1K ); break;
                case gpio_1L:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1L ); break;
                case gpio_1M:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1M ); break;
                case gpio_1N:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1N ); break;
                case gpio_1O:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1O ); break;
                case gpio_1P:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_1P ); break;
                case gpio_4A:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_4A ); break;
                case gpio_4B:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_4B ); break;
                case gpio_4C:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_4C ); break;
                case gpio_4D:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_4D ); break;
                case gpio_4E:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_4E ); break;
                case gpio_4F:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_4F ); break;
                case gpio_8A:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_8A ); break;
                case gpio_8B:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_8B ); break;
                case gpio_8C:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_8C ); break;
                case gpio_8D:  TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_8D ); break;
                case gpio_16A: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_16A); break;
                case gpio_16B: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_16B); break;
                case gpio_16C: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_16C); break;
              //case gpio_16D: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_16D); break;
              //case gpio_32A: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_32A); break;
              //case gpio_32B: TRIGGERABLE_SETUP_EVENT_VECTOR(get_port(gpio_id), event_gpio_32B); break;
                default: xassert(0); break;
                }

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

