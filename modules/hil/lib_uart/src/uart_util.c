// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "uart_util.h"

void init_buffer(uart_buffer_t *buff_cfg, char *buffer, unsigned size){
    buff_cfg->buffer = buffer;
    buff_cfg->size = size;
    buff_cfg->tail_idx = 0;
    buff_cfg->head_idx = 0;

}

unsigned get_buffer_fill_level(uart_buffer_t *buff_cfg){
    unsigned fill_level = 0;
    if (buff_cfg->head_idx == buff_cfg->tail_idx){
        fill_level = 0;
    }
    else if(buff_cfg->head_idx > buff_cfg->tail_idx){
        fill_level = buff_cfg->head_idx - buff_cfg->tail_idx;
    }
    else {
        fill_level = buff_cfg->size + buff_cfg->tail_idx - buff_cfg->head_idx;
    }
    return fill_level;
}

uart_buffer_error_t push_char_into_buffer(uart_buffer_t *buff_cfg, char data){
    uart_buffer_error_t err = UART_BUFFER_OK;
    if(get_buffer_fill_level(buff_cfg) < buff_cfg->size){
        buff_cfg->buffer[buff_cfg->head_idx] = data;
        buff_cfg->head_idx += 1;
        if(buff_cfg->head_idx == buff_cfg->size){
           buff_cfg->head_idx = 0; 
        }
    } else {
       err = UART_BUFFER_FULL;
    }
    return err;
}

uart_buffer_error_t pop_char_from_buffer(uart_buffer_t *buff_cfg, char *data){    
    uart_buffer_error_t err = UART_BUFFER_OK;
    if(get_buffer_fill_level(buff_cfg)){
        *data = buff_cfg->buffer[buff_cfg->tail_idx];
        buff_cfg->tail_idx += 1;
        if(buff_cfg->tail_idx == buff_cfg->size){
           buff_cfg->tail_idx = 0; 
        }
    } else {
        err = UART_BUFFER_EMPTY;
    }
    return err;
}


int buffer_used(uart_buffer_t *buff_cfg){
    return((buff_cfg->size && buff_cfg->buffer != NULL));
}