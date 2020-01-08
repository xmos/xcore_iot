// Copyright (c) 2015-2016, XMOS Ltd, All rights reserved
#include <xs1.h>
#include <xclib.h>
#include <stdio.h>
#include <stdlib.h>

#include "spi.h"

#include "debug_print.h"

typedef struct {
    unsigned client_id;
    unsigned device_index;
    unsigned speed_in_khz;
    spi_mode_t mode;
    size_t               buffer_nbytes;
    unsigned             buffer_transfer_width;
    uint32_t * movable   buffer_tx;
    uint32_t * movable   buffer_rx;

} transaction_request;
#define NBYTES_UNASSIGNED (-1)

static void transfer8_async(
        out buffered port:32 sclk,
        out buffered port:32 ?mosi,
        in buffered port:32 miso,
        uint8_t data){

    if(!isnull(mosi)) {
        clearbuf(mosi);
        mosi <: (bitrev(data)>>24);
    }

    clearbuf(miso); //TODO remove- if possible

    partout(sclk, 16, 0xaaaa);
}

static void transfer32_async(
        out buffered port:32 sclk,
        out buffered port:32 ?mosi,
        in buffered port:32 miso,
        uint32_t data){

    if(!isnull(mosi)) {
        clearbuf(mosi);
        mosi <: bitrev(data);
    }

    clearbuf(miso); //TODO remove - if possible

    //output 64 bits of clock
    sclk <: 0xaaaaaaaa;
    sclk <: 0xaaaaaaaa;
}

static void setup_new_transaction(
        out buffered port:32 sclk,
        out port p_ss[],
        clock cb0,
        spi_mode_t mode,
        unsigned new_device_index,
        unsigned speed_in_khz,
        unsigned &currently_selected_device){
    //xassert(device_index < num_slaves);
    switch(mode){
    case SPI_MODE_0:
        set_port_inv(sclk);
        partout(sclk,1,1);
        break;
    case SPI_MODE_1:
        set_port_no_inv(sclk);
        partout(sclk,1,0);
        break;
    case SPI_MODE_2:
        set_port_inv(sclk);
        partout(sclk,1,0);
        break;
    case SPI_MODE_3:
        set_port_no_inv(sclk);
        partout(sclk,1,1);
        break;
    }
    sync(sclk);

    //Wait for the chip deassert time if need be
    if(new_device_index == currently_selected_device)
     sync(p_ss[currently_selected_device]);

    //Set the clock divider
    stop_clock(cb0);
    unsigned d = (XS1_TIMER_KHZ + 4*speed_in_khz - 1)/(4*speed_in_khz);
    configure_clock_ref(cb0, d);
    start_clock(cb0);

    //Do a slave select
    currently_selected_device = new_device_index;
    p_ss[currently_selected_device] <: 0;
    sync(p_ss[currently_selected_device]);
}

static void init_init_transfer_array_8(
        out buffered port:32 sclk,
        out buffered port:32 ?mosi,
        in buffered port:32 miso,
        spi_mode_t active_mode,
        clock cb1,
        uint8_t data
        ){
    if(!isnull(mosi))
        asm volatile ("settw res[%0], %1"::"r"(mosi), "r"(8));
    asm volatile ("settw res[%0], %1"::"r"(miso), "r"(8));

    if((active_mode == SPI_MODE_1 || active_mode == SPI_MODE_2) && (!isnull(mosi))){
            unsigned b = data>>7;
            asm volatile ("setclk res[%0], %1"::"r"(mosi), "r"(XS1_CLKBLK_REF));
            partout(mosi, 1, b);
            asm volatile ("setclk res[%0], %1"::"r"(mosi), "r"(cb1));
            data = data<<1; //This is shifted up as the MSB is already on this pin
    }
    transfer8_async(sclk, mosi, miso,  data);
}

static void first_transfer_array_32(
        out buffered port:32 sclk,
        out buffered port:32 ?mosi,
        in buffered port:32 miso,
        spi_mode_t active_mode,
        clock cb1,
        uint32_t data){

    if(!isnull(mosi))
        asm volatile ("settw res[%0], %1"::"r"(mosi), "r"(32));
    asm volatile ("settw res[%0], %1"::"r"(miso), "r"(32));

    if((active_mode == SPI_MODE_1 || active_mode == SPI_MODE_2) && (!isnull(mosi))){
            unsigned b = bitrev(data);
            asm volatile ("setclk res[%0], %1"::"r"(mosi), "r"(XS1_CLKBLK_REF));
            partout(mosi, 1, b);
            asm volatile ("setclk res[%0], %1"::"r"(mosi), "r"(cb1));
            data = data<<1; //This is shifted up as the MSB is already on this pin
    }
    transfer32_async(sclk, mosi, miso, data);
}

