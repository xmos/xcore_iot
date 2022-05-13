// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdint.h>
#include <xcore/assert.h>
#include <xcore/interrupt_wrappers.h>

#include "uart.h"


DECLARE_INTERRUPT_CALLBACK(uart_rx_handle_event, callback_info);

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
        uint8_t *buffer,
        size_t buffer_size,
        void(*uart_callback_fptr)(uart_callback_t callback_info)
        ){

    uart_cfg->rx_port = rx_port;
    uart_cfg->bit_time_ticks = XS1_TIMER_HZ / baud_rate;
    uart_cfg->next_event_time_ticks = 0;
    xassert(num_data_bits <= 8 && num_data_bits >= 5);
    uart_cfg->num_data_bits = num_data_bits;
    xassert(parity == UART_PARITY_NONE || parity == UART_PARITY_EVEN || parity == UART_PARITY_ODD);
    uart_cfg->parity = parity;
    uart_cfg->stop_bits = stop_bits;
    uart_cfg->current_data_bit = 0;
    uart_cfg->uart_data = 0;
    uart_cfg->state = UART_IDLE;

    //HW timer will be replaced by poll if set to zero
    uart_cfg->tmr = tmr;

    init_buffer(&uart_cfg->buffer, buffer, buffer_size);
    uart_cfg->uart_callback_fptr = uart_callback_fptr;

    //Assert if buffer is used but no timer as we need the timer for buffered mode 
    if(buffer_used(&uart_cfg->buffer) && !tmr){
        xassert(0);    
    }

    port_enable(rx_port);

    //TODO work out if buffer can be used without HW timer
    if(buffer_used(&uart_cfg->buffer)){
        init_buffer(&uart_cfg->buffer, buffer, buffer_size);

        //Setup interrupts
        interrupt_mask_all();
        port_in(rx_port); //Ensure port is input and clear trigger
        port_set_trigger_in_equal(rx_port, 0); //Trigger on low (start of start bit)
        triggerable_setup_interrupt_callback(rx_port, uart_cfg, INTERRUPT_CALLBACK(uart_rx_handle_event) );

        hwtimer_clear_trigger_time(uart_cfg->tmr);
        triggerable_setup_interrupt_callback(tmr, uart_cfg, INTERRUPT_CALLBACK(uart_rx_handle_event) );

        //Initial ISR will be on falling edge of rx, followed by timer ISRs for sampling the values
        triggerable_set_trigger_enabled(uart_cfg->rx_port, 1);
        triggerable_set_trigger_enabled(uart_cfg->tmr, 0);

        interrupt_unmask_all();
    } else {
        init_buffer(&uart_cfg->buffer, NULL, 0);
    }
}

