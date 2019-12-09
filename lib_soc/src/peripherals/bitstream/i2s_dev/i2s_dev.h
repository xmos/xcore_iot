// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef I2S_DEV_H_
#define I2S_DEV_H_

#if __soc_conf_h_exists__
#include "soc_conf.h"
#else
#warning soc_conf.h not found
#endif

#include "i2s_dev_conf_defaults.h"

void i2s_dev(
        soc_peripheral_t *peripheral,
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c,
        in port p_mclk,
        out buffered port:32 p_lrclk,
        out port p_bclk,
        out buffered port:32 (&?p_dout)[num_out],
        static const size_t num_out,
        clock bclk);

#endif /* I2S_DEV_H_ */
