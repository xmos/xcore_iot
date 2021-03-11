// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef PERSON_DETECT_TASK_H_
#define PERSON_DETECT_TASK_H_

#include "rtos/drivers/intertile/api/rtos_intertile.h"
#include "rtos/drivers/gpio/api/rtos_gpio.h"

void person_detect_app_task_create(
        rtos_intertile_address_t *intertile_addr,
        rtos_gpio_t *gpio_ctx,
        unsigned priority,
        QueueHandle_t input_queue);

void person_detect_model_runner_task_create(
        rtos_intertile_address_t *intertile_addr,
        unsigned priority);

#endif /* PERSON_DETECT_TASK_H_ */
