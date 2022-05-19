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
#include "uart_test_common.h"


#define NUM_RX_WORDS    4

port_t p_uart_tx = XS1_PORT_1A;
port_t p_uart_rx = XS1_PORT_1B;

volatile unsigned bytes_received = 0;
volatile unsigned test_abort = 0;

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
        case UART_FRAMING_ERROR:
            printstrln("UART_FRAMING_ERROR");
            test_abort = 1;
            break;
        case UART_OVERRUN_ERROR:
            printstrln("UART_OVERRUN_ERROR");
            break;
        case UART_UNDERRUN_ERROR:
            printstrln("UART_UNDERRUN_ERROR");
            break;
        case UART_RX_COMPLETE:
            bytes_received += 1;
            break;
    }
}


DEFINE_INTERRUPT_PERMITTED(UART_RX_INTERRUPTABLE_FUNCTIONS, void, test, void){
    uart_rx_t uart;
    hwtimer_t tmr = hwtimer_alloc();

    char buffer[64];
    char test_rx[NUM_RX_WORDS];

    // printf("UART setting: %d %d %d %d\n", TEST_BAUD, TEST_DATA_BITS, TEST_PARITY, TEST_STOP_BITS);


#if TEST_BUFFER
    uart_rx_init(&uart, p_uart_rx, TEST_BAUD, TEST_DATA_BITS, TEST_PARITY, TEST_STOP_BITS, tmr,
        buffer, sizeof(buffer), rx_callback);
#else
    uart_rx_init(&uart, p_uart_rx, TEST_BAUD, TEST_DATA_BITS, TEST_PARITY, TEST_STOP_BITS, tmr,
        NULL, 0, rx_callback);
#endif

    //Tester waits until it can see the tx_port driven to idle by other task

#if TEST_BUFFER
    while(bytes_received < NUM_RX_WORDS && !test_abort);
#endif

    for(int i = 0; i < NUM_RX_WORDS; i++){
        test_rx[i] = uart_rx(&uart);
    }
    for(int i = 0; i < NUM_RX_WORDS; i++){
        printf("0x%02x\n", test_rx[i]);
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

DECLARE_JOB(start_sim, (void));
void start_sim(void){
    hwtimer_t tmr = hwtimer_alloc();
    uint32_t time = hwtimer_get_time(tmr);
    hwtimer_wait_until(tmr, time + 200); //2us - enough to allow uart_rx to be entered
    port_enable(p_uart_tx);
    port_out(p_uart_tx, 0);
    //Tester will now transmit the bytes
    hwtimer_free(tmr);
    port_disable(p_uart_tx);
    burn();
}




int main(void) {
    PAR_JOBS (
        PJOB(INTERRUPT_PERMITTED(test), ()),
        PJOB(start_sim, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ())
    );
    return 0;
}
