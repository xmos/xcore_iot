// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdint.h>
#include <xcore/assert.h>
#include <xcore/interrupt_wrappers.h>

#include "uart.h"

DECLARE_INTERRUPT_CALLBACK(uart_tx_handle_event, callback_info);

void uart_tx_blocking_init(
        uart_tx_t *uart_cfg,
        port_t tx_port,
        uint32_t baud_rate,
        uint8_t num_data_bits,
        uart_parity_t parity,
        uint8_t stop_bits,
        hwtimer_t tmr){

    uart_tx_init(uart_cfg, tx_port, baud_rate, num_data_bits, parity, stop_bits, tmr,
                 NULL, 0, NULL);
}

void uart_tx_init(
        uart_tx_t *uart_cfg,
        port_t tx_port,
        uint32_t baud_rate,
        uint8_t num_data_bits,
        uart_parity_t parity,
        uint8_t stop_bits,

        hwtimer_t tmr,
        uint8_t *buffer,
        size_t buffer_size,
        void(*uart_callback_fptr)(uart_callback_t callback_info)
        ){

    uart_cfg->tx_port = tx_port;
    uart_cfg->bit_time_ticks = XS1_TIMER_HZ / baud_rate;
    uart_cfg->next_event_time_ticks = 0;
    xassert(num_data_bits <= 8);
    uart_cfg->num_data_bits = num_data_bits;
    xassert(parity == UART_PARITY_NONE || parity == UART_PARITY_EVEN || parity == UART_PARITY_ODD);
    uart_cfg->parity = parity;
    uart_cfg->stop_bits = stop_bits;
    uart_cfg->current_stop_bit = 0;
    uart_cfg->current_data_bit = 0;
    uart_cfg->uart_data = 0;
    uart_cfg->state = UART_IDLE;

    init_buffer(&uart_cfg->buffer, buffer, buffer_size);

    //HW timer will be replaced by poll if set to zero
    uart_cfg->tmr = tmr;

    //Assert if buffer is used but no timer as we need the timer for buffered mode  
    if(buffer_used(&uart_cfg->buffer) && !tmr){
        xassert(0);    
    }
    //TODO work out if buffer can be used without HW timer
    if(buffer_used(&uart_cfg->buffer)){
        //Setup interrupt
        uart_cfg->uart_callback_fptr = uart_callback_fptr;
        triggerable_setup_interrupt_callback(tmr, uart_cfg, INTERRUPT_CALLBACK(uart_tx_handle_event) );
        interrupt_unmask_all();
    }

    port_enable(tx_port);
    port_out(tx_port, 1); //Set to idle
}


void uart_tx_deinit(uart_tx_t *uart_cfg){
    if(buffer_used(&uart_cfg->buffer)){
        triggerable_disable_trigger(uart_cfg->tmr);
    }
    port_disable(uart_cfg->tx_port);
}

__attribute__((always_inline))
static inline uint32_t get_current_time(uart_tx_t *uart_cfg){
    if(uart_cfg->tmr){
        return hwtimer_get_time(uart_cfg->tmr);
    }
    return get_reference_time();
}

__attribute__((always_inline))
static inline void sleep_until_next_transition(uart_tx_t *uart_cfg){
    if(buffer_used(&uart_cfg->buffer)){
        //Setup next interrupt
        hwtimer_set_trigger_time(uart_cfg->tmr, uart_cfg->next_event_time_ticks);
    } 
    else if(uart_cfg->tmr){
        //Wait on a the timer
        hwtimer_wait_until(uart_cfg->tmr, uart_cfg->next_event_time_ticks);
    }else{
        //Poll the timer
        while(get_current_time(uart_cfg) < uart_cfg->next_event_time_ticks);
    }
}

__attribute__((always_inline))
static inline void buffered_uart_tx_char_finished(uart_tx_t *uart_cfg){
    uart_buffer_error_t err = pop_byte_from_buffer(&uart_cfg->buffer, &uart_cfg->uart_data);
    if(err == UART_BUFFER_OK){
        uart_cfg->state = UART_START;
        hwtimer_set_trigger_time(uart_cfg->tmr, uart_cfg->next_event_time_ticks);
    } else {
        uart_cfg->state = UART_IDLE;
    }
}

// There was additional latency introduced in the start bit causing a slight stretching.
// This is due to UART_START get current_time not including the ISR or fn call
// It has been measured on the simulator to be 170-174ns for polling/event and 230-260ns for ISR 
#define INTERRUPT_LATENCY_COMPENSATION_TICKS (XS1_TIMER_MHZ * 245 / 1000)
#define BLOCKING_LATENCY_COMPENSATION_TICKS  (XS1_TIMER_MHZ * 170 / 1000)


