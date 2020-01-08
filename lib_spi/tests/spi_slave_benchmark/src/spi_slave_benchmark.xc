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

   // printf("Testing: %d %d\n", kbps, initial_clock_delay);

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

    unsigned cd_max = 40000;
    unsigned cd_min = 0;
    unsigned cd = (cd_max + cd_min)/2;

    unsigned kbps_max = 250000;
    unsigned kbps_min = 0;
    unsigned kbps = (kbps_max + kbps_min)/2;
// unsigned kbps = 7692;

    int finding_speed = 1;

    uint8_t rx_data_8[16] = {
            0xaa, 0xf7, 0xfb, 0xef, 0xdf, 0xbf, 0xfd, 0x7f,
            0x01, 0x08, 0x04, 0x10, 0x20, 0x04, 0x02, 0x80,
    };

    uint32_t rx_data_32[4] = {
            0xaaf7fbef,0xdfbffd7f, 0x01080410, 0x20040280
    };

    uint8_t tx_data_8[16] = {
            0xaa, 0x02, 0x04, 0x08, 0x10, 0x20, 0x04, 0x80,
            0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f,
    };

    uint32_t tx_data_32[4] = {
            0xaa020408, 0x10200480,
            0xfefdfbf7, 0xefdfbf7f
    };
    int error = 0;
    broadcast_settings(setup_strobe_port, setup_data_port,
            SPI_MODE, mosi_enabled, miso_enabled, bpt*4, kbps, cd);
    unsigned rx_count = 0;
    unsigned tx_count = 0;

    unsigned rep_count = 0;
    while(1){
        select {
            case spi_i.master_requires_data() -> uint32_t r:{

                switch(tt){
                case SPI_TRANSFER_SIZE_8:
                    tx_count &= 0xf;
                    r = tx_data_8[tx_count++];
                    break;
                case SPI_TRANSFER_SIZE_32:
                    tx_count &= 0x3;
                    r = tx_data_32[tx_count++];
                    break;
                default:
                    __builtin_unreachable();
                    break;
                }
                break;
            }
            case spi_i.master_supplied_data(uint32_t datum, uint32_t valid_bits):{

                switch(tt){
                case SPI_TRANSFER_SIZE_8:
                    rx_count &= 0xf;
                    if(datum != rx_data_8[rx_count])
                        error=1;
                    break;
                case SPI_TRANSFER_SIZE_32:
                    rx_count &= 0x3;
                    if(datum != rx_data_32[rx_count])
                        error=1;
                    break;
                default:
                    __builtin_unreachable();
                    break;
                }

                rx_count++;
                if(valid_bits != bpt)
                    error = 1;
                break;
            }

            case spi_i.master_ends_transaction():{
                error |= request_response(setup_strobe_port, setup_resp_port);

                if(finding_speed){
                    if(error == 1)
                        kbps_max = kbps;
                    else
                        kbps_min = kbps;

                    unsigned next_kbps = (kbps_max + kbps_min)/2;
                    if(next_kbps == kbps){
                        rep_count++;
                        if(rep_count == 8){
                            finding_speed = 0;
                            rep_count = 0;
                        }
                    } else{
                        if(rep_count){
                            kbps_max = next_kbps - 1;
                            kbps_min = next_kbps - 1;
                            kbps = next_kbps - 1;
                            rep_count = 0;
                        } else {
                            kbps = next_kbps;
                        }
                    }
                } else {
                    if(error == 1)
                        cd_min = cd;
                    else
                        cd_max = cd;
                    unsigned next_cd = (cd_max + cd_min)/2;
                    if(next_cd == cd){
                        rep_count++;
                        if(rep_count == 8){
                            printf("%d %d %d %d %d %d %d\n", SPI_MODE, TRANSFER_SIZE, BURNT_THREADS, miso_enabled, mosi_enabled, cd, kbps);
                            _Exit(1);
                        }

                    } else{
                        if(rep_count){
                            cd_max = next_cd + 1;
                            cd_min = next_cd + 1;
                            cd = next_cd + 1;
                            rep_count = 0;
                        } else {
                            cd = next_cd;
                        }
                    }

                }
                broadcast_settings(setup_strobe_port, setup_data_port,
                        SPI_MODE, mosi_enabled, miso_enabled,  bpt*4, kbps, cd);
                error = 0;
                rx_count = 0;
                tx_count = 0;
                break;
            }
        }
    }
}

static void load(static const unsigned num_threads){
    switch(num_threads){
    case 2: par {par(int i=0;i<2;i++) while(1);}break;
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
