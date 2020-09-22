// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef SOC_CONF_H_
#define SOC_CONF_H_

/* Tile descriptors */
#define SOC_MULTITILE        1
#define SOC_TILE_0_INCLUDE   SOC_TILE_HAS_BOTH
#define SOC_TILE_1_INCLUDE   SOC_TILE_HAS_BITSTREAM
#define SOC_TILE_2_INCLUDE   SOC_TILE_UNUSED
#define SOC_TILE_3_INCLUDE   SOC_TILE_UNUSED

/* Peripherals */
#define SOC_ETHERNET_PERIPHERAL_USED        (0)
#define SOC_GPIO_PERIPHERAL_USED            (1)
#define SOC_I2C_PERIPHERAL_USED             (1)
#define SOC_I2S_PERIPHERAL_USED             (0)
#define SOC_MICARRAY_PERIPHERAL_USED        (0)
#define SOC_SDRAM_PERIPHERAL_USED           (0)
#define SOC_SPI_PERIPHERAL_USED             (1)
#define SOC_QSPI_FLASH_PERIPHERAL_USED      (0)
#define SOC_AI_PERIPHERAL_USED              (1)

/*
 * Peripheral Configuration
 */
/* I2C Config */
#define I2CCONF_MAX_BUF_LEN         (128)

/* SPI Config */
#define SPICONF_BUFFER_LEN          (18441)  /* Buffer large enough to handle entire image in 1 spi_transaction */

/* AI Config */
#define AI_INPUT_CHUNK_BYTES_LEN    (4096)
#define AI_OUTPUT_BYTES_LEN         (512)

#endif /* SOC_CONF_H_ */
