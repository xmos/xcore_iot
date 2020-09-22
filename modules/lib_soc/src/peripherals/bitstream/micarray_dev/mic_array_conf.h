// Copyright (c) 2019, XMOS Ltd, All rights reserved

#ifndef MIC_ARRAY_CONF_H_
#define MIC_ARRAY_CONF_H_

#if __soc_conf_h_exists__
#include "soc_conf.h"
#endif

#include "micarray_dev_conf_defaults.h"

#define MIC_ARRAY_WORD_LENGTH_SHORT     MICARRAYCONF_WORD_LENGTH_SHORT
#define MIC_ARRAY_MAX_FRAME_SIZE_LOG2   MICARRAYCONF_MAX_FRAME_SIZE_LOG2
#define MIC_ARRAY_NUM_MICS              MICARRAYCONF_NUM_MICS

#endif /* MIC_ARRAY_CONF_H_ */
