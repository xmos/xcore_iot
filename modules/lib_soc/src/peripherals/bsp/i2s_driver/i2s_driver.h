// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved

#ifndef I2S_DRIVER_H_
#define I2S_DRIVER_H_

#include "soc.h"

/**
 * Initialize the i2s device
 *
 * \param[in]     device_id      ID of i2s device to use
 * \param[in]     rx_desc_count  Maximum number of rx descriptors
 * \param[in]     rx_buf_size    Size of each of rx descriptor buffer
 * \param[in]     tx_desc_count  Maximum number of tx descriptors
 * \param[in]     app_data  	 App data pointer
 * \param[in]     isr_core       FreeRTOS core to handle interrupts from device pointer
 * \param[in]     isr            ISR to handle interrupts from device
 *
 * \returns       Initialized i2s device
 */
soc_peripheral_t i2s_driver_init(
        int device_id,
        int rx_desc_count,
        int rx_buf_size,
        int tx_desc_count,
        void *app_data,
        int isr_core,
        rtos_irq_isr_t isr);

#endif /* I2S_DRIVER_H_ */