static inline void sleep_until_start_transition(uart_rx_t *uart_cfg){
    if(uart_cfg->tmr){
        //Wait on a port transition to low
        port_in_when_pinseq(uart_cfg->rx_port, PORT_UNBUFFERED, 0);
    }else{
        //Poll the port
        while(port_in(uart_cfg->rx_port) & 0x1);
    }
    uart_cfg->next_event_time_ticks = get_current_time(uart_cfg);
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

// Interrupt latency overhead has been measured at 360ns @ 75Mhz thread speed 
// and 320ns at 120Mhz thread speed. Hence use mid point of 340ns
// Latency for polling mode is between 210ns and 190 so use 200ns
#define INTERRUPT_LATENCY_COMPENSATION_TICKS (XS1_TIMER_MHZ * 340 / 1000)
#define BLOCKING_LATENCY_COMPENSATION_TICKS  (XS1_TIMER_MHZ * 200 / 1000)


DEFINE_INTERRUPT_CALLBACK(UART_RX_INTERRUPTABLE_FUNCTIONS, uart_rx_handle_event, callback_info){
    uart_rx_t *uart_cfg = (uart_rx_t*) callback_info;
    switch(uart_cfg->state){
        case UART_IDLE: {
            if(buffer_used(&uart_cfg->buffer)){
                uart_cfg->next_event_time_ticks = get_current_time(uart_cfg);
            }
            //Double check line still low
            uint32_t pin = port_in(uart_cfg->rx_port) & 0x1;
            if(pin != 0){
                (*uart_cfg->uart_callback_fptr)(UART_START_BIT_ERROR);
            }
            uart_cfg->next_event_time_ticks += uart_cfg->bit_time_ticks >> 1; //Halfway through start bit
            uart_cfg->state = UART_START;
            if(buffer_used(&uart_cfg->buffer)){
                uart_cfg->next_event_time_ticks -= INTERRUPT_LATENCY_COMPENSATION_TICKS;
                triggerable_set_trigger_enabled(uart_cfg->rx_port, 0);
                port_clear_trigger_in(uart_cfg->rx_port);
                hwtimer_set_trigger_time(uart_cfg->tmr, uart_cfg->next_event_time_ticks);
                triggerable_set_trigger_enabled(uart_cfg->tmr, 1);
            } else {
                uart_cfg->next_event_time_ticks -= BLOCKING_LATENCY_COMPENSATION_TICKS;
            }
            break;
        }

        case UART_START: {
            uint32_t pin = port_in(uart_cfg->rx_port) & 0x1;
            if(pin != 0){
                (*uart_cfg->uart_callback_fptr)(UART_START_BIT_ERROR);
            }
            uart_cfg->state = UART_DATA;
            uart_cfg->uart_data = 0;
            uart_cfg->current_data_bit = 0;
            uart_cfg->next_event_time_ticks += uart_cfg->bit_time_ticks;
            if(buffer_used(&uart_cfg->buffer)){
                hwtimer_set_trigger_time(uart_cfg->tmr, uart_cfg->next_event_time_ticks);
            }
            break;
        }

        case UART_DATA: { 
            uint32_t pin = port_in(uart_cfg->rx_port) & 0x1;
            uart_cfg->uart_data |= pin << uart_cfg->current_data_bit;
            uart_cfg->current_data_bit += 1;

            if(uart_cfg->current_data_bit == uart_cfg->num_data_bits){
                if(uart_cfg->parity == UART_PARITY_NONE){
                    uart_cfg->state = UART_STOP;
                } else {
                    uart_cfg->state = UART_PARITY;
                }
            }
            uart_cfg->next_event_time_ticks += uart_cfg->bit_time_ticks;
            if(buffer_used(&uart_cfg->buffer)){
                hwtimer_set_trigger_time(uart_cfg->tmr, uart_cfg->next_event_time_ticks);
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
                (*uart_cfg->uart_callback_fptr)(UART_PARITY_ERROR);
            }
            uart_cfg->state = UART_STOP;
            uart_cfg->next_event_time_ticks += uart_cfg->bit_time_ticks;
            if(buffer_used(&uart_cfg->buffer)){
                hwtimer_set_trigger_time(uart_cfg->tmr, uart_cfg->next_event_time_ticks);
            }
            break;
        }
     
        case UART_STOP: {   
            uint32_t pin = port_in(uart_cfg->rx_port) & 0x1;
            if(pin != 1){
                (*uart_cfg->uart_callback_fptr)(UART_FRAMING_ERROR);
            } else if(buffer_used(&uart_cfg->buffer)){
                (*uart_cfg->uart_callback_fptr)(UART_RX_COMPLETE);
            }
            uart_cfg->state = UART_IDLE;

            //Go back to waiting for next start bit transition
            if(buffer_used(&uart_cfg->buffer)){
                uart_buffer_error_t err = push_byte_into_buffer(&uart_cfg->buffer, uart_cfg->uart_data);
                if(err == UART_BUFFER_FULL){
                    (*uart_cfg->uart_callback_fptr)(UART_OVERRUN_ERROR);
                }
                hwtimer_clear_trigger_time(uart_cfg->tmr);
                triggerable_set_trigger_enabled(uart_cfg->tmr, 0);

                port_set_trigger_in_equal(uart_cfg->rx_port, 0); //Trigger on low (start of start bit)
                triggerable_set_trigger_enabled(uart_cfg->rx_port, 1);

            }
            break;
        }

        default: {
            xassert(0);
        }
    }
}


uint8_t uart_rx(uart_rx_t *uart_cfg){
    if(buffer_used(&uart_cfg->buffer)){
        uint8_t rx_data = 0;
        uart_buffer_error_t err = pop_byte_from_buffer(&uart_cfg->buffer, &rx_data);
        if(err == UART_BUFFER_EMPTY){
            (*uart_cfg->uart_callback_fptr)(UART_UNDERRUN_ERROR);
        }
        return rx_data;
    } else {
        uart_cfg->state = UART_IDLE;
        sleep_until_start_transition(uart_cfg);
        do{
            uart_rx_handle_event(uart_cfg);
            sleep_until_next_sample(uart_cfg);
        } while(uart_cfg->state != UART_IDLE);

        return uart_cfg->uart_data;
    }
}

void uart_rx_deinit(uart_rx_t *uart_cfg){
    interrupt_mask_all();
    if(buffer_used(&uart_cfg->buffer)){        
        triggerable_set_trigger_enabled(uart_cfg->rx_port, 0);
        triggerable_set_trigger_enabled(uart_cfg->tmr, 0);
    }
    port_disable(uart_cfg->rx_port);
    interrupt_unmask_all();
}

