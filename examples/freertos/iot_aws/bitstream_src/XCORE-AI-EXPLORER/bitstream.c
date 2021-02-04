// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#include "soc.h"
#include "bitstream.h"
#include "bitstream_devices.h"

static int initialized;

soc_peripheral_t bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_COUNT];
soc_peripheral_t bitstream_spi_devices[BITSTREAM_SPI_DEVICE_COUNT];
soc_peripheral_t bitstream_qspi_flash_devices[BITSTREAM_QSPI_FLASH_DEVICE_COUNT];

void device_register(
        chanend t0_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend spi_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
		chanend qspi_flash_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT]
	   )
{
    bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_A] = soc_peripheral_register(t0_gpio_dev_ch);
    bitstream_spi_devices[BITSTREAM_SPI_DEVICE_A] = soc_peripheral_register(spi_dev_ch);
    bitstream_qspi_flash_devices[BITSTREAM_QSPI_FLASH_DEVICE_A] = soc_peripheral_register(qspi_flash_dev_ch);

    initialized = 1;
}

int soc_tile0_bitstream_initialized(void)
{
    return initialized;
}

void soc_tile0_bitstream(
        int tile,
        chanend xTile0Chan,
        chanend xTile1Chan,
        chanend xTile2Chan,
        chanend xTile3Chan)
{
    tile0_device_instantiate();
}

void soc_tile1_bitstream(
        int tile,
        chanend xTile0Chan,
        chanend xTile1Chan,
        chanend xTile2Chan,
        chanend xTile3Chan)
{
    tile1_device_instantiate();
}
