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

#include "uart_test_common.h"

volatile unsigned tx_empty = 0;

UART_CALLBACK_ATTR void tx_callback(uart_callback_t callback_info){
    switch(callback_info){
        case UART_TX_EMPTY:
            tx_empty = 1;
            break;
        case UART_START_BIT_ERROR:
            printstrln("UART_START_BIT_ERROR");
            break;
        case UART_PARITY_ERROR:
            printstrln("UART_PARITY_ERROR");
            break;
        case UART_FRAMING_ERROR:
            printstrln("UART_FRAMING_ERROR");
            break;
        case UART_OVERRUN_ERROR:
            printstrln("UART_OVERRUN_ERROR");
            break;
        case UART_UNDERRUN_ERROR:
            printstrln("UART_UNDERRUN_ERROR");
            break;
        case UART_RX_COMPLETE:
            printstrln("UART_UNDERRUN_ERROR");
            break;
    }
}

port_t p_uart_tx = XS1_PORT_1A;


DEFINE_INTERRUPT_PERMITTED(UART_TX_INTERRUPTABLE_FUNCTIONS, void, test, void){
    uint8_t tx_data[] = {0xff, 0x00, 0x08, 0x55};

    uint8_t buffer[64];

    uart_tx_t uart;
    hwtimer_t tmr = hwtimer_alloc();
    // printf("UART setting: %d %d %d %d\n", TEST_BAUD, TEST_DATA_BITS, TEST_PARITY, TEST_STOP_BITS);

#if TEST_BUFFER
    uart_tx_init(&uart, p_uart_tx, TEST_BAUD, TEST_DATA_BITS, TEST_PARITY, TEST_STOP_BITS, tmr, buffer, sizeof(buffer), tx_callback);
#else
    uart_tx_blocking_init(&uart, p_uart_tx, TEST_BAUD, TEST_DATA_BITS, TEST_PARITY, TEST_STOP_BITS, tmr);
#endif

    for(int i = 0; i < sizeof(tx_data); i++){
        uart_tx(&uart, tx_data[i]);
    }

#if TEST_BUFFER
    while(!tx_empty);
#endif

    uart_tx_deinit(&uart);

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
        PJOB(INTERRUPT_PERMITTED(test), ()),
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
