// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef AI_DEV_H_
#define AI_DEV_H_

#if __soc_conf_h_exists__
#include "soc_conf.h"
#else
#warning soc_conf.h not found
#endif

#include "ai_dev_conf_defaults.h"
#include "ai_dev_ctrl.h"

#ifdef __XC__
extern "C" {
#endif //__XC__

void ai_dev(
        soc_peripheral_t peripheral,
		chanend data_to_dma_c,
        chanend data_from_dma_c,
		chanend ctrl_c,
        chanend ctrl_swmem_c);

#ifdef __XC__
}
#endif //__XC__

#endif /* AI_DEV_H_ */
