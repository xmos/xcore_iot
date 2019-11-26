// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include "soc.h"

#include "xcore_c.h"

chanend soc_channel_establish(
        chanend remote_tile_chanend,
        soc_channel_direction_t direction)
{
    chanend local_c;

    chanend_alloc(&local_c);

    if (direction & soc_channel_input) {
        s_chan_out_word(remote_tile_chanend, local_c);
        s_chan_out_ct_end(remote_tile_chanend);
    }

    if (direction & soc_channel_output) {
        chanend remote_c;
        s_chan_in_word(remote_tile_chanend, (uint32_t *) &remote_c);
        s_chan_check_ct_end(remote_tile_chanend);
        chanend_set_dest(local_c, remote_c);
    }

    return local_c;
}
