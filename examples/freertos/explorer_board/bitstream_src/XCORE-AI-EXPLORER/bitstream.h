// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef BITSTREAM_H_
#define BITSTREAM_H_

#include "soc.h"

#ifdef __XC__
#include "micarray_dev.h"
#include "i2c_dev.h"
#include "i2s_dev.h"
#include "gpio_dev.h"
#include "spi_master_dev.h"
#include "qspi_flash_dev.h"
#endif //__XC__

/*
 * Bitstream specific
 *
 * Internal to the bitstream.
 */
void tile0_device_instantiate(
        chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend mic_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT]
);
/*
 * Bitstream specific
 *
 * Internal to the bitstream.
 */
void tile1_device_instantiate(
        chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend mic_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT]
);

#ifdef __XC__
extern "C" {
#endif //__XC__

/*
 * Bitstream specific
 *
 * Called by the hardware side, to be
 * implemented on the software side.
 */
void device_register(
        chanend mic_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend i2c_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend t0_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend spi_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
		chanend qspi_flash_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT]
		);

#ifdef __XC__
}
#endif //__XC__

#endif /* BITSTREAM_H_ */
