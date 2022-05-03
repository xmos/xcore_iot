// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdint.h>
#include <xcore/assert.h>
#include <xcore/interrupt_wrappers.h>

#include "uart.h"




DEFINE_INTERRUPT_CALLBACK(UART_INTERRUPTABLE_FUNCTIONS, uart_rx_sample_isr, callback_info){
    uart_rx_t *uart_cfg = (uart_rx_t*) callback_info;
    // uart_tx_handle_transition(uart_cfg);
}


void uart_rx_init(
        uart_rx_t *uart_cfg,
        port_t rx_port,
        uint32_t baud_rate,
        uint8_t num_data_bits,
        uart_parity_t parity,
        uint8_t stop_bits,

        hwtimer_t tmr,
        char *buffer,
        size_t buffer_size,
        void(*uart_callback_fptr)(uart_callback_t callback_info)
        ){

    uart_cfg->rx_port = rx_port;
    uart_cfg->bit_time_ticks = XS1_TIMER_HZ / baud_rate;
    uart_cfg->next_event_time_ticks = 0;
    xassert(num_data_bits <= 8);
    uart_cfg->num_data_bits = num_data_bits;
    xassert(parity == UART_PARITY_NONE || parity == UART_PARITY_EVEN || parity == UART_PARITY_ODD);
    uart_cfg->parity = parity;
    uart_cfg->stop_bits = stop_bits;
    uart_cfg->current_data_bit = 0;
    uart_cfg->uart_data = 0;
    uart_cfg->state = UART_IDLE;


    //HW timer will be replaced by poll if set to zero
    uart_cfg->tmr = tmr;

    unsigned buffer_used = (buffer_size && buffer != NULL);
    //Assert if buffer is used but no timer as we need the timer for buffered mode  
    if(buffer_used && !tmr){
        xassert(0);    
    }
    //TODO work out if buffer can be used without HW timer
    if(buffer_used){
        uart_cfg->buffer_size = buffer_size;
        uart_cfg->buffer = buffer;
        uart_cfg->uart_callback_fptr = uart_callback_fptr;
        uart_cfg->buff_head_idx = 0;
        uart_cfg->buff_tail_idx = 0;
        //Setup interrupt
        triggerable_setup_interrupt_callback(tmr, uart_cfg, INTERRUPT_CALLBACK(uart_rx_sample_isr) );
        interrupt_unmask_all();
    } else {
        uart_cfg->buffer_size = 0;
        uart_cfg->buffer = NULL;
    }

    port_enable(rx_port);
    port_in(rx_port); //Ensure port is input
}

char uart_rx(uart_rx_t *uart){
    return 0;
}

void uart_rx_deinit(uart_rx_t *uart_cfg){
    unsigned buffer_used = (uart_cfg->buffer_size && uart_cfg->buffer != NULL);
    if(buffer_used){
        triggerable_disable_trigger(uart_cfg->tmr);
    }
    port_disable(uart_cfg->rx_port);
}

