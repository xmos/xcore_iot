// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef RTOS_PERIPHERALS_CONF_H_
#define RTOS_PERIPHERALS_CONF_H_

/* I2S Config */
#define I2SCONF_SAMPLE_FREQ         (48000)
#define I2SCONF_MASTER_CLK_FREQ     (24576000)
#define I2SCONF_AUDIO_FRAME_LEN     (256)
#define I2SCONF_FRAME_BUF_CNT       (4)


/* I2C Config */
#define I2CCONF_MAX_BUF_LEN         (128)


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


#endif /* RTOS_PERIPHERALS_CONF_H_ */
