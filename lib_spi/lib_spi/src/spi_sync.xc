// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <xclib.h>
#include <stdio.h>
#include <stdlib.h>

#include "spi.h"

static const uint16_t ziptable[256] = { 0x0000, 0xc000, 0x3000, 0xf000, 0x0c00,
        0xcc00, 0x3c00, 0xfc00, 0x0300, 0xc300, 0x3300, 0xf300, 0x0f00, 0xcf00,
        0x3f00, 0xff00, 0x00c0, 0xc0c0, 0x30c0, 0xf0c0, 0x0cc0, 0xccc0, 0x3cc0,
        0xfcc0, 0x03c0, 0xc3c0, 0x33c0, 0xf3c0, 0x0fc0, 0xcfc0, 0x3fc0, 0xffc0,
        0x0030, 0xc030, 0x3030, 0xf030, 0x0c30, 0xcc30, 0x3c30, 0xfc30, 0x0330,
        0xc330, 0x3330, 0xf330, 0x0f30, 0xcf30, 0x3f30, 0xff30, 0x00f0, 0xc0f0,
        0x30f0, 0xf0f0, 0x0cf0, 0xccf0, 0x3cf0, 0xfcf0, 0x03f0, 0xc3f0, 0x33f0,
        0xf3f0, 0x0ff0, 0xcff0, 0x3ff0, 0xfff0, 0x000c, 0xc00c, 0x300c, 0xf00c,
        0x0c0c, 0xcc0c, 0x3c0c, 0xfc0c, 0x030c, 0xc30c, 0x330c, 0xf30c, 0x0f0c,
        0xcf0c, 0x3f0c, 0xff0c, 0x00cc, 0xc0cc, 0x30cc, 0xf0cc, 0x0ccc, 0xcccc,
        0x3ccc, 0xfccc, 0x03cc, 0xc3cc, 0x33cc, 0xf3cc, 0x0fcc, 0xcfcc, 0x3fcc,
        0xffcc, 0x003c, 0xc03c, 0x303c, 0xf03c, 0x0c3c, 0xcc3c, 0x3c3c, 0xfc3c,
        0x033c, 0xc33c, 0x333c, 0xf33c, 0x0f3c, 0xcf3c, 0x3f3c, 0xff3c, 0x00fc,
        0xc0fc, 0x30fc, 0xf0fc, 0x0cfc, 0xccfc, 0x3cfc, 0xfcfc, 0x03fc, 0xc3fc,
        0x33fc, 0xf3fc, 0x0ffc, 0xcffc, 0x3ffc, 0xfffc, 0x0003, 0xc003, 0x3003,
        0xf003, 0x0c03, 0xcc03, 0x3c03, 0xfc03, 0x0303, 0xc303, 0x3303, 0xf303,
        0x0f03, 0xcf03, 0x3f03, 0xff03, 0x00c3, 0xc0c3, 0x30c3, 0xf0c3, 0x0cc3,
        0xccc3, 0x3cc3, 0xfcc3, 0x03c3, 0xc3c3, 0x33c3, 0xf3c3, 0x0fc3, 0xcfc3,
        0x3fc3, 0xffc3, 0x0033, 0xc033, 0x3033, 0xf033, 0x0c33, 0xcc33, 0x3c33,
        0xfc33, 0x0333, 0xc333, 0x3333, 0xf333, 0x0f33, 0xcf33, 0x3f33, 0xff33,
        0x00f3, 0xc0f3, 0x30f3, 0xf0f3, 0x0cf3, 0xccf3, 0x3cf3, 0xfcf3, 0x03f3,
        0xc3f3, 0x33f3, 0xf3f3, 0x0ff3, 0xcff3, 0x3ff3, 0xfff3, 0x000f, 0xc00f,
        0x300f, 0xf00f, 0x0c0f, 0xcc0f, 0x3c0f, 0xfc0f, 0x030f, 0xc30f, 0x330f,
        0xf30f, 0x0f0f, 0xcf0f, 0x3f0f, 0xff0f, 0x00cf, 0xc0cf, 0x30cf, 0xf0cf,
        0x0ccf, 0xcccf, 0x3ccf, 0xfccf, 0x03cf, 0xc3cf, 0x33cf, 0xf3cf, 0x0fcf,
        0xcfcf, 0x3fcf, 0xffcf, 0x003f, 0xc03f, 0x303f, 0xf03f, 0x0c3f, 0xcc3f,
        0x3c3f, 0xfc3f, 0x033f, 0xc33f, 0x333f, 0xf33f, 0x0f3f, 0xcf3f, 0x3f3f,
        0xff3f, 0x00ff, 0xc0ff, 0x30ff, 0xf0ff, 0x0cff, 0xccff, 0x3cff, 0xfcff,
        0x03ff, 0xc3ff, 0x33ff, 0xf3ff, 0x0fff, 0xcfff, 0x3fff, 0xffff
};

