// Copyright 2015-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <string.h>
#include <xclib.h>
#include <stdio.h>
#include <stdlib.h>
#include <xcore/clock.h>
#include <xcore/parallel.h>
#include <xcore/port.h>
#include <xcore/hwtimer.h>
#include <xcore/triggerable.h>
#include <xcore/interrupt_wrappers.h>
#include <xcore/interrupt.h>
#include "spi.h"

port_t p_miso = XS1_PORT_1A;
port_t p_cs   = XS1_PORT_1B;
port_t p_sclk = XS1_PORT_1C;
port_t p_mosi = XS1_PORT_1D;
xclock_t cb   = XS1_CLKBLK_1;

port_t setup_strobe_port = XS1_PORT_1E;
port_t setup_data_port = XS1_PORT_16B;
port_t setup_resp_port = XS1_PORT_1F;

#define NUMBER_OF_TEST_BYTES 16
#define KBPS 25000

#ifndef INITIAL_CLOCK_DELAY
#define INITIAL_CLOCK_DELAY 2000
#endif

static const uint8_t tx_data[NUMBER_OF_TEST_BYTES] = {
        0xaa, 0x02, 0x04, 0x08, 0x10, 0x20, 0x04, 0x80,
        0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f
};

static const uint8_t rx_data[NUMBER_OF_TEST_BYTES] = {
        0xaa, 0xf7, 0xfb, 0xef, 0xdf, 0xbf, 0xfd, 0x7f,
        0x01, 0x08, 0x04, 0x10, 0x20, 0x04, 0x02, 0x80,
};

#if SPI_MODE == 0
#define CPOL 0
#define CPHA 0
#endif

#if SPI_MODE == 1
#define CPOL 0
#define CPHA 1
#endif

#if SPI_MODE == 2
#define CPOL 1
#define CPHA 0
#endif

#if SPI_MODE == 3
#define CPOL 1
#define CPHA 1
#endif

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

typedef struct app_data {
    unsigned num_bits;
} app_data_t;

#define SET_FAST_MODE() asm volatile("setsr %0" : : "n"(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK))

DECLARE_JOB(burn, (void));

void burn(void) {
    SET_FAST_MODE();
    for(;;);
}

static void send_data_to_tester(
        port_t setup_strobe_port,
        port_t setup_data_port,
        unsigned data){
    port_out(setup_data_port, data);
    asm volatile("syncr res[%0]" : : "r" (setup_data_port));
    port_out(setup_strobe_port, 1);
    port_out(setup_strobe_port, 0);
    asm volatile("syncr res[%0]" : : "r" (setup_data_port));
}

static void broadcast_settings(
        port_t setup_strobe_port,
        port_t setup_data_port,
        unsigned cpol,
        unsigned cpha,
        int mosi_enabled,
        int miso_enabled,
        unsigned num_bits,
        unsigned kbps,
        unsigned initial_clock_delay) {  // in ns
    port_out(setup_strobe_port, 0);

    send_data_to_tester(setup_strobe_port, setup_data_port, cpol);
    send_data_to_tester(setup_strobe_port, setup_data_port, cpha);
    send_data_to_tester(setup_strobe_port, setup_data_port, miso_enabled);
    send_data_to_tester(setup_strobe_port, setup_data_port, num_bits);
    send_data_to_tester(setup_strobe_port, setup_data_port, kbps);
    send_data_to_tester(setup_strobe_port, setup_data_port, initial_clock_delay);
}

static uint32_t request_response(
        port_t setup_strobe_port,
        port_t setup_resp_port) {
    port_enable(setup_resp_port);
    port_out(setup_strobe_port, 1);
    port_out(setup_strobe_port, 0);
    uint32_t tmp = port_in(setup_resp_port);
    return tmp;
}

static uint8_t input_buffer[NUMBER_OF_TEST_BYTES];

SPI_CALLBACK_ATTR
void start(void *app_data, uint8_t **out_buf, size_t *outbuf_len, uint8_t **in_buf, size_t *inbuf_len) {
    if (MISO_ENABLED) {
#if IN_PLACE_TRANSACTION
        memcpy(input_buffer, tx_data, NUMBER_OF_TEST_BYTES);
        *out_buf = (uint8_t*)input_buffer;
#else
        *out_buf = (uint8_t*)tx_data;
#endif
        *outbuf_len = NUMBER_OF_TEST_BYTES;
    } else {
        *out_buf = NULL;
        *outbuf_len = 0;
    }
    *in_buf =  (uint8_t*)input_buffer;
    *inbuf_len = NUMBER_OF_TEST_BYTES;
}

