// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XCORE VocalFusion Licence.

#ifndef __ASRC_FIFO_TYPES__
#define __ASRC_FIFO_TYPES__
#include <stdint.h>

//Shared FIFO return types
typedef enum fifo_ret_t {
  FIFO_SUCCESS = 0,
  FIFO_FULL,
  FIFO_EMPTY
} fifo_ret_t;

/////////////////////////////////////////////////////////////////////////
//Shared memory FIFO (sample by sample or block)
//Can be any size
//
//Note that the actual storage for the FIFO is declared externally
//and a reference to the base address of the storage is passed in along
//with the size of the storage. This way, multiple instances may be
//different sizes.
//
/////////////////////////////////////////////////////////////////////////

typedef struct mem_fifo_t {
    const unsigned size;            //Size in samples
    int8_t * const data_base_ptr;   //Base of the data array - declared externally so we can have differnt sized FIFOs
    unsigned write_idx;             //index in samples
    unsigned read_idx;              //index in samples
} mem_fifo_t;

#endif
