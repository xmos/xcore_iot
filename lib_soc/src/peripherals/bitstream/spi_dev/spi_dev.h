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

#include "spi_fast.h"

void spi_dev(
        soc_peripheral_t *peripheral,
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c);

#endif /* SPI_DEV_H_ */
