// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef MICARRAY_DEV_H_
#define MICARRAY_DEV_H_

#if __rtos_peripherals_conf_h_exists__
#include "rtos_peripherals_conf.h"
#endif

#include "micarray_dev_conf_defaults.h"

#if( MICARRAYCONF_DECIMATOR_COUNT != 1 )
#error Library currently does not support decimator counts other than 1
#endif

void micarray_dev_init(
        clock pdmclk,
        in port p_mclk,
        out port p_pdm_clk,
        buffered in port:32 p_pdm_mics);

[[combinable]]
void micarray_dev_to_dma(
        chanend c,
        streaming chanend c_ds_output[]);

void micarray_dev_task(
        in buffered port:32 p_pdm_mics,
        streaming chanend c_ds_output[]);

void micarray_dev(
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c,
        in buffered port:32 p_pdm_mics);

#endif /* MICARRAY_DEV_H_ */
