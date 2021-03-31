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

typedef struct {
    rtos_osal_queue_t *queue;
    size_t payload_len;
    uint8_t *payload;
    control_resid_t resid;
    control_cmd_t cmd;
    uint8_t buf[];
} cmd_to_servicer_t;

void resource_table_init(device_control_t *ctx);

int resource_table_add(device_control_t *ctx,
                       const control_resid_t resources[],
                       size_t num_resources,
                       uint8_t servicer);

int resource_table_search(device_control_t *ctx,
                          control_resid_t resid,
                          uint8_t *servicer);

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

static control_ret_t write_command(device_control_t *ctx,
                                   uint8_t servicer,
                                   control_resid_t resid,
                                   control_cmd_t cmd,
                                   uint8_t payload[],
                                   unsigned payload_len)
{
    if (resid == CONTROL_SPECIAL_RESID) {
        rtos_printf("ignoring write to special resource %d\n", CONTROL_SPECIAL_RESID);
        return CONTROL_BAD_COMMAND;
    } else {

        rtos_intertile_t *intertile_ctx = ctx->servicer_table[servicer].intertile_ctx;
        rtos_osal_queue_t *queue = ctx->servicer_table[servicer].queue;

        cmd_to_servicer_t c = {
                .queue = queue,
                .resid = resid,
                .cmd = cmd,
                .payload_len = payload_len,
                .payload = payload,
                .buf = {}
        };
        cmd_to_servicer_t *c_ptr = &c;

        if (intertile_ctx == NULL) {
            /* Put to queue */
            rtos_osal_queue_send(queue, &c_ptr, RTOS_OSAL_WAIT_FOREVER);

            int *ptr;
            rtos_osal_queue_receive(&ctx->gateway_queue, &ptr, RTOS_OSAL_WAIT_FOREVER);
            rtos_printf("Got back %d\n", *ptr);
            rtos_osal_free(ptr);

        } else {
            /* Send to intertile */

            /*
             * TODO: If intertile_tx is split up to allow multiple calls, then this
             * malloc nonsense shouldn't be necessary.
             */

            c_ptr = rtos_osal_malloc(sizeof(cmd_to_servicer_t) + payload_len);

            memcpy(c_ptr, &c, sizeof(cmd_to_servicer_t));
            memcpy(c_ptr->buf, payload, payload_len);

            rtos_intertile_tx(intertile_ctx, ctx->intertile_port, c_ptr, sizeof(cmd_to_servicer_t) + payload_len);
            rtos_osal_free(c_ptr);

            int *ptr;
            if (rtos_intertile_rx(intertile_ctx, ctx->intertile_port, (void **) &ptr, 1) != 0) {
                rtos_printf("Got back %d\n", *ptr);
                rtos_osal_free(ptr);
            }
        }


        rtos_printf("%d write command %d, %d, %d\n", servicer, resid, cmd, payload_len);
        //control_ret_t ret = i[ifnum].write_command(resid, cmd, payload, payload_len);
        control_ret_t ret = CONTROL_SUCCESS;
        return ret;
    }
}

