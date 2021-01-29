// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef MIC_ARRAY_CONF_H_
#define MIC_ARRAY_CONF_H_

#define MIC_ARRAY_NUM_MICS             4 /* To make lib_mic_array happy */
#define MIC_ARRAY_MAX_FRAME_SIZE_LOG2  8

#define MIC_DUAL_ENABLED               1
#define MIC_DUAL_FRAME_SIZE            (1 << MIC_ARRAY_MAX_FRAME_SIZE_LOG2)
#define MIC_DUAL_NUM_REF_CHANNELS      0

#endif /* MIC_ARRAY_CONF_H_ */