static const uint8_t unshuffle[256] = {
        0x00, 0x10, 0x01, 0x11, 0x20, 0x30, 0x21, 0x31,
        0x02, 0x12, 0x03, 0x13, 0x22, 0x32, 0x23, 0x33,
        0x40, 0x50, 0x41, 0x51, 0x60, 0x70, 0x61, 0x71,
        0x42, 0x52, 0x43, 0x53, 0x62, 0x72, 0x63, 0x73,
        0x04, 0x14, 0x05, 0x15, 0x24, 0x34, 0x25, 0x35,
        0x06, 0x16, 0x07, 0x17, 0x26, 0x36, 0x27, 0x37,
        0x44, 0x54, 0x45, 0x55, 0x64, 0x74, 0x65, 0x75,
        0x46, 0x56, 0x47, 0x57, 0x66, 0x76, 0x67, 0x77,
        0x80, 0x90, 0x81, 0x91, 0xa0, 0xb0, 0xa1, 0xb1,
        0x82, 0x92, 0x83, 0x93, 0xa2, 0xb2, 0xa3, 0xb3,
        0xc0, 0xd0, 0xc1, 0xd1, 0xe0, 0xf0, 0xe1, 0xf1,
        0xc2, 0xd2, 0xc3, 0xd3, 0xe2, 0xf2, 0xe3, 0xf3,
        0x84, 0x94, 0x85, 0x95, 0xa4, 0xb4, 0xa5, 0xb5,
        0x86, 0x96, 0x87, 0x97, 0xa6, 0xb6, 0xa7, 0xb7,
        0xc4, 0xd4, 0xc5, 0xd5, 0xe4, 0xf4, 0xe5, 0xf5,
        0xc6, 0xd6, 0xc7, 0xd7, 0xe6, 0xf6, 0xe7, 0xf7,
        0x08, 0x18, 0x09, 0x19, 0x28, 0x38, 0x29, 0x39,
        0x0a, 0x1a, 0x0b, 0x1b, 0x2a, 0x3a, 0x2b, 0x3b,
        0x48, 0x58, 0x49, 0x59, 0x68, 0x78, 0x69, 0x79,
        0x4a, 0x5a, 0x4b, 0x5b, 0x6a, 0x7a, 0x6b, 0x7b,
        0x0c, 0x1c, 0x0d, 0x1d, 0x2c, 0x3c, 0x2d, 0x3d,
        0x0e, 0x1e, 0x0f, 0x1f, 0x2e, 0x3e, 0x2f, 0x3f,
        0x4c, 0x5c, 0x4d, 0x5d, 0x6c, 0x7c, 0x6d, 0x7d,
        0x4e, 0x5e, 0x4f, 0x5f, 0x6e, 0x7e, 0x6f, 0x7f,
        0x88, 0x98, 0x89, 0x99, 0xa8, 0xb8, 0xa9, 0xb9,
        0x8a, 0x9a, 0x8b, 0x9b, 0xaa, 0xba, 0xab, 0xbb,
        0xc8, 0xd8, 0xc9, 0xd9, 0xe8, 0xf8, 0xe9, 0xf9,
        0xca, 0xda, 0xcb, 0xdb, 0xea, 0xfa, 0xeb, 0xfb,
        0x8c, 0x9c, 0x8d, 0x9d, 0xac, 0xbc, 0xad, 0xbd,
        0x8e, 0x9e, 0x8f, 0x9f, 0xae, 0xbe, 0xaf, 0xbf,
        0xcc, 0xdc, 0xcd, 0xdd, 0xec, 0xfc, 0xed, 0xfd,
        0xce, 0xde, 0xcf, 0xdf, 0xee, 0xfe, 0xef, 0xff
};



