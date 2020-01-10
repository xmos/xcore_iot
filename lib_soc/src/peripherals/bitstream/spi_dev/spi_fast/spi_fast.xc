// Copyright (c) 2016-2020, XMOS Ltd, All rights reserved
#include "spi_fast.h"
#include <xclib.h>

static unsigned compute_port_ticks(unsigned nanoseconds, unsigned clock_divide) {
    // Default clock tick is the reference clock, if divided then it is scaled by
    // 2*clock_divide
    unsigned port_tick_nanoseconds = clock_divide ? 10 * clock_divide * 2 : 10;

    // Do a ceiling function to ensure the delay is always at least that requested
    return (nanoseconds + port_tick_nanoseconds - 1) / port_tick_nanoseconds;
}

static unsigned compute_port_value(spi_fast_ports &p,
                                   uint32_t p_ss_bit,
                                   uint32_t bit_value) {
    // Get the current state of the port
    unsigned current_port_value = peek(p.cs);
    // Zero the p_ss_bit
    unsigned new_port_value = (current_port_value & ~(1 << p_ss_bit));
    // Or desired bit value into p_ss_bit
    return new_port_value | (bit_value << p_ss_bit);
}

void drive_cs_port_now(spi_fast_ports &p,
                       uint32_t p_ss_bit,
                       uint32_t bit_value) {

    unsigned new_port_value = compute_port_value(p, p_ss_bit, bit_value);

    p.cs <: new_port_value;
}

void drive_cs_port_at_time(spi_fast_ports &p,
                           uint32_t p_ss_bit,
                           uint32_t bit_value,
                           unsigned *time) {
    unsigned new_port_value = compute_port_value(p, p_ss_bit, bit_value);

    p.cs @ *time <: new_port_value;
}

void drive_cs_port_get_time(spi_fast_ports &p,
                            uint32_t p_ss_bit,
                            uint32_t bit_value,
                            unsigned *time) {

    unsigned new_port_value = compute_port_value(p, p_ss_bit, bit_value);

    p.cs <: new_port_value @ *time;
}

void spi_fast_init(spi_fast_ports &p){
    stop_clock(p.cb);
    configure_clock_ref(p.cb, p.clock_divide);
    configure_out_port(p.clk, p.cb, p.cpol ? 0xFFFFFFFF : 0x00000000);
    configure_in_port(p.miso, p.cb);
    configure_out_port(p.mosi, p.cb, 0);
    set_port_clock(p.cs, p.cb);
    start_clock(p.cb);
    drive_cs_port_now(p, p.cs_port_bit, 1);
    p.cs_to_data_delay_ticks = compute_port_ticks(p.cs_to_data_delay_ns, p.clock_divide);
    p.byte_setup_ticks = compute_port_ticks(p.byte_setup_ns, p.clock_divide);
    p.clock_bits = p.cpol ? 0xAAAA : 0x5555;
    p.clock_delay = p.cpha ? 0 : 1;
}

void spi_fast(unsigned num_bytes, char *buffer, spi_fast_ports &p, spi_direction_t direction) {

    // Prepare the outgoing data
    // TODO: optimise the data reversal
    for (int i = 0; i < num_bytes; i++) {
        buffer[i] = byterev(bitrev(buffer[i]));
    }

    unsigned start_time;
    drive_cs_port_get_time(p, p.cs_port_bit, 0, &start_time);

    unsigned port_time = start_time + p.cs_to_data_delay_ticks;

    partout_timed(p.clk, 16, p.clock_bits, port_time + p.clock_delay);
    partout_timed(p.mosi, 16, zip(buffer[0], buffer[0], 0), port_time);
    asm volatile ("setpt res[%0], %1":: "r"(p.miso), "r"(port_time+15));
    asm volatile ("setpsc res[%0], %1":: "r"(p.miso), "r"(16));

    port_time += 16 + p.byte_setup_ticks;

    unsigned i;
    unsigned tmp;
    for (i = 1; i < num_bytes; i++) {
        partout_timed(p.clk, 16, p.clock_bits, port_time + p.clock_delay);
        partout_timed(p.mosi, 16, zip(buffer[i], buffer[i], 0), port_time);
        asm volatile ("in %0, res[%1]": "=r"(tmp) : "r"(p.miso));
        asm volatile ("setpt res[%0], %1":: "r"(p.miso), "r"(port_time+15));
        asm volatile ("setpsc res[%0], %1":: "r"(p.miso), "r"(16));
        if (direction != SPI_WRITE) {
            {buffer[i-1], void} = unzip(tmp >> 16, 0);
        }

        port_time += 16 + p.byte_setup_ticks;
    }
    asm volatile ("in %0, res[%1]": "=r"(tmp) : "r"(p.miso));
    if (direction != SPI_WRITE) {
        {buffer[i-1], void} = unzip(tmp >> 16, 0);
    }

    delay_microseconds(10); // Makes some devices work better... TODO: make configurable
    drive_cs_port_now(p, p.cs_port_bit, 1);
    delay_microseconds(5); // Makes some devices work better... TODO: make configurable

    // Prepare the received data
    // TODO: optimise the data reversal
    for (int i = 0; i < num_bytes; i++) {
        buffer[i] = byterev(bitrev(buffer[i]));
    }
}
