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
#include <xcore/interrupt_wrappers.h>


volatile int uart_working = 1;

UART_CALLBACK_ATTR void uart_callback(uart_callback_t callback_info){
    debug_printf("ISR callback 0x%x\n", callback_info);
    uart_working = 0;
}


DEFINE_INTERRUPT_PERMITTED(uart_isr_grp, void, uart_demo, uart_tx_t* uart)
{
    char tx_msg[] = "\x55\0\xaa"; //U = 0x55
    // char tx_msg[] = {0x55};

    port_t p_uart_tx = WIFI_CLK;
    hwtimer_t tmr = hwtimer_alloc();
    // tmr = 0;

    char tx_buff[64];

    uart_tx_init(uart, p_uart_tx, 115200, 8, UART_PARITY_NONE, 1,
                tmr, tx_buff, sizeof(tx_buff), uart_callback);

    uart_tx_blocking_init(uart, p_uart_tx, 921600, 8, UART_PARITY_NONE, 1, tmr);
    uart_tx_blocking_init(uart, p_uart_tx, 1843200, 8, UART_PARITY_NONE, 1, tmr);
    uart_tx_blocking_init(uart, p_uart_tx, 2764800, 8, UART_PARITY_NONE, 1, tmr);


    for(int i=0; i<sizeof(tx_msg); i++){
        uart_tx(uart, tx_msg[i]);
        debug_printf("uart sent 0x%x (%c)\n", tx_msg[i], tx_msg[i]);

    }
    
    while(uart_working);
    exit(0);
}
