// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef UART_DEMO_H_
#define UART_DEMO_H_

#include "FreeRTOS.h"

void uart_demo_create(UBaseType_t priority);
void uart_rx_pre_os_startup_init(void);

#endif //UART_DEMO_H_
