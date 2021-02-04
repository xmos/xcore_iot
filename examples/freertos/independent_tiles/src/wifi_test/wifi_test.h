// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#ifndef WIFI_TEST_H_
#define WIFI_TEST_H_

#include "rtos/drivers/spi/api/rtos_spi_master.h"
#include "rtos/drivers/gpio/api/rtos_gpio.h"

void wifi_test_start(rtos_spi_master_device_t *wifi_device_ctx, rtos_gpio_t *gpio_ctx);

#endif /* WIFI_TEST_H_ */
