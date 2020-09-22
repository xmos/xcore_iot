// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef SDRAM_DEV_H_
#define SDRAM_DEV_H_

#if __soc_conf_h_exists__
#include "soc_conf.h"
#else
#warning soc_conf.h not found
#endif

#include "sdram_dev_conf_defaults.h"
#include "sdram_dev_ctrl.h"

#include "sdram.h"

void sdram_dev(
        chanend ?data_to_dma_c,
        chanend ?data_from_dma_c,
        chanend ?ctrl_c,
        out buffered port:32 p_sdram_dq_ah,
        out buffered port:32 p_sdram_cas,
        out buffered port:32 p_sdram_ras,
        out buffered port:8 p_sdram_we,
        out port p_sdram_clk,
        clock sdram_cb_clk);

#endif /* SDRAM_DEV_H_ */
