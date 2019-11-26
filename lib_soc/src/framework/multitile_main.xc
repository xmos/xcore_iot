// Copyright (c) 2019, XMOS Ltd, All rights reserved

#include <platform.h>
#include "soc.h"


#if SOC_TILE_0_INCLUDE
static void tile0(
        int tile,
        chanend ?xTile0Chan,
        chanend ?xTile1Chan,
        chanend ?xTile2Chan,
        chanend ?xTile3Chan)
{
    par {
#if (SOC_TILE_0_INCLUDE & SOC_TILE_HAS_BITSTREAM)
        soc_tile0_bitstream(tile, xTile0Chan, xTile1Chan, xTile2Chan, xTile3Chan);
#endif
#if (SOC_TILE_0_INCLUDE & SOC_TILE_HAS_SOFTWARE)
        {
            while (soc_tile0_bitstream_initialized() == 0);
            soc_tile0_main(tile);
        }
#endif
    }
}
#endif

#if SOC_TILE_1_INCLUDE
static void tile1(
        int tile,
        chanend ?xTile0Chan,
        chanend ?xTile1Chan,
        chanend ?xTile2Chan,
        chanend ?xTile3Chan)
{
    par {
#if (SOC_TILE_1_INCLUDE & SOC_TILE_HAS_BITSTREAM)
        soc_tile1_bitstream(tile, xTile0Chan, xTile1Chan, xTile2Chan, xTile3Chan);
#endif
#if (SOC_TILE_1_INCLUDE & SOC_TILE_HAS_SOFTWARE)
        {
            while (soc_tile1_bitstream_initialized() == 0);
            soc_tile1_main(tile);
        }
#endif
    }
}
#endif

#if SOC_TILE_2_INCLUDE
static void tile2(
        int tile,
        chanend ?xTile0Chan,
        chanend ?xTile1Chan,
        chanend ?xTile2Chan,
        chanend ?xTile3Chan)
{
    par {
#if (SOC_TILE_2_INCLUDE & SOC_TILE_HAS_BITSTREAM)
        soc_tile2_bitstream(tile, xTile0Chan, xTile1Chan, xTile2Chan, xTile3Chan);
#endif
#if (SOC_TILE_2_INCLUDE & SOC_TILE_HAS_SOFTWARE)
        {
            while (soc_tile2_bitstream_initialized() == 0);
            soc_tile2_main(tile);
        }
#endif
    }
}
#endif

#if SOC_TILE_3_INCLUDE
static void tile3(
        int tile,
        chanend ?xTile0Chan,
        chanend ?xTile1Chan,
        chanend ?xTile2Chan,
        chanend ?xTile3Chan)
{
    par {
#if (SOC_TILE_3_INCLUDE & SOC_TILE_HAS_BITSTREAM)
        soc_tile3_bitstream(tile, xTile0Chan, xTile1Chan, xTile2Chan, xTile3Chan);
#endif
#if (SOC_TILE_3_INCLUDE & SOC_TILE_HAS_SOFTWARE)
        {
            while (soc_tile3_bitstream_initialized() == 0);
            soc_tile3_main(tile);
        }
#endif
    }
}
#endif

int main(void)
{
#if SOC_TILE_0_INCLUDE && SOC_TILE_1_INCLUDE
    chan c_t0_t1;
#endif
#if SOC_TILE_0_INCLUDE && SOC_TILE_2_INCLUDE
    chan c_t0_t2;
#endif
#if SOC_TILE_0_INCLUDE && SOC_TILE_3_INCLUDE
    chan c_t0_t3;
#endif
#if SOC_TILE_1_INCLUDE && SOC_TILE_2_INCLUDE
    chan c_t1_t2;
#endif
#if SOC_TILE_1_INCLUDE && SOC_TILE_3_INCLUDE
    chan c_t1_t3;
#endif
#if SOC_TILE_2_INCLUDE && SOC_TILE_3_INCLUDE
    chan c_t2_t3;
#endif

    par {

#if SOC_TILE_0_INCLUDE
        on tile[0]: tile0(
                0,
                null,
#if SOC_TILE_1_INCLUDE
                c_t0_t1,
#else
                null,
#endif
#if SOC_TILE_2_INCLUDE
                c_t0_t2,
#else
                null,
#endif
#if SOC_TILE_3_INCLUDE
                c_t0_t3
#else
                null
#endif
        );
#endif

#if SOC_TILE_1_INCLUDE
        on tile[1]: tile1(
                1,
#if SOC_TILE_0_INCLUDE
                c_t0_t1,
#else
                null,
#endif
                null,
#if SOC_TILE_2_INCLUDE
                c_t1_t2,
#else
                null,
#endif
#if SOC_TILE_3_INCLUDE
                c_t1_t3
#else
                null
#endif
        );
#endif

#if SOC_TILE_2_INCLUDE
        on tile[2]: tile2(
                2,
#if SOC_TILE_0_INCLUDE
                c_t0_t2,
#else
                null,
#endif
#if SOC_TILE_1_INCLUDE
                c_t1_t2,
#else
                null,
#endif
                null,
#if SOC_TILE_3_INCLUDE
                c_t2_t3
#else
                null
#endif
        );
#endif

#if SOC_TILE_3_INCLUDE
        on tile[3]: tile3(
                3,
#if SOC_TILE_0_INCLUDE
                c_t0_t3,
#else
                null,
#endif
#if SOC_TILE_1_INCLUDE
                c_t1_t3,
#else
                null,
#endif
#if SOC_TILE_2_INCLUDE
                c_t2_t3,
#else
                null,
#endif
                null
        );
#endif

    }

    return 0;
}
