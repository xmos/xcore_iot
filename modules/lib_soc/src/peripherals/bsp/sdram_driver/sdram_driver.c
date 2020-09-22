// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"
#include "soc_bsp_common.h"
#include "bitstream_devices.h"

#include "sdram_driver.h"

#if ( SOC_SDRAM_PERIPHERAL_USED == 0 )
#define BITSTREAM_SDRAM_DEVICE_COUNT 0
soc_peripheral_t bitstream_sdram_devices[BITSTREAM_SDRAM_DEVICE_COUNT];
#endif /* SOC_SDRAM_PERIPHERAL_USED */

int sdram_driver_write(
        soc_peripheral_t dev,
        unsigned address,
        unsigned word_count,
        void *buffer
        )
{
    int dummy;
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c, SDRAM_DEV_WRITE);

    soc_peripheral_varlist_tx(
            c, 2,
            sizeof(address), &address,
            sizeof(word_count), &word_count);

    soc_peripheral_varlist_tx(
            c, 1,
            word_count, buffer);

    /* RX something to know that the write was complete */
    soc_peripheral_varlist_rx(
            c, 1,
            sizeof( dummy ), &dummy);

    return 1;
}

int sdram_driver_read(
        soc_peripheral_t dev,
        unsigned address,
        unsigned word_count,
        void *buffer
        )
{
    chanend c = soc_peripheral_ctrl_chanend(dev);

    soc_peripheral_function_code_tx(c, SDRAM_DEV_READ);

    soc_peripheral_varlist_tx(
            c, 2,
            sizeof(address), &address,
            sizeof(word_count), &word_count);

    soc_peripheral_varlist_rx(
            c, 1,
            word_count, buffer);

    return 1;
}

soc_peripheral_t sdram_driver_init(
        int device_id,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr)
{
    soc_peripheral_t device;

    xassert(device_id >= 0 && device_id < BITSTREAM_SDRAM_DEVICE_COUNT);

    device = bitstream_sdram_devices[device_id];

    return device;
}
