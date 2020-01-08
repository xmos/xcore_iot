// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <xclib.h>
#include <stdio.h>
#include <stdlib.h>

#include "spi.h"

#define ASSERTED 1

 [[combinable]]
void spi_slave(client spi_slave_callback_if spi_i,
               in port sclk,
               in buffered port:32 mosi,
               out buffered port:32 ?miso,
               in port ss,
               clock clk,
               static const spi_mode_t mode,
               static const spi_transfer_type_t transfer_type){

     //first setup the ports

     set_port_inv(ss);

     stop_clock(clk);
     set_clock_src(clk, sclk);

     if(!isnull(miso))
         configure_out_port_strobed_slave(miso, ss, clk, 0);
     configure_in_port_strobed_slave(mosi, ss, clk);

     start_clock(clk);

     switch(mode){
     case SPI_MODE_0:
     case SPI_MODE_2:
         set_port_inv(sclk);
         break;
     case SPI_MODE_1:
     case SPI_MODE_3:
         set_port_no_inv(sclk);
         break;
     }
     sync(sclk);

     int ss_val;

     //set the transfer width
     if(transfer_type == SPI_TRANSFER_SIZE_8){
         if(!isnull(miso))
             asm volatile ("settw res[%0], %1"::"r"(miso), "r"(8));
         asm volatile ("settw res[%0], %1"::"r"(mosi), "r"(8));
     }

     uint32_t buffer;

     ss when pinseq(!ASSERTED) :> ss_val;
     while(1){
         select {
             case ss when pinsneq(ss_val) :> ss_val:{

                 if(!isnull(miso))
                     clearbuf(miso);

                 if(ss_val != ASSERTED){
                     unsigned remaining_bits = endin(mosi);
                     uint32_t data;
                     mosi :> data;
                     if(remaining_bits){ //FIXME can this be more then tw?
                         data = bitrev(data);
                         if(transfer_type == SPI_TRANSFER_SIZE_8)
                             data >>= (32-8);
                         spi_i.master_supplied_data(data, remaining_bits);
                     }
                     clearbuf(mosi);
                     spi_i.master_ends_transaction();
                     break;
                 }
                 if(!isnull(miso)){
                     uint32_t data = spi_i.master_requires_data();

                     if(transfer_type == SPI_TRANSFER_SIZE_8){
                         data = (bitrev(data)>>24);
                         if((mode == SPI_MODE_1) || (mode == SPI_MODE_2)){
                             asm volatile ("setclk res[%0], %1"::"r"(miso), "r"(XS1_CLKBLK_REF));
                             partout(miso, 1, data);
                             asm volatile ("setclk res[%0], %1"::"r"(miso), "r"(clk));
                             data = data>>1;
                             partout(miso, 7, data);
                         } else {
                             partout(miso, 8, data);
                         }
                     } else {
                         data = bitrev(data);
                         if((mode == SPI_MODE_1) || (mode == SPI_MODE_2)){
                             asm volatile ("setclk res[%0], %1"::"r"(miso), "r"(XS1_CLKBLK_REF));
                             partout(miso, 1, data);
                             asm volatile ("setclk res[%0], %1"::"r"(miso), "r"(clk));
                             data = data>>1;
                             partout(miso, 31, data);
                         } else {
                             miso <: data;
                         }
                     }
                     buffer = spi_i.master_requires_data();

                     if(transfer_type == SPI_TRANSFER_SIZE_8){
                         buffer = (bitrev(buffer)>>24);
                     } else {
                         buffer = bitrev(buffer);
                     }
                 }
                 clearbuf(mosi);
                 break;
             }

             case mosi :> int i:{
                 if(transfer_type == SPI_TRANSFER_SIZE_8){
                     if(!isnull(miso)){
                         //clearbuf(miso);//FIXME this is not correct - do something better
                         partout(miso, 8, buffer);
                         buffer = spi_i.master_requires_data();
                         buffer = (bitrev(buffer)>>24);
                     }
                     spi_i.master_supplied_data(bitrev(i)>>24, 8);

                 } else {
                     if(!isnull(miso)){
                         //clearbuf(miso);//FIXME this is not correct - do something better
                         miso <: buffer;
                         buffer = spi_i.master_requires_data();
                         buffer = bitrev(buffer);
                     }
                     spi_i.master_supplied_data(bitrev(i), 32);
                 }
                 break;
             }

         }
     }
 }
