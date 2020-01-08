// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#include <platform.h>
#include <xclib.h>
#include <stdio.h>
#include <stdlib.h>
#include "spi.h"
#include "spi_sync_tester.h"

in buffered port:32   p_miso  = XS1_PORT_1A;
out port              p_ss[1] = {XS1_PORT_1B};
out buffered port:32  p_sclk  = XS1_PORT_1C;
out buffered port:32  p_mosi  = XS1_PORT_1D;
clock                 cb      = XS1_CLKBLK_1;

out port setup_strobe_port = XS1_PORT_1E;
out port setup_data_port = XS1_PORT_16B;

void app(client interface spi_master_if i, int mosi_enabled, int miso_enabled){

#define SPEED_TESTS 1
    unsigned speed_lut[SPEED_TESTS] = {1500};

    for(spi_mode_t mode = SPI_MODE_0; mode <= SPI_MODE_3; mode ++){
        for(unsigned speed_index = 0; speed_index < SPEED_TESTS; speed_index++){
            test_transfer8(i, setup_strobe_port, setup_data_port, 0, 100,
                    mode, speed_lut[speed_index], mosi_enabled, miso_enabled);
        }
    }
    for(spi_mode_t mode = SPI_MODE_0; mode <= SPI_MODE_3; mode ++){
        for(unsigned speed_index = 0; speed_index < SPEED_TESTS; speed_index++){
            test_transfer32(i, setup_strobe_port, setup_data_port, 0, 100,
                    mode, speed_lut[speed_index], mosi_enabled, miso_enabled);
        }
    }
    _Exit(1);
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

#if MISO_ENABLED
#define MISO p_miso
#else
#define MISO null
#endif

#if CB_ENABLED
#define CB cb
#else
#define CB null
#endif

int main(){
    interface spi_master_if i[1];
    par {
        spi_master(i, 1, p_sclk, MOSI, MISO, p_ss, 1, CB);
        app(i[0], MOSI_ENABLED, MISO_ENABLED);
        load(BURNT_THREADS);
    }
    return 0;
}
