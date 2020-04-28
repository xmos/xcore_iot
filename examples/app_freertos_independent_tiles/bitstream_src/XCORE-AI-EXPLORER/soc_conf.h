// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef SOC_CONF_H_
#define SOC_CONF_H_

/* Tile descriptors */
#define SOC_MULTITILE        1
#define SOC_TILE_0_INCLUDE   SOC_TILE_HAS_BOTH
#define SOC_TILE_1_INCLUDE   SOC_TILE_HAS_BOTH
#define SOC_TILE_2_INCLUDE   SOC_TILE_UNUSED
#define SOC_TILE_3_INCLUDE   SOC_TILE_UNUSED

/* Peripherals */
#define SOC_INTERTILE_PERIPHERAL_USED       (1)
#define SOC_ETHERNET_PERIPHERAL_USED        (0)
#define SOC_GPIO_PERIPHERAL_USED            (0)
#define SOC_I2C_PERIPHERAL_USED             (0)
#define SOC_I2S_PERIPHERAL_USED             (0)
#define SOC_MICARRAY_PERIPHERAL_USED        (0)
#define SOC_SDRAM_PERIPHERAL_USED           (0)
#define SOC_SPI_PERIPHERAL_USED             (0)

/*
 * Software Configuration
 */
#define impconfINTERTILE_MAX_PIPES                 (4)
#define impconfINTERTILE_EVENT_QUEUE_LEN           (4)
#define impconfNUM_INTERTILE_BUFFER_DESCRIPTORS    (10)
#define impconfNUM_RX_INTERTILE_BUFFER_DESCRIPTORS (5)

/*
 * Peripheral Configuration
 */

/* Intertile Config */
#define INTERTILE_DEV_HANDLER_COUNT         (3)
#define INTERTILE_DEV_BUFSIZE               (1024)

#endif /* SOC_CONF_H_ */
