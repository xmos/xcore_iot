// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef MICARRAY_DRIVER_H_
#define MICARRAY_DRIVER_H_

#include "soc.h"

xcore_freertos_device_t micarray_driver_init(
        int device_id,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr);

#endif /* MICARRAY_DRIVER_H_ */
