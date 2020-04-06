// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <xcore/triggerable.h>
#include <xcore/chanend.h>

#include "rtos_support.h"
#include "soc.h"

#include "intertile_dev.h"

typedef enum {
    EVENT_SLAVE_TO_MASTER,
    EVENT_CONTROL_CHANNEL
} event_choice_t;

void intertile_dev(
        soc_peripheral_t peripheral,
        chanend m_ctrl_c,
        chanend s_data_from_dma_c,
        chanend s_data_to_dma_c)
{
    uint32_t cmd;
    uint8_t buf[INTERTILE_DEV_BUFSIZE];
    int len;

    TRIGGERABLE_SETUP_EVENT_VECTOR(m_ctrl_c, event_ctrl);
    TRIGGERABLE_SETUP_EVENT_VECTOR(s_data_from_dma_c, event_data_from_remote_tile);

    triggerable_disable_all();
    triggerable_enable_trigger(s_data_from_dma_c);
    triggerable_enable_trigger(m_ctrl_c);

    for (;;) {

		if (peripheral != NULL) {
			len = soc_peripheral_rx_dma_direct_xfer(peripheral, buf, sizeof(buf));

			if (len > 0) {
				if (s_data_to_dma_c != 0) {
					soc_peripheral_tx_dma_xfer(s_data_to_dma_c, buf, len);
				}
			}
		}

		TRIGGERABLE_WAIT_EVENT(event_ctrl, event_data_from_remote_tile);

		event_ctrl: {
			soc_peripheral_function_code_rx(m_ctrl_c, &cmd);
			switch( cmd ) {
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
			continue;
		}

		event_data_from_remote_tile: {
			soc_peripheral_rx_dma_ready(s_data_from_dma_c);
			len = soc_peripheral_rx_dma_xfer(s_data_from_dma_c, buf, sizeof(buf));

			if (peripheral != 0) {
				soc_peripheral_tx_dma_direct_xfer(peripheral, buf, len);
			}
			continue;
		}
    }
}
