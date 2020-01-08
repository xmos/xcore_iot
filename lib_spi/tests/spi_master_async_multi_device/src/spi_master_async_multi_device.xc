// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#include <platform.h>
#include <xclib.h>
#include <stdio.h>
#include <stdlib.h>
#include "spi.h"
#include "spi_async_tester.h"

#define NUM_SS 2

in buffered port:32    p_miso   = XS1_PORT_1A;
out port               p_ss[2]  = {XS1_PORT_1B, XS1_PORT_1G};
out buffered port:32   p_sclk   = XS1_PORT_1C;
out buffered port:32   p_mosi   = XS1_PORT_1D;
clock                  cb0      = XS1_CLKBLK_1;
clock                  cb1      = XS1_CLKBLK_2;

out port setup_strobe_port = XS1_PORT_1E;
out port setup_data_port = XS1_PORT_16B;

#define KBPS 50000
static int test_transfer8(client interface spi_master_async_if i,
        out port setup_strobe_port,
        out port setup_data_port,
        unsigned device_id,
        unsigned inter_frame_gap,
        spi_mode_t mode,
        unsigned speed_in_kbps,
        int mosi_enabled,
        int miso_enabled,
        unsigned byte_count
){
    int error = 0;
    broadcast_settings(setup_strobe_port, setup_data_port, mode, speed_in_kbps,
            mosi_enabled, miso_enabled, device_id, inter_frame_gap, byte_count);

    uint8_t tx[NUMBER_OF_TEST_BYTES];
    uint8_t rx[NUMBER_OF_TEST_BYTES];
    uint8_t * movable tx_ptr = tx;
    uint8_t * movable rx_ptr = rx;

    //replace with memcpy
    for(unsigned i=0;i<byte_count;i++)
        tx_ptr[i] = tx_data[i];

    //it would be good to test segmented transfers too
    i.begin_transaction(device_id, speed_in_kbps, mode);
    if(byte_count){
        i.init_transfer_array_8(move(rx_ptr), move(tx_ptr), byte_count);

        select {
            case i.transfer_complete():{
                break;
            }
        }

        i.retrieve_transfer_buffers_8(rx_ptr, tx_ptr);
    }
    i.end_transaction(inter_frame_gap);

    for(unsigned j=0;j<byte_count;j++){
        uint8_t rx = rx_ptr[j];
        if(miso_enabled){
            if(rx != rx_data[j]) error = 1;
            if(VERBOSE && (rx != rx_data[j]))
                printf("%02x %02x\n", rx, rx_data[j]);
        }
    }
    if(error)
        printf("ERROR: master got the wrong data\n");
    if(VERBOSE)
        printf("Transfer complete, error:%d\n", error);

    return error;
}

static int test_transfer32(client interface spi_master_async_if i,
        out port setup_strobe_port,
        out port setup_data_port,
        unsigned device_id,
        unsigned inter_frame_gap,
        spi_mode_t mode,
        unsigned speed_in_kbps,
        int mosi_enabled,
        int miso_enabled){

    int error = 0;

    uint32_t tx[NUMBER_OF_TEST_WORDS];
    uint32_t rx[NUMBER_OF_TEST_WORDS];
    uint32_t * movable tx_ptr = tx;
    uint32_t * movable rx_ptr = rx;

    broadcast_settings(setup_strobe_port, setup_data_port,
            mode, speed_in_kbps, mosi_enabled, miso_enabled, device_id, inter_frame_gap, NUMBER_OF_TEST_BYTES);

    for(unsigned j=0;j<NUMBER_OF_TEST_WORDS;j++)
        tx_ptr[j] = byterev((tx_data, unsigned[])[j]);



    i.begin_transaction(device_id, speed_in_kbps, mode);
    i.init_transfer_array_32(move(rx_ptr), move(tx_ptr), NUMBER_OF_TEST_WORDS);
    select {
        case i.transfer_complete():{
            break;
        }
    }
    i.retrieve_transfer_buffers_32(rx_ptr, tx_ptr);
    i.end_transaction(inter_frame_gap);


    for(unsigned j=0;j<NUMBER_OF_TEST_WORDS;j++){
        uint32_t rx = rx_ptr[j];
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

void app(client interface spi_master_async_if i, unsigned num_ss,
        int mosi_enabled, int miso_enabled){
    //set fast mode on
    unsigned ifg = 1000;

    spi_mode_t mode = SPI_MODE_0;

    test_transfer8 (i, setup_strobe_port, setup_data_port, 1, ifg,
            mode, KBPS, mosi_enabled, miso_enabled, NUMBER_OF_TEST_BYTES);

    test_transfer8 (i, setup_strobe_port, setup_data_port, 0, ifg,
            mode, KBPS, mosi_enabled, miso_enabled, NUMBER_OF_TEST_BYTES);
    test_transfer32(i, setup_strobe_port, setup_data_port, 0, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);
    test_transfer32(i, setup_strobe_port, setup_data_port, 0, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);
    test_transfer8 (i, setup_strobe_port, setup_data_port, 0, ifg,
            mode, KBPS, mosi_enabled, miso_enabled, NUMBER_OF_TEST_BYTES);

    test_transfer8 (i, setup_strobe_port, setup_data_port, 1, ifg,
            mode, KBPS, mosi_enabled, miso_enabled, NUMBER_OF_TEST_BYTES);
    test_transfer32(i, setup_strobe_port, setup_data_port, 1, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);
    test_transfer32(i, setup_strobe_port, setup_data_port, 1, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);
    test_transfer8 (i, setup_strobe_port, setup_data_port, 1, ifg,
            mode, KBPS, mosi_enabled, miso_enabled, NUMBER_OF_TEST_BYTES);


    test_transfer8 (i, setup_strobe_port, setup_data_port, 0, ifg,
            mode, KBPS, mosi_enabled, miso_enabled, NUMBER_OF_TEST_BYTES);
    test_transfer32(i, setup_strobe_port, setup_data_port, 0, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);
    test_transfer32(i, setup_strobe_port, setup_data_port, 0, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);
    test_transfer8 (i, setup_strobe_port, setup_data_port, 0, ifg,
            mode, KBPS, mosi_enabled, miso_enabled, NUMBER_OF_TEST_BYTES);

    _Exit(0);
}

static void load(static const unsigned num_threads){
    switch(num_threads){
    case 3: par {par(int i=0;i<3;i++) while(1);}break;
    case 4: par {par(int i=0;i<4;i++) while(1);}break;
    case 5: par {par(int i=0;i<5;i++) while(1);}break;
    case 6: par {par(int i=0;i<6;i++) while(1);}break;
    case 7: par {par(int i=0;i<7;i++) while(1);}break;
    }
}

#if MOSI_ENABLED
#define MOSI p_mosi
#else
#define MOSI null
#endif

int main(){
    interface spi_master_async_if i[1];
    par {
        spi_master_async(i, 1, p_sclk, MOSI, p_miso, p_ss, 2, cb0, cb1);
        app(i[0], NUM_SS, MOSI_ENABLED, 1);
        load(BURNT_THREADS);
    }
    return 0;
}
