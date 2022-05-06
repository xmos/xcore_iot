// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "uart_util.h"
#include <string.h>

void init_buffer(uart_buffer_t *buff_cfg, uint8_t *buffer, unsigned size){
    buff_cfg->buffer = buffer;
    buff_cfg->size = size;
    buff_cfg->next_write_idx = 0;
    buff_cfg->next_read_idx = 0;
    buff_cfg->size_overlow_flag = 0;
    memset(buffer, 0, size);
}

inline unsigned get_buffer_fill_level(uart_buffer_t *buff_cfg){
    unsigned fill_level = 0;
    if(buff_cfg->next_write_idx >= buff_cfg->next_read_idx){
        fill_level = buff_cfg->next_write_idx - buff_cfg->next_read_idx;
        fill_level += buff_cfg->size_overlow_flag ? buff_cfg->size : 0;
    } else {
        fill_level = buff_cfg->size - buff_cfg->next_read_idx + buff_cfg->next_write_idx;
    }
    return fill_level;
}

uart_buffer_error_t push_byte_into_buffer(uart_buffer_t *buff_cfg, uint8_t data){
    if(buff_cfg->next_read_idx == buff_cfg->next_write_idx && buff_cfg->size_overlow_flag){
        return UART_BUFFER_FULL;
    }
    buff_cfg->buffer[buff_cfg->next_write_idx] = data;
    buff_cfg->next_write_idx += 1;
    if (buff_cfg->next_write_idx == buff_cfg->size){
        buff_cfg->next_write_idx = 0; //Check for wrap
    }
    if(buff_cfg->next_read_idx == buff_cfg->next_write_idx){
        buff_cfg->size_overlow_flag = 1;
    }
    return UART_BUFFER_OK;
}

uart_buffer_error_t pop_byte_from_buffer(uart_buffer_t *buff_cfg, uint8_t *data){    
    if(buff_cfg->next_write_idx == buff_cfg->next_read_idx){
        if(!buff_cfg->size_overlow_flag){
            return UART_BUFFER_EMPTY;
        }
    }
    *data = buff_cfg->buffer[buff_cfg->next_read_idx];
    buff_cfg->next_read_idx += 1;
    if(buff_cfg->next_read_idx == buff_cfg->size){ //wrap
       buff_cfg->next_read_idx = 0;
    }
    buff_cfg->size_overlow_flag = 0; //QUicker to always zero rather than test
    return UART_BUFFER_OK;
}

