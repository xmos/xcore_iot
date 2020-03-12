// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef INTERTILE_DEV_H_
#define INTERTILE_DEV_H_

#if __soc_conf_h_exists__
#include "soc_conf.h"
#else
#warning soc_conf.h not found
#endif

#include "intertile_dev_conf_defaults.h"
#include "intertile_dev_ctrl.h"

#ifdef __XC__
extern "C" {
#endif //__XC__

void intertile_dev(
        soc_peripheral_t peripheral,
        chanend m_ctrl_c,
        chanend s_data_to_dma_c,
        chanend s_data_from_dma_c);

#ifdef __XC__
}
#endif //__XC__

#endif /* INTERTILE_DEV_H_ */
