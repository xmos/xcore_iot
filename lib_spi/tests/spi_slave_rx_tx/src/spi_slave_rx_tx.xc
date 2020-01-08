// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#include <platform.h>
#include <xclib.h>
#include <stdio.h>
#include <stdlib.h>
#include "spi.h"

out buffered port:32    p_miso = XS1_PORT_1A;
in port                 p_ss   = XS1_PORT_1B;
in port                 p_sclk = XS1_PORT_1C;
in buffered port:32     p_mosi = XS1_PORT_1D;
clock                   cb     = XS1_CLKBLK_1;

out port setup_strobe_port = XS1_PORT_1E;
out port setup_data_port = XS1_PORT_16B;
in port setup_resp_port = XS1_PORT_1F;

#define NUMBER_OF_TEST_BYTES 16
#define KBPS 1000

static const uint8_t tx_data[NUMBER_OF_TEST_BYTES] = {
        0xaa, 0x02, 0x04, 0x08, 0x10, 0x20, 0x04, 0x80,
        0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f
};

static const uint8_t rx_data[NUMBER_OF_TEST_BYTES] = {
        0xaa, 0xf7, 0xfb, 0xef, 0xdf, 0xbf, 0xfd, 0x7f,
        0x01, 0x08, 0x04, 0x10, 0x20, 0x04, 0x02, 0x80,
};


#if (TRANSFER_SIZE == SPI_TRANSFER_SIZE_8)
#define BITS_PER_TRANSFER 8
#elif (TRANSFER_SIZE == SPI_TRANSFER_SIZE_32)
#define BITS_PER_TRANSFER 32
#else
#error Invalid transfer size given
#endif


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
        int mosi_enabled,
        int miso_enabled,
        unsigned num_bits,
        unsigned kbps,
        unsigned initial_clock_delay // in ns
){
    unsigned cpha, cpol;

    set_mode_bits(mode, cpol, cpha);

    setup_strobe_port <: 0;

    send_data_to_tester(setup_strobe_port, setup_data_port, cpol);
    send_data_to_tester(setup_strobe_port, setup_data_port, cpha);
    send_data_to_tester(setup_strobe_port, setup_data_port, miso_enabled);
    send_data_to_tester(setup_strobe_port, setup_data_port, num_bits);
    send_data_to_tester(setup_strobe_port, setup_data_port, kbps);
    send_data_to_tester(setup_strobe_port, setup_data_port, initial_clock_delay);
}

static int request_response(
        out port setup_strobe_port,
        in port setup_resp_port
){
    int r;
    setup_strobe_port <: 1;
    setup_strobe_port <: 0;
    setup_resp_port :> r;
    return r;

}
[[combinable]]
void app(server interface spi_slave_callback_if spi_i,
        int mosi_enabled, int miso_enabled){

    unsigned bpt = 0;
    spi_transfer_type_t tt = TRANSFER_SIZE;
    switch(tt){
    case SPI_TRANSFER_SIZE_8:bpt = 8;break;
    case SPI_TRANSFER_SIZE_32:bpt = 32;break;
    }


    unsigned num_bits = NUMBER_OF_TEST_BYTES*8;

    //First check a multi byte transfer
    broadcast_settings(setup_strobe_port, setup_data_port,
            SPI_MODE, mosi_enabled, miso_enabled, num_bits, KBPS, 2000);

    unsigned rx_byte_no = 0;
    unsigned tx_byte_no = 0;
    while(1){
        select {
            case spi_i.master_requires_data() -> uint32_t r:{
                if(tx_byte_no < NUMBER_OF_TEST_BYTES){
                    switch(tt){
                    case SPI_TRANSFER_SIZE_8:
                        r = tx_data[tx_byte_no];
                        tx_byte_no++;
                        break;
                    case SPI_TRANSFER_SIZE_32:
                        r =   (tx_data[tx_byte_no+3]<<0)
                            | (tx_data[tx_byte_no+2]<<8)
                            | (tx_data[tx_byte_no+1]<<16)
                            | (tx_data[tx_byte_no+0]<<24);
                        tx_byte_no+=4;
                        break;
                    }
                }
                if(!miso_enabled){
                    printf("Error: master cannot require data when miso is not enabled\n");
                    _Exit(1);
                }
                break;
            }
            case spi_i.master_supplied_data(uint32_t datum, uint32_t valid_bits):{
                for(unsigned i=0; i<valid_bits/8;i++){
                    uint8_t d = (datum >> (valid_bits - 8))&0xff;
                    if(rx_data[rx_byte_no] != d){
                        printf("Error: Expected %02x from master but got %02x for transfer of %d\n",
                                rx_data[rx_byte_no], d, num_bits);
                        _Exit(1);
                    }
                    rx_byte_no++;
                    datum <<= 8;
                }

                if(valid_bits < 8){
                    datum <<= (8-valid_bits);
                } else
                    datum >>= (valid_bits-8);
                datum &= 0xff;

                if(valid_bits&0x7){
                    uint32_t d =(rx_data[rx_byte_no]>>(8-(valid_bits&0x7)))<<(8-(valid_bits&0x7));
                    if(datum != d){
                        printf("Error: Expected %02x from master but got %02x for transfer of %d\n",
                                d, datum, num_bits);
                        _Exit(1);
                    }
                }
                break;
            }

            case spi_i.master_ends_transaction():{
                //Then check all sub word transfers
                if(num_bits == NUMBER_OF_TEST_BYTES*8)
                    num_bits = 0;
                num_bits++;

                int r = request_response(setup_strobe_port, setup_resp_port);
                if(r){
                    printf("Error: Master Rx error\n");
                    _Exit(1);

                }
                if(num_bits > bpt)
                    _Exit(1);

                broadcast_settings(setup_strobe_port, setup_data_port,
                        SPI_MODE, mosi_enabled, miso_enabled, num_bits, KBPS, 2000);
                rx_byte_no = 0;
                tx_byte_no = 0;
                break;
            }
        }
    }
}

static void load(static const unsigned num_threads){
    switch(num_threads){
    case 3: par {par(int i=0;i<3;i++) while(1);}break;
    case 6: par {par(int i=0;i<6;i++) while(1);}break;
    case 7: par {par(int i=0;i<7;i++) while(1);}break;
    }
}
#define MOSI_ENABLED 1

#if MISO_ENABLED
#define MISO p_miso
#else
#define MISO null
#endif

int main(){
    interface spi_slave_callback_if i;
    par {
#if COMBINED == 1
        [[combine]]
        par {
            spi_slave(i, p_sclk, p_mosi, MISO, p_ss, cb, SPI_MODE, TRANSFER_SIZE);
            app(i, MOSI_ENABLED, MISO_ENABLED);
        }
#else
        spi_slave(i, p_sclk, p_mosi, MISO, p_ss, cb, SPI_MODE, TRANSFER_SIZE);
        app(i, MOSI_ENABLED, MISO_ENABLED);
#endif
        load(BURNT_THREADS);
    }
    return 0;
}
