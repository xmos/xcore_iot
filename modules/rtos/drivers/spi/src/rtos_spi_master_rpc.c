// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "rtos_rpc.h"
#include "rtos_spi_master.h"

enum {
    fcode_transaction_start,
    fcode_transfer,
    fcode_delay_before_next_transfer,
    fcode_transaction_end,
};

__attribute__((fptrgroup("rtos_spi_master_transaction_start_fptr_grp")))
static void spi_master_remote_transaction_start(
        rtos_spi_master_device_t *dev_ctx)
{
    rtos_spi_master_t *bus_ctx = dev_ctx->bus_ctx;
    rtos_intertile_address_t *host_address = &bus_ctx->rpc_config->host_address;
    rtos_spi_master_device_t *host_dev_ctx_ptr = dev_ctx->host_dev_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(dev_ctx),
            RPC_PARAM_LIST_END
    };

    rtos_osal_mutex_get(&bus_ctx->lock, RTOS_OSAL_WAIT_FOREVER);

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_transaction_start, rpc_param_desc,
            &host_dev_ctx_ptr);
}

__attribute__((fptrgroup("rtos_spi_master_transfer_fptr_grp")))
static void spi_master_remote_transfer(
        rtos_spi_master_device_t *dev_ctx,
        uint8_t *data_out,
        uint8_t *data_in,
        size_t len)
{
    rtos_spi_master_t *bus_ctx = dev_ctx->bus_ctx;
    rtos_intertile_address_t *host_address = &bus_ctx->rpc_config->host_address;
    rtos_spi_master_device_t *host_dev_ctx_ptr = dev_ctx->host_dev_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(dev_ctx),
            RPC_PARAM_IN_BUFFER(data_out, data_out != NULL ? len : 0),
            RPC_PARAM_OUT_BUFFER(data_in, data_in != NULL ? len : 0),
            RPC_PARAM_TYPE(len),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_transfer, rpc_param_desc,
            &host_dev_ctx_ptr, data_out, data_in, &len);
}

__attribute__((fptrgroup("rtos_spi_master_delay_before_next_transfer_fptr_grp")))
static void spi_master_remote_delay_before_next_transfer(
        rtos_spi_master_device_t *dev_ctx,
        uint32_t delay_ticks)
{
    rtos_spi_master_t *bus_ctx = dev_ctx->bus_ctx;
    rtos_intertile_address_t *host_address = &bus_ctx->rpc_config->host_address;
    rtos_spi_master_device_t *host_dev_ctx_ptr = dev_ctx->host_dev_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(dev_ctx),
            RPC_PARAM_TYPE(delay_ticks),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_delay_before_next_transfer, rpc_param_desc,
            &host_dev_ctx_ptr, &delay_ticks);
}

__attribute__((fptrgroup("rtos_spi_master_transaction_end_fptr_grp")))
static void spi_master_remote_transaction_end(
        rtos_spi_master_device_t *dev_ctx)
{
    rtos_spi_master_t *bus_ctx = dev_ctx->bus_ctx;
    rtos_intertile_address_t *host_address = &bus_ctx->rpc_config->host_address;
    rtos_spi_master_device_t *host_dev_ctx_ptr = dev_ctx->host_dev_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(dev_ctx),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_transaction_end, rpc_param_desc,
            &host_dev_ctx_ptr);

    rtos_osal_mutex_put(&bus_ctx->lock);
}

static int spi_transaction_start_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_spi_master_device_t *dev_ctx;

    rpc_request_unmarshall(
            rpc_msg,
            &dev_ctx);

    rtos_spi_master_transaction_start(dev_ctx);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            dev_ctx);

    return msg_length;
}

static int spi_transfer_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_spi_master_device_t *dev_ctx;
    uint8_t *data_out;
    uint8_t *data_in;
    size_t len;

    rpc_request_unmarshall(
            rpc_msg,
            &dev_ctx, &data_out, &data_in, &len);

    if (data_in != NULL) {
        data_in = rtos_osal_malloc(len);
    }

    rtos_spi_master_transfer(dev_ctx, data_out, data_in, len);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            dev_ctx, data_out, data_in, len);

    rtos_osal_free(data_in);

    return msg_length;
}

static int spi_delay_before_next_transfer_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_spi_master_device_t *dev_ctx;
    uint32_t delay_ticks;

    rpc_request_unmarshall(
            rpc_msg,
            &dev_ctx, &delay_ticks);

    rtos_spi_master_delay_before_next_transfer(dev_ctx, delay_ticks);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            dev_ctx, delay_ticks);

    return msg_length;
}

static int spi_transaction_end_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_spi_master_device_t *dev_ctx;

    rpc_request_unmarshall(
            rpc_msg,
            &dev_ctx);

    rtos_spi_master_transaction_end(dev_ctx);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            dev_ctx);

    return msg_length;
}

