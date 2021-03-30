// Copyright 2016 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

//#define DEBUG_UNIT CONTROL
#include <rtos_printf.h>
#include <string.h>

#include "rtos/drivers/osal/api/rtos_osal.h"

#include "device_control.h"

typedef struct {
    int cmd;
    void *rsvd;
    uint8_t *data;
} req_from_servicer_t;

typedef struct {
    rtos_osal_queue_t *queue;
    size_t num_resources;
    control_resid_t resources[];
} servicer_init_data_t;

/*
 *
 */
typedef struct {
    rtos_osal_queue_t *queue; /* servicer's queue */
    control_resid_t resid;
    control_cmd_t cmd;
    size_t payload_len;
    uint8_t payload[];
} cmd_to_servicer_t;

void resource_table_init(device_control_t *ctx);

int resource_table_add(device_control_t *ctx,
                       const control_resid_t resources[],
                       size_t num_resources,
                       int servicer);

int resource_table_search(device_control_t *ctx,
                          control_resid_t resid,
                          uint8_t *ifnum);

static control_ret_t special_read_command(control_cmd_t cmd,
                                          uint8_t payload[],
                                          unsigned payload_len)
{
    if (cmd == CONTROL_GET_VERSION) {
        rtos_printf("read version\n");
        if (payload_len != sizeof(control_version_t)) {
            rtos_printf("wrong payload size %d for read version command, need %d\n",
                    payload_len, sizeof(control_version_t));

            return CONTROL_BAD_COMMAND;
        } else {
            *((control_version_t*) payload) = CONTROL_VERSION;
            return CONTROL_SUCCESS;
        }
    } else {
        rtos_printf("unrecognised special resource command %d\n", cmd);
        return CONTROL_BAD_COMMAND;
    }
}

static control_ret_t write_command(int ifnum,
                                   control_resid_t resid,
                                   control_cmd_t cmd,
                                   const uint8_t payload[],
                                   unsigned payload_len)
{
    if (resid == CONTROL_SPECIAL_RESID) {
        rtos_printf("ignoring write to special resource %d\n", CONTROL_SPECIAL_RESID);
        return CONTROL_BAD_COMMAND;
    } else {
        rtos_printf("%d write command %d, %d, %d\n", ifnum, resid, cmd, payload_len);
        //control_ret_t ret = i[ifnum].write_command(resid, cmd, payload, payload_len);
        control_ret_t ret = CONTROL_SUCCESS;
        return ret;
    }
}

static control_ret_t read_command(unsigned char ifnum,
                                  control_resid_t resid,
                                  control_cmd_t cmd,
                                  uint8_t payload[],
                                  unsigned payload_len)
{
    if (resid == CONTROL_SPECIAL_RESID) {
        return special_read_command(cmd, payload, payload_len);
    } else {
        rtos_printf("%d read command %d, %d, %d\n", ifnum, resid, cmd, payload_len);
        //control_ret_t ret = i[ifnum].read_command(resid, cmd, payload, payload_len);
        // Just read from the QUEUE here
        control_ret_t ret = CONTROL_SUCCESS;
        return ret;
    }
}



void device_control_request(device_control_t *ctx,
                            control_resid_t resid,
                            control_cmd_t cmd,
                            size_t payload_len)
{
    ctx->requested_resid = resid;
    ctx->requested_cmd = cmd;
    ctx->requested_payload_len = payload_len;
}

control_ret_t device_control_payload_transfer(device_control_t *ctx,
                                              uint8_t *payload_buf,
                                              size_t buf_size,
                                              control_direction_t direction)
{
    uint8_t ifnum;

    const size_t requested_payload_len = ctx->requested_payload_len;
    const control_resid_t requested_resid = ctx->requested_resid;
    const control_cmd_t requested_cmd = ctx->requested_cmd;

    if (resource_table_search(ctx, requested_resid, &ifnum) != 0) {
        rtos_printf("resource %d not found\n", requested_resid);
        return CONTROL_BAD_COMMAND;
    }

    if (requested_payload_len > buf_size) {
        return CONTROL_DATA_LENGTH_ERROR;
    }

    if (direction == CONTROL_DEVICE_TO_HOST && IS_CONTROL_CMD_READ(requested_cmd)) {
        return read_command(ifnum, requested_resid, requested_cmd, payload_buf, requested_payload_len);
    } else if (direction == CONTROL_HOST_TO_DEVICE && !IS_CONTROL_CMD_READ(requested_cmd)) {
        return write_command(ifnum, requested_resid, requested_cmd, payload_buf, requested_payload_len);
    } else {
        if (buf_size > 0) {
            return CONTROL_BAD_COMMAND;
        } else {
            return CONTROL_SUCCESS;
        }
    }
}

