// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1

#include <string.h>
#include <xcore/triggerable.h>
#include <xcore/assert.h>

#include "FreeRTOS.h"

#include "example_pipeline/example_pipeline.h"
#include "rtos/drivers/rpc/api/rtos_rpc.h"

static rtos_intertile_address_t adr;
static rtos_intertile_address_t* intertile_addr = &adr;

static void remote_cli_gain_thread(void *arg)
{
    (void) arg;
    int msg_length;
    uint8_t *msg;
    rtos_intertile_t *intertile_ctx = intertile_addr->intertile_ctx;
    uint8_t intertile_port = intertile_addr->port;
    uint32_t retval = 0;

    for (;;) {
        msg_length = rtos_intertile_rx(intertile_ctx, intertile_port, (void **) &msg, portMAX_DELAY);

        configASSERT(msg_length == sizeof(int32_t));

        switch(*(int32_t*)msg) {
            case GET_GAIN_VAL:
                retval = audiopipeline_get_stage1_gain();
                rtos_intertile_tx(intertile_ctx, intertile_port, &retval, sizeof(int32_t));
                break;
            case SET_GAIN_VAL:
                vPortFree(msg);
                msg_length = rtos_intertile_rx(intertile_ctx, intertile_port, (void **) &msg, portMAX_DELAY);

                configASSERT(msg_length == sizeof(int32_t));

                audiopipeline_set_stage1_gain(*(int32_t*)msg);
                break;
            default:
                break;
        }
        vPortFree(msg);
    }
}

void remote_cli_gain_init(
    rtos_intertile_t *host_intertile_ctx,
    unsigned intertile_port,
    unsigned host_task_priority)
{
    xassert(intertile_port >= 0);

    intertile_addr->intertile_ctx = host_intertile_ctx;
    intertile_addr->port = intertile_port;

    xTaskCreate((TaskFunction_t) remote_cli_gain_thread,
                "remote_cli_gain_thread",
                RTOS_THREAD_STACK_SIZE(remote_cli_gain_thread),
                NULL,
                host_task_priority,
                NULL);
}
