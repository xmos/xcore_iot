// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * This file contains the prototypes for the FIFO used by the buffered UART modes (bare metal only)
 */

#pragma once
#include <stddef.h>
#include <stdint.h>

typedef enum {
    UART_BUFFER_OK = 0,
    UART_BUFFER_EMPTY,
    UART_BUFFER_FULL
} uart_buffer_error_t;

typedef struct {
    unsigned size;
    unsigned next_write_idx;
    unsigned next_read_idx;
    unsigned size_overlow_flag;
    uint8_t * buffer;
} uart_buffer_t;

void init_buffer(uart_buffer_t *buff_cfg, uint8_t *buffer, unsigned size);
unsigned get_buffer_fill_level(uart_buffer_t *uart_cfg);
uart_buffer_error_t push_byte_into_buffer(uart_buffer_t *buff_cfg, uint8_t data);
uart_buffer_error_t pop_byte_from_buffer(uart_buffer_t *buff_cfg, uint8_t *data);

static int buffer_used(uart_buffer_t *buff_cfg){
    return((buff_cfg->size && buff_cfg->buffer != NULL));
}
