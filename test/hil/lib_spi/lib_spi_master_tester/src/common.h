// Copyright 2015-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef COMMON_H_
#define COMMON_H_

#include "xcore/port.h"
#include "xcore/parallel.h"

#define SET_FAST_MODE() asm volatile("setsr %0" : : "n"(XS1_SR_QUEUE_MASK | XS1_SR_FAST_MASK))

DECLARE_JOB(burn, (void));

void burn(void) {
    SET_FAST_MODE();
    for(;;);
}

static void send_data_to_tester(
        port_t setup_strobe_port,
        port_t setup_data_port,
        unsigned data) {
    port_out(setup_data_port, data);
    asm volatile("syncr res[%0]" : : "r" (setup_data_port));
    port_out(setup_strobe_port, 1);
    port_out(setup_strobe_port, 0);
    asm volatile("syncr res[%0]" : : "r" (setup_data_port));
}

static void broadcast_settings(
        port_t setup_strobe_port,
        port_t setup_data_port,
        unsigned cpha,
        unsigned cpol,
        unsigned speed_in_khz,
        int mosi_enabled,
        int miso_enabled,
        unsigned device_id,
        unsigned inter_frame_gap,
        unsigned num_bytes) {
    port_out(setup_strobe_port, 0);

    send_data_to_tester(setup_strobe_port, setup_data_port, cpol);
    send_data_to_tester(setup_strobe_port, setup_data_port, cpha);
    send_data_to_tester(setup_strobe_port, setup_data_port, speed_in_khz);
    send_data_to_tester(setup_strobe_port, setup_data_port, mosi_enabled);
    send_data_to_tester(setup_strobe_port, setup_data_port, miso_enabled);
    send_data_to_tester(setup_strobe_port, setup_data_port, device_id);
    send_data_to_tester(setup_strobe_port, setup_data_port, inter_frame_gap);
    send_data_to_tester(setup_strobe_port, setup_data_port, num_bytes);
}

#define NUMBER_OF_TEST_BYTES 16
#define NUMBER_OF_TEST_WORDS (NUMBER_OF_TEST_BYTES/sizeof(uint32_t))

static const uint8_t tx_data[NUMBER_OF_TEST_BYTES] = {
        0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x04, 0x80,
        0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f
};

static const uint8_t rx_data[NUMBER_OF_TEST_BYTES] = {
        0xfe, 0xf7, 0xfb, 0xef, 0xdf, 0xbf, 0xfd, 0x7f,
        0x01, 0x08, 0x04, 0x10, 0x20, 0x04, 0x02, 0x80,
};


#endif /* COMMON_H_ */
