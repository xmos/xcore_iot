// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include "soc.h"
#include "bitstream.h"
#include "bitstream_devices.h"

static int initialized;

soc_peripheral_t bitstream_qspi_flash_devices[BITSTREAM_QSPI_FLASH_DEVICE_COUNT];
soc_peripheral_t bitstream_ai_devices[BITSTREAM_AI_DEVICE_COUNT];

void device_register(
		chanend qspi_flash_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
		chanend ai_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT]
	   )
{
    bitstream_qspi_flash_devices[BITSTREAM_QSPI_FLASH_DEVICE_A] = soc_peripheral_register(qspi_flash_dev_ch);
    bitstream_ai_devices[BITSTREAM_AI_DEVICE_A] = soc_peripheral_register(ai_dev_ch);

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
    chanend ai_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];
    chanend swmem_t1_ctrl_ch;

    ai_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    ai_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    ai_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    ai_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    swmem_t1_ctrl_ch = soc_channel_establish(xTile1Chan, soc_channel_inout);

    tile0_device_instantiate(ai_dev_ch, swmem_t1_ctrl_ch);
}

void soc_tile1_bitstream(
        int tile,
        chanend xTile0Chan,
        chanend xTile1Chan,
        chanend xTile2Chan,
        chanend xTile3Chan)
{
    chanend ai_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];
    chanend swmem_t1_ctrl_ch;

    ai_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    ai_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    ai_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    ai_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;

    swmem_t1_ctrl_ch = soc_channel_establish(xTile0Chan, soc_channel_inout);

    tile1_device_instantiate(ai_dev_ch, swmem_t1_ctrl_ch);
}
