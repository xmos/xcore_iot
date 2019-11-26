// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef SOC_H_
#define SOC_H_

#include <stdint.h>
#include <xccompat.h>

#define SOC_TILE_UNUSED        0
#define SOC_TILE_HAS_SOFTWARE  1
#define SOC_TILE_HAS_BITSTREAM 2
#define SOC_TILE_HAS_BOTH (SOC_TILE_HAS_SOFTWARE | SOC_TILE_HAS_BITSTREAM)

#if __soc_conf_h_exists__
#include "soc_conf.h"
#endif

#include "soc_conf_defaults.h"

#include "soc_channel.h"
#include "soc_peripheral_hub.h"
#include "soc_peripheral_control.h"

/*
 * Bitstream may optionally implement this function if
 * software is also present on the tile 0 and must wait for
 * the bitstream code to complete initialization.
 */
int soc_tile0_bitstream_initialized(void);

/*
 * Bitstream may optionally implement this function if
 * software is also present on the tile 1 and must wait for
 * the bitstream code to complete initialization.
 */
int soc_tile1_bitstream_initialized(void);

/*
 * Bitstream may optionally implement this function if
 * software is also present on the tile 2 and must wait for
 * the bitstream code to complete initialization.
 */
int soc_tile2_bitstream_initialized(void);

/*
 * Bitstream may optionally implement this function if
 * software is also present on the tile 3 and must wait for
 * the bitstream code to complete initialization.
 */
int soc_tile3_bitstream_initialized(void);

void soc_tile0_bitstream(
        int tile,
        NULLABLE_RESOURCE(chanend, xTile0Chan),
        NULLABLE_RESOURCE(chanend, xTile1Chan),
        NULLABLE_RESOURCE(chanend, xTile2Chan),
        NULLABLE_RESOURCE(chanend, xTile3Chan));

void soc_tile0_main(
        int tile);

void soc_tile1_bitstream(
        int tile,
        NULLABLE_RESOURCE(chanend, xTile0Chan),
        NULLABLE_RESOURCE(chanend, xTile1Chan),
        NULLABLE_RESOURCE(chanend, xTile2Chan),
        NULLABLE_RESOURCE(chanend, xTile3Chan));

void soc_tile1_main(
        int tile);

void soc_tile2_bitstream(
        int tile,
        NULLABLE_RESOURCE(chanend, xTile0Chan),
        NULLABLE_RESOURCE(chanend, xTile1Chan),
        NULLABLE_RESOURCE(chanend, xTile2Chan),
        NULLABLE_RESOURCE(chanend, xTile3Chan));

void soc_tile2_main(
        int tile);

void soc_tile3_bitstream(
        int tile,
        NULLABLE_RESOURCE(chanend, xTile0Chan),
        NULLABLE_RESOURCE(chanend, xTile1Chan),
        NULLABLE_RESOURCE(chanend, xTile2Chan),
        NULLABLE_RESOURCE(chanend, xTile3Chan));

void soc_tile3_main(
        int tile);

#endif /* SOC_H_ */
