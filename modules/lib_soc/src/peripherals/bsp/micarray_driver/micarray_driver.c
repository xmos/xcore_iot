// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"
#include "soc_bsp_common.h"
#include "bitstream_devices.h"

#if ( SOC_MICARRAY_PERIPHERAL_USED == 0 )
#define BITSTREAM_MICARRAY_DEVICE_COUNT 0
soc_peripheral_t bitstream_micarray_devices[BITSTREAM_MICARRAY_DEVICE_COUNT];
#endif /* SOC_MICARRAY_PERIPHERAL_USED */

soc_peripheral_t micarray_driver_init(
        int device_id,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr)
{
    soc_peripheral_t device;

    xassert(device_id >= 0 && device_id < BITSTREAM_MICARRAY_DEVICE_COUNT);

    device = bitstream_micarray_devices[device_id];

    soc_peripheral_common_dma_init(
            device,
            rx_desc_count,
            rx_buf_size,
            tx_desc_count,
            app_data,
            isr_core,
            isr);

    return device;
}
