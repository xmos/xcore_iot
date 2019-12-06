// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef I2S_DEV_CONF_DEFAULTS_H_
#define I2S_DEV_CONF_DEFAULTS_H_

#ifndef I2SCONF_SAMPLE_FREQ
#define I2SCONF_SAMPLE_FREQ         (48000)
#endif

#ifndef I2SCONF_MASTER_CLK_FREQ
#define I2SCONF_MASTER_CLK_FREQ     (24576000)
#endif

#ifndef I2SCONF_AUDIO_FRAME_LEN
#define I2SCONF_AUDIO_FRAME_LEN     (256)
#endif

#ifndef I2SCONF_FRAME_BUF_CNT
#define I2SCONF_FRAME_BUF_CNT       (4)
#endif

#ifndef I2SCONF_OFF_TILE
#define I2SCONF_OFF_TILE            (1)
#endif

#endif /* I2S_DEV_CONF_DEFAULTS_H_ */
