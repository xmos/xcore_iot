// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef BITSTREAM_DEVICES_H_
#define BITSTREAM_DEVICES_H_

enum {
    BITSTREAM_MICARRAY_DEVICE_A,
    BITSTREAM_MICARRAY_DEVICE_COUNT
};
extern soc_peripheral_t bitstream_micarray_devices[BITSTREAM_MICARRAY_DEVICE_COUNT];

enum {
    BITSTREAM_I2S_DEVICE_A,
    BITSTREAM_I2S_DEVICE_COUNT
};
extern soc_peripheral_t bitstream_i2s_devices[BITSTREAM_I2S_DEVICE_COUNT];

enum {
    BITSTREAM_I2C_DEVICE_A,
    BITSTREAM_I2C_DEVICE_COUNT
};
extern soc_peripheral_t bitstream_i2c_devices[BITSTREAM_I2C_DEVICE_COUNT];

enum {
    BITSTREAM_GPIO_DEVICE_A,
    BITSTREAM_GPIO_DEVICE_B,
    BITSTREAM_GPIO_DEVICE_COUNT
};
extern soc_peripheral_t bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_COUNT];

enum {
    BITSTREAM_SPI_DEVICE_A,
    BITSTREAM_SPI_DEVICE_COUNT
};
extern soc_peripheral_t bitstream_spi_devices[BITSTREAM_SPI_DEVICE_COUNT];

enum {
    BITSTREAM_QSPI_FLASH_DEVICE_A,
    BITSTREAM_QSPI_FLASH_DEVICE_COUNT
};
extern soc_peripheral_t bitstream_qspi_flash_devices[BITSTREAM_QSPI_FLASH_DEVICE_COUNT];


#endif /* BITSTREAM_DEVICES_H_ */
