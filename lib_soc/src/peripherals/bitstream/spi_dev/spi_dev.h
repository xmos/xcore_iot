// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef SPI_DEV_H_
#define SPI_DEV_H_

#if __soc_conf_h_exists__
#include "soc_conf.h"
#else
#warning soc_conf.h not found
#endif

#include "spi_dev_conf_defaults.h"
#include "spi_dev_ctrl.h"

#include "spi.h"

void spi_dev(
        soc_peripheral_t *peripheral,
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c,
        out buffered port:32 sclk,
        out buffered port:32 ?mosi,
        in buffered port:32 ?miso,
        out port p_ss[num_slaves],
        static const size_t num_slaves,
        clock ?clk0,
        clock ?clk1);

#endif /* SPI_DEV_H_ */
