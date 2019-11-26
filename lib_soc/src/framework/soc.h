/*
 * soc.h
 *
 *  Created on: Sep 24, 2019
 *      Author: mbruno
 */


#ifndef SOC_H_
#define SOC_H_

#include <stdint.h>
#include <xccompat.h>

#define XCORE_FREERTOS_TILE_UNUSED        0
#define XCORE_FREERTOS_TILE_HAS_SOFTWARE  1
#define XCORE_FREERTOS_TILE_HAS_BITSTREAM 2
#define XCORE_FREERTOS_TILE_HAS_BOTH (XCORE_FREERTOS_TILE_HAS_SOFTWARE | XCORE_FREERTOS_TILE_HAS_BITSTREAM)

#if __soc_conf_h_exists__
#include "soc_conf.h"
#endif

#include "soc_conf_defaults.h"

#include "xcore_freertos_channel.h"
#include "xcore_freertos_dma.h"
#include "xcore_freertos_peripheral_control.h"

/*
 * Bitstream may optionally implement this function if
 * software is also present on the tile 0 and must wait for
 * the bitstream code to complete initialization.
 */
int xcore_freertos_tile0_bitstream_initialized(void);

/*
 * Bitstream may optionally implement this function if
 * software is also present on the tile 1 and must wait for
 * the bitstream code to complete initialization.
 */
int xcore_freertos_tile1_bitstream_initialized(void);

/*
 * Bitstream may optionally implement this function if
 * software is also present on the tile 2 and must wait for
 * the bitstream code to complete initialization.
 */
int xcore_freertos_tile2_bitstream_initialized(void);

/*
 * Bitstream may optionally implement this function if
 * software is also present on the tile 3 and must wait for
 * the bitstream code to complete initialization.
 */
int xcore_freertos_tile3_bitstream_initialized(void);

void xcore_freertos_tile0_bitstream(
        int tile,
        NULLABLE_RESOURCE(chanend, xTile0Chan),
        NULLABLE_RESOURCE(chanend, xTile1Chan),
        NULLABLE_RESOURCE(chanend, xTile2Chan),
        NULLABLE_RESOURCE(chanend, xTile3Chan));

void xcore_freertos_tile0_main(
        int tile);

void xcore_freertos_tile1_bitstream(
        int tile,
        NULLABLE_RESOURCE(chanend, xTile0Chan),
        NULLABLE_RESOURCE(chanend, xTile1Chan),
        NULLABLE_RESOURCE(chanend, xTile2Chan),
        NULLABLE_RESOURCE(chanend, xTile3Chan));

void xcore_freertos_tile1_main(
        int tile);

void xcore_freertos_tile2_bitstream(
        int tile,
        NULLABLE_RESOURCE(chanend, xTile0Chan),
        NULLABLE_RESOURCE(chanend, xTile1Chan),
        NULLABLE_RESOURCE(chanend, xTile2Chan),
        NULLABLE_RESOURCE(chanend, xTile3Chan));

void xcore_freertos_tile2_main(
        int tile);

void xcore_freertos_tile3_bitstream(
        int tile,
        NULLABLE_RESOURCE(chanend, xTile0Chan),
        NULLABLE_RESOURCE(chanend, xTile1Chan),
        NULLABLE_RESOURCE(chanend, xTile2Chan),
        NULLABLE_RESOURCE(chanend, xTile3Chan));

void xcore_freertos_tile3_main(
        int tile);

#endif /* SOC_H_ */