SPI_CALLBACK_ATTR
void end(void *app_data, uint8_t **out_buf, size_t bytes_written, uint8_t **in_buf, size_t bytes_read, size_t read_bits) {
    app_data_t *data = (app_data_t*)app_data;

    /* Check that we received the expected number of bytes */
#if MOSI_ENABLED
    if (((bytes_read * 8) + read_bits) != data->num_bits) {
        printf("Error: Expected %d bits from master but got %d\n", data->num_bits, ((bytes_read * 8) + read_bits));
        _Exit(1);
    }
#else
    if (((bytes_read * 8) + read_bits) != 0) {
        printf("Error: Expected %d bits from master but got %d\n", 0, ((bytes_read * 8) + read_bits));
        _Exit(1);
    }
#endif

    /* Multiword transfer complete, now test all sub word transfers */
    if (data->num_bits == NUMBER_OF_TEST_BYTES*8) {
        data->num_bits = 0;
    }
    data->num_bits++;

    // printf("Read %d bytes and %d bits from master\n", bytes_read, read_bits);
    // printf("Wrote %d bytes to master\n", bytes_written);

    /* Check full bytes that we recieved */
    for (int i=0; i<bytes_read; i++) {
        if (rx_data[i] != (*in_buf)[i]) {
            printf("Error: Expected 0x%X from master but got 0x%X\n", rx_data[i], (*in_buf)[i]);
            _Exit(1);
        }
    }

    /* If we received a partial byte, check the bits */
    if (read_bits > 0) {
        uint8_t partial_byte = (*in_buf)[bytes_read];
        uint8_t cmp_val = rx_data[bytes_read];

        cmp_val = cmp_val >> (8-read_bits);

        if (cmp_val != partial_byte) {
            printf("Error: Expected 0x%02x from master but got 0x%02x for bit transfer of %d\n",
                    cmp_val, partial_byte, data->num_bits-1);
            _Exit(1);
        }
    }

    /* Complete if we have tested every subset of partial transfers */
    if (data->num_bits > 32) {
        printf("Test completed\n");
        _Exit(1);
    }

    uint32_t r = request_response(setup_strobe_port, setup_resp_port);

    if (r) {
        printf("Error: Master Rx error\n");
        _Exit(1);
    }

    memset(input_buffer, 0x00, NUMBER_OF_TEST_BYTES);

    //printf("sending settings: %d, %d, %d, %d, %d, %d, %d\n",
    //    CPOL, CPHA, MOSI_ENABLED, MISO_ENABLED, data->num_bits, KBPS, 2000);

    broadcast_settings(setup_strobe_port, setup_data_port,
            CPOL, CPHA, MOSI_ENABLED, MISO_ENABLED, data->num_bits, KBPS, INITIAL_CLOCK_DELAY);
}

DEFINE_INTERRUPT_PERMITTED(spi_isr_grp, void, app, void)
{
    app_data_t app_data = {
        .num_bits = NUMBER_OF_TEST_BYTES*8
    };
    spi_slave_callback_group_t spi_cbg = {
        .slave_transaction_started = (slave_transaction_started_t) start,
        .slave_transaction_ended = (slave_transaction_ended_t) end,
        .app_data = &app_data
    };

    port_enable(setup_strobe_port);
    port_enable(setup_data_port);

    printf("Send initial settings\n");
    /* First check a multi byte transfer */
    broadcast_settings(setup_strobe_port, setup_data_port,
            CPOL, CPHA, MOSI_ENABLED, MISO_ENABLED, app_data.num_bits, KBPS, INITIAL_CLOCK_DELAY);

    spi_slave(&spi_cbg, p_sclk, MOSI, MISO, p_cs, cb, CPOL, CPHA);
}

int main(void) {
    PAR_JOBS(
        PJOB(INTERRUPT_PERMITTED(app), ()),
#if FULL_LOAD == 1
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
        PJOB(burn, ()),
#endif
        PJOB(burn, ())
    );

    return 0;
}
