// Copyright (c) 2019-2020, XMOS Ltd, All rights reserved

#ifndef SOC_CONF_H_
#define SOC_CONF_H_

/* Tile descriptors */
#define SOC_MULTITILE        0
#define SOC_TILE_0_INCLUDE   SOC_TILE_HAS_BOTH
#define SOC_TILE_1_INCLUDE   SOC_TILE_UNUSED
#define SOC_TILE_2_INCLUDE   SOC_TILE_UNUSED
#define SOC_TILE_3_INCLUDE   SOC_TILE_UNUSED

/* Peripherals */
#define SOC_ETHERNET_PERIPHERAL_USED        (0)
#define SOC_GPIO_PERIPHERAL_USED            (1)
#define SOC_I2C_PERIPHERAL_USED             (0)
#define SOC_I2S_PERIPHERAL_USED             (0)
#define SOC_MICARRAY_PERIPHERAL_USED        (0)
#define SOC_SDRAM_PERIPHERAL_USED           (0)
#define SOC_SPI_PERIPHERAL_USED             (1)
#define SOC_QSPI_FLASH_PERIPHERAL_USED      (1)

/*
 * Peripheral Configuration
 */
/* QSPI Flash Config */
#define QSPI_FLASH_DEV_WRITE_BUFSIZE  4096

#endif /* SOC_CONF_H_ */
