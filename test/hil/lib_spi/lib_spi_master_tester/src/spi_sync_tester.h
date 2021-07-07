// Copyright 2015-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef SPI_SYNC_TESTER_H_
#define SPI_SYNC_TESTER_H_

#define VERBOSE 0

#include "common.h"

int test_transfer(spi_master_device_t *spi_ctx,
        port_t setup_strobe_port,
        port_t setup_data_port,
        unsigned device_id,
        unsigned inter_frame_gap,
        unsigned cpol,
        unsigned cpha,
        unsigned speed_in_kbps,
        int mosi_enabled,
        int miso_enabled) {
    int error = 0;
    uint8_t rx[NUMBER_OF_TEST_BYTES];

    broadcast_settings(setup_strobe_port, setup_data_port,
                       cpha, cpol,
                       speed_in_kbps,
                       mosi_enabled, miso_enabled,
                       device_id, inter_frame_gap,
                       NUMBER_OF_TEST_BYTES);

    spi_master_start_transaction(spi_ctx);
    spi_master_transfer(spi_ctx, (uint8_t *)tx_data, (uint8_t *)rx, NUMBER_OF_TEST_BYTES);
    spi_master_end_transaction(spi_ctx);

    for (unsigned j=0;j<NUMBER_OF_TEST_BYTES;j++) {
        if(miso_enabled){
            if(rx[j] != rx_data[j]) error = 1;
            if(VERBOSE && (rx[j] != rx_data[j]))
                printf("%02x %02x\n", rx[j], rx_data[j]);
        }
    }

    if (error) {
        printf("ERROR: master got the wrong data\n");
    }

    return error;
}

#endif /* SPI_SYNC_TESTER_H_ */
