// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdio.h>
#include <stdlib.h>
#include <xcore/parallel.h>
#include <xcore/assert.h>
#define SETSR(c) asm volatile("setsr %0" : : "n"(c));

#include "uart_util.h"

// #define BUFFER_SIZE 1234
#define BUFFER_SIZE 77 //SOmething not a power of 2 and bigger than typical size (16)

//Just for debugs
void dump_fifo(uart_buffer_t *buff){
    printf("size: %d\n", buff->size);
    printf("fill: %d\n", get_buffer_fill_level(buff));
    printf("next_write_idx: %d\n", buff->next_write_idx);
    printf("next_read_idx: %d\n", buff->next_read_idx);
    printf("size_overlow_flag: %d\n", buff->size_overlow_flag);
    printf("contents: "); for(int i=0; i<buff->size;i++) printf("%d, ",buff->buffer[i]);
    printf("\n\n");
}

void test_empty_after_init(uart_buffer_t *buff){
    xassert(get_buffer_fill_level(buff) == 0);
    uint8_t data;
    uart_buffer_error_t err = pop_byte_from_buffer(buff, &data);
    xassert(err == UART_BUFFER_EMPTY);
    printf("test_empty_after_init: PASS\n");
}


void test_fill_level(uart_buffer_t *buff){
    xassert(get_buffer_fill_level(buff) == 0);
    uint8_t data = 11;
    for(int i = 0; i < BUFFER_SIZE + 5; i++){
        unsigned fill_level = get_buffer_fill_level(buff);
        push_byte_into_buffer(buff, data);
        unsigned expected = i > BUFFER_SIZE ? BUFFER_SIZE : i;

        if(fill_level != expected){
            printf("ERROR: wrong fill level on up, expected: %d got: %d\n", expected, fill_level);
            xassert(0);
        }
    }

    for(int i = BUFFER_SIZE; i > -5; i--){
        unsigned fill_level = get_buffer_fill_level(buff);
        pop_byte_from_buffer(buff, &data);
        unsigned expected = i > 0 ? i : 0;
        if(fill_level != expected){
            printf("ERROR: wrong fill level on down, expected: %d got: %d\n", expected, fill_level);
            xassert(0);
        }
    }
    

    printf("test_fill_level: PASS\n");
}

void test_full(uart_buffer_t *buff){
    //fully drain
    for(int i = 0; i < BUFFER_SIZE; i++){
        uint8_t data = 0;
        pop_byte_from_buffer(buff, &data);
        // dump_fifo(buffs);
    }


    uart_buffer_error_t err = UART_BUFFER_OK;
    for(int i = 0; i < BUFFER_SIZE; i++){
        uint8_t data = (i + 1) % (UCHAR_MAX + 1);
        err = push_byte_into_buffer(buff, data);
        // dump_fifo(buff);

        if(err != UART_BUFFER_OK){
            printf("ERROR: FIFO size too small %d (%d)\n", i + 1, BUFFER_SIZE);
            xassert(0);
        }
    }
    err = push_byte_into_buffer(buff, 22);
    // dump_fifo(buff);

    if(err != UART_BUFFER_FULL){
        printf("ERROR: FIFO size too large, expected to fail push\n");
        xassert(0);
    }

    if(get_buffer_fill_level(buff) != buff->size){
        printf("ERROR: FIFO fill level wrong, expected: %d got: %d\n", buff->size, get_buffer_fill_level(buff));
        xassert(0);
    }


    uint8_t data = 0;
    err =  pop_byte_from_buffer(buff, &data);
    // dump_fifo(buff);

    if(err != UART_BUFFER_OK){
        printf("ERROR: FIFO unexpectedly empty\n");
        xassert(0);
    }

    uint8_t expected = (0 + 1) % (UCHAR_MAX + 1);
    if(data != expected){
        printf("ERROR: wrong data expected: %d got: %d\n", expected, data);
        xassert(0);
    }

    if(get_buffer_fill_level(buff) != buff->size - 1){
        printf("ERROR: FIFO fill level wrong, expected: %d got: %d\n", buff->size - 1, get_buffer_fill_level(buff));
        xassert(0);
    }


    printf("test_full: PASS\n");
}

void test_empty(uart_buffer_t *buff){

    //fully drain
    for(int i = 0; i < BUFFER_SIZE; i++){
        uint8_t data = 0;
        pop_byte_from_buffer(buff, &data);
        // dump_fifo(buffs);
    }

    int test_fill_sizes[] = {BUFFER_SIZE/3 , BUFFER_SIZE/2, BUFFER_SIZE}; //Test for wrapping too by part filling fifo
    char *test_string[]= {"third", "half", "all"};
    for(int test_fill_size_idx = 0; test_fill_size_idx < sizeof(test_fill_sizes) / sizeof(int); test_fill_size_idx++){
        //drain
        int test_fill_size = test_fill_sizes[test_fill_size_idx];

        uint8_t data = 0;
        for(int i = 0; i < test_fill_size; i++){
            pop_byte_from_buffer(buff, &data);
            // dump_fifo(buffs);
        }

        uart_buffer_error_t err = pop_byte_from_buffer(buff, &data);
        if(err != UART_BUFFER_EMPTY){
            printf("ERROR: FIFO unexpectedly not empty\n");
            xassert(0);
        }

        //fill
        uint8_t expect = (57 * test_fill_size) % 256;
        for(int i = 0; i < test_fill_size; i++){
            push_byte_into_buffer(buff, expect);
            // dump_fifo(buff);

        }
        for(int i = 0; i < test_fill_size; i++){
            data = 0;
            uart_buffer_error_t err =  pop_byte_from_buffer(buff, &data);
            if(err != UART_BUFFER_OK){
                printf("ERROR: FIFO unexpectedly empty\n");
                xassert(0);
            }
            if(data != expect){
                printf("ERROR: wrong data expected: %d got: %d\n", expect, data);
                xassert(0);
            }
        }
        data = 0;
        err =  pop_byte_from_buffer(buff, &data);
        if(err != UART_BUFFER_EMPTY){
            printf("ERROR: FIFO unexpectedly not empty\n");
            xassert(0);
        }
    printf("test_empty %s: PASS\n", test_string[test_fill_size_idx], test_fill_size);
    }
}

void test() {
    uart_buffer_t buff;
    uint8_t storage[BUFFER_SIZE];

    init_buffer(&buff, storage, sizeof(storage));
    test_fill_level(&buff);
    test_empty_after_init(&buff);
    test_full(&buff);
    test_empty(&buff);
    
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