#pragma unsafe arrays
static uint8_t transfer8_sync_zero_clkblk(
        out buffered port:32 sclk,
        out buffered port:32 ?mosi,
        in buffered port:32 ?miso,
        uint8_t data, const unsigned period,
        unsigned cpol, unsigned cpha){
    unsigned time, d, c = 0xaaaa>>(cpol ^ cpha);
    time = partout_timestamped(sclk, 1, cpol);
    time += 40;

    for(unsigned i=0;i<8;i++){
        partout_timed(sclk, 1, c, time);
        c>>=1;
        //sclk @ time <:>> c;

        if(!isnull(mosi)){
            partout_timed(mosi, 1, data>>7, time);
            data<<=1;
        }
        time += period / 2;

        partout_timed(sclk, 1, c, time);
        c>>=1;
        if(!isnull(miso)){
            unsigned t;
            miso @ time - 1 :> t;
            d = (d<<1) + (t&1);
        }
        time += (period + 1)/2;
    }
    partout_timed(sclk, 1, cpol, time);
    sync(sclk);
    return d;
}

#pragma unsafe arrays
static uint32_t transfer32_sync_zero_clkblk(
        out buffered port:32 sclk,
        out buffered port:32 ?mosi,
        in buffered port:32 ?miso,
        uint32_t data, const unsigned period,
        const unsigned cpol, const unsigned cpha){
    unsigned time;
    uint32_t d;
    time = partout_timestamped(sclk, 1, cpol);
    time += 100;

    //bitrev the data
    for(unsigned j=0;j<2;j++){
        unsigned c = 0xaaaaaaaa>>(cpol ^ cpha);
        for(unsigned i=0;i<16;i++){
          partout_timed(sclk, 1, c, time);
          if(!isnull(mosi)){
              partout_timed(mosi, 1, data>>31, time);
              data<<=1;
          }
          c>>=1;
          time += period / 2;
          partout_timed(sclk, 1, c, time);
          c>>=1;
          if(!isnull(miso)){
              unsigned t;
              miso @ time - 1 :> t;
              d = (d<<1) + (t&1);
          }
          time += (period + 1)/2;
        }
        time += 80;
    }
    partout_timed(sclk, 1, cpol, time);
    sync(sclk);
    return d;
}

#pragma unsafe arrays
static unsigned zip8(uint8_t a){
       return ziptable[a];
}

#pragma unsafe arrays
static void zip32(uint32_t a, uint32_t &x, uint32_t &y){
    //This can be improved
   y =  zip8(a&0xff);
   a=a>>8;
   y = (y << 16)| (zip8(a&0xff));
   a=a>>8;
   x =  zip8(a&0xff);
   a=a>>8;
   x = (x << 16)| (zip8(a&0xff));
}

#pragma unsafe arrays
static uint8_t unzip_16(unsigned d){
    d = d & 0x55aa;
    d = d | (d>>8);
    return bitrev(unshuffle[d&0xff])>>24;
}

#pragma unsafe arrays
static uint16_t unzip_32(unsigned d){
    return unzip_16(d&0xffff) |(unzip_16(d>>16)<<8);
}

static unsigned make_8bit_clock(unsigned cpol, unsigned cpha){
    return (0xaaaa >> (cpha))  ^ - cpol;
}
static uint8_t transfer8_sync_one_clkblk(
        out buffered port:32 sclk,
        out buffered port:32 ?mosi,
        in buffered port:32 ?miso,
        uint8_t data,
        unsigned cpol, unsigned cpha){
    unsigned double_clock = make_8bit_clock(cpol, cpha);
    uint16_t double_data = zip8(data);
    unsigned t = partout_timestamped(sclk, 1, cpol);
    t+=80;
    partout_timed(sclk, 17, double_clock, t);
    if(!isnull(mosi))partout_timed(mosi, 16, double_data, t);
    if(!isnull(miso)) miso  @ t + 31 :> double_data;
    return unzip_16(double_data);
}

static uint32_t transfer32_sync_one_clkblk(
        out buffered port:32 sclk,
        out buffered port:32 ?mosi,
        in buffered port:32 ?miso,
        uint32_t data,
        unsigned cpol, unsigned cpha){

    unsigned t;
    unsigned double_clock = 0xaaaaaaaa>>(cpol ^ cpha);

    uint32_t double_data_0;
    uint32_t double_data_1;
    zip32(data, double_data_0, double_data_1);
    t = partout_timestamped(sclk, 1, cpol);
    t+=80;
    sclk @ t <: double_clock;
    if(!isnull(mosi))mosi @ t<: double_data_0;
    sclk <: double_clock;
    if(!isnull(mosi))mosi <: double_data_1;
    if(!isnull(miso))miso @ t + 31:> double_data_0;
    if(!isnull(miso))miso :> double_data_1;

    return byterev(unzip_32(double_data_0) | (unzip_32(double_data_1)<<16));
}

