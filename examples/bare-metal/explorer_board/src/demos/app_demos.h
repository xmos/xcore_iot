// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_DEMOS_H_
#define APP_DEMOS_H_

#include <xcore/parallel.h>
#include <xcore/chanend.h>

#include "spi.h"
#include "qspi_flash.h"

DECLARE_JOB(spi_demo, (spi_master_device_t*));
DECLARE_JOB(gpio_server, (chanend_t, chanend_t));
DECLARE_JOB(flash_demo, (qspi_flash_ctx_t *));

#endif /* APP_CONF_H_ */
