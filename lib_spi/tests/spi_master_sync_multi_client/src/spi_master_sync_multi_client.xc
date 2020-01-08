// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#include <platform.h>
#include <xclib.h>
#include <xs1.h>
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

void app(client interface spi_master_if i, int mosi_enabled, int miso_enabled, chanend c){
    set_core_fast_mode_on();
    while(1){
        i.begin_transaction(0, 736, SPI_MODE_3);
        i.transfer8(0xff);
        i.end_transaction(100);
        c <: 1;
    }
}
#define CLIENTS 3
void watcher(chanend c[CLIENTS]){
    int seen[CLIENTS] = {0};
    set_core_fast_mode_on();
    //TODO maybe put a timeout in here
    while(1){
        select{
            case c[int i] :> int:{
                seen[i] = 1;
                int any_not_seen = 0;
                for(unsigned j=0;j<CLIENTS;j++)
                    any_not_seen |= (seen[j] == 0);

                if(!any_not_seen){
                  _Exit(0);
                }
                break;
            }
        }
    }
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
/*
 * Tests:
 *  - no client is ever starved
 */

int main(){
    chan c[3];
    interface spi_master_if i[3];
    par {
        spi_master(i, 3, p_sclk, MOSI, MISO, p_ss, 1, CB);
        app(i[0], MOSI_ENABLED, MISO_ENABLED, c[0]);
        app(i[1], MOSI_ENABLED, MISO_ENABLED, c[1]);
        app(i[2], MOSI_ENABLED, MISO_ENABLED, c[2]);
        watcher(c);
        load(BURNT_THREADS);
    }
    return 0;
}
