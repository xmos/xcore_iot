// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef CIFAR10_TASK_H_
#define CIFAR10_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "rtos_intertile.h"

void cifar10_app_task_create(
        rtos_intertile_address_t *intertile_addr,
        unsigned priority);

void cifar10_image_classifier_task_create(
        rtos_intertile_address_t *intertile_addr,
        unsigned priority);

#ifdef __cplusplus
};
#endif

#endif /* CIFAR10_TASK_H_ */
