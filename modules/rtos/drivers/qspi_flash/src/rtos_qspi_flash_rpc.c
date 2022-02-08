// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <string.h>
#include <xcore/assert.h>

#include "rtos_rpc.h"
#include "rtos_qspi_flash.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))

enum {
    fcode_lock,
    fcode_unlock,
    fcode_read,
    fcode_write,
    fcode_erase,
};

__attribute__((fptrgroup("rtos_qspi_flash_lock_fptr_grp")))
static void qspi_flash_remote_lock(
        rtos_qspi_flash_t *ctx)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_qspi_flash_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_LIST_END
    };

    rtos_osal_mutex_get(&ctx->mutex, RTOS_OSAL_WAIT_FOREVER);

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_lock, rpc_param_desc,
            &host_ctx_ptr);
}

__attribute__((fptrgroup("rtos_qspi_flash_unlock_fptr_grp")))
static void qspi_flash_remote_unlock(
        rtos_qspi_flash_t *ctx)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_qspi_flash_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_unlock, rpc_param_desc,
            &host_ctx_ptr);

    rtos_osal_mutex_put(&ctx->mutex);
}

__attribute__((fptrgroup("rtos_qspi_flash_read_fptr_grp")))
static void qspi_flash_remote_read(
        rtos_qspi_flash_t *ctx,
        uint8_t *data,
        unsigned address,
        size_t len)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_qspi_flash_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    rtos_osal_mutex_get(&ctx->mutex, RTOS_OSAL_WAIT_FOREVER);

    do {
        size_t read_len = MIN(len, RTOS_QSPI_FLASH_READ_CHUNK_SIZE);

        const rpc_param_desc_t rpc_param_desc[] = {
                RPC_PARAM_TYPE(ctx),
                RPC_PARAM_OUT_BUFFER(data, read_len),
                RPC_PARAM_TYPE(address),
                RPC_PARAM_TYPE(read_len),
                RPC_PARAM_LIST_END
        };

        rpc_client_call_generic(
                host_address->intertile_ctx, host_address->port, fcode_read, rpc_param_desc,
                &host_ctx_ptr, data, &address, &read_len);

        len -= read_len;
        data += read_len;
        address += read_len;
    } while (len > 0);

    rtos_osal_mutex_put(&ctx->mutex);
}

__attribute__((fptrgroup("rtos_qspi_flash_write_fptr_grp")))
static void qspi_flash_remote_write(
        rtos_qspi_flash_t *ctx,
        const uint8_t *data,
        unsigned address,
        size_t len)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_qspi_flash_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_IN_BUFFER(data, len),
            RPC_PARAM_TYPE(address),
            RPC_PARAM_TYPE(len),
            RPC_PARAM_LIST_END
    };

    rtos_osal_mutex_get(&ctx->mutex, RTOS_OSAL_WAIT_FOREVER);

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_write, rpc_param_desc,
            &host_ctx_ptr, data, &address, &len);

    rtos_osal_mutex_put(&ctx->mutex);
}

__attribute__((fptrgroup("rtos_qspi_flash_erase_fptr_grp")))
static void qspi_flash_remote_erase(
        rtos_qspi_flash_t *ctx,
        unsigned address,
        size_t len)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_qspi_flash_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_TYPE(address),
            RPC_PARAM_TYPE(len),
            RPC_PARAM_LIST_END
    };

    rtos_osal_mutex_get(&ctx->mutex, RTOS_OSAL_WAIT_FOREVER);

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_erase, rpc_param_desc,
            &host_ctx_ptr, &address, &len);

    rtos_osal_mutex_put(&ctx->mutex);
}

static int qspi_flash_lock_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_qspi_flash_t *ctx;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx);

    rtos_qspi_flash_lock(ctx);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx);

    return msg_length;
}

static int qspi_flash_unlock_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_qspi_flash_t *ctx;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx);

    rtos_qspi_flash_unlock(ctx);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx);

    return msg_length;
}

static int qspi_flash_read_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_qspi_flash_t *ctx;
    uint8_t *data;
    unsigned address;
    size_t len;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx, &data, &address, &len);

    if (len > 0) {
        data = rtos_osal_malloc(len);
    } else {
        data = NULL;
    }

    rtos_qspi_flash_read(ctx, data, address, len);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, data, address, len);

    if (len > 0) {
        rtos_osal_free(data);
    }

    return msg_length;
}

static int qspi_flash_write_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_qspi_flash_t *ctx;
    uint8_t *data;
    unsigned address;
    size_t len;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx, &data, &address, &len);

    rtos_qspi_flash_write(ctx, data, address, len);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, data, address, len);

    return msg_length;
}

static int qspi_flash_erase_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_qspi_flash_t *ctx;
    unsigned address;
    size_t len;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx, &address, &len);

    rtos_qspi_flash_erase(ctx, address, len);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, address, len);

    return msg_length;
}

