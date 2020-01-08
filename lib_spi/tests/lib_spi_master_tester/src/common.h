// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#ifndef COMMON_H_
#define COMMON_H_

static void set_mode_bits(spi_mode_t mode, unsigned &cpol, unsigned &cpha){
    switch(mode){
        case SPI_MODE_0:cpol = 0; cpha= 1; break;
        case SPI_MODE_1:cpol = 0; cpha= 0; break;
        case SPI_MODE_2:cpol = 1; cpha= 0; break;
        case SPI_MODE_3:cpol = 1; cpha= 1; break;
    }
}

static void send_data_to_tester(
        out port setup_strobe_port,
        out port setup_data_port,
        unsigned data){
    setup_data_port <: data;
    sync(setup_data_port);
    setup_strobe_port <: 1;
    setup_strobe_port <: 0;
}

static void broadcast_settings(
        out port setup_strobe_port,
        out port setup_data_port,
        spi_mode_t mode,
        unsigned speed_in_khz,
        int mosi_enabled,
        int miso_enabled,
        unsigned device_id,
        unsigned inter_frame_gap,
        unsigned num_bytes
){
    unsigned cpha, cpol;

    set_mode_bits(mode, cpol, cpha);

    setup_strobe_port <: 0;

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
