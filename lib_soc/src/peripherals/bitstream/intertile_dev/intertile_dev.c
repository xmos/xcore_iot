// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include "rtos_support.h"
#include "xcore_c.h"
#include "soc.h"

#include "intertile_dev.h"

typedef enum {
    EVENT_SLAVE_TO_MASTER,
    EVENT_CONTROL_CHANNEL
} event_choice_t;

void intertile_dev(
        soc_peripheral_t peripheral,
        chanend m_ctrl_c,
        chanend s_data_to_dma_c,
        chanend s_data_from_dma_c)
{
    uint32_t cmd;
    uint8_t buf[INTERTILE_DEV_BUFSIZE];
    int len;

    select_disable_trigger_all();

    chanend_setup_select( s_data_to_dma_c, EVENT_SLAVE_TO_MASTER );
    chanend_enable_trigger( s_data_to_dma_c );

    chanend_setup_select( m_ctrl_c, EVENT_CONTROL_CHANNEL );
    chanend_enable_trigger( m_ctrl_c );

    for( ;; )
    {
        int event_id;

        event_id = select_wait();

        do {
            if( peripheral != NULL )
            {
                len = soc_peripheral_rx_dma_direct_xfer(peripheral, buf, sizeof(buf));

                if (len > 0)
                {
                    if (s_data_from_dma_c != 0)
                    {
                        soc_peripheral_tx_dma_xfer(s_data_from_dma_c, buf, len);
                    }
                }
            }

            switch( event_id )
            {
                case EVENT_CONTROL_CHANNEL:
                {
                    soc_peripheral_function_code_rx(m_ctrl_c, &cmd);
                    switch( cmd )
                    {
                    case SOC_PERIPHERAL_DMA_TX:
                        /*
                         * The application has added a new DMA TX buffer. This
                         * ensures that this select statement wakes up and gets
                         * the TX data in the code above.
                         */
                        break;
                    default:
                        rtos_printf( "Invalid CMD\n" );
                        break;
                    }
                    break;
                }
                case EVENT_SLAVE_TO_MASTER:
                {
                    soc_peripheral_rx_dma_ready(s_data_to_dma_c);
                    len = soc_peripheral_rx_dma_xfer(s_data_to_dma_c, buf, sizeof(buf));

                    if (peripheral != 0) {
                        soc_peripheral_tx_dma_direct_xfer(peripheral, buf, len);
                    }
                    break;
                }
                default:
                {
                    rtos_printf( "Invalid event\n" );
                    xassert(0);
                    break;
                }
            }

            event_id = select_no_wait( -1 );
        } while( event_id != -1 );
    }
}