static void get_mode_bits(spi_mode_t mode, unsigned &cpol, unsigned &cpha){
    switch(mode){
        case SPI_MODE_0:cpol = 0; cpha= 1; break;
        case SPI_MODE_1:cpol = 0; cpha= 0; break;
        case SPI_MODE_2:cpol = 1; cpha= 0; break;
        case SPI_MODE_3:cpol = 1; cpha= 1; break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma unsafe arrays
[[distributable]]
void spi_master(server interface spi_master_if i[num_clients],
        static const size_t num_clients,
        out buffered port:32 sclk,
        out buffered port:32 ?mosi,
        in buffered port:32 ?miso,
        out port p_ss[num_slaves],
        static const size_t num_slaves,
        clock ?cb){

    for(unsigned i=0;i<num_slaves;i++)
        p_ss[i] <: 1;

    if(!isnull(cb)){
        stop_clock(cb);
        configure_clock_ref(cb, 2);
        if(!isnull(miso))configure_in_port(miso,  cb);
        if(!isnull(miso))set_port_sample_delay(miso);
        if(!isnull(mosi))configure_out_port(mosi, cb, 0);
        configure_out_port(sclk, cb, 0);
        start_clock(cb);
    } else {
        if(!isnull(miso)) set_port_sample_delay(miso);
    }

    if(!isnull(mosi))
        mosi <: 0xffffffff;
    unsigned cpol, cpha, period;
    unsigned selected_device = 0;
    int accepting_new_transactions = 1;

    while(1){
        select {
            case accepting_new_transactions => i[int x].begin_transaction(unsigned device_index,
                    unsigned speed_in_khz, spi_mode_t mode):{
                //Get the mode bits from the spi_mode
                get_mode_bits(mode, cpol, cpha);

                //xassert(device_index < num_slaves);

                sync(sclk);
                //Wait for the chip deassert time if need be
                if(device_index == selected_device)
                 sync(p_ss[selected_device]);

                //Set the expected clock idle state on the clock port
                partout(sclk, 1, cpol);
                sync(sclk);

                if(isnull(cb)){
                    //Calculate the clock period from the speed_in_khz
                    period = (XS1_TIMER_KHZ + speed_in_khz - 1)/speed_in_khz;//round up
                } else {
                    //Set the clock divider
                    stop_clock(cb);
                    unsigned d = (XS1_TIMER_KHZ + 4*speed_in_khz - 1)/(4*speed_in_khz);//FIXME this has to round up too
                    configure_clock_ref(cb, d);
                    start_clock(cb);
                }

                //Lock the begin transaction
                accepting_new_transactions = 0;

                //Do a slave select
                selected_device = device_index;
                p_ss[selected_device] <: 0;
                break;
            }
            case i[int x].end_transaction(unsigned ss_deassert_time):{
                //Unlock the transaction
                accepting_new_transactions = 1;

                unsigned time;
                partout(sclk, 1, cpol);
                sync(sclk);
                p_ss[selected_device] <: 1 @ time;

                //TODO should this be allowed? (0.6ms max without it)
                if(ss_deassert_time > 0xffff)
                   delay_ticks(ss_deassert_time&0xffff0000);

                time += ss_deassert_time;

                p_ss[selected_device] @ time <: 1;
                break;
            }
            case i[int x].transfer8(uint8_t data)-> uint8_t r :{
                if(isnull(cb)) {
                    r = transfer8_sync_zero_clkblk(sclk, mosi, miso, data, period, cpol, cpha);
                } else {
                    r = transfer8_sync_one_clkblk(sclk, mosi, miso, data, cpol, cpha);
                }
                break;
            }
            case i[int x].transfer32(uint32_t data) -> uint32_t r:{
                if(isnull(cb)) {
                    r = transfer32_sync_zero_clkblk(sclk, mosi, miso, data, period, cpol, cpha);
                } else {
                    r = transfer32_sync_one_clkblk(sclk, mosi, miso, data, cpol, cpha);
                }
                break;
            }
        }
    }

}

