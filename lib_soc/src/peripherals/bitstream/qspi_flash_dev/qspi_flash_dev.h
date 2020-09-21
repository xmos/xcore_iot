// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef QSPI_FLASH_DEV_H_
#define QSPI_FLASH_DEV_H_

#if __soc_conf_h_exists__
#include "soc_conf.h"
#else
#warning soc_conf.h not found
#endif

#include "qspi_flash_dev_conf_defaults.h"
#include "qspi_flash_dev_ctrl.h"

#ifdef __XC__
extern "C" {
#endif //__XC__

void qspi_flash_dev(
        soc_peripheral_t peripheral,
		chanend data_to_dma_c,
        chanend data_from_dma_c,
		chanend ctrl_c,
		chanend ctrl_swmem_local,
		chanend ctrl_swmem_remote,
		int page_count,
		const flash_ports_t        *flash_ports,
		const flash_clock_config_t *flash_clock_config,
		const flash_qe_config_t    *flash_qe_config);

#ifdef __XC__
}
#endif //__XC__

#endif /* QSPI_FLASH_DEV_H_ */
