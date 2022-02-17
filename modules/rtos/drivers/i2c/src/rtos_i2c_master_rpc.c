// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "rtos_rpc.h"
#include "rtos_i2c_master.h"

enum {
    fcode_write,
    fcode_read,
    fcode_stop_bit_send,
    fcode_reg_write,
    fcode_reg_read
};

__attribute__((fptrgroup("rtos_i2c_master_write_fptr_grp")))
static i2c_res_t i2c_master_remote_write(
        rtos_i2c_master_t *i2c_master_ctx,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        size_t *num_bytes_sent,
        int send_stop_bit)
{
    rtos_intertile_address_t *host_address = &i2c_master_ctx->rpc_config->host_address;
    rtos_i2c_master_t *host_ctx_ptr = i2c_master_ctx->rpc_config->host_ctx_ptr;
    i2c_res_t ret;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(i2c_master_ctx),
            RPC_PARAM_TYPE(device_addr),
            RPC_PARAM_IN_BUFFER(buf, n),
            RPC_PARAM_TYPE(n),
            RPC_PARAM_RETURN(size_t),
            RPC_PARAM_TYPE(send_stop_bit),
            RPC_PARAM_RETURN(i2c_res_t),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_write, rpc_param_desc,
            &host_ctx_ptr, &device_addr, buf, &n, num_bytes_sent, &send_stop_bit, &ret);

    return ret;
}

__attribute__((fptrgroup("rtos_i2c_master_read_fptr_grp")))
static i2c_res_t i2c_master_remote_read(
        rtos_i2c_master_t *i2c_master_ctx,
        uint8_t device_addr,
        uint8_t buf[],
        size_t n,
        int send_stop_bit)
{
    rtos_intertile_address_t *host_address = &i2c_master_ctx->rpc_config->host_address;
    rtos_i2c_master_t *host_ctx_ptr = i2c_master_ctx->rpc_config->host_ctx_ptr;
    i2c_res_t ret;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(i2c_master_ctx),
            RPC_PARAM_TYPE(device_addr),
            RPC_PARAM_OUT_BUFFER(buf, n),
            RPC_PARAM_TYPE(n),
            RPC_PARAM_TYPE(send_stop_bit),
            RPC_PARAM_RETURN(i2c_res_t),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_read, rpc_param_desc,
            &host_ctx_ptr, &device_addr, buf, &n, &send_stop_bit, &ret);

    return ret;
}

__attribute__((fptrgroup("rtos_i2c_master_stop_bit_send_fptr_grp")))
static void i2c_master_remote_stop_bit_send(
        rtos_i2c_master_t *i2c_master_ctx)
{
    rtos_intertile_address_t *host_address = &i2c_master_ctx->rpc_config->host_address;
    rtos_i2c_master_t *host_ctx_ptr = i2c_master_ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(i2c_master_ctx),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_stop_bit_send, rpc_param_desc,
            &host_ctx_ptr);
}

__attribute__((fptrgroup("rtos_i2c_master_reg_write_fptr_grp")))
static i2c_regop_res_t i2c_master_remote_reg_write(
        rtos_i2c_master_t *i2c_master_ctx,
        uint8_t device_addr,
        uint8_t reg_addr,
        uint8_t data)
{
    rtos_intertile_address_t *host_address = &i2c_master_ctx->rpc_config->host_address;
    rtos_i2c_master_t *host_ctx_ptr = i2c_master_ctx->rpc_config->host_ctx_ptr;
    i2c_regop_res_t ret;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(i2c_master_ctx),
            RPC_PARAM_TYPE(device_addr),
            RPC_PARAM_TYPE(reg_addr),
            RPC_PARAM_TYPE(data),
            RPC_PARAM_RETURN(i2c_regop_res_t),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_reg_write, rpc_param_desc,
            &host_ctx_ptr, &device_addr, &reg_addr, &data, &ret);

    return ret;
}