DEFINE_INTERRUPT_CALLBACK(UART_TX_INTERRUPTABLE_FUNCTIONS, uart_tx_handle_event, callback_info){
    uart_tx_t *uart_cfg = (uart_tx_t*) callback_info;
    switch(uart_cfg->state){
        case UART_START: {
            uart_cfg->next_event_time_ticks = get_current_time(uart_cfg);
            port_out(uart_cfg->tx_port, 0);
            uart_cfg->state = UART_DATA;
            uart_cfg->current_data_bit = 0;
            uart_cfg->next_event_time_ticks += uart_cfg->bit_time_ticks;
            if(buffer_used(&uart_cfg->buffer)){
                uart_cfg->next_event_time_ticks -= INTERRUPT_LATENCY_COMPENSATION_TICKS;
            } else {
                uart_cfg->next_event_time_ticks -= BLOCKING_LATENCY_COMPENSATION_TICKS;
            }
            break;
        }

        case UART_DATA: {    
            uint32_t port_val = (uart_cfg->uart_data >> uart_cfg->current_data_bit) & 0x1;
            port_out(uart_cfg->tx_port, port_val);
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
            uint32_t parity_setting = (uart_cfg->parity == UART_PARITY_EVEN) ? 0 : 1;
            uint32_t parity = (unsigned)uart_cfg->uart_data;
            // crc32(parity, parity_setting, 1); //http://bugzilla/show_bug.cgi?id=18663
            asm volatile("crc32 %0, %2, %3" : "=r" (parity) : "0" (parity), "r" (parity_setting), "r" (1));
            port_out(uart_cfg->tx_port, parity);
            uart_cfg->state = UART_STOP;
            uart_cfg->next_event_time_ticks += uart_cfg->bit_time_ticks;
            break;
        }
     
        case UART_STOP: {   
            port_out(uart_cfg->tx_port, 1);
            uart_cfg->current_stop_bit += 1;
            uart_cfg->next_event_time_ticks += uart_cfg->bit_time_ticks; //do before buffered_uart_tx_char_finished

            if(uart_cfg->current_stop_bit == uart_cfg->stop_bits){
                uart_cfg->current_stop_bit = 0;
                if(buffer_used(&uart_cfg->buffer)){
                    buffered_uart_tx_char_finished(uart_cfg);//Next state is set here
                } else {
                    uart_cfg->state = UART_IDLE;
                }
            }
            break;
        }

        case UART_IDLE: {
            //This state is only entered in buffered mode at end stream and holds the stop bit
            //Final check for new data to see if we need to start again or not in case write came in during stop bit
            buffered_uart_tx_char_finished(uart_cfg);
            if(uart_cfg->state == UART_IDLE){
                triggerable_disable_trigger(uart_cfg->tmr);
                (*uart_cfg->uart_callback_fptr)(UART_TX_EMPTY);
            }
            break;
        }

        default: {
            xassert(0);
        }
    }
    if(buffer_used(&uart_cfg->buffer)){
        sleep_until_next_transition(uart_cfg);
    }
}



void uart_tx(uart_tx_t *uart_cfg, uint8_t data){
    uint32_t mask = 0;
    asm volatile("mkmsk %0, %1": "=r"(mask) : "r"(uart_cfg->num_data_bits));
    data &= mask;//So pariy gets calc'd properly
    //Check to see if we are using interrupts/buffered mode
    if(buffer_used(&uart_cfg->buffer)){
        if(get_buffer_fill_level(&uart_cfg->buffer) == 0 && uart_cfg->state == UART_IDLE){//Kick off a transmit
            uart_cfg->uart_data = data;
            uart_cfg->state = UART_START;
            uart_cfg->next_event_time_ticks = get_current_time(uart_cfg); 
            sleep_until_next_transition(uart_cfg);//Set event for now
            triggerable_enable_trigger(uart_cfg->tmr);
        } else {//Transaction already underway
            uart_buffer_error_t err = push_byte_into_buffer(&uart_cfg->buffer, data);
        }
    } else {
        uart_cfg->uart_data = data;
        uart_cfg->state = UART_START;
        //Blocking call
        do {
            uart_tx_handle_event(uart_cfg);
            sleep_until_next_transition(uart_cfg);
        } while(uart_cfg->state != UART_IDLE);
    }
}
