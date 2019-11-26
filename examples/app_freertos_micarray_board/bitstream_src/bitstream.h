/*
 * bitstream.h
 *
 *  Created on: Sep 27, 2019
 *      Author: mbruno
 */


#ifndef BITSTREAM_H_
#define BITSTREAM_H_

#include "soc.h"

#ifdef __XC__
#include "micarray_dev.h"
#include "eth_dev.h"
#include "i2c_dev.h"
#include "i2s_dev.h"
#include "gpio_dev.h"
#endif //__XC__

/*
 * This will be a device specific function
 * provided by the device driver.
 */
void device_input(const int dev_id, chanend dev_c);

/*
 * Bitstream specific
 *
 * Internal to the bitstream.
 */
void tile0_device_instantiate(
        chanend eth_dev_ch[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT],
        chanend i2s_dev_ch[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT],
        chanend i2c_dev_ch[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT],
        chanend t1_gpio_dev_ch[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT]);
/*
 * Bitstream specific
 *
 * Internal to the bitstream.
 */
void tile1_device_instantiate(
        chanend eth_dev_ch[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT],
        chanend i2s_dev_ch[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT],
        chanend i2c_dev_ch[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT],
        chanend t1_gpio_dev_ch[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT]);

/*
 * Bitstream specific
 *
 * Called by the hardware side, to be
 * implemented on the software side.
 */
void device_register(
        chanend mic_dev_ch[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT],
        chanend eth_dev_ch[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT],
        chanend i2s_dev_ch[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT],
        chanend i2c_dev_ch[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT],
        chanend t0_gpio_dev_ch[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT],
        chanend t1_gpio_dev_ch[XCORE_FREERTOS_DMA_DEVICE_CHANNEL_COUNT]);

#endif /* BITSTREAM_H_ */
