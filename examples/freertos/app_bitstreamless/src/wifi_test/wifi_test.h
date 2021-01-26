// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef WIFI_TEST_H_
#define WIFI_TEST_H_

#include "drivers/rtos/spi/api/rtos_spi_master.h"
#include "drivers/rtos/gpio/api/rtos_gpio.h"

void wifi_test_start(rtos_spi_master_device_t *wifi_device_ctx, rtos_gpio_t *gpio_ctx);

#endif /* WIFI_TEST_H_ */
