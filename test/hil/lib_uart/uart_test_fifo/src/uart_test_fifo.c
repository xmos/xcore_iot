// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdlib.h>
#include <xcore/parallel.h>
#include <xcore/assert.h>
#define SETSR(c) asm volatile("setsr %0" : : "n"(c));

#include "uart_util.h"

#define BUFFER_SIZE 1234


void test_empty_init(uart_buffer_t *buff){
    xassert(get_buffer_fill_level(buff) == 0);
    char data;
    uart_buffer_error_t err = pop_char_from_buffer(buff, &data);
    xassert(err == UART_BUFFER_EMPTY);
}

void test_full(uart_buffer_t *buff){
    
}

void test_empty(uart_buffer_t *buff){
    
}


void test() {
    uart_buffer_t buff;
    char storage[BUFFER_SIZE];

    init_buffer(&buff, storage, sizeof(storage));
    printf("Init complete\n");

    test_empty_init(&buff);
    
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
