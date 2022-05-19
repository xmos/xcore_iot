// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_DEMOS_H_
#define APP_DEMOS_H_

#include <xcore/parallel.h>
#include <xcore/chanend.h>

#include "uart.h"


DECLARE_JOB(uart_demo, (uart_tx_t*));

#endif /* APP_CONF_H_ */
