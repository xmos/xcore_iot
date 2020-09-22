// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef BITSTREAM_H_
#define BITSTREAM_H_

#include "soc.h"

#ifdef __XC__
#include "qspi_flash_dev.h"
#include "ai_dev.h"
#endif //__XC__

/*
 * Bitstream specific
 *
 * Internal to the bitstream.
 */
void tile0_device_instantiate(
        chanend ai_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend swmem_ctrl_ch
);
/*
 * Bitstream specific
 *
 * Internal to the bitstream.
 */
void tile1_device_instantiate(
        chanend ai_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend swmem_ctrl_ch
);

#ifdef __XC__
extern "C" {
#endif //__XC__

/*
 * Bitstream specific
 *
 * Called by the hardware side, to be
 * implemented on the software side.
 */
void device_register(
		chanend qspi_flash_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT],
        chanend ai_dev_ch[SOC_PERIPHERAL_CHANNEL_COUNT]
		);

#ifdef __XC__
}
#endif //__XC__

#endif /* BITSTREAM_H_ */
