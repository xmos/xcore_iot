// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <stdbool.h>

/* Platform headers */
#include "soc.h"
#include "xcore_utils.h"
#include "uart.h"

/* App headers */
#include "app_conf.h"
#include "app_demos.h"

static bool logged_error = false;

void uart_rx_demo(uart_rx_t* uart_rx_ctx)
{
    uint8_t expected = 0;

    while(1) {
        uint8_t rx = uart_rx(uart_rx_ctx);
        if(rx != expected && !logged_error){
            debug_printf("UART data error, expected: %d got: %d\n", expected, rx);
            debug_printf("Further UART errors will NOT be printed. Have you connected pins X1D36 and X1D39?\n\n");
            logged_error = true;
        }
        expected++;
    }
}


void uart_tx_demo(uart_tx_t* uart_tx_ctx)
{
    debug_printf("Starting tx ramp test @ %ubaud..\n", XS1_TIMER_HZ / uart_tx_ctx->bit_time_ticks);

    uint8_t tx_data = 0;
    uint32_t time_now = get_reference_time();
    while(get_reference_time() < (time_now + 100000)); // Wait for a millisecond

    while(1) {
        for(int i=0; i<256; i++){
            uart_tx(uart_tx_ctx, tx_data);
            tx_data+=1;
        }
        uint32_t time_now = get_reference_time();
        while(get_reference_time() < (time_now + 10000000)); // Wait for 100 milliseconds
        // debug_printf("Sent: %u\n", tx_data);
    }   
}
