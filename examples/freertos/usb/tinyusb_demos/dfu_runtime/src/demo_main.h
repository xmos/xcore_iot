// Copyright 2021-2022 XMOS LIMITED. This Software is subject to the terms of the
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef DEMO_MAIN_H_
#define DEMO_MAIN_H_

#include "rtos_gpio.h"
#include "rtos_qspi_flash.h"

typedef struct demo_args {
  rtos_gpio_t *gpio_ctx;
  rtos_qspi_flash_t *qspi_ctx;
} demo_args_t;

void create_tinyusb_demo(demo_args_t *args, unsigned priority);

void set_dfu_mode(void);
void set_rt_mode(void);
int check_dfu_mode(void);

#endif /* DEMO_MAIN_H_ */
