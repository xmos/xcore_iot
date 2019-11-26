// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef I2S_DRIVER_H_
#define I2S_DRIVER_H_

#include "xcore_freertos.h"

xcore_freertos_device_t i2s_driver_init(
        int device_id,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        freertos_ici_cb_t isr);

#endif /* I2S_DRIVER_H_ */