__attribute__((fptrgroup("rtos_i2c_master_reg_read_fptr_grp")))
static i2c_regop_res_t  i2c_master_remote_reg_read(
        rtos_i2c_master_t *i2c_master_ctx,
        uint8_t device_addr,
        uint8_t reg_addr,
        uint8_t *data)
{
    rtos_intertile_address_t *host_address = &i2c_master_ctx->rpc_config->host_address;
    rtos_i2c_master_t *host_ctx_ptr = i2c_master_ctx->rpc_config->host_ctx_ptr;
    i2c_regop_res_t ret;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(i2c_master_ctx),
            RPC_PARAM_TYPE(device_addr),
            RPC_PARAM_TYPE(reg_addr),
            RPC_PARAM_RETURN(uint8_t),
            RPC_PARAM_RETURN(i2c_regop_res_t),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_reg_read, rpc_param_desc,
            &host_ctx_ptr, &device_addr, &reg_addr, data, &ret);

    return ret;
}

static int i2c_master_write_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_i2c_master_t *i2c_master_ctx;
    uint8_t device_addr;
    uint8_t *buf;
    size_t n;
    size_t num_bytes_sent;
    int send_stop_bit;
    i2c_res_t ret;

    rpc_request_unmarshall(
            rpc_msg,
            &i2c_master_ctx, &device_addr, &buf, &n, &num_bytes_sent, &send_stop_bit, &ret);

    ret = rtos_i2c_master_write(i2c_master_ctx, device_addr, buf, n, &num_bytes_sent, send_stop_bit);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            i2c_master_ctx, device_addr, buf, n, num_bytes_sent, send_stop_bit, ret);

    return msg_length;
}

static int i2c_master_read_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_i2c_master_t *i2c_master_ctx;
    uint8_t device_addr;
    uint8_t *buf;
    size_t n;
    int send_stop_bit;
    i2c_res_t ret;

    rpc_request_unmarshall(
            rpc_msg,
            &i2c_master_ctx, &device_addr, &buf, &n, &send_stop_bit, &ret);

    buf = rtos_osal_malloc(n);

    ret = rtos_i2c_master_read(i2c_master_ctx, device_addr, buf, n, send_stop_bit);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            i2c_master_ctx, device_addr, buf, n, send_stop_bit, ret);

    rtos_osal_free(buf);

    return msg_length;
}

static int i2c_master_stop_bit_send_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_i2c_master_t *i2c_master_ctx;

    rpc_request_unmarshall(
            rpc_msg,
            &i2c_master_ctx);

    rtos_i2c_master_stop_bit_send(i2c_master_ctx);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            i2c_master_ctx);

    return msg_length;
}

static int i2c_master_reg_write_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_i2c_master_t *i2c_master_ctx;
    uint8_t device_addr;
    uint8_t reg_addr;
    uint8_t data;
    i2c_regop_res_t ret;

    rpc_request_unmarshall(
            rpc_msg,
            &i2c_master_ctx, &device_addr, &reg_addr, &data, &ret);

    ret = rtos_i2c_master_reg_write(i2c_master_ctx, device_addr, reg_addr, data);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            i2c_master_ctx, device_addr, reg_addr, data, ret);

    return msg_length;
}

static int i2c_master_reg_read_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_i2c_master_t *i2c_master_ctx;
    uint8_t device_addr;
    uint8_t reg_addr;
    uint8_t data;
    i2c_regop_res_t ret;

    rpc_request_unmarshall(
            rpc_msg,
            &i2c_master_ctx, &device_addr, &reg_addr, &data, &ret);

    ret = rtos_i2c_master_reg_read(i2c_master_ctx, device_addr, reg_addr, &data);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            i2c_master_ctx, device_addr, reg_addr, data, ret);

    return msg_length;
}

