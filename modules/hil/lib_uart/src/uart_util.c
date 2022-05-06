// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "uart_util.h"

void init_buffer(uart_buffer_t *buff_cfg, uint8_t *buffer, unsigned size){
    buff_cfg->buffer = buffer;
    buff_cfg->size = size;
    buff_cfg->tail_idx = 0;
    buff_cfg->head_idx = 0;

}

inline unsigned get_buffer_fill_level(uart_buffer_t *buff_cfg){
    unsigned fill_level = 0;
    if(buff_cfg->head_idx >= buff_cfg->tail_idx){
        fill_level = buff_cfg->head_idx - buff_cfg->tail_idx;
    } else {
        fill_level = buff_cfg->size + buff_cfg->tail_idx - buff_cfg->head_idx;
    }
    return fill_level;
}

uart_buffer_error_t push_char_into_buffer(uart_buffer_t *buff_cfg, uint8_t data){
    unsigned next_idx = buff_cfg->head_idx + 1;
    if (next_idx == buff_cfg->size){
        next_idx = 0; //Check for wrap
    }
    if(next_idx == buff_cfg->tail_idx){
         return UART_BUFFER_FULL;
    }
    buff_cfg->buffer[buff_cfg->head_idx] = data;
    buff_cfg->head_idx = next_idx;

    return UART_BUFFER_OK;
}

uart_buffer_error_t pop_char_from_buffer(uart_buffer_t *buff_cfg, uint8_t *data){    
    uart_buffer_error_t err = UART_BUFFER_OK;
    if(buff_cfg->head_idx == buff_cfg->tail_idx){
        return UART_BUFFER_EMPTY;
    } else {
        *data = buff_cfg->buffer[buff_cfg->tail_idx];
        buff_cfg->tail_idx += 1;
        if(buff_cfg->tail_idx == buff_cfg->size){
           buff_cfg->tail_idx = 0;
       }
    }
    return UART_BUFFER_OK;
}

