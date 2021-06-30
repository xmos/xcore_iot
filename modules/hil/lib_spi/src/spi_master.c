// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdint.h>

#include "spi.h"

void spi_master_start_transaction(
        spi_master_device_t *dev)
{
    spi_master_t *spi = dev->spi_master_ctx;

    /* enable fast mode and high priority */
    SPI_IO_SETSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);

    if (dev->cs_assert_val != spi->current_device) {
        spi->current_device = dev->cs_assert_val;

        if (dev->source_clock == spi_master_source_clock_ref) {
            clock_set_source_clk_ref(spi->clock_block);
        } else {
            clock_set_source_clk_xcore(spi->clock_block);
        }
        clock_set_divide(spi->clock_block, dev->clock_divisor);

        if (spi->miso_port != 0) {
            if ((dev->miso_sample_delay & 1) == 0) {
                port_set_sample_falling_edge(spi->miso_port);
            } else {
                port_set_sample_rising_edge(spi->miso_port);
            }
            SPI_IO_RESOURCE_SETC(spi->miso_port, SPI_IO_SETC_PAD_DELAY(dev->miso_pad_delay));
        }

        /* Output the clock idle value */
        clock_start(spi->clock_block);
        spi_io_port_outpw(spi->sclk_port, dev->clock_bits >> 1, 1);
        port_sync(spi->sclk_port);
        clock_stop(spi->clock_block);

        /*
         * This transaction is with a different chip
         * than last time, so there is no need to wait
         * for the minimum CS to CS time.
         */
        port_clear_trigger_time(spi->cs_port);
    } else {
        /*
         * This ensures that the CS_high to CS_low
         * minimum time is met.
         */
        port_sync(spi->cs_port);
    }

    /*
     * The first transfer will sync on CS before starting to
     * ensure the minimum CS to data time is met.
     */
    spi_master_delay_before_next_transfer(dev, dev->cs_to_clk_delay_ticks);
}

__attribute__((always_inline))
inline uint32_t load_data_out(
        const uint8_t *data_out,
        const int len)
{
    uint32_t tmp;
    uint32_t word_out;

    tmp = data_out[0] << 8;
    if (len > 1) {
        tmp |= data_out[1];
    }
    word_out = tmp;
    asm volatile("zip %0, %1, 0" :"+r"(tmp), "+r"(word_out));
    return bitrev(word_out);
}

__attribute__((always_inline))
inline void save_data_in(
        uint8_t *data_in,
        uint32_t word_in,
        size_t bytes)
{
    uint32_t tmp;

    word_in = bitrev(word_in);
    asm volatile("unzip %0, %1, 0" :"+r"(tmp), "+r"(word_in));
    if (bytes == 1) {
        data_in[0] = word_in;
    } else {
        data_in[1] = word_in;
        data_in[0] = (word_in >> 8) & 0xFF;
    }
}

void spi_master_transfer(
        spi_master_device_t *dev,
        uint8_t *data_out,
        uint8_t *data_in,
        size_t len)
{
    const uint32_t start_time = 1;
    spi_master_t *spi = dev->spi_master_ctx;
    uint32_t word_count;
    uint32_t remainder;
    uint32_t tw;
    uint32_t word;
    const int do_output = data_out != NULL && spi->mosi_port != 0;
    const int do_input = data_in != NULL && spi->miso_port != 0;

    if (len == 0) {
        return;
    }

    word_count = len / sizeof(uint16_t);
    remainder = len & sizeof(uint16_t) - 1; /* get the byte remainder */

    if (spi->delay_before_transfer) {
        /* Ensure the delay time is met */
        port_sync(spi->cs_port);
        spi->delay_before_transfer = 0;
    } else {
        port_clear_trigger_time(spi->cs_port);
    }

    port_set_trigger_time(spi->sclk_port, start_time + dev->clock_delay);

    if (do_output) {
        port_set_trigger_time(spi->mosi_port, start_time);
    }

    tw = len == 1 ? 16 : 32;

    spi_io_port_outpw(spi->sclk_port, dev->clock_bits, tw);

    if (do_output) {
        spi_io_port_outpw(spi->mosi_port, load_data_out(data_out, len), tw);
        data_out += 2;
    }
    if (do_input) {
        port_set_trigger_time(spi->miso_port, start_time + (tw - 2) + dev->miso_initial_trigger_delay); /* don't ask. port timing is weird */
    }

    clock_start(spi->clock_block);

    if (word_count > 0) {
        while (word_count-- != 1) {
            port_out(spi->sclk_port, dev->clock_bits);

            if (do_output) {
                word = load_data_out(data_out, 2);
                port_out(spi->mosi_port, word);
            }
            if (do_input) {
                word = port_in(spi->miso_port);
                save_data_in(data_in, word, 2);
            }
            data_out += 2;
            data_in += 2;
        }

        if (remainder > 0) {
            spi_io_port_outpw(spi->sclk_port, dev->clock_bits, 16);

            if (do_output) {
                word = load_data_out(data_out, 1);
                spi_io_port_outpw(spi->mosi_port, word, 16);
            }
            if (do_input) {
                word = port_in(spi->miso_port);
                port_set_shift_count(spi->miso_port, 16);
                save_data_in(data_in, word, 2);
                data_in += 2;
            }
        }
    }

    if (do_input) {
        word = port_in(spi->miso_port);
        save_data_in(data_in, word, remainder);
    }

    port_sync(spi->sclk_port);
    clock_stop(spi->clock_block);

    /* Assert CS again now */
    port_out(spi->cs_port, dev->cs_assert_val);
    port_sync(spi->cs_port);

    /*
     * And assert CS again, scheduled for earliest time CS
     * is allowed to deassert.
     */
    if (dev->clk_to_cs_delay_ticks >= SPI_MASTER_MINIMUM_DELAY) {
        port_out_at_time(spi->cs_port, port_get_trigger_time(spi->cs_port) + dev->clk_to_cs_delay_ticks, dev->cs_assert_val);
    }
}

