// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include "soc.h"
#include "bitstream.h"
#include "bitstream_devices.h"

static int t0_initialized, t1_initialized;

soc_peripheral_t bitstream_intertile_devices[BITSTREAM_INTERTILE_DEVICE_COUNT];

#if THIS_XCORE_TILE == 0
void tile0_device_register(
        chanend intertile_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT])
{
    bitstream_intertile_devices[BITSTREAM_INTERTILE_DEVICE_A] = soc_peripheral_register(intertile_dev_ch);

    t0_initialized = 1;
}

int soc_tile0_bitstream_initialized(void)
{
    return t0_initialized;
}

void soc_tile0_bitstream(
        int tile,
        chanend xTile0Chan,
        chanend xTile1Chan,
        chanend xTile2Chan,
        chanend xTile3Chan)
{
    chanend intertile_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];

    intertile_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    intertile_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = soc_channel_establish(xTile1Chan, soc_channel_inout);
    intertile_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = 0;//soc_channel_establish(xTile1Chan, soc_channel_inout);
    intertile_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;//soc_channel_establish(xTile1Chan, soc_channel_inout);

    tile0_device_instantiate( intertile_dev_ch);
}
#endif

#if THIS_XCORE_TILE == 1
void tile1_device_register(
        chanend intertile_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT])
{
    bitstream_intertile_devices[BITSTREAM_INTERTILE_DEVICE_A] = soc_peripheral_register(intertile_dev_ch);

    t1_initialized = 1;
}

int soc_tile1_bitstream_initialized(void)
{
    return t1_initialized;
}

void soc_tile1_bitstream(
        int tile,
        chanend xTile0Chan,
        chanend xTile1Chan,
        chanend xTile2Chan,
        chanend xTile3Chan)
{
    chanend intertile_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT];

    intertile_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    intertile_dev_ch[SOC_PERIPHERAL_TO_DMA_CH] = soc_channel_establish(xTile0Chan, soc_channel_inout);
    intertile_dev_ch[SOC_PERIPHERAL_CONTROL_CH] = 0;//soc_channel_establish(xTile0Chan, soc_channel_inout);
    intertile_dev_ch[SOC_PERIPHERAL_IRQ_CH] = 0;//soc_channel_establish(xTile0Chan, soc_channel_inout);

    tile1_device_instantiate( intertile_dev_ch );
}
#endif
