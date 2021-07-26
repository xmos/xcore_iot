// Copyright 2015-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <string.h>
#include <xclib.h>
#include <stdio.h>
#include <stdlib.h>
#include <xcore/clock.h>
#include <xcore/port.h>
#include <xcore/parallel.h>
#include "spi.h"
#include "spi_sync_tester.h"

port_t p_miso  = XS1_PORT_1A;
port_t p_ss = XS1_PORT_4A;
port_t p_sclk = XS1_PORT_1C;
port_t p_mosi = XS1_PORT_1D;
xclock_t cb = XS1_CLKBLK_1;

port_t setup_strobe_port = XS1_PORT_1E;
port_t setup_data_port = XS1_PORT_16B;


#if MOSI_ENABLED
#define MOSI p_mosi
#else
#define MOSI 0
#endif

#if MISO_ENABLED
#define MISO p_miso
#else
#define MISO 0
#endif

#if MODE == 0
#define CPOL 0
#define CPHA 0
#elif MODE == 1
#define CPOL 0
#define CPHA 1
#elif MODE == 2
#define CPOL 1
#define CPHA 0
#else
#define CPOL 1
#define CPHA 1
#endif

void app(spi_master_t *spi_ctx, int mosi_enabled, int miso_enabled) {
    spi_master_device_t spi_dev_0;
    spi_master_device_t spi_dev_1;

    spi_master_device_init(&spi_dev_0, spi_ctx,
        0,
        CPOL, CPHA,
        spi_master_source_clock_xcore,
        DIV,
        spi_master_sample_delay_0,
        0, 0 ,0 ,0 );

    spi_master_device_init(&spi_dev_1, spi_ctx,
        1,
        CPOL, CPHA,
        spi_master_source_clock_ref,
        DIV,
        spi_master_sample_delay_0,
        0, 0 ,0 ,0 );

    for (int i=0; i<2; i++) {
        test_transfer(&spi_dev_0, setup_strobe_port, setup_data_port, 0, 0,
                CPOL, CPHA, 800000/(DIV*4), mosi_enabled, miso_enabled);
        printf("Transfers to device 0 complete\n");

        test_transfer(&spi_dev_1, setup_strobe_port, setup_data_port, 1, 0,
                CPOL, CPHA, 800000/(DIV*4), mosi_enabled, miso_enabled);
        printf("Transfers to device 1 complete\n");
    }

    _Exit(1);
}

int main() {
    spi_master_t spi_ctx;

    spi_master_init(&spi_ctx, cb, p_ss, p_sclk, p_mosi, p_miso);

    port_enable(setup_strobe_port);
    port_enable(setup_data_port);

    PAR_JOBS(
        PJOB(app,(&spi_ctx, MOSI_ENABLED, MISO_ENABLED)),
#if FULL_LOAD == 1
        PJOB(burn,()),
        PJOB(burn,()),
        PJOB(burn,()),
        PJOB(burn,()),
        PJOB(burn,()),
        PJOB(burn,()),
#endif
        PJOB(burn,())
    );

    return 0;
}
