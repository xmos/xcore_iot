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


port_t p_uart_tx = XS1_PORT_1A;

DECLARE_JOB(test, (void));


void test() {
    uint8_t tx_data[] = {0xff, 0x00, 0x08, 0x55};

    uart_tx_t uart;
    hwtimer_t tmr = hwtimer_alloc();
    // printf("UART setting: %d %d %d %d\n", TEST_BAUD, TEST_DATA_BITS, TEST_PARITY, TEST_STOP_BITS);

#if TEST_USE_BUFFERED
#else
    uart_tx_blocking_init(&uart, p_uart_tx, TEST_BAUD, TEST_DATA_BITS, TEST_PARITY, TEST_STOP_BITS, tmr);
#endif

    for(int i = 0; i < sizeof(tx_data); i++){
        uart_tx(&uart, tx_data[i]);
    }

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