control_ret_t device_control_resources_register(device_control_t *ctx,
                                                const size_t servicer_count)
{
    servicer_init_data_t *init_cmd;
    int registered_count = 0;

    ctx->servicer_table = rtos_osal_malloc(servicer_count * sizeof(device_control_servicer_t));

    TickType_t start_time = xTaskGetTickCount();

    //TODO PARAMETERIZE TIMEOUT
    //TODO tick count should be in osal
    while (registered_count < servicer_count && xTaskGetTickCount() - start_time < 100) {
        rtos_osal_status_t ret;

        ret = rtos_osal_queue_receive(&ctx->gateway_queue, &init_cmd, 0);

        if (ret == RTOS_OSAL_SUCCESS) {
            ctx->servicer_table[registered_count].queue = init_cmd->queue;
            ctx->servicer_table[registered_count].intertile_ctx = NULL;
            resource_table_add(ctx, init_cmd->resources, init_cmd->num_resources, registered_count);
            rtos_osal_free(init_cmd);
            registered_count++;
        }

        for (int i = 0; i < ctx->intertile_count && registered_count < servicer_count; i++) {
            uint32_t len;

            len = rtos_intertile_rx(ctx->client_intertile[i], ctx->intertile_port, (void **) &init_cmd, 0);

            if (len != 0) {
                ctx->servicer_table[registered_count].queue = init_cmd->queue;
                ctx->servicer_table[registered_count].intertile_ctx = ctx->client_intertile[i];
                resource_table_add(ctx, init_cmd->resources, init_cmd->num_resources, registered_count);
                rtos_osal_free(init_cmd);
                registered_count++;
            }
        }
    }

    if (registered_count == servicer_count) {
        return CONTROL_SUCCESS;
    } else {
        return CONTROL_REGISTRATION_FAILED;
    }
}

/**
 * TODO: Create the queue? Initialize a "servicer" instance?
 */
control_ret_t device_control_servicer_register(device_control_t *ctx,
                                               rtos_osal_queue_t *queue,
                                               const control_resid_t resources[],
                                               size_t num_resources)
{
    const size_t len = sizeof(servicer_init_data_t) + sizeof(control_resid_t) * num_resources;
    servicer_init_data_t *init_data = rtos_osal_malloc(len);

    init_data->num_resources = num_resources;
    init_data->queue = queue;
    memcpy(init_data->resources, resources, sizeof(control_resid_t) * num_resources);

    if (ctx->resource_table != NULL) {

        rtos_osal_queue_send(&ctx->gateway_queue, &init_data, RTOS_OSAL_WAIT_FOREVER);

    } else {
        /* Resource table is NULL on client tiles */
        /*
         * TODO: If update to intertile driver is made, this could be two sends,
         * no malloc() required.
         */
        rtos_intertile_tx(ctx->host_intertile, ctx->intertile_port, init_data, len);
        rtos_osal_free(init_data);
    }

    return CONTROL_SUCCESS;
}

static void device_control_client_thread(device_control_t *ctx)
{
    uint32_t msg_length;
    uint8_t *req_msg;

    for (;;) {
        msg_length = rtos_intertile_rx(ctx->host_intertile,
                                       ctx->intertile_port,
                                       (void **) &req_msg,
                                       RTOS_OSAL_WAIT_FOREVER);


    }


}

control_ret_t device_control_start(device_control_t *ctx,
                                   uint8_t intertile_port,
                                   unsigned priority)
{
    control_ret_t ret;

    ctx->intertile_port = intertile_port;

    if (ctx->resource_table == NULL) {
        /* Resource table is NULL on client tiles */
        rtos_osal_status_t status;

        status = rtos_osal_thread_create(
                NULL,
                "dc_client",
                (rtos_osal_entry_function_t) device_control_client_thread,
                ctx,
                RTOS_THREAD_STACK_SIZE(device_control_client_thread),
                priority);

        if (status == RTOS_OSAL_SUCCESS) {
            ret = CONTROL_SUCCESS;
        } else {
            ret = CONTROL_REGISTRATION_FAILED;
        }

    } else {
        rtos_osal_queue_create(&ctx->gateway_queue, "dc_gw_q", 1, sizeof(void *));
        ret = CONTROL_SUCCESS;
    }

    return ret;
}

control_ret_t device_control_init(device_control_t *ctx,
                                  int mode,
                                  rtos_intertile_t *intertile_ctx[],
                                  size_t intertile_count)
{
    memset(ctx, 0, sizeof(device_control_t));

    if (mode == DEVICE_CONTROL_HOST_MODE) {
        resource_table_init(ctx);
    }

    if (mode == DEVICE_CONTROL_CLIENT_MODE) {
        xassert(intertile_count == 1);
        if (intertile_count != 1) {
            return CONTROL_REGISTRATION_FAILED;
        }

        ctx->host_intertile = intertile_ctx[0];
    } else {
        xassert(intertile_count <= 3);
        if (intertile_count > 3) {
            return CONTROL_REGISTRATION_FAILED;
        }
        ctx->intertile_count = intertile_count;
        for (int i = 0; i < intertile_count; i++) {
            ctx->client_intertile[i] = intertile_ctx[i];
        }
    }

    return CONTROL_SUCCESS;
}
