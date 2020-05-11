// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include "soc.h"
#include "bitstream.h"
#include "bitstream_devices.h"

static int initialized;

soc_peripheral_t bitstream_micarray_devices[BITSTREAM_MICARRAY_DEVICE_COUNT];
soc_peripheral_t bitstream_i2s_devices[BITSTREAM_I2S_DEVICE_COUNT];
soc_peripheral_t bitstream_i2c_devices[BITSTREAM_I2C_DEVICE_COUNT];
soc_peripheral_t bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_COUNT];
soc_peripheral_t bitstream_spi_devices[BITSTREAM_SPI_DEVICE_COUNT];
soc_peripheral_t bitstream_qspi_flash_devices[BITSTREAM_QSPI_FLASH_DEVICE_COUNT];

void device_register(
        chanend mic_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend i2c_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend t0_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend spi_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
		chanend qspi_flash_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT]
	   )
{
    bitstream_micarray_devices[BITSTREAM_MICARRAY_DEVICE_A] = soc_peripheral_register(mic_dev_ch);
    bitstream_i2s_devices[BITSTREAM_I2S_DEVICE_A] = soc_peripheral_register(i2s_dev_ch);
    bitstream_i2c_devices[BITSTREAM_I2C_DEVICE_A] = soc_peripheral_register(i2c_dev_ch);
    bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_A] = soc_peripheral_register(t0_gpio_dev_ch);
    bitstream_gpio_devices[BITSTREAM_GPIO_DEVICE_B] = soc_peripheral_register(t1_gpio_dev_ch);
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
    chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];
    chanend mic_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];
    chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];

    i2s_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    i2s_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = 0;
    i2s_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = 0;
    i2s_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    mic_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = 0;
    mic_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    mic_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = 0;
    mic_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    t1_gpio_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = 0;
    t1_gpio_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = 0;
    t1_gpio_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    t1_gpio_dev_ch[SOC_PERIPHERAL_IRQ_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);

    tile0_device_instantiate(i2s_dev_ch, mic_dev_ch, t1_gpio_dev_ch);
}

void soc_tile1_bitstream(
        int tile,
        chanend xTile0Chan,
        chanend xTile1Chan,
        chanend xTile2Chan,
        chanend xTile3Chan)
{
    chanend i2s_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];
    chanend mic_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];
    chanend t1_gpio_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];

    i2s_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    i2s_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = 0;
    i2s_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = 0;
    i2s_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    mic_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = 0;
    mic_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    mic_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = 0;
    mic_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    t1_gpio_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = 0;
    t1_gpio_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = 0;
    t1_gpio_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    t1_gpio_dev_ch[SOC_PERIPHERAL_IRQ_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);

    tile1_device_instantiate(i2s_dev_ch, mic_dev_ch, t1_gpio_dev_ch);
}
