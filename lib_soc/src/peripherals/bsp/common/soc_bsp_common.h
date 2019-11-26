// Copyright (c) 2019, XMOS Ltd, All rights reserved


#ifndef SOC_BSP_COMMON_H_
#define SOC_BSP_COMMON_H_

#include "soc.h"
#include "rtos_support.h"

void soc_peripheral_common_dma_init(
        soc_peripheral_t device,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr);

#endif /* SOC_BSP_COMMON_H_ */
