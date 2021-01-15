// Copyright (c) 2021, XMOS Ltd, All rights reserved

#include "platform.h"

extern "C" {
    void main_tile0(chanend c);
    void main_tile1(chanend c);
    void main_tile2(chanend c);
    void main_tile3(chanend c);
}

int main(void)
{
	chan c;
    par {
        #if (PLATFORM_SUPPORTS_TILE_0 == 1) && (PLATFORM_USES_TILE_0 == 1)
            on tile[0]: main_tile0(c);
        #endif

        #if (PLATFORM_SUPPORTS_TILE_1 == 1) && (PLATFORM_USES_TILE_1 == 1)
            on tile[1]: main_tile1(c);
        #endif

        #if (PLATFORM_SUPPORTS_TILE_2 == 1) && (PLATFORM_USES_TILE_2 == 1)
            on tile[2]: main_tile2(c);
        #endif

        #if (PLATFORM_SUPPORTS_TILE_3 == 1) && (PLATFORM_USES_TILE_3 == 1)
            on tile[3]: main_tile3(c);
        #endif
    }

    return 0;
}
