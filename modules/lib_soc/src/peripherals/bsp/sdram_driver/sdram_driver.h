// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef SDRAM_DRIVER_H_
#define SDRAM_DRIVER_H_

#include "soc.h"
#include "sdram_dev_ctrl.h"

int sdram_driver_write(
        soc_peripheral_t dev,
        unsigned address,
        unsigned word_count,
        void *buffer
        );

int sdram_driver_read(
        soc_peripheral_t dev,
        unsigned address,
        unsigned word_count,
        void *buffer
        );

soc_peripheral_t sdram_driver_init(
        int device_id,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr);

#endif /* SDRAM_DRIVER_H_ */
