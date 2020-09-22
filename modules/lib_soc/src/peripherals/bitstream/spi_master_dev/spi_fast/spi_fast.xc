// Copyright (c) 2016-2020, XMOS Ltd, All rights reserved
#include "spi_fast.h"
#include "soc.h"
#include "xassert.h"
#include "spi_master_dev.h"
#include <xs1.h>
#include <xclib.h>

static unsigned compute_port_ticks(unsigned nanoseconds, unsigned clock_divide)
{
    // Default clock tick is the reference clock, if divided then it is scaled by
    // 2*clock_divide
    unsigned port_tick_nanoseconds = clock_divide ? 10 * clock_divide * 2 : 10;

    // Do a ceiling function to ensure the delay is always at least that requested
    return (nanoseconds + port_tick_nanoseconds - 1) / port_tick_nanoseconds;
}

#pragma unsafe arrays
static unsigned compute_port_value(spi_fast_ports *p,
                                   uint32_t bit_value)
{
    unsigned current_port_value = peek(p->cs);

    if (bit_value) {
        return current_port_value | (1 << p->cs_port_bit);
    } else {
        return current_port_value & ~(1 << p->cs_port_bit);
    }
}

#pragma unsafe arrays
void drive_cs_port_now(spi_fast_ports *p,
                       uint32_t bit_value)
{
    unsigned new_port_value = compute_port_value(p, bit_value);

    p->cs <: new_port_value;
}

#pragma unsafe arrays
void drive_cs_port_at_time(spi_fast_ports *p,
                           uint32_t bit_value,
                           uint32_t time)
{
    unsigned new_port_value = compute_port_value(p, bit_value);

    p->cs @ time <: new_port_value;
}

#pragma unsafe arrays
void drive_cs_port_get_time(spi_fast_ports *p,
                            uint32_t bit_value,
                            uint32_t *time)
{

    unsigned new_port_value = compute_port_value(p, bit_value);

    p->cs <: new_port_value @ *time;
}

#pragma unsafe arrays
void spi_fast_init(spi_fast_ports *p)
{
    stop_clock(p->cb);
    configure_clock_ref(p->cb, p->clock_divide);
    configure_out_port(p->clk, p->cb, p->cpol ? 0xFFFFFFFF : 0x00000000);
    configure_in_port(p->miso, p->cb);
    set_port_sample_delay(p->miso);
    configure_out_port(p->mosi, p->cb, 0);
    set_port_clock(p->cs, p->cb);
    start_clock(p->cb);
    drive_cs_port_now(p, 1);

    /* There is a worst case minimum time of approximately 500 ns (exact time depends on
     * system clock frequency and number of cores) between CS going low and the first
     * data bit going out due to the number of instructions between CS output and first
     * data output.
     */
    unsigned cs_to_data_delay_ns = p->cs_to_data_delay_ns >= SPICONF_MIN_CS_TO_DATA_DELAY_NS ? p->cs_to_data_delay_ns : SPICONF_MIN_CS_TO_DATA_DELAY_NS;

#if !SPICONF_INTERBYTE_DELAY_ENABLE
    xassert(p->byte_setup_ns == 0);
#endif

    p->cs_to_data_delay_ticks = compute_port_ticks(cs_to_data_delay_ns,   p->clock_divide);
    p->byte_setup_ticks       = compute_port_ticks(p->byte_setup_ns,       p->clock_divide);
    p->data_to_cs_delay_ticks = compute_port_ticks(p->data_to_cs_delay_ns, p->clock_divide);

    p->clock_bits = p->cpol ? 0xAAAA : 0x5555;
    p->clock_delay = p->cpha ? 0 : 1;
}

extern "C" {
void spi_fast_xfer(
        unsigned num_bytes,
        char *buffer,
        spi_fast_ports *p,
        int save_input,
        uint32_t start_time,
        uint32_t end_time,
        unsigned cs_disable_value);
}

#pragma unsafe arrays
void spi_fast(unsigned num_bytes, char *buffer, spi_fast_ports *p, spi_direction_t direction)
{
    uint32_t start_time;
    uint32_t end_time;
    int i;
    int save_input = direction != SPI_WRITE;

    unsigned current_cs_port_value = peek(p->cs);

    unsigned cs_disable_value = current_cs_port_value | (1 << p->cs_port_bit);
    unsigned cs_enable_value = current_cs_port_value & ~(1 << p->cs_port_bit);

    /* Prepare the outgoing data */
    for (int i = 0; i < num_bytes; i++) {
        buffer[i] = byterev(bitrev(buffer[i]));
    }

#if SPICONF_INTERBYTE_DELAY_ENABLE
    end_time = (num_bytes - 1) * (16 + p->byte_setup_ticks) + 16 + p->clock_delay + p->data_to_cs_delay_ticks;
#else
    end_time = num_bytes * 16 + p->clock_delay + p->data_to_cs_delay_ticks;
#endif

    p->cs <: cs_enable_value @ start_time;

    start_time += p->cs_to_data_delay_ticks;
    end_time += start_time;

    spi_fast_xfer(num_bytes, buffer, p, save_input, start_time, end_time, cs_disable_value);

    if (save_input) {
        /* Prepare the received data */
        for (i = 0; i < num_bytes; i++) {
            buffer[i] = byterev(bitrev(buffer[i]));
        }
    }
}
