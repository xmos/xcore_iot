// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* SDK headers */
#include "soc.h"
#include "xcore_utils.h"
#include "uart.h"

/* App headers */
#include "app_conf.h"
#include "app_demos.h"

#include <xcore/port.h>
#include <xcore/hwtimer.h>

void uart_demo(uart_tx_t* uart)
{
    char tx_buff[] = "U\0U"; //0x55

    port_t p_uart_tx = WIFI_CLK;
    hwtimer_t tmr = hwtimer_alloc();


    uart_tx_init(uart, p_uart_tx, 115200, 8, UART_PARITY_NONE, 1);

    for(int i=0; i<sizeof(tx_buff); i++){
        uart_tx(uart, tx_buff[i]);
        debug_printf("uart sent 0x%x (%c)\n", tx_buff[i], tx_buff[i]);

    }
   

    exit(0);
}
