// Copyright (c) 2019-2021, XMOS Ltd, All rights reserved

#ifndef SOC_CHANNEL_H_
#define SOC_CHANNEL_H_

#ifdef __XC__
extern "C" {
#endif //__XC__

#include <xcore/chanend.h>

typedef enum {
    soc_channel_input  = 0x1,
    soc_channel_output = 0x2,
    soc_channel_inout  = 0x3,
} soc_channel_direction_t;

/**
 * Establishes a channel between two tiles. This must be called by both tiles on
 * either end of the channel at the same time.
 *
 * \param remote_tile_chanend Is an already established channel to the remote tile.
 *
 * \param direction specifies the direction of data flow from the calling tile's
 *                  point of view. See soc_channel_direction_t.
 *                  Normally use soc_channel_inout as this is required when
 *                  the new channel will be used as a "regular" channel, either from
 *                  xC functions or with the standard non-streaming channel functions
 *                  from lib_xcore_c (e.g. chan_out_word()).
 *
 * \return the channel end of the newly established channel that can be used by the
 *         calling tile.
 */
chanend_t soc_channel_establish(
        chanend_t remote_tile_chanend,
        soc_channel_direction_t direction);

#ifdef __XC__
}
#endif //__XC__

#endif /* SOC_CHANNEL_H_ */