static void qspi_flash_rpc_thread(rtos_intertile_address_t *client_address)
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
        case fcode_lock:
            msg_length = qspi_flash_lock_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_unlock:
            msg_length = qspi_flash_unlock_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_read:
            msg_length = qspi_flash_read_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_write:
            msg_length = qspi_flash_write_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_erase:
            msg_length = qspi_flash_erase_rpc_host(&rpc_msg, &resp_msg);
            break;
        }

        rtos_osal_free(req_msg);

        /* send RPC response message to client */
        rtos_intertile_tx(intertile_ctx, intertile_port, resp_msg, msg_length);
        rtos_osal_free(resp_msg);
    }
}

__attribute__((fptrgroup("rtos_driver_rpc_host_start_fptr_grp")))
static void qspi_flash_rpc_start(
        rtos_driver_rpc_t *rpc_config)
{
    xassert(rpc_config->host_task_priority >= 0);

    for (int i = 0; i < rpc_config->remote_client_count; i++) {

        rtos_intertile_address_t *client_address = &rpc_config->client_address[i];

        xassert(client_address->port >= 0);

        rtos_osal_thread_create(
                NULL,
                "qspi_flash_rpc_thread",
                (rtos_osal_entry_function_t) qspi_flash_rpc_thread,
                client_address,
                RTOS_THREAD_STACK_SIZE(qspi_flash_rpc_thread),
                rpc_config->host_task_priority);
    }
}

void rtos_qspi_flash_rpc_config(
        rtos_qspi_flash_t *qspi_flash_ctx,
        unsigned intertile_port,
        unsigned host_task_priority)
{
    rtos_driver_rpc_t *rpc_config = qspi_flash_ctx->rpc_config;

    if (rpc_config->remote_client_count == 0) {
        /* This is a client */
        rpc_config->host_address.port = intertile_port;

        rtos_osal_mutex_create(&qspi_flash_ctx->mutex, "qspi_lock", RTOS_OSAL_RECURSIVE);

    } else {
        for (int i = 0; i < rpc_config->remote_client_count; i++) {
            rpc_config->client_address[i].port = intertile_port;
        }
        rpc_config->host_task_priority = host_task_priority;
    }
}

void rtos_qspi_flash_rpc_client_init(
        rtos_qspi_flash_t *qspi_flash_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *host_intertile_ctx)
{
    qspi_flash_ctx->rpc_config = rpc_config;
    qspi_flash_ctx->lock = qspi_flash_remote_lock;
    qspi_flash_ctx->unlock = qspi_flash_remote_unlock;
    qspi_flash_ctx->read = qspi_flash_remote_read;
    qspi_flash_ctx->write = qspi_flash_remote_write;
    qspi_flash_ctx->erase = qspi_flash_remote_erase;
    rpc_config->rpc_host_start = NULL;
    rpc_config->remote_client_count = 0;
    rpc_config->host_task_priority = -1;

    /* This must be configured later with rtos_qspi_flash_rpc_config() */
    rpc_config->host_address.port = -1;

    rpc_config->host_address.intertile_ctx = host_intertile_ctx;
    rpc_config->host_ctx_ptr = (void *) s_chan_in_word(host_intertile_ctx->c);
    qspi_flash_ctx->flash_size = s_chan_in_word(host_intertile_ctx->c);
    qspi_flash_ctx->ctx.page_size_bytes = s_chan_in_word(host_intertile_ctx->c);
    qspi_flash_ctx->ctx.page_count = s_chan_in_word(host_intertile_ctx->c);
    qspi_flash_ctx->ctx.erase_info[0].size_log2 = s_chan_in_word(host_intertile_ctx->c);
}

void rtos_qspi_flash_rpc_host_init(
        rtos_qspi_flash_t *qspi_flash_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *client_intertile_ctx[],
        size_t remote_client_count)
{
    qspi_flash_ctx->rpc_config = rpc_config;
    rpc_config->rpc_host_start = qspi_flash_rpc_start;
    rpc_config->remote_client_count = remote_client_count;

    /* This must be configured later with rtos_qspi_flash_rpc_config() */
    rpc_config->host_task_priority = -1;

    for (int i = 0; i < remote_client_count; i++) {
        rpc_config->client_address[i].intertile_ctx = client_intertile_ctx[i];
        s_chan_out_word(client_intertile_ctx[i]->c, (uint32_t) qspi_flash_ctx);
        s_chan_out_word(client_intertile_ctx[i]->c, (uint32_t) qspi_flash_ctx->flash_size);
        s_chan_out_word(client_intertile_ctx[i]->c, (uint32_t) qspi_flash_ctx->ctx.page_size_bytes);
        s_chan_out_word(client_intertile_ctx[i]->c, (uint32_t) qspi_flash_ctx->ctx.page_count);
        s_chan_out_word(client_intertile_ctx[i]->c, (uint32_t) qspi_flash_ctx->ctx.erase_info[0].size_log2);

        /* This must be configured later with rtos_qspi_flash_rpc_config() */
        rpc_config->client_address[i].port = -1;
    }
}
