// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#ifndef BITSTREAM_DEVICES_H_
#define BITSTREAM_DEVICES_H_

enum {
    BITSTREAM_I2C_DEVICE_A,
    BITSTREAM_I2C_DEVICE_COUNT
};
extern soc_peripheral_t bitstream_i2c_devices[BITSTREAM_I2C_DEVICE_COUNT];

enum {
    BITSTREAM_GPIO_DEVICE_A,
    BITSTREAM_GPIO_DEVICE_COUNT
};
extern soc_peripheral_t bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_COUNT];

enum {
    BITSTREAM_SPI_DEVICE_A,
    BITSTREAM_SPI_DEVICE_COUNT
};
extern soc_peripheral_t bitstream_spi_devices[BITSTREAM_SPI_DEVICE_COUNT];

enum {
    BITSTREAM_AI_DEVICE_A,
    BITSTREAM_AI_DEVICE_COUNT
};
extern soc_peripheral_t bitstream_ai_devices[BITSTREAM_AI_DEVICE_COUNT];

#endif /* BITSTREAM_DEVICES_H_ */
