// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdint.h>
#include <xcore/assert.h>
#include <xcore/interrupt_wrappers.h>

#include "uart.h"

void uart_tx_handle_transition(uart_tx_t *uart_cfg);


DEFINE_INTERRUPT_CALLBACK(UART_INTERRUPTABLE_FUNCTIONS, transition_isr, callback_info){
    uart_tx_t *uart_cfg = (uart_tx_t*) callback_info;
    uart_tx_handle_transition(uart_cfg);
}



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
        char *buffer,
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
        triggerable_setup_interrupt_callback(tmr, uart_cfg, INTERRUPT_CALLBACK(transition_isr) );
        interrupt_unmask_all();
    } else {
        uart_cfg->buffer_size = 0;
        uart_cfg->buffer = NULL;
    }

    port_enable(tx_port);
    port_out(tx_port, 1); //Set to idle
}


void uart_tx_deinit(uart_tx_t *uart_cfg){
    unsigned buffer_used = (uart_cfg->buffer_size && uart_cfg->buffer != NULL);
    if(buffer_used){
        triggerable_disable_trigger(uart_cfg->tmr);
    }
    port_disable(uart_cfg->tx_port);
}



static inline uint32_t get_current_time(uart_tx_t *uart_cfg){
    if(uart_cfg->tmr){
        return hwtimer_get_time(uart_cfg->tmr);
    }
    return get_reference_time();
}