static void spi_master_rpc_thread(rtos_intertile_address_t *client_address)
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
        case fcode_transaction_start:
            msg_length = spi_transaction_start_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_transfer:
            msg_length = spi_transfer_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_delay_before_next_transfer:
            msg_length = spi_delay_before_next_transfer_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_transaction_end:
            msg_length = spi_transaction_end_rpc_host(&rpc_msg, &resp_msg);
            break;
        }

        rtos_osal_free(req_msg);

        /* send RPC response message to client */
        rtos_intertile_tx(intertile_ctx, intertile_port, resp_msg, msg_length);
        rtos_osal_free(resp_msg);
    }
}

__attribute__((fptrgroup("rtos_driver_rpc_host_start_fptr_grp")))
static void spi_master_rpc_start(
        rtos_driver_rpc_t *rpc_config)
{
    xassert(rpc_config->host_task_priority >= 0);

    for (int i = 0; i < rpc_config->remote_client_count; i++) {

        rtos_intertile_address_t *client_address = &rpc_config->client_address[i];

        xassert(client_address->port >= 0);

        rtos_osal_thread_create(
                NULL,
                "spi_master_rpc_thread",
                (rtos_osal_entry_function_t) spi_master_rpc_thread,
                client_address,
                RTOS_THREAD_STACK_SIZE(spi_master_rpc_thread),
                rpc_config->host_task_priority);
    }
}

void rtos_spi_master_rpc_config(
        rtos_spi_master_t *spi_master_ctx,
        unsigned intertile_port,
        unsigned host_task_priority)
{
    rtos_driver_rpc_t *rpc_config = spi_master_ctx->rpc_config;

    if (rpc_config->remote_client_count == 0) {
        /* This is a client */
        rpc_config->host_address.port = intertile_port;

        rtos_osal_mutex_create(&spi_master_ctx->lock, "spi_master_lock", RTOS_OSAL_NOT_RECURSIVE);

    } else {
        for (int i = 0; i < rpc_config->remote_client_count; i++) {
            rpc_config->client_address[i].port = intertile_port;
        }
        rpc_config->host_task_priority = host_task_priority;
    }
}

void rtos_spi_master_rpc_client_init(
        rtos_spi_master_t *spi_master_ctx,
        rtos_spi_master_device_t *spi_device_ctx[],
        size_t spi_device_count,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *host_intertile_ctx)
{
    spi_master_ctx->rpc_config = rpc_config;
    spi_master_ctx->transaction_start = spi_master_remote_transaction_start;
    spi_master_ctx->transfer = spi_master_remote_transfer;
    spi_master_ctx->delay_before_next_transfer = spi_master_remote_delay_before_next_transfer;
    spi_master_ctx->transaction_end = spi_master_remote_transaction_end;
    rpc_config->rpc_host_start = NULL;
    rpc_config->remote_client_count = 0;
    rpc_config->host_task_priority = -1;

    /* This must be configured later with rtos_spi_master_rpc_config() */
    rpc_config->host_address.port = -1;

    rpc_config->host_address.intertile_ctx = host_intertile_ctx;
    rpc_config->host_ctx_ptr = (void *) s_chan_in_word(host_intertile_ctx->c);

    for (int i = 0; i < spi_device_count; i++) {
        spi_device_ctx[i]->bus_ctx = spi_master_ctx;
        spi_device_ctx[i]->host_dev_ctx_ptr = (void *) s_chan_in_word(host_intertile_ctx->c);
    }
}

void rtos_spi_master_rpc_host_init(
        rtos_spi_master_t *spi_master_ctx,
        rtos_spi_master_device_t *spi_device_ctx[],
        size_t spi_device_count,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *client_intertile_ctx[],
        size_t remote_client_count)
{
    spi_master_ctx->rpc_config = rpc_config;
    rpc_config->rpc_host_start = spi_master_rpc_start;
    rpc_config->remote_client_count = remote_client_count;

    /* This must be configured later with rtos_spi_master_rpc_config() */
    rpc_config->host_task_priority = -1;

    for (int i = 0; i < remote_client_count; i++) {
        rpc_config->client_address[i].intertile_ctx = client_intertile_ctx[i];
        s_chan_out_word(client_intertile_ctx[i]->c, (uint32_t) spi_master_ctx);

        for (int j = 0; j < spi_device_count; j++) {
            s_chan_out_word(client_intertile_ctx[i]->c, (uint32_t) spi_device_ctx[i]);
        }

        /* This must be configured later with rtos_spi_master_rpc_config() */
        rpc_config->client_address[i].port = -1;
    }
}
