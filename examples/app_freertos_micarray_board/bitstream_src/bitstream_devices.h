/*
 * bitstream_devices.h
 *
 *  Created on: Sep 30, 2019
 *      Author: mbruno
 */


#ifndef BITSTREAM_DEVICES_H_
#define BITSTREAM_DEVICES_H_

enum {
    BITSTREAM_MICARRAY_DEVICE_A,
    BITSTREAM_MICARRAY_DEVICE_COUNT
};
extern xcore_freertos_device_t bitstream_micarray_devices[BITSTREAM_MICARRAY_DEVICE_COUNT];

enum {
    BITSTREAM_ETHERNET_DEVICE_A,
    BITSTREAM_ETHERNET_DEVICE_COUNT
};
extern xcore_freertos_device_t bitstream_ethernet_devices[BITSTREAM_ETHERNET_DEVICE_COUNT];

enum {
    BITSTREAM_I2S_DEVICE_A,
    BITSTREAM_I2S_DEVICE_COUNT
};
extern xcore_freertos_device_t bitstream_i2s_devices[BITSTREAM_I2S_DEVICE_COUNT];

enum {
    BITSTREAM_I2C_DEVICE_A,
    BITSTREAM_I2C_DEVICE_COUNT
};
extern xcore_freertos_device_t bitstream_i2c_devices[BITSTREAM_I2C_DEVICE_COUNT];

enum {
    BITSTREAM_GPIO_DEVICE_A,
    BITSTREAM_GPIO_DEVICE_B,
    BITSTREAM_GPIO_DEVICE_COUNT
};
extern xcore_freertos_device_t bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_COUNT];


#endif /* BITSTREAM_DEVICES_H_ */
