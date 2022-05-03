// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdint.h>
#include <xcore/assert.h>
#include <xcore/interrupt_wrappers.h>

#include "uart.h"



DEFINE_INTERRUPT_CALLBACK(UART_INTERRUPTABLE_FUNCTIONS, uart_rx_sample_isr, callback_info){
    uart_rx_t *uart_cfg = (uart_rx_t*) callback_info;
    // uart_tx_handle_transition(uart_cfg);
}


static inline uint32_t get_current_time(uart_rx_t *uart_cfg){
    if(uart_cfg->tmr){
        return hwtimer_get_time(uart_cfg->tmr);
    }
    return get_reference_time();
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

    //Assert if buffer is used but no timer as we need the timer for buffered mode  
    if(buffer_used(&uart_cfg->buffer) && !tmr){
        xassert(0);    
    }
    //TODO work out if buffer can be used without HW timer
    if(buffer_used(&uart_cfg->buffer)){
        init_buffer(&uart_cfg->buffer, buffer, buffer_size);

        //Setup interrupt
        uart_cfg->uart_callback_fptr = uart_callback_fptr;
        triggerable_setup_interrupt_callback(tmr, uart_cfg, INTERRUPT_CALLBACK(uart_rx_sample_isr) );
        interrupt_unmask_all();
    } else {
        init_buffer(&uart_cfg->buffer, NULL, 0);
    }

    port_enable(rx_port);
    port_in(rx_port); //Ensure port is input
}

static inline void sleep_until_start_transition(uart_rx_t *uart_cfg){
    if(buffer_used(&uart_cfg->buffer)){
        //Wait on a the port interrupt
        // hwtimer_wait_until(uart_cfg->tmr, uart_cfg->next_event_time_ticks);
    }else{
        //Poll the port
        while(port_in(uart_cfg->rx_port) & 0x1);
    }
}

static inline void sleep_until_next_sample(uart_rx_t *uart_cfg){
    if(uart_cfg->tmr){
        //Wait on a the timer
        hwtimer_wait_until(uart_cfg->tmr, uart_cfg->next_event_time_ticks);
    }else{
        //Poll the timer
        while(get_current_time(uart_cfg) < uart_cfg->next_event_time_ticks);
    }
}

static inline void enable_start_bit_transition_interrupt(uart_rx_t *uart_cfg, int enable){

}

static inline void enable_sample_timer_interrupt(uart_rx_t *uart_cfg, int enable){

}

void uart_rx_handle_event(uart_rx_t *uart_cfg){
    // printf("state: %d\n", uart_cfg->state);
    switch(uart_cfg->state){
        case UART_IDLE: {
            uart_cfg->next_event_time_ticks = get_current_time(uart_cfg);
            uart_cfg->next_event_time_ticks += uart_cfg->bit_time_ticks >> 1;
            uart_cfg->state = UART_START;
            enable_start_bit_transition_interrupt(uart_cfg, 0);
            enable_sample_timer_interrupt(uart_cfg, 1);
            break;
        }

        case UART_START: {
            uint32_t pin = port_in(uart_cfg->rx_port) & 0x1;
            if(pin != 0){
                //TODO handle error
            }
            uart_cfg->state = UART_DATA;
            uart_cfg->current_data_bit = 0;
            uart_cfg->next_event_time_ticks += uart_cfg->bit_time_ticks;
            break;
        }

        case UART_DATA: { 
            uint32_t pin = port_in(uart_cfg->rx_port) & 0x1;
            uart_cfg->uart_data |= pin << uart_cfg->current_data_bit;
            uart_cfg->current_data_bit++;
            uart_cfg->next_event_time_ticks += uart_cfg->bit_time_ticks;
            if(uart_cfg->current_data_bit == uart_cfg->num_data_bits){
                if(uart_cfg->parity == UART_PARITY_NONE){
                    uart_cfg->state = UART_STOP;
                } else {
                    uart_cfg->state = UART_PARITY;
                }
            }
            break;
        }

        case UART_PARITY: {
            uint32_t pin = port_in(uart_cfg->rx_port) & 0x1;
            uint32_t parity_setting = (uart_cfg->parity == UART_PARITY_EVEN) ? 0 : 1;
            uint32_t parity = (unsigned)uart_cfg->uart_data;
            // crc32(parity, parity_setting, 1); //http://bugzilla/show_bug.cgi?id=18663
            asm volatile("crc32 %0, %2, %3" : "=r" (parity) : "0" (parity), "r" (parity_setting), "r" (1));
            parity &= 1;
            if(pin != parity){
                //TODO handle error
            }
            uart_cfg->state = UART_STOP;
            uart_cfg->next_event_time_ticks += uart_cfg->bit_time_ticks;
        }
     
        case UART_STOP: {   
            uint32_t pin = port_in(uart_cfg->rx_port) & 0x1;
            if(pin != 1){
                //TODO handle error
            }
            //TODO do we wait for negative edge or end of stop symbol? Probs stop symbol
            if(buffer_used(&uart_cfg->buffer)){
                enable_sample_timer_interrupt(uart_cfg, 0);
                enable_start_bit_transition_interrupt(uart_cfg, 1);
            }
            uart_cfg->state = UART_IDLE;
            break;
        }

        default: {
            xassert(0);
        }
    }
     if(buffer_used(&uart_cfg->buffer)){
        if(uart_cfg->state == UART_IDLE){
            // buffered_uart_tx_char_finished(uart_cfg);
        } else {
            // sleep_until_start_transition(uart_cfg);
            // sleep_until_next_sample(uart_cfg);
        }
     }
}


char uart_rx(uart_rx_t *uart_cfg){
    if(buffer_used(&uart_cfg->buffer)){
        printf("BUFFER USED\n");
        enable_start_bit_transition_interrupt(uart_cfg, 1);
    } else {
        // printf("BUFFER NOT USED\n");
        uart_cfg->uart_data = 0;
        uart_cfg->current_data_bit = 0;
        uart_cfg->state = UART_IDLE;
        sleep_until_start_transition(uart_cfg);
        // printf("START BIT EDGE\n");
        do{
            uart_rx_handle_event(uart_cfg);
            sleep_until_next_sample(uart_cfg);
        } while(uart_cfg->state != UART_IDLE);

        return uart_cfg->uart_data;
    }
}

void uart_rx_deinit(uart_rx_t *uart_cfg){
    //TODO make buffer used a fn
    if(buffer_used(&uart_cfg->buffer)){
        triggerable_disable_trigger(uart_cfg->tmr);
    }
    port_disable(uart_cfg->rx_port);
}

