// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#ifndef CIFAR10_TASK_H_
#define CIFAR10_TASK_H_

#include "rtos/drivers/intertile/api/rtos_intertile.h"

void cifar10_app_task_create(
        rtos_intertile_address_t *intertile_addr,
        unsigned priority);

void cifar10_model_runner_task_create(
        rtos_intertile_address_t *intertile_addr,
        unsigned priority);

#endif /* CIFAR10_TASK_H_ */
