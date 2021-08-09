// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <string.h>
#include <xcore/assert.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"

/* Library headers */

/* App headers */
#include "app_conf.h"
#include "example_pipeline/example_pipeline.h"
#include "platform/driver_instances.h"

static void remote_cli_gain_thread(void *arg)
{
    (void) arg;
    int msg_length;
    uint8_t *msg;
    uint32_t retval = 0;

    for (;;) {
        msg_length = rtos_intertile_rx(intertile_ctx, appconfCLI_RPC_PROCESS_COMMAND_PORT, (void **) &msg, portMAX_DELAY);

        configASSERT(msg_length == sizeof(int32_t));

        switch(*(int32_t*)msg) {
            case GET_GAIN_VAL:
                retval = audiopipeline_get_stage1_gain();
                rtos_intertile_tx(intertile_ctx, appconfCLI_RPC_PROCESS_COMMAND_PORT, &retval, sizeof(int32_t));
                break;
            case SET_GAIN_VAL:
                vPortFree(msg);
                msg_length = rtos_intertile_rx(intertile_ctx, appconfCLI_RPC_PROCESS_COMMAND_PORT, (void **) &msg, portMAX_DELAY);

                configASSERT(msg_length == sizeof(int32_t));

                audiopipeline_set_stage1_gain(*(int32_t*)msg);
                break;
            default:
                break;
        }
        vPortFree(msg);
    }
}

void remote_cli_gain_init( UBaseType_t priority )
{
    xTaskCreate((TaskFunction_t) remote_cli_gain_thread,
                "remote_cli_gain_thread",
                RTOS_THREAD_STACK_SIZE(remote_cli_gain_thread),
                NULL,
                priority,
                NULL);
}
