// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#pragma once
#include <stddef.h>

#include <print.h> //TODO Remove me

typedef enum {
    UART_BUFFER_OK = 0,
    UART_BUFFER_EMPTY,
    UART_BUFFER_FULL
} uart_buffer_error_t;

typedef struct {
    unsigned size;
    unsigned head_idx;
    unsigned tail_idx;
    char* buffer;
} uart_buffer_t;

void init_buffer(uart_buffer_t *buff_cfg, char *buffer, unsigned size);
unsigned get_buffer_fill_level(uart_buffer_t *uart_cfg);
uart_buffer_error_t push_char_into_buffer(uart_buffer_t *buff_cfg, char data);
uart_buffer_error_t pop_char_from_buffer(uart_buffer_t *buff_cfg, char *data);

static int buffer_used(uart_buffer_t *buff_cfg){
    return((buff_cfg->size && buff_cfg->buffer != NULL));
}
