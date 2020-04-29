// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef BITSTREAM_H_
#define BITSTREAM_H_

#include "soc.h"

#ifdef __XC__
#include "intertile_dev.h"
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
        chanend intertile_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT]);
/*
 * Bitstream specific
 *
 * Internal to the bitstream.
 */
void tile1_device_instantiate(
        chanend intertile_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT]);

#ifdef __XC__
extern "C" {
#endif //__XC__

/*
 * Bitstream specific
 *
 * Called by the hardware side, to be
 * implemented on the software side.
 */
void tile0_device_register(
        chanend intertile_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT]);

void tile1_device_register(
        chanend intertile_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT]);

#ifdef __XC__
}
#endif //__XC__

#endif /* BITSTREAM_H_ */
