// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#ifndef SPI_SYNC_TESTER_H_
#define SPI_SYNC_TESTER_H_

#define VERBOSE 0

#include "common.h"

int test_transfer8(client interface spi_master_if i,
        out port setup_strobe_port,
        out port setup_data_port,
        unsigned device_id,
        unsigned inter_frame_gap,
        spi_mode_t mode,
        unsigned speed_in_kbps,
        int mosi_enabled,
        int miso_enabled){

    int error = 0;
    broadcast_settings(setup_strobe_port, setup_data_port, mode, speed_in_kbps,
            mosi_enabled, miso_enabled, device_id, inter_frame_gap, NUMBER_OF_TEST_BYTES);

    i.begin_transaction(device_id, speed_in_kbps, mode);

    for(unsigned j=0;j<NUMBER_OF_TEST_BYTES;j++){
        uint8_t rx = i.transfer8(tx_data[j]);
        if(miso_enabled){
            if(rx != rx_data[j]) error = 1;
            if(VERBOSE && (rx != rx_data[j]))
                printf("%02x %02x\n", rx, rx_data[j]);
        }
    }

    i.end_transaction(inter_frame_gap);

    if(error)
        printf("ERROR: master got the wrong data\n");

    return error;
}

int test_transfer32(client interface spi_master_if i,
        out port setup_strobe_port,
        out port setup_data_port,
        unsigned device_id,
        unsigned inter_frame_gap,
        spi_mode_t mode,
        unsigned speed_in_kbps,
        int mosi_enabled,
        int miso_enabled){

    int error = 0;
    broadcast_settings(setup_strobe_port, setup_data_port,
            mode, speed_in_kbps, mosi_enabled, miso_enabled, device_id, inter_frame_gap, NUMBER_OF_TEST_BYTES);
    i.begin_transaction(device_id, speed_in_kbps, mode);
    for(unsigned j=0;j<NUMBER_OF_TEST_WORDS;j++){
        uint32_t rx = i.transfer32(byterev((tx_data, unsigned[])[j]));
        rx = byterev(rx);
        if(miso_enabled){
            if(rx != (rx_data, unsigned[])[j]) error = 1;
            if(VERBOSE && (rx != (rx_data, unsigned[])[j]))
               printf("%08x %08x\n", rx ,(rx_data, unsigned[])[j]);
        }
    }
    i.end_transaction(inter_frame_gap);

    if(error)
        printf("ERROR: master got the wrong data\n");
    return error;
}



#endif /* SPI_SYNC_TESTER_H_ */