static void i2c_master_rpc_thread(rtos_intertile_address_t *client_address)
{
    int msg_length;
    uint8_t *req_msg;
    uint8_t *resp_msg;
    rpc_msg_t rpc_msg;
    rtos_intertile_t *intertile_ctx = client_address->intertile_ctx;
    uint8_t intertile_port = client_address->port;

    for (;;) {
        /* receive RPC request message from client */
        msg_length = rtos_intertile_rx(intertile_ctx, intertile_port, (void **) &req_msg, RTOS_OSAL_WAIT_FOREVER);

        rpc_request_parse(&rpc_msg, req_msg);

        switch (rpc_msg.fcode) {
        case fcode_write:
            msg_length = i2c_master_write_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_read:
            msg_length = i2c_master_read_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_stop_bit_send:
            msg_length = i2c_master_stop_bit_send_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_reg_write:
            msg_length = i2c_master_reg_write_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_reg_read:
            msg_length = i2c_master_reg_read_rpc_host(&rpc_msg, &resp_msg);
            break;
        }

        rtos_osal_free(req_msg);

        /* send RPC response message to client */
        rtos_intertile_tx(intertile_ctx, intertile_port, resp_msg, msg_length);
        rtos_osal_free(resp_msg);
    }
}

__attribute__((fptrgroup("rtos_driver_rpc_host_start_fptr_grp")))
static void i2c_master_rpc_start(
        rtos_driver_rpc_t *rpc_config)
{
    xassert(rpc_config->host_task_priority >= 0);

    for (int i = 0; i < rpc_config->remote_client_count; i++) {

        rtos_intertile_address_t *client_address = &rpc_config->client_address[i];

        xassert(client_address->port >= 0);

        rtos_osal_thread_create(
                NULL,
                "i2c_master_rpc_thread",
                (rtos_osal_entry_function_t) i2c_master_rpc_thread,
                client_address,
                RTOS_THREAD_STACK_SIZE(i2c_master_rpc_thread),
                rpc_config->host_task_priority);
    }
}

void rtos_i2c_master_rpc_config(
        rtos_i2c_master_t *i2c_master_ctx,
        unsigned intertile_port,
        unsigned host_task_priority)
{
    rtos_driver_rpc_t *rpc_config = i2c_master_ctx->rpc_config;

    if (rpc_config->remote_client_count == 0) {
        /* This is a client */
        rpc_config->host_address.port = intertile_port;
    } else {
        for (int i = 0; i < rpc_config->remote_client_count; i++) {
            rpc_config->client_address[i].port = intertile_port;
        }
        rpc_config->host_task_priority = host_task_priority;
    }
}

void rtos_i2c_master_rpc_client_init(
        rtos_i2c_master_t *i2c_master_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *host_intertile_ctx)
{
    i2c_master_ctx->rpc_config = rpc_config;
    i2c_master_ctx->write = i2c_master_remote_write;
    i2c_master_ctx->read = i2c_master_remote_read;
    i2c_master_ctx->stop_bit_send = i2c_master_remote_stop_bit_send;
    i2c_master_ctx->reg_write = i2c_master_remote_reg_write;
    i2c_master_ctx->reg_read = i2c_master_remote_reg_read;
    rpc_config->rpc_host_start = NULL;
    rpc_config->remote_client_count = 0;
    rpc_config->host_task_priority = -1;

    /* This must be configured later with rtos_i2c_master_rpc_config() */
    rpc_config->host_address.port = -1;

    rpc_config->host_address.intertile_ctx = host_intertile_ctx;
    rpc_config->host_ctx_ptr = (void *) s_chan_in_word(host_intertile_ctx->c);
}

void rtos_i2c_master_rpc_host_init(
        rtos_i2c_master_t *i2c_master_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *client_intertile_ctx[],
        size_t remote_client_count)
{
    i2c_master_ctx->rpc_config = rpc_config;
    rpc_config->rpc_host_start = i2c_master_rpc_start;
    rpc_config->remote_client_count = remote_client_count;

    /* This must be configured later with rtos_i2c_master_rpc_config() */
    rpc_config->host_task_priority = -1;

    for (int i = 0; i < remote_client_count; i++) {
        rpc_config->client_address[i].intertile_ctx = client_intertile_ctx[i];
        s_chan_out_word(client_intertile_ctx[i]->c, (uint32_t) i2c_master_ctx);

        /* This must be configured later with rtos_i2c_master_rpc_config() */
        rpc_config->client_address[i].port = -1;
    }
}
