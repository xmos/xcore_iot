// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xcore/clock.h>
#include <xcore/parallel.h>
#include <xcore/port.h>
#include "spi.h"

DECLARE_JOB(dummy_thread, (void));

void app(spi_master_t *spi_ctx);

// SPI interface ports
port_t p_miso = XS1_PORT_1A;
port_t p_ss[1] = {XS1_PORT_1B};
port_t p_sclk = XS1_PORT_1C;
port_t p_mosi = XS1_PORT_1D;
xclock_t cb   = XS1_CLKBLK_1;

int main(void) {
    spi_master_t spi_ctx;

    spi_master_init(&spi_ctx, cb, p_ss[0], p_sclk, p_mosi, p_miso);

    PAR_JOBS(
        PJOB(app,(&spi_ctx)),
        PJOB(dummy_thread, ())
    );

    return 0;
}

void app(spi_master_t *spi_ctx) {
    spi_master_device_t spi_dev;

    static int cpol = 1;
    static int cpha = 1;
    uint8_t tx_buf[10];
    uint8_t rx_buf[10];

    spi_master_device_init(&spi_dev, spi_ctx,
        0,
        cpol, cpha,
        spi_master_source_clock_ref,
        0,
        spi_master_sample_delay_0,
        0, 0 ,0 ,0 );

    spi_master_start_transaction(&spi_dev);
    spi_master_transfer(&spi_dev, tx_buf, rx_buf, 10);
    spi_master_end_transaction(&spi_dev);

    for(;;);
}

void dummy_thread(void) {
    for(;;);
}
