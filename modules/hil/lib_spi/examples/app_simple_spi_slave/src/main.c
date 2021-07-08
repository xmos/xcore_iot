// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xcore/clock.h>
#include <xcore/parallel.h>
#include <xcore/port.h>
#include "spi.h"

DECLARE_JOB(dummy_thread, (void));

void start(void *app_data, uint8_t **out_buf, size_t *outbuf_len, uint8_t **in_buf, size_t *inbuf_len);
void end(void *app_data, uint8_t **out_buf, size_t bytes_written, uint8_t **in_buf, size_t bytes_read, size_t read_bits);

// SPI interface ports
port_t p_miso = XS1_PORT_1A;
port_t p_cs   = XS1_PORT_1B;
port_t p_sclk = XS1_PORT_1C;
port_t p_mosi = XS1_PORT_1D;
xclock_t cb   = XS1_CLKBLK_1;

int main(void) {
    static int cpol = 1;
    static int cpha = 1;
    static spi_slave_callback_group_t spi_cbg = {
        .slave_transaction_started = (slave_transaction_started_t) start,
        .slave_transaction_ended = (slave_transaction_ended_t) end,
        .app_data = NULL
    };

    PAR_JOBS(
        PJOB(spi_slave, (&spi_cbg, p_sclk, p_mosi, p_miso, p_cs, cb, cpol, cpha)),
        PJOB(dummy_thread, ())
    );

    return 0;
}

SPI_CALLBACK_ATTR
void start(void *app_data, uint8_t **out_buf, size_t *outbuf_len, uint8_t **in_buf, size_t *inbuf_len) {
    return;
}

SPI_CALLBACK_ATTR
void end(void *app_data, uint8_t **out_buf, size_t bytes_written, uint8_t **in_buf, size_t bytes_read, size_t read_bits) {
    return;
}

void dummy_thread(void) {
    for(;;);
}
