// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#include <platform.h>
#include <xclib.h>
#include <stdio.h>
#include <stdlib.h>
#include "spi.h"

in buffered port:32   p_miso  = XS1_PORT_1A;
out port              p_ss[1] = {XS1_PORT_1B};
out buffered port:32  p_sclk  = XS1_PORT_1C;
out buffered port:32  p_mosi  = XS1_PORT_1D;
clock                 cb      = XS1_CLKBLK_1;

out port setup_strobe_port = XS1_PORT_1E;
out port setup_data_port = XS1_PORT_16B;

static unsigned get_max_byte_speed(client interface spi_master_if i){
    unsigned now, then;
    timer t;
    t:> then;
    i.begin_transaction(0, 100, SPI_MODE_3);
    i.transfer8(0xff);
    i.end_transaction(100);
    t:> now;
    unsigned best_time_so_far = now-then;
    unsigned min = 0000, max = 6000;
    while(1){
        //unsigned test_speed = (min + max)/2;
        unsigned test_speed = min + (max-min)/32;
        t:> then;
        i.begin_transaction(0, test_speed, SPI_MODE_3);
        i.transfer8(test_speed);
        i.end_transaction(100);
        t:> now;
        if(now-then < best_time_so_far){
            best_time_so_far = (now-then);
            min = test_speed;
        } else {
            if(max == test_speed)
                return test_speed;
            max = test_speed;
        }
    }
    return 1000;
}

static unsigned get_max_word_speed(client interface spi_master_if i){
    unsigned now, then;
    timer t;
    t:> then;
    i.begin_transaction(0, 100, SPI_MODE_3);
    i.transfer32(0xff);
    i.transfer32(0xff);
    i.end_transaction(100);
    t:> now;
    unsigned best_time_so_far = now-then;
    unsigned min = 0000, max = 6000;
    while(1){
        //unsigned test_speed = (min + max)/2;
        unsigned test_speed = min + (max-min)/32;
        t:> then;
        i.begin_transaction(0, test_speed, SPI_MODE_3);
        i.transfer32(0xff);
        i.transfer32(0xff);
        i.end_transaction(100);
        t:> now;
        if(now-then < best_time_so_far){
            best_time_so_far = (now-then);
            min = test_speed;
        } else {
            if(max == test_speed)
                return test_speed;
            max = test_speed;
        }
    }
    return 1000;
}

void app(client interface spi_master_if i, int mosi_enabled, int miso_enabled){
    printf("%d\n", get_max_byte_speed(i));
    printf("%d\n", get_max_word_speed(i));
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
