// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <stdio.h>
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

port_t p_uart_rx = XS1_PORT_1B;
port_t p_uart_tx = XS1_PORT_1A;

DECLARE_JOB(test, (void));

volatile data_ready = 0;

void rx_callback(uart_callback_t callback_info){
    data_ready = 1;
}

void test() {
    uart_rx_t uart;
    hwtimer_t tmr = hwtimer_alloc();

    char buffer[64];

    //Tester waits until it can see the tx_port driven to idle
    port_enable(p_uart_tx);
    port_out(p_uart_tx, 1); //Set to idle


    uart_rx_init(&uart, p_uart_rx, 921600, 8, UART_PARITY_NONE, 1, tmr,
        buffer, sizeof(buffer), rx_callback);

    for(int i = 0; i < NUM_RX_WORDS; i++){
        while(!data_ready);
    }

    for(int i = 0; i < NUM_RX_WORDS; i++){
        printf("0x%x\n", uart_rx(&uart));
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
        PJOB(test, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ())
    );
    return 0;
}