static inline void sleep_until_next_transition(uart_tx_t *uart_cfg){
    unsigned buffer_used = (uart_cfg->buffer_size && uart_cfg->buffer != NULL);
    if(buffer_used){
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


typedef enum {
    UART_BUFFER_OK = 0,
    UART_BUFFER_EMPTY,
    UART_BUFFER_FULL
} uart_buffer_error_t;

static inline unsigned get_buffer_fill_level(uart_tx_t *uart_cfg){
    unsigned fill_level = 0;
    if (uart_cfg->buff_head_idx == uart_cfg->buff_tail_idx){
        fill_level = 0;
    }
    else if(uart_cfg->buff_head_idx > uart_cfg->buff_tail_idx){
        fill_level = uart_cfg->buff_head_idx - uart_cfg->buff_tail_idx;
    }
    else {
        fill_level = uart_cfg->buffer_size + uart_cfg->buff_tail_idx - uart_cfg->buff_head_idx;
    }
    return fill_level;
}

static inline uart_buffer_error_t push_char_into_buffer(uart_tx_t *uart_cfg, char data){
    uart_buffer_error_t err = UART_BUFFER_OK;
    if(get_buffer_fill_level(uart_cfg) < uart_cfg->buffer_size){
        uart_cfg->buffer[uart_cfg->buff_head_idx] = data;
        uart_cfg->buff_head_idx += 1;
        if(uart_cfg->buff_head_idx == uart_cfg->buffer_size){
           uart_cfg->buff_head_idx = 0; 
        }
    } else {
       err = UART_BUFFER_FULL;
    }
    return err;
}

static inline uart_buffer_error_t pop_char_from_buffer(uart_tx_t *uart_cfg, char *data){    
    uart_buffer_error_t err = UART_BUFFER_OK;
    if(get_buffer_fill_level(uart_cfg)){
        *data = uart_cfg->buffer[uart_cfg->buff_tail_idx];
        uart_cfg->buff_tail_idx += 1;
        if(uart_cfg->buff_tail_idx == uart_cfg->buffer_size){
           uart_cfg->buff_tail_idx = 0; 
        }
    } else {
        err = UART_BUFFER_EMPTY;
    }
    return err;
}






static inline void buffered_uart_tx_char_finished(uart_tx_t *uart_cfg){
    uart_buffer_error_t err = pop_char_from_buffer(uart_cfg, &uart_cfg->uart_data);
    if(err == UART_BUFFER_OK){
        uart_cfg->current_data_bit = 0;
        pop_char_from_buffer(uart_cfg, &uart_cfg->uart_data);
        uart_cfg->state = UART_START;
    } else {
        triggerable_disable_trigger(uart_cfg->tmr);
        (*uart_cfg->uart_callback_fptr)(UART_TX_EMPTY);
    }
}

void uart_tx_handle_transition(uart_tx_t *uart_cfg){
    switch(uart_cfg->state){
        case UART_START: {
            uart_cfg->next_event_time_ticks = get_current_time(uart_cfg);
            port_out(uart_cfg->tx_port, 0);
            uart_cfg->state = UART_DATA;
            uart_cfg->next_event_time_ticks += uart_cfg->bit_time_ticks;
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
            parity &= 1;
            port_out(uart_cfg->tx_port, parity);
            uart_cfg->state = UART_PARITY;
            uart_cfg->next_event_time_ticks += uart_cfg->bit_time_ticks;
        }
     
        case UART_STOP: {   
            port_out(uart_cfg->tx_port, 1);
            uart_cfg->state = UART_IDLE;
            uart_cfg->next_event_time_ticks += uart_cfg->bit_time_ticks;
            break;
        }
        case UART_IDLE:
        default: {
            xassert(0);
        }
    }
     unsigned buffer_used = (uart_cfg->buffer_size && uart_cfg->buffer != NULL);
     if(buffer_used){
        if(uart_cfg->state == UART_IDLE){
            buffered_uart_tx_char_finished(uart_cfg);
        } else {
            sleep_until_next_transition(uart_cfg);
        }
     }
}



void uart_tx(uart_tx_t *uart_cfg, char data){
    //CHeck to see if we are using interrupts/buffered mode
    unsigned buffer_used = (uart_cfg->buffer_size && uart_cfg->buffer != NULL);
    if(buffer_used){
        if(get_buffer_fill_level(uart_cfg) == 0 && uart_cfg->state == UART_IDLE){//Kick off a transmit
            uart_cfg->uart_data = data;
            uart_cfg->current_data_bit = 0;
            uart_cfg->state = UART_START;
            uart_cfg->next_event_time_ticks = get_current_time(uart_cfg);
            sleep_until_next_transition(uart_cfg);
            triggerable_enable_trigger(uart_cfg->tmr);
        } else {//Transaction already underway
            uart_buffer_error_t err = push_char_into_buffer(uart_cfg, data);
        }
    } else {
        uart_cfg->uart_data = data;
        uart_cfg->current_data_bit = 0;
        uart_cfg->state = UART_START;
        while(uart_cfg->state != UART_IDLE){
            uart_tx_handle_transition(uart_cfg);
            sleep_until_next_transition(uart_cfg);
        }
    }
}



// __attribute__((always_inline))
// inline uint32_t load_data_out(
//         const uint8_t *data_out,
//         const int len)
// {
//     uint32_t tmp;
//     uint32_t word_out;

//     tmp = data_out[0] << 8;
//     if (len > 1) {
//         tmp |= data_out[1];
//     }
//     word_out = tmp;
//     asm volatile("zip %0, %1, 0" :"+r"(tmp), "+r"(word_out));
//     return bitrev(word_out);
// }

// __attribute__((always_inline))
// inline void save_data_in(
//         uint8_t *data_in,
//         uint32_t word_in,
//         size_t bytes)
// {
//     uint32_t tmp;

//     word_in = bitrev(word_in);
//     asm volatile("unzip %0, %1, 0" :"+r"(tmp), "+r"(word_in));
//     if (bytes == 1) {
//         data_in[0] = word_in;
//     } else {
//         data_in[1] = word_in;
//         data_in[0] = (word_in >> 8) & 0xFF;
//     }
// }

// void spi_master_transfer(
//         spi_master_device_t *dev,
//         uint8_t *data_out,
//         uint8_t *data_in,
//         size_t len)
// {
//     const uint32_t start_time = 1;
//     spi_master_t *spi = dev->spi_master_ctx;
//     uint32_t word_count;
//     uint32_t remainder;
//     uint32_t tw;
//     uint32_t word;
//     const int do_output = data_out != NULL && spi->mosi_port != 0;
//     const int do_input = data_in != NULL && spi->miso_port != 0;

//     if (len == 0) {
//         return;
//     }

//     word_count = len / sizeof(uint16_t);
//     remainder = len & sizeof(uint16_t) - 1; /* get the byte remainder */

//     if (spi->delay_before_transfer) {
//         /* Ensure the delay time is met */
//         port_sync(spi->cs_port);
//         spi->delay_before_transfer = 0;
//     } else {
//         port_clear_trigger_time(spi->cs_port);
//     }

//     port_set_trigger_time(spi->sclk_port, start_time + dev->clock_delay);

//     if (do_output) {
//         port_set_trigger_time(spi->mosi_port, start_time);
//     }

//     tw = len == 1 ? 16 : 32;

//     spi_io_port_outpw(spi->sclk_port, dev->clock_bits, tw);

//     if (do_output) {
//         spi_io_port_outpw(spi->mosi_port, load_data_out(data_out, len), tw);
//         data_out += 2;
//     }
//     if (do_input) {
//         port_set_trigger_time(spi->miso_port, start_time + (tw - 2) + dev->miso_initial_trigger_delay); /* don't ask. port timing is weird */
//     }

//     clock_start(spi->clock_block);

//     if (word_count > 0) {
//         while (word_count-- != 1) {
//             port_out(spi->sclk_port, dev->clock_bits);

//             if (do_output) {
//                 word = load_data_out(data_out, 2);
//                 port_out(spi->mosi_port, word);
//             }
//             if (do_input) {
//                 word = port_in(spi->miso_port);
//                 save_data_in(data_in, word, 2);
//             }
//             data_out += 2;
//             data_in += 2;
//         }

//         if (remainder > 0) {
//             spi_io_port_outpw(spi->sclk_port, dev->clock_bits, 16);

//             if (do_output) {
//                 word = load_data_out(data_out, 1);
//                 spi_io_port_outpw(spi->mosi_port, word, 16);
//             }
//             if (do_input) {
//                 word = port_in(spi->miso_port);
//                 port_set_shift_count(spi->miso_port, 16);
//                 save_data_in(data_in, word, 2);
//                 data_in += 2;
//             }
//         }
//     }

//     if (do_input) {
//         word = port_in(spi->miso_port);
//         save_data_in(data_in, word, remainder);
//     }

//     port_sync(spi->sclk_port);
//     clock_stop(spi->clock_block);

//     /* Assert CS again now */
//     port_out(spi->cs_port, dev->cs_assert_val);
//     port_sync(spi->cs_port);

//     /*
//      * And assert CS again, scheduled for earliest time CS
//      * is allowed to deassert.
//      */
//     if (dev->clk_to_cs_delay_ticks >= SPI_MASTER_MINIMUM_DELAY) {
//         port_out_at_time(spi->cs_port, port_get_trigger_time(spi->cs_port) + dev->clk_to_cs_delay_ticks, dev->cs_assert_val);
//     }
// }

// void spi_master_end_transaction(
//         spi_master_device_t *dev)
// {
//     const uint32_t cs_deassert_val = 0xFFFFFFFF;
//     spi_master_t *spi = dev->spi_master_ctx;

//     /* enable fast mode and high priority */
//     SPI_IO_CLRSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);

//     port_sync(spi->cs_port);

//     port_out(spi->cs_port, cs_deassert_val);
//     port_sync(spi->cs_port);

//     /*
//      * Deassert CS again, scheduled for earliest time CS
//      * is allowed to be re-asserted. The next transaction
//      * will sync on CS before starting to ensure the minimum
//      * CS to CS time is met.
//      */
//     if (dev->cs_to_cs_delay_ticks >= SPI_MASTER_MINIMUM_DELAY) {
//         port_out_at_time(spi->cs_port, port_get_trigger_time(spi->cs_port) + dev->cs_to_cs_delay_ticks, cs_deassert_val);
//     }
// }

// void spi_master_deinit(
//         spi_master_t *spi)
// {
//     port_disable(spi->cs_port);
//     if (spi->mosi_port != 0) {
//         port_disable(spi->mosi_port);
//     }
//     if (spi->miso_port != 0) {
//         port_disable(spi->miso_port);
//     }
//     port_disable(spi->sclk_port);
//     clock_disable(spi->clock_block);
// }

// void spi_master_device_init(
//         spi_master_device_t *dev,
//         spi_master_t *spi,
//         uint32_t cs_pin,
//         int cpol,
//         int cpha,
//         spi_master_source_clock_t source_clock,
//         uint32_t clock_divisor,
//         spi_master_sample_delay_t miso_sample_delay,
//         uint32_t miso_pad_delay,
//         uint32_t cs_to_clk_delay_ticks,
//         uint32_t clk_to_cs_delay_ticks,
//         uint32_t cs_to_cs_delay_ticks)
// {
//     dev->spi_master_ctx = spi;

//     dev->source_clock = source_clock;
//     dev->clock_divisor = clock_divisor;
//     dev->miso_sample_delay = miso_sample_delay;

//     dev->miso_initial_trigger_delay = (miso_sample_delay + 1) >> 1;
//     dev->miso_pad_delay = miso_pad_delay;

//     dev->cs_assert_val = 0xFFFFFFFF & ~(1 << cs_pin);
//     dev->clock_delay = cpha ? 0 : 1;
//     dev->clock_bits = cpol ? 0xAAAAAAAA : 0x55555555;

//     dev->cs_to_clk_delay_ticks = cs_to_clk_delay_ticks;
//     dev->clk_to_cs_delay_ticks = clk_to_cs_delay_ticks;
//     dev->cs_to_cs_delay_ticks = cs_to_cs_delay_ticks;
// }

// void spi_master_init(
//         spi_master_t *spi,
//         xclock_t clock_block,
//         port_t cs_port,
//         port_t sclk_port,
//         port_t mosi_port,
//         port_t miso_port)
// {
//     /* Setup the clock block */
//     spi->clock_block = clock_block;
//     clock_enable(spi->clock_block);

//     /* Setup the chip select port */
//     spi->cs_port = cs_port;
//     port_enable(spi->cs_port);
//     port_set_clock(spi->cs_port, XS1_CLKBLK_REF);
//     port_out(spi->cs_port, 0xFFFFFFFF);
//     port_sync(spi->cs_port);
//     spi->current_device = 0xFFFFFFFF;

//     /* Setup the SCLK port */
//     spi->sclk_port = sclk_port;
//     port_start_buffered(spi->sclk_port, 32);
//     port_set_clock(spi->sclk_port, spi->clock_block);

//     /* Setup the MOSI port */
//     spi->mosi_port = mosi_port;
//     if (mosi_port != 0) {
//         port_start_buffered(spi->mosi_port, 32);
//         port_set_clock(spi->mosi_port, spi->clock_block);
//         port_clear_buffer(spi->mosi_port);
//     }

//     /* Setup the MISO port */
//     spi->miso_port = miso_port;
//     if (miso_port != 0) {
//         port_start_buffered(spi->miso_port, 32);
//         port_set_clock(spi->miso_port, spi->clock_block);
//         port_clear_buffer(spi->miso_port);
//     }
// }