[[combinable]]
void spi_master_async(server interface spi_master_async_if i[num_clients],
        static const size_t num_clients,
        out buffered port:32 sclk,
        out buffered port:32 ?mosi,
        in buffered port:32 miso,
        out port p_ss[num_slaves],
        static const size_t num_slaves,
        clock cb0,
        clock cb1){

    //These buffer are for the transaction requests
    transaction_request tr_buffer[8]; ///FIXME num_clients
    unsigned            tr_tail = 0;
    unsigned            tr_fill = 0;

    //These buffers are for the active transaction
    uint32_t * movable  buffer_tx;
    uint32_t * movable  buffer_rx;
    size_t              buffer_nbytes;
    unsigned            current_index;
    unsigned            buffer_transfer_width;

    //These variables are for the active transaction state
    unsigned active_device;
    unsigned active_client;
    unsigned active_mode;
    int currently_performing_a_transaction = 0;

    for(unsigned i=0;i<num_slaves;i++)
        p_ss[i] <: 1;

    stop_clock(cb0);

    configure_clock_ref(cb0, 1);
    configure_out_port(sclk, cb0, 0);

    stop_clock(cb1);
    configure_clock_src(cb1, sclk);
    set_port_no_sample_delay(miso);
    configure_in_port(miso,  cb1);
    if(!isnull(mosi))
        configure_out_port(mosi,  cb1, 0);
    start_clock(cb1);

    start_clock(cb0);

    if(!isnull(mosi))
        mosi <: 0xffffffff;

    clearbuf(miso);

    while(1){
        select {

            case i[int x].begin_transaction(unsigned device_index,
                    unsigned speed_in_khz, spi_mode_t mode):{

                //if doing a transaction the buffer this one
                if(currently_performing_a_transaction){
                    //Note, the tr_fill should never exceed num_clients if the calling protocol is respected
                    unsigned index = (tr_tail + tr_fill)%num_clients; //FIXME div?
                    tr_buffer[index].device_index = device_index;
                    tr_buffer[index].speed_in_khz = speed_in_khz;
                    tr_buffer[index].mode = mode;
                    tr_buffer[index].client_id = x;
                    tr_buffer[index].buffer_nbytes = NBYTES_UNASSIGNED;
                    tr_fill++;
                    break;
                }

                active_client = x;
                active_device = device_index;
                setup_new_transaction(sclk, p_ss, cb0, mode, device_index,
                        speed_in_khz, active_device);
                active_mode = mode;

                current_index = 0;
                currently_performing_a_transaction = 1;
                break;
            }
            //Note, end transaction can only be called from the active_client
            case i[int x].end_transaction(unsigned ss_deassert_time):{
                //xassert(x == active_client);

                //An end_transaction can only be completed after all transfers
                //have been completed
                //TODO should there be some guarding here?

                switch(active_mode){
                case SPI_MODE_0:
                    partout(sclk,1,1);
                    break;
                case SPI_MODE_1:
                    partout(sclk,1,0);
                    break;
                case SPI_MODE_2:
                    partout(sclk,1,0);
                    break;
                case SPI_MODE_3:
                    partout(sclk,1,1);
                    break;
                }
                sync(sclk);
                unsigned time;
                p_ss[active_device] <: 1 @ time;

                //TODO should this be allowed? (0.6ms max without it)
                if(ss_deassert_time > 0xffff)
                   delay_ticks(ss_deassert_time&0xffff0000);
                time += ss_deassert_time;
                p_ss[active_device] @ time <: 1;

                if(tr_fill > 0){
                    //begin a new transaction - the tail of the list is the next one to go
                    unsigned index = tr_tail%num_clients;
                    unsigned new_device_index = tr_buffer[index].device_index;
                    unsigned speed_in_khz = tr_buffer[index].speed_in_khz;
                    spi_mode_t mode = tr_buffer[index].mode;

                    active_client = tr_buffer[index].client_id;

                    tr_fill--;
                    tr_tail++;

                    setup_new_transaction(sclk, p_ss, cb0, mode, new_device_index,
                            speed_in_khz, active_device);
                    active_device = new_device_index;
                    active_mode = mode;

                    buffer_nbytes = tr_buffer[index].buffer_nbytes;

                    if(buffer_nbytes != NBYTES_UNASSIGNED){
                        buffer_tx = move(tr_buffer[index].buffer_tx);
                        buffer_rx = move(tr_buffer[index].buffer_rx);
                        buffer_transfer_width = tr_buffer[index].buffer_transfer_width;
                        if(buffer_transfer_width == 8){
                            init_init_transfer_array_8(sclk, mosi, miso, active_mode, cb1, ((uint8_t*movable)buffer_tx)[0]);
                        } else {
                            first_transfer_array_32(sclk, mosi, miso, active_mode, cb1, buffer_tx[0]);
                        }
                        current_index = 0;
                    }
                } else {
                    currently_performing_a_transaction = 0;
                }
                break;
            }
            case i[int x].init_transfer_array_8(uint8_t * movable inbuf,
                    uint8_t * movable outbuf, size_t nbytes) :{

                if(x != active_client){
                    unsigned index;
                    for(unsigned j=0;j<num_clients;j++){
                        if(tr_buffer[j].client_id==x){
                            index = j;
                            break;
                        }
                    }
                    tr_buffer[index].buffer_nbytes = nbytes;
                    tr_buffer[index].buffer_tx = (uint32_t * movable)move(outbuf);
                    tr_buffer[index].buffer_rx = (uint32_t * movable)move(inbuf);
                    tr_buffer[index].buffer_transfer_width = 8;
                } else {
                    buffer_nbytes = nbytes*sizeof(uint8_t);
                    buffer_tx = (uint32_t * movable)move(outbuf);
                    buffer_rx = (uint32_t * movable)move(inbuf);
                    if(buffer_nbytes == 0){
                        i[x].transfer_complete();
                    } else {
                        buffer_transfer_width = 8;
                        init_init_transfer_array_8(sclk, mosi, miso, active_mode, cb1, ((uint8_t*movable)buffer_tx)[0]);
                    }
                }

                break;
            }
            case i[int x].init_transfer_array_32(uint32_t * movable inbuf,
                    uint32_t * movable outbuf,
                    size_t nwords):{

                if(x != active_client){
                    unsigned index;

                    for(unsigned j=0;j<num_clients;j++){
                        if(tr_buffer[j].client_id==x){
                            index = j;
                            break;
                        }
                    }
                    tr_buffer[index].buffer_nbytes = nwords*sizeof(uint32_t);
                    tr_buffer[index].buffer_tx = move(outbuf);
                    tr_buffer[index].buffer_rx = move(inbuf);
                    tr_buffer[index].buffer_transfer_width = 32;

                } else {
                    buffer_nbytes = nwords*sizeof(uint32_t);
                    buffer_tx = move(outbuf);
                    buffer_rx = move(inbuf);

                    if(buffer_nbytes == 0){
                        i[x].transfer_complete();
                    } else {
                        buffer_transfer_width = 32;
                        first_transfer_array_32(sclk, mosi, miso, active_mode, cb1, buffer_tx[0]);
                    }
                }
                break;
            }
            case miso :> uint32_t data :{
                //put the data into the correct array and send the next data if need be
                if(buffer_transfer_width == 8){
                    data = bitrev(data<<24);
//if(current_index >5) debug_printf("%d\n", current_index);
if(current_index >5) debug_printf("%d %d\n", buffer_nbytes, current_index*sizeof(uint8_t) );
                    ((uint8_t*movable)buffer_rx)[current_index] = (uint8_t)data;
                    current_index++;
                    if((current_index*sizeof(uint8_t)) == buffer_nbytes){
                        i[active_client].transfer_complete();
                    } else {
debug_printf("tx ndx:%d\n", current_index);
                        transfer8_async(sclk, mosi, miso,
                                ((uint8_t*movable)buffer_tx)[current_index]);
                    }
                } else {
                    data = bitrev(data);
                    buffer_rx[current_index] = data;
                    current_index++;
                    if((current_index*sizeof(uint32_t)) == buffer_nbytes){
                       i[active_client].transfer_complete();
                    } else {
                        transfer32_async(sclk, mosi, miso, buffer_tx[current_index]);
                    }
                }
                break;
            }
            case i[int x].retrieve_transfer_buffers_8(uint8_t * movable &inbuf,
                    uint8_t * movable &outbuf):{
                inbuf = (uint8_t*movable)move(buffer_rx);
                outbuf = (uint8_t*movable)move(buffer_tx);
                break;
            }

            case i[int x].retrieve_transfer_buffers_32(uint32_t * movable &inbuf,
                    uint32_t * movable &outbuf):{
                inbuf = move(buffer_rx);
                outbuf = move(buffer_tx);
                break;
            }
        }
    }

}
