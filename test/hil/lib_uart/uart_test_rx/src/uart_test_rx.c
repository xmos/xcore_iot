// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <stdio.h>
#include <print.h>
#include <string.h>
#include <stdlib.h>
#include <xcore/parallel.h>
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include <xcore/triggerable.h>
#include <xcore/interrupt.h>
#include <xcore/interrupt_wrappers.h>

#include "uart.h"

#define SETSR(c) asm volatile("setsr %0" : : "n"(c));
#define NUM_RX_WORDS    4

port_t p_uart_tx = XS1_PORT_1A;
port_t p_uart_rx = XS1_PORT_1B;

volatile unsigned bytes_received = 0;

UART_CALLBACK_ATTR void rx_callback(uart_callback_t callback_info){
    switch(callback_info){
        case UART_TX_EMPTY:
            printstrln("UART_TX_EMPTY");
            break;
        case UART_START_BIT_ERROR:
            printstrln("UART_START_BIT_ERROR");
            break;
        case UART_PARITY_ERROR:
            printstrln("UART_PARITY_ERROR");
            break;
        case UART_STOP_BIT_ERROR:
            printstrln("UART_STOP_BIT_ERROR");
            break;
        case UART_RX_COMPLETE:
            bytes_received += 1;
            break;
    }
}


DEFINE_INTERRUPT_PERMITTED(UART_INTERRUPTABLE_FUNCTIONS, void, test, void){
    uart_rx_t uart;
    hwtimer_t tmr = hwtimer_alloc();

    char buffer[64];

    uart_rx_init(&uart, p_uart_rx, 115200, 8, UART_PARITY_NONE, 1, tmr,
        buffer, sizeof(buffer), rx_callback);

    //Tester waits until it can see the tx_port driven to idle
    port_enable(p_uart_tx);
    port_out(p_uart_tx, 1); //Set to drive idle

    // uart_rx_init(&uart, p_uart_rx, 115200, 8, UART_PARITY_NONE, 1, tmr,
    //     NULL, 0, NULL);

    //Tester will now transmit the bytes

    while(bytes_received < NUM_RX_WORDS);
 
    for(int i = 0; i < NUM_RX_WORDS; i++){
        printf("0x%02x\n", uart_rx(&uart));
    }

    uart_rx_deinit(&uart);
    hwtimer_free(tmr);

    exit(0);
}

DECLARE_JOB(burn, (void));

void burn(void) {
    SETSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);
    for(;;);
}

int main(void) {
    PAR_JOBS (
        PJOB(INTERRUPT_PERMITTED(test), ())

        // PJOB(INTERRUPT_PERMITTED(test), ()),
        // PJOB(burn, ()),
        // PJOB(burn, ()),
        // PJOB(burn, ()),
        // PJOB(burn, ()),
        // PJOB(burn, ()),
        // PJOB(burn, ()),
        // PJOB(burn, ())
    );
    return 0;
}
