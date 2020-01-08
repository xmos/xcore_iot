// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#include <platform.h>
#include <xclib.h>
#include <stdio.h>
#include <stdlib.h>
#include "spi.h"
#include "spi_sync_tester.h"

#define NUM_SS 2

in buffered port:32   p_miso  = XS1_PORT_1A;
out port              p_ss[2] = {XS1_PORT_1B, XS1_PORT_1G};
out buffered port:32  p_sclk  = XS1_PORT_1C;
out buffered port:32  p_mosi  = XS1_PORT_1D;
clock                 cb      = XS1_CLKBLK_1;

out port setup_strobe_port = XS1_PORT_1E;
out port setup_data_port = XS1_PORT_16B;

#define KBPS 1000

void app(client interface spi_master_if i, unsigned num_ss,
        int mosi_enabled, int miso_enabled){
    //set fast mode on
    unsigned ifg = 1000;

    spi_mode_t mode = SPI_MODE_0;

    test_transfer8 (i, setup_strobe_port, setup_data_port, 0, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);
    test_transfer8 (i, setup_strobe_port, setup_data_port, 0, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);
    test_transfer32(i, setup_strobe_port, setup_data_port, 0, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);
    test_transfer32(i, setup_strobe_port, setup_data_port, 0, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);
    test_transfer8 (i, setup_strobe_port, setup_data_port, 0, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);

    test_transfer8 (i, setup_strobe_port, setup_data_port, 1, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);
    test_transfer32(i, setup_strobe_port, setup_data_port, 1, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);
    test_transfer32(i, setup_strobe_port, setup_data_port, 1, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);
    test_transfer8 (i, setup_strobe_port, setup_data_port, 1, ifg,
            mode, KBPS, mosi_enabled, miso_enabled);

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
        spi_master(i, 1, p_sclk, MOSI, MISO, p_ss, NUM_SS, CB);
        app(i[0], NUM_SS,  MOSI_ENABLED, MISO_ENABLED);
        load(BURNT_THREADS);
    }
    return 0;
}
