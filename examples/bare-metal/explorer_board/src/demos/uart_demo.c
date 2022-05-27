// Copyright 2021 XMOS LIMITED.
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

void uart_rx_demo(uart_rx_t* uart_rx_ctx)
{
    uint8_t expected = 0;
    hwtimer_t tmr = hwtimer_alloc();

    while(1) {
        uint8_t rx = uart_rx(uart_rx_ctx);
        if(rx != expected){
            debug_printf("UART data error, expected: %d got: %d\n", expected, rx);
        }
        expected++;
        hwtimer_delay(tmr, 20); //Max slippage delay 200ns
    }
}


void uart_tx_demo(uart_tx_t* uart_tx_ctx)
{
    uint8_t tx_data = 0;
    debug_printf("Starting tx ramp test @ %ubaud..\n", XS1_TIMER_HZ / uart_tx_ctx->bit_time_ticks);

    hwtimer_t tmr = hwtimer_alloc();
    hwtimer_delay(tmr, XS1_TIMER_KHZ); // Wait for a millisecond

    while(1) {
        uart_tx(uart_tx_ctx, tx_data);
        tx_data+=1;
    }   
}