static control_ret_t read_command(device_control_t *ctx,
                                  uint8_t servicer,
                                  control_resid_t resid,
                                  control_cmd_t cmd,
                                  uint8_t payload[],
                                  unsigned payload_len)
{
    if (resid == CONTROL_SPECIAL_RESID) {
        return special_read_command(cmd, payload, payload_len);
    } else {
        rtos_printf("%d read command %d, %d, %d\n", servicer, resid, cmd, payload_len);
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
    uint8_t servicer;

    const size_t requested_payload_len = ctx->requested_payload_len;
    const control_resid_t requested_resid = ctx->requested_resid;
    const control_cmd_t requested_cmd = ctx->requested_cmd;

    if (resource_table_search(ctx, requested_resid, &servicer) != 0) {
        rtos_printf("resource %d not found\n", requested_resid);
        return CONTROL_BAD_COMMAND;
    }

    if (requested_payload_len > buf_size) {
        return CONTROL_DATA_LENGTH_ERROR;
    }

    if (direction == CONTROL_DEVICE_TO_HOST && IS_CONTROL_CMD_READ(requested_cmd)) {
        return read_command(ctx, servicer, requested_resid, requested_cmd, payload_buf, requested_payload_len);
    } else if (direction == CONTROL_HOST_TO_DEVICE && !IS_CONTROL_CMD_READ(requested_cmd)) {
        return write_command(ctx, servicer, requested_resid, requested_cmd, payload_buf, requested_payload_len);
    } else {
        if (buf_size > 0) {
            return CONTROL_BAD_COMMAND;
        } else {
            return CONTROL_SUCCESS;
        }
    }
}

static int servicer_register(device_control_t *ctx,
                              servicer_init_data_t *init_cmd,
                              rtos_intertile_t *intertile_ctx,
                              int servicer_index)
{
    int ret;
    ctx->servicer_table[servicer_index].queue = init_cmd->queue;
    ctx->servicer_table[servicer_index].intertile_ctx = intertile_ctx;
    ret = resource_table_add(ctx, init_cmd->resources, init_cmd->num_resources, servicer_index);
    rtos_osal_free(init_cmd);
    return ret;
}

control_ret_t device_control_resources_register(device_control_t *ctx,
                                                const size_t servicer_count,
                                                unsigned timeout)
{
    servicer_init_data_t *init_cmd;
    int registered_count = 0;
    int ret = 0;

    ctx->servicer_table = rtos_osal_malloc(servicer_count * sizeof(*ctx->servicer_table));

    //TODO tick count should be in osal
    TickType_t start_time = xTaskGetTickCount();
    while (registered_count < servicer_count && ret == 0 && xTaskGetTickCount() - start_time < timeout) {

        if (rtos_osal_queue_receive(&ctx->gateway_queue, &init_cmd, 1) == RTOS_OSAL_SUCCESS) {
            ret = servicer_register(ctx, init_cmd, NULL, registered_count);
            if (ret == 0) {
                registered_count++;
            }
        }

        for (int i = 0; i < ctx->intertile_count && registered_count < servicer_count && ret == 0; i++) {
            if (rtos_intertile_rx(ctx->client_intertile[i], ctx->intertile_port, (void **) &init_cmd, 1) != 0) {
                ret = servicer_register(ctx, init_cmd, ctx->client_intertile[i], registered_count);
                if (ret == 0) {
                    registered_count++;
                }
            }
        }
    }

    if (registered_count == servicer_count) {
        return CONTROL_SUCCESS;
    } else {
        return CONTROL_REGISTRATION_FAILED;
    }
}

control_ret_t device_control_servicer_cmd_recv(device_control_servicer_t *ctx,
//                                               control_resid_t *resid,
//                                               control_cmd_t *cmd,
//                                               size_t *payload_len,
//                                               uint8_t *payload,
                                               unsigned timeout)
{
    device_control_t *device_control_ctx = ctx->device_control_ctx;
    rtos_osal_status_t status;
    control_ret_t ret = CONTROL_ERROR;
    cmd_to_servicer_t *c_ptr;

    status = rtos_osal_queue_receive(&ctx->queue, &c_ptr, timeout);
    if (status == RTOS_OSAL_SUCCESS) {
        rtos_printf("Servicer on tile %d received command %02x for resid %d\n", THIS_XCORE_TILE, c_ptr->cmd, c_ptr->resid);
        rtos_printf("The command has %d bytes\n", c_ptr->payload_len);
        for (int i = 0; i < c_ptr->payload_len; i++) {
            rtos_printf("%d ", c_ptr->payload[i]);
        }
        rtos_printf("\n");

        if (device_control_ctx->resource_table != NULL) {
            int *m = rtos_osal_malloc(4);
            *m = 42;
            status = rtos_osal_queue_send(&device_control_ctx->gateway_queue, &m, 0); /* THIS SHOULD NOT EVER BLOCK */
            xassert(status == RTOS_OSAL_SUCCESS);
        } else {
            /* gateway is on another tile */

            rtos_osal_free(c_ptr);

            int m = 43;
            rtos_intertile_tx(device_control_ctx->host_intertile, device_control_ctx->intertile_port,
                              &m, sizeof(m));
        }
    }

    return status == RTOS_OSAL_SUCCESS ? CONTROL_SUCCESS : CONTROL_ERROR;
}

control_ret_t device_control_servicer_register(device_control_servicer_t *ctx,
                                               device_control_t *device_control_ctx,
                                               const control_resid_t resources[],
                                               size_t num_resources)
{
    const size_t len = sizeof(servicer_init_data_t) + sizeof(control_resid_t) * num_resources;
    servicer_init_data_t *init_data = rtos_osal_malloc(len);

    ctx->device_control_ctx = device_control_ctx;
    init_data->num_resources = num_resources;
    rtos_osal_queue_create(&ctx->queue, "servicer_q", 1, sizeof(void *));
    init_data->queue = &ctx->queue;
    memcpy(init_data->resources, resources, sizeof(control_resid_t) * num_resources);

    if (device_control_ctx->resource_table != NULL) {

        rtos_osal_queue_send(&device_control_ctx->gateway_queue, &init_data, RTOS_OSAL_WAIT_FOREVER);

    } else {
        /* Resource table is NULL on client tiles */
        /*
         * TODO: If update to intertile driver is made, this could be two sends,
         * no malloc() required.
         */
        rtos_intertile_tx(device_control_ctx->host_intertile, device_control_ctx->intertile_port, init_data, len);
        rtos_osal_free(init_data);
    }

    return CONTROL_SUCCESS;
}

static void device_control_client_thread(device_control_t *ctx)
{
    uint32_t msg_length;
    cmd_to_servicer_t *c_ptr;

    for (;;) {
        msg_length = rtos_intertile_rx(ctx->host_intertile,
                                       ctx->intertile_port,
                                       (void **) &c_ptr,
                                       RTOS_OSAL_WAIT_FOREVER);

        if (msg_length != 0) {
            c_ptr->payload = c_ptr->buf;
            rtos_osal_queue_send(c_ptr->queue, &c_ptr, RTOS_OSAL_WAIT_FOREVER);
        }
    }
}

/*
 * TODO: Consider making this just for "client" tiles and
 * moving the queue_create into init() for the "host" tile.
 *
 * OR just combine the two functions? Do we really need both
 * init and start. It doesn't need to be like the drivers.
 */
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
