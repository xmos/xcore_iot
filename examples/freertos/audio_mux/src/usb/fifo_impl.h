// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.


#ifndef __FIFO__
#define __FIFO__
#include <string.h> //memcpy

//Asynch FIFO implementaion
//Note these are in the include file to allow the compiler to inline for performance

///////////////////////////////////////
//Shared memory FIFO (sample by sample)
//Can be any size
///////////////////////////////////////

#pragma once

#include "fifo_types.h"

static void fifo_reset_fill(volatile mem_fifo_t * fifo, unsigned fill /*in samples*/, unsigned subslot_size) {
    fifo->read_idx = 0;
    fifo->write_idx = fill;
    memset(fifo->data_base_ptr, 0, fill * subslot_size);
}

static inline unsigned fifo_get_fill(volatile mem_fifo_t * fifo) {
    unsigned fifo_fill = 0;
    unsigned read_idx = fifo->read_idx;
    unsigned write_idx = fifo->write_idx;
    if (write_idx >= read_idx){
        fifo_fill = write_idx - read_idx;
    }
    else {
        fifo_fill = (fifo->size + write_idx) - read_idx;
    }
    return (fifo_fill); //return fill level in samples
}

static inline void fifo_init(volatile mem_fifo_t * fifo, unsigned subslot_size) {
    fifo->write_idx = 0;
    fifo->read_idx = (fifo->size * 2) / 4;
    memset(fifo->data_base_ptr , 0, fifo->size * subslot_size);
}

static inline fifo_ret_t fifo_block_push_fast(volatile mem_fifo_t * fifo, uint8_t *data, unsigned n, unsigned subslot_size) {
    //check there is a block of space large enough
    unsigned space_remaining = fifo->size - fifo_get_fill(fifo) - 1; //no. of samples worth of remaining space
    if (n > space_remaining) {
        return FIFO_FULL;
    }
    //We will write either one or two blocks depending on wrap
    unsigned first_block_size = 0;
    unsigned second_block_size = 0;

    //See if we need to wrap during block writes
    unsigned space_left_at_top = fifo->size - fifo->write_idx;
    //printf("space_left_at_top %d\n", space_left_at_top);
    //Yes, we do need to wrap
    if (n > space_left_at_top){
        first_block_size = space_left_at_top;
        second_block_size = n - space_left_at_top;
        memcpy(&fifo->data_base_ptr[fifo->write_idx * subslot_size],
               &data[0],
               first_block_size * subslot_size);
        memcpy(&fifo->data_base_ptr[0],
               &data[first_block_size*subslot_size],
               second_block_size * subslot_size);
        fifo->write_idx = second_block_size;
    }
    //No wrap, do all in one go
    else{
        first_block_size = n;
        second_block_size = 0;
        memcpy(&fifo->data_base_ptr[fifo->write_idx * subslot_size],
               &data[0],
               first_block_size * subslot_size);
        fifo->write_idx += first_block_size;
    }

    return FIFO_SUCCESS;
}

static inline fifo_ret_t fifo_block_pop_fast(volatile mem_fifo_t * fifo, uint8_t *data, unsigned n, unsigned subslot_size) {
    //Check we have a block big enough to send
    if (n > fifo_get_fill(fifo)){
        return FIFO_EMPTY;
    }
    //We will read either one or two blocks depending on wrap
    unsigned first_block_size = 0;
    unsigned second_block_size = 0;

    //See if we need to wrap during block read
    unsigned num_read_at_top = fifo->size - fifo->read_idx;
    // printf("num_read_at_top %d\n", num_read_at_top);
    //Yes, we do need to wrap
    if (n > num_read_at_top){
        first_block_size = num_read_at_top;
        second_block_size = n - num_read_at_top;
        memcpy(&data[0],
               &fifo->data_base_ptr[fifo->read_idx * subslot_size],
               first_block_size * subslot_size);
        memcpy(&data[first_block_size * subslot_size],
               &fifo->data_base_ptr[0],
               second_block_size * subslot_size);
        fifo->read_idx = second_block_size;
        // printf("wrap\n");
    }
    //No wrap, do all in one go
    else{
        first_block_size = n;
        second_block_size = 0;
        memcpy(&data[0],
               &fifo->data_base_ptr[fifo->read_idx * subslot_size],
               first_block_size * subslot_size);
        fifo->read_idx += first_block_size;
        // printf("no wrap\n");

    }

    return FIFO_SUCCESS;
}

//Version of above that returns fill level relative to half full
static inline int fifo_get_fill_relative_half(volatile mem_fifo_t * fifo){
    int fifo_fill = (int)fifo_get_fill(fifo);
    fifo_fill -= (fifo->size / 2);
    return fifo_fill;
}


#endif
