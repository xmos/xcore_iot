// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdlib.h>
#include <xcore/parallel.h>
#include <xcore/assert.h>
#define SETSR(c) asm volatile("setsr %0" : : "n"(c));

#include "uart_util.h"

// #define BUFFER_SIZE 1234
#define BUFFER_SIZE 12


void test_empty_after_init(uart_buffer_t *buff){
    xassert(get_buffer_fill_level(buff) == 0);
    uint8_t data;
    uart_buffer_error_t err = pop_char_from_buffer(buff, &data);
    xassert(err == UART_BUFFER_EMPTY);
    printf("test_empty_after_init: PASS\n");

}

void test_full(uart_buffer_t *buff){
    uart_buffer_error_t err = UART_BUFFER_OK;
    for(int i = 0; i < BUFFER_SIZE - 1; i++){
        uint8_t data = i % (UCHAR_MAX + 1);
        err = push_char_into_buffer(buff, i);
        printf("Fill level: %d\n", get_buffer_fill_level(buff));
        if(err != UART_BUFFER_OK){
            printf("ERROR: FIFO size too small %d (%d)\n", i + 1, BUFFER_SIZE);
            xassert(0);
        }
    }
    err = push_char_into_buffer(buff, 0);
    if(err != UART_BUFFER_FULL){
        printf("ERROR: FIFO size too large\n");
        xassert(0);
    }

    uint8_t data = 0;
    err =  pop_char_from_buffer(buff, &data);
    xassert(err == UART_BUFFER_OK);

    xassert(data == (BUFFER_SIZE - 1) % (UCHAR_MAX + 1));
    printf("test_full: PASS\n");
}

void test_empty(uart_buffer_t *buff){
    
}


void test() {
    uart_buffer_t buff;
    char storage[BUFFER_SIZE];

    init_buffer(&buff, storage, sizeof(storage));
    printf("Init complete\n");

    test_empty_after_init(&buff);
    test_full(&buff);
    
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
