// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef SOC_CONF_H_
#define SOC_CONF_H_

/* Tile descriptors */
#define SOC_MULTITILE        1
#define SOC_TILE_0_INCLUDE   SOC_TILE_HAS_BOTH
#define SOC_TILE_1_INCLUDE   SOC_TILE_HAS_BITSTREAM
#define SOC_TILE_2_INCLUDE   SOC_TILE_UNUSED
#define SOC_TILE_3_INCLUDE   SOC_TILE_UNUSED

/* Peripherals */
#define SOC_ETHERNET_PERIPHERAL_USED        (1)
#define SOC_GPIO_PERIPHERAL_USED            (0)
#define SOC_I2C_PERIPHERAL_USED             (0)
#define SOC_I2S_PERIPHERAL_USED             (0)
#define SOC_MICARRAY_PERIPHERAL_USED        (1)
#define SOC_SDRAM_PERIPHERAL_USED           (1)

/*
 * Peripheral Configuration
 */

/* ETH Config */
#define ETHCONF_MII_BUFSIZE         (4096)
#define ETHCONF_SMI_PHY_ADDRESS     (0)

#define ETHCONF_SMI_MDIO_BIT_POS    (1)
#define ETHCONF_SMI_MDC_BIT_POS     (0)

/* RT Mac requires 4 cores, instead of 2 */
#define ETHCONF_USE_RT_MAC          (0)

#define ETHCONF_RT_MII_RX_BUFSIZE   (ETHCONF_MII_BUFSIZE)
#define ETHCONF_RT_MII_TX_BUFSIZE   (ETHCONF_MII_BUFSIZE)
#define ETHCONF_USE_SHAPER          (0)

/* MicArray Config */
#define MICARRAYCONF_WORD_LENGTH_SHORT              (0)
#define MICARRAYCONF_MAX_FRAME_SIZE_LOG2            (8)
#define MICARRAYCONF_NUM_MICS                       (4)
#define MICARRAYCONF_DECIMATOR_COUNT                (1)
#define MICARRAYCONF_NUM_FRAME_BUFFERS              (2)
#define MICARRAYCONF_PDM_INTEGRATION_FACTOR         (32)
#define MICARRAYCONF_SAMPLE_RATE                    (48000)
#define MICARRAYCONF_MASTER_TO_PDM_CLOCK_DIVIDER    (8)
#define MICARRAYCONF_MASTER_CLOCK_FREQUENCY         (24576000)

/* SDRAM Config */
#define SDRAMCONF_READ_BUFFER_SIZE       256
#define SDRAMCONF_WRITE_BUFFER_SIZE      256

#endif /* SOC_CONF_H_ */
