// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include "soc.h"

#if (SOC_TILE_0_INCLUDE & SOC_TILE_HAS_SOFTWARE)
__attribute__((weak))
int soc_tile0_bitstream_initialized(void)
{
    return 1;
}

__attribute__((weak))
void soc_tile0_main(
        int tile)
{
    rtos_printf("Warning: tile 0 software entry point missing\n");
    return;
}
#endif

#if (SOC_TILE_0_INCLUDE & SOC_TILE_HAS_BITSTREAM)
__attribute__((weak))
void soc_tile0_bitstream(
        int tile,
        chanend xTile0Chan,
        chanend xTile1Chan,
        chanend xTile2Chan,
        chanend xTile3Chan)
{
    rtos_printf("Warning: tile 0 bitstream entry point missing\n");
    return;
}
#endif

#if (SOC_TILE_1_INCLUDE & SOC_TILE_HAS_SOFTWARE)
__attribute__((weak))
int soc_tile1_bitstream_initialized(void)
{
    return 1;
}

__attribute__((weak))
void soc_tile1_main(
        int tile)
{
    rtos_printf("Warning: tile 1 software entry point missing\n");
    return;
}
#endif

#if (SOC_TILE_1_INCLUDE & SOC_TILE_HAS_BITSTREAM)
__attribute__((weak))
void soc_tile1_bitstream(
        int tile,
        chanend xTile0Chan,
        chanend xTile1Chan,
        chanend xTile2Chan,
        chanend xTile3Chan)
{
    rtos_printf("Warning: tile 1 bitstream entry point missing\n");
    return;
}
#endif

#if (SOC_TILE_2_INCLUDE & SOC_TILE_HAS_SOFTWARE)
__attribute__((weak))
int soc_tile2_bitstream_initialized(void)
{
    return 1;
}

__attribute__((weak))
void soc_tile2_main(
        int tile)
{
    rtos_printf("Warning: tile 2 software entry point missing\n");
    return;
}
#endif

#if (SOC_TILE_2_INCLUDE & SOC_TILE_HAS_BITSTREAM)
__attribute__((weak))
void soc_tile2_bitstream(
        int tile,
        chanend xTile0Chan,
        chanend xTile1Chan,
        chanend xTile2Chan,
        chanend xTile3Chan)
{
    rtos_printf("Warning: tile 2 bitstream entry point missing\n");
    return;
}
#endif

#if (SOC_TILE_3_INCLUDE & SOC_TILE_HAS_SOFTWARE)
__attribute__((weak))
int soc_tile3_bitstream_initialized(void)
{
    return 1;
}

__attribute__((weak))
void soc_tile3_main(
        int tile)
{
    rtos_printf("Warning: tile 3 software entry point missing\n");
    return;
}
#endif

#if (SOC_TILE_3_INCLUDE & SOC_TILE_HAS_BITSTREAM)
__attribute__((weak))
void soc_tile3_bitstream(
        int tile,
        chanend xTile0Chan,
        chanend xTile1Chan,
        chanend xTile2Chan,
        chanend xTile3Chan)
{
    rtos_printf("Warning: tile 3 bitstream entry point missing\n");
    return;
}
#endif
