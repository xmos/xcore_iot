// Copyright (c) 2019-2021, XMOS Ltd, All rights reserved

#include "soc.h"

#include <xcore/chanend.h>
#include <xcore/assert.h>

chanend_t soc_channel_establish(
        chanend_t remote_tile_chanend,
        soc_channel_direction_t direction)
{
    chanend_t local_c = chanend_alloc();
    xassert(local_c != 0);

    if (direction & soc_channel_input) {
        chanend_out_word(remote_tile_chanend, local_c);
        chanend_out_end_token(remote_tile_chanend);
    }

    if (direction & soc_channel_output) {
        chanend_t remote_c;
        remote_c = (chanend_t)chanend_in_word(remote_tile_chanend);
        chanend_check_end_token(remote_tile_chanend);
        chanend_set_dest(local_c, remote_c);
    }

    return local_c;
}
