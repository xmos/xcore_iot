// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <platform.h>
#include <stdint.h>
#include <timer.h>

#include "xassert.h"
#include "soc.h"
#include "bitstream.h"
#include "bitstream_devices.h"

#if THIS_XCORE_TILE == 0
void tile0_device_instantiate(
        chanend intertile_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT])
{
    chan this_intertile_dev_ctrl_ch;

    par {
        unsafe {
            unsafe chanend this_intertile_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT] = {null, null, this_intertile_dev_ctrl_ch, null};

            tile0_device_register(this_intertile_dev_ch);
            soc_peripheral_hub();
        }

        {
            while (soc_tile0_bitstream_initialized() == 0);
            par {
                intertile_dev(
                        bitstream_intertile_devices[BITSTREAM_INTERTILE_DEVICE_A],
                        this_intertile_dev_ctrl_ch,
                        intertile_dev_ch[SOC_PERIPHERAL_FROM_DMA_CH],
                        intertile_dev_ch[SOC_PERIPHERAL_TO_DMA_CH]);
            }
        }
    }
}
#endif

#if THIS_XCORE_TILE == 1
void tile1_device_instantiate(
        chanend intertile_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT])
{
    par {
        unsafe {
            tile1_device_register(intertile_dev_ch);
            soc_peripheral_hub();
        }
    }
}
#endif
