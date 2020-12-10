// Copyright (c) 2020, XMOS Ltd, All rights reserved

/*
 * Facilitates channel communication between tiles.
 * Essentially a thin wrapper around a streaming channel.
 *
 * Recommend limiting to one per tile pair. There should be at
 * least one more RTOS core usable by all tasks that use these
 * intertile links to handle the case where a transmit occurs
 * on both sides of all links at the same time. There must be
 * at least one core available to handle a receive or else
 * dead-lock may occur.
 */

#ifndef RTOS_INTERTILE_H_
#define RTOS_INTERTILE_H_

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

#include <xcore/channel.h>
#include <xcore/channel_transaction.h>

typedef struct {
    chanend_t c;

    SemaphoreHandle_t lock;
    EventGroupHandle_t event_group;
} rtos_intertile_t;

typedef struct {
    rtos_intertile_t *intertile_ctx;
    int port;
} rtos_intertile_address_t;

void rtos_intertile_tx(
        rtos_intertile_t *ctx,
        uint8_t port,
        void *msg,
        uint32_t len);

uint32_t rtos_intertile_rx(
        rtos_intertile_t *ctx,
        uint8_t port,
        void **msg,
        unsigned timeout);

void rtos_intertile_start(
        rtos_intertile_t *intertile_ctx);

void rtos_intertile_init(
        rtos_intertile_t *intertile_ctx,
        chanend_t c);

#endif /* RTOS_INTERTILE_H_ */
