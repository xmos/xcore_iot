// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"
#include "bitstream_devices.h"

xcore_freertos_device_t i2s_driver_init(
        int device_id,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr)
{
    xcore_freertos_device_t device;

    xassert(device_id >= 0 && device_id < BITSTREAM_I2S_DEVICE_COUNT);

    device = bitstream_i2s_devices[device_id];

    xcore_freertos_dma_device_common_init(
            device,
            rx_desc_count,
            rx_buf_size,
            tx_desc_count,
            app_data,
            isr_core,
            isr);

    return device;
}
