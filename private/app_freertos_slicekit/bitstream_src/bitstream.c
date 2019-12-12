// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"
#include "bitstream.h"
#include "bitstream_devices.h"

static int initialized;

soc_peripheral_t bitstream_micarray_devices[BITSTREAM_MICARRAY_DEVICE_COUNT];
soc_peripheral_t bitstream_ethernet_devices[BITSTREAM_ETHERNET_DEVICE_COUNT];
soc_peripheral_t bitstream_sdram_devices[BITSTREAM_SDRAM_DEVICE_COUNT];

void device_register(
        chanend mic_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend eth_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend sdram_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT])
{
    bitstream_micarray_devices[BITSTREAM_MICARRAY_DEVICE_A] = soc_peripheral_register(mic_dev_ch);
    bitstream_ethernet_devices[BITSTREAM_ETHERNET_DEVICE_A] = soc_peripheral_register(eth_dev_ch);
    bitstream_sdram_devices[BITSTREAM_SDRAM_DEVICE_A] = soc_peripheral_register(sdram_dev_ch);

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
    chanend eth_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];
    chanend sdram_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];

    eth_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    eth_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    eth_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    eth_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    sdram_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = 0;
    sdram_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = 0;
    sdram_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    sdram_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    tile0_device_instantiate(eth_dev_ch, sdram_dev_ch);
}

void soc_tile1_bitstream(
        int tile,
        chanend xTile0Chan,
        chanend xTile1Chan,
        chanend xTile2Chan,
        chanend xTile3Chan)
{
    chanend eth_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];
    chanend sdram_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];

    eth_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    eth_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    eth_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    eth_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    sdram_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = 0;
    sdram_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = 0;
    sdram_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    sdram_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    tile1_device_instantiate(eth_dev_ch, sdram_dev_ch);
}