void spi_master_end_transaction(
        spi_master_device_t *dev)
{
    const uint32_t cs_deassert_val = 0xFFFFFFFF;
    spi_master_t *spi = dev->spi_master_ctx;

    /* enable fast mode and high priority */
    SPI_IO_CLRSR(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK);

    port_sync(spi->cs_port);

    port_out(spi->cs_port, cs_deassert_val);
    port_sync(spi->cs_port);

    /*
     * Deassert CS again, scheduled for earliest time CS
     * is allowed to be re-asserted. The next transaction
     * will sync on CS before starting to ensure the minimum
     * CS to CS time is met.
     */
    if (dev->cs_to_cs_delay_ticks >= SPI_MASTER_MINIMUM_DELAY) {
        port_out_at_time(spi->cs_port, port_get_trigger_time(spi->cs_port) + dev->cs_to_cs_delay_ticks, cs_deassert_val);
    }
}

void spi_master_deinit(
        spi_master_t *spi)
{
    port_disable(spi->cs_port);
    if (spi->mosi_port != 0) {
        port_disable(spi->mosi_port);
    }
    if (spi->miso_port != 0) {
        port_disable(spi->miso_port);
    }
    port_disable(spi->sclk_port);
    clock_disable(spi->clock_block);
}

void spi_master_device_init(
        spi_master_device_t *dev,
        spi_master_t *spi,
        uint32_t cs_pin,
        int cpol,
        int cpha,
        spi_master_source_clock_t source_clock,
        uint32_t clock_divisor,
        spi_master_sample_delay_t miso_sample_delay,
        uint32_t miso_pad_delay,
        uint32_t cs_to_clk_delay_ticks,
        uint32_t clk_to_cs_delay_ticks,
        uint32_t cs_to_cs_delay_ticks)
{
    dev->spi_master_ctx = spi;

    dev->source_clock = source_clock;
    dev->clock_divisor = clock_divisor;
    dev->miso_sample_delay = miso_sample_delay;

    dev->miso_initial_trigger_delay = (miso_sample_delay + 1) >> 1;
    dev->miso_pad_delay = miso_pad_delay;

    dev->cs_assert_val = 0xFFFFFFFF & ~(1 << cs_pin);
    dev->clock_delay = cpha ? 0 : 1;
    dev->clock_bits = cpol ? 0xAAAAAAAA : 0x55555555;

    dev->cs_to_clk_delay_ticks = cs_to_clk_delay_ticks;
    dev->clk_to_cs_delay_ticks = clk_to_cs_delay_ticks;
    dev->cs_to_cs_delay_ticks = cs_to_cs_delay_ticks;
}

void spi_master_init(
        spi_master_t *spi,
        xclock_t clock_block,
        port_t cs_port,
        port_t sclk_port,
        port_t mosi_port,
        port_t miso_port)
{
    /* Setup the clock block */
    spi->clock_block = clock_block;
    clock_enable(spi->clock_block);

    /* Setup the chip select port */
    spi->cs_port = cs_port;
    port_enable(spi->cs_port);
    port_set_clock(spi->cs_port, XS1_CLKBLK_REF);
    port_out(spi->cs_port, 0xFFFFFFFF);
    port_sync(spi->cs_port);
    spi->current_device = 0xFFFFFFFF;

    /* Setup the SCLK port */
    spi->sclk_port = sclk_port;
    port_start_buffered(spi->sclk_port, 32);
    port_set_clock(spi->sclk_port, spi->clock_block);

    /* Setup the MOSI port */
    spi->mosi_port = mosi_port;
    if (mosi_port != 0) {
        port_start_buffered(spi->mosi_port, 32);
        port_set_clock(spi->mosi_port, spi->clock_block);
        port_clear_buffer(spi->mosi_port);
    }

    /* Setup the MISO port */
    spi->miso_port = miso_port;
    if (miso_port != 0) {
        port_start_buffered(spi->miso_port, 32);
        port_set_clock(spi->miso_port, spi->clock_block);
        port_clear_buffer(spi->miso_port);
    }
}
