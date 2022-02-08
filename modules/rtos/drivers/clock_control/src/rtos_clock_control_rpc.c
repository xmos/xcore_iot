// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <xcore/assert.h>

#include "rtos_rpc.h"
#include "rtos_clock_control.h"

enum {
    fcode_set_ref_clk_div,
    fcode_set_processor_clk_div,
    fcode_set_switch_clk_div,
    fcode_get_ref_clk_div,
    fcode_get_processor_clk_div,
    fcode_get_switch_clk_div,
    fcode_get_processor_clock,
    fcode_get_ref_clock,
    fcode_get_switch_clock,
    fcode_set_node_pll_ratio,
    fcode_get_node_pll_ratio,
    fcode_scale_links,
    fcode_reset_links,
    fcode_get_local_lock,
    fcode_release_local_lock
};

__attribute__((fptrgroup("rtos_clock_control_set_ref_clk_div_fptr_grp")))
static void clock_control_remote_set_ref_clk_div(
        rtos_clock_control_t *ctx,
        unsigned divider)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_TYPE(divider),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_set_ref_clk_div,
            rpc_param_desc, &host_ctx_ptr, &divider);
}

__attribute__((fptrgroup("rtos_clock_control_set_processor_clk_div_fptr_grp")))
static void clock_control_remote_set_processor_clk_div(
        rtos_clock_control_t *ctx,
        unsigned divider)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_TYPE(divider),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_set_processor_clk_div,
            rpc_param_desc, &host_ctx_ptr, &divider);
}

__attribute__((fptrgroup("rtos_clock_control_set_switch_clk_div_fptr_grp")))
static void clock_control_remote_set_switch_clk_div(
        rtos_clock_control_t *ctx,
        unsigned divider)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_TYPE(divider),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_set_switch_clk_div,
            rpc_param_desc, &host_ctx_ptr, &divider);
}

__attribute__((fptrgroup("rtos_clock_control_get_ref_clk_div_fptr_grp")))
static unsigned clock_control_remote_get_ref_clk_div(
        rtos_clock_control_t *ctx)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;
    unsigned retval = 0;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_RETURN(unsigned),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_get_ref_clk_div,
            rpc_param_desc, &host_ctx_ptr, &retval);

    return retval;
}

__attribute__((fptrgroup("rtos_clock_control_get_processor_clk_div_fptr_grp")))
static unsigned clock_control_remote_get_processor_clk_div(
        rtos_clock_control_t *ctx)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;
    unsigned retval = 0;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_RETURN(unsigned),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_get_processor_clk_div,
            rpc_param_desc, &host_ctx_ptr, &retval);

    return retval;
}

__attribute__((fptrgroup("rtos_clock_control_get_switch_clk_div_fptr_grp")))
static unsigned clock_control_remote_get_switch_clk_div(
        rtos_clock_control_t *ctx)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;
    unsigned retval = 0;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_RETURN(unsigned),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_get_switch_clk_div,
            rpc_param_desc, &host_ctx_ptr, &retval);

    return retval;
}

__attribute__((fptrgroup("rtos_clock_control_get_processor_clock_fptr_grp")))
static unsigned clock_control_remote_get_processor_clock(
        rtos_clock_control_t *ctx)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;
    unsigned retval = 0;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_RETURN(unsigned),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_get_processor_clock,
            rpc_param_desc, &host_ctx_ptr, &retval);

    return retval;
}

__attribute__((fptrgroup("rtos_clock_control_get_ref_clock_fptr_grp")))
static unsigned clock_control_remote_get_ref_clock(
        rtos_clock_control_t *ctx)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;
    unsigned retval = 0;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_RETURN(unsigned),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_get_ref_clock,
            rpc_param_desc, &host_ctx_ptr, &retval);

    return retval;
}

__attribute__((fptrgroup("rtos_clock_control_get_switch_clock_fptr_grp")))
static unsigned clock_control_remote_get_switch_clock(
        rtos_clock_control_t *ctx)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;
    unsigned retval = 0;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_RETURN(unsigned),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_get_switch_clock,
            rpc_param_desc, &host_ctx_ptr, &retval);

    return retval;
}

__attribute__((fptrgroup("rtos_clock_control_set_node_pll_ratio_fptr_grp")))
static void clock_control_remote_set_node_pll_ratio(
        rtos_clock_control_t *ctx,
        unsigned pre_div,
        unsigned mul,
        unsigned post_div)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_TYPE(pre_div),
            RPC_PARAM_TYPE(mul),
            RPC_PARAM_TYPE(post_div),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_set_node_pll_ratio,
            rpc_param_desc, &host_ctx_ptr, &pre_div, &mul, &post_div);
}

__attribute__((fptrgroup("rtos_clock_control_get_node_pll_ratio_fptr_grp")))
static void clock_control_remote_get_node_pll_ratio(
        rtos_clock_control_t *ctx,
        unsigned *pre_div,
        unsigned *mul,
        unsigned *post_div)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_RETURN(unsigned),
            RPC_PARAM_RETURN(unsigned),
            RPC_PARAM_RETURN(unsigned),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_get_node_pll_ratio,
            rpc_param_desc, &host_ctx_ptr, pre_div, mul, post_div);
}

__attribute__((fptrgroup("rtos_clock_control_scale_links_fptr_grp")))
static void clock_control_remote_scale_links(
        rtos_clock_control_t *ctx,
        unsigned start_addr,
        unsigned end_addr,
        unsigned delay_intra,
        unsigned delay_inter)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_TYPE(start_addr),
            RPC_PARAM_TYPE(end_addr),
            RPC_PARAM_TYPE(delay_intra),
            RPC_PARAM_TYPE(delay_inter),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_scale_links,
            rpc_param_desc, &host_ctx_ptr, &start_addr, &end_addr, &delay_intra, &delay_inter);
}

__attribute__((fptrgroup("rtos_clock_control_reset_links_fptr_grp")))
static void clock_control_remote_reset_links(
        rtos_clock_control_t *ctx,
        unsigned start_addr,
        unsigned end_addr)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_TYPE(start_addr),
            RPC_PARAM_TYPE(end_addr),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_reset_links,
            rpc_param_desc, &host_ctx_ptr, &start_addr, &end_addr);
}

__attribute__((fptrgroup("rtos_clock_control_get_local_lock_fptr_grp")))
static void clock_control_remote_lock(
        rtos_clock_control_t *ctx)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_get_local_lock,
            rpc_param_desc, &host_ctx_ptr);
}

__attribute__((fptrgroup("rtos_clock_control_release_local_lock_fptr_grp")))
static void clock_control_remote_unlock(
        rtos_clock_control_t *ctx)
{
    rtos_intertile_address_t *host_address = &ctx->rpc_config->host_address;
    rtos_clock_control_t *host_ctx_ptr = ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(ctx),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_release_local_lock,
            rpc_param_desc, &host_ctx_ptr);
}

static int clock_control_set_ref_clk_div_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;
    unsigned divider;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx, &divider);

    set_local_node_ref_clk_div(divider);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, divider);

    return msg_length;
}

static int clock_control_set_processor_clk_div_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;
    unsigned divider;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx, &divider);

    set_local_tile_processor_clk_div(divider);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, divider);

    return msg_length;
}

static int clock_control_set_switch_clk_div_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;
    unsigned divider;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx, &divider);

    set_local_node_switch_clk_div(divider);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, divider);

    return msg_length;
}

static int clock_control_get_ref_clk_div_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;
    unsigned ret;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx);

    ret = get_local_node_ref_clk_div();

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, ret);

    return msg_length;
}

static int clock_control_get_processor_clk_div_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;
    unsigned ret;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx);

    ret = get_local_tile_processor_clk_div();

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, ret);

    return msg_length;
}

static int clock_control_get_switch_clk_div_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;
    unsigned ret;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx);

    ret = get_local_node_switch_clk_div();

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, ret);

    return msg_length;
}

static int clock_control_get_processor_clk_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;
    unsigned ret;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx);

    ret = get_local_core_clock();

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, ret);

    return msg_length;
}

static int clock_control_get_ref_clk_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;
    unsigned ret;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx);

    ret = get_local_ref_clock();

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, ret);

    return msg_length;
}

static int clock_control_get_switch_clk_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;
    unsigned ret;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx);

    ret = get_local_switch_clock();

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, ret);

    return msg_length;
}

static int clock_control_set_node_pll_ratio_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;
    unsigned pre_div;
    unsigned mul;
    unsigned post_div;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx, &pre_div, &mul, &post_div);

    set_local_node_pll_ratio(pre_div, mul, post_div);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, pre_div, mul, post_div);

    return msg_length;
}

static int clock_control_get_node_pll_ratio_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;
    unsigned pre_div;
    unsigned mul;
    unsigned post_div;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx, &pre_div, &mul, &post_div);

    get_local_node_pll_ratio(&pre_div, &mul, &post_div);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, pre_div, mul, post_div);

    return msg_length;
}

static int clock_control_scale_links_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;
    unsigned start_addr;
    unsigned end_addr;
    unsigned delay_intra;
    unsigned delay_inter;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx, &start_addr, &end_addr, &delay_intra, &delay_inter);

    scale_links(start_addr, end_addr, delay_intra, delay_inter);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, start_addr, end_addr, delay_intra, delay_inter);

    return msg_length;
}

static int clock_control_reset_links_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;
    unsigned start_addr;
    unsigned end_addr;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx, &start_addr, &end_addr);

    reset_local_links(start_addr, end_addr);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx, start_addr, end_addr);

    return msg_length;
}

static int clock_control_get_local_lock_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx);

    mrsw_lock_writer_get(&ctx->local_lock, RTOS_OSAL_PORT_WAIT_FOREVER);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx);

    return msg_length;
}

static int clock_control_release_local_lock_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;
    rtos_clock_control_t *ctx;

    rpc_request_unmarshall(
            rpc_msg,
            &ctx);

    mrsw_lock_writer_put(&ctx->local_lock);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            ctx);

    return msg_length;
}


static void clock_control_rpc_thread(rtos_intertile_address_t *client_address)
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

        switch (rpc_msg.fcode)
        {
            case fcode_set_ref_clk_div:
                msg_length = clock_control_set_ref_clk_div_rpc_host(&rpc_msg, &resp_msg);
                break;
            case fcode_set_processor_clk_div:
                msg_length = clock_control_set_processor_clk_div_rpc_host(&rpc_msg, &resp_msg);
                break;
            case fcode_set_switch_clk_div:
                msg_length = clock_control_set_switch_clk_div_rpc_host(&rpc_msg, &resp_msg);
                break;
            case fcode_get_ref_clk_div:
                msg_length = clock_control_get_ref_clk_div_rpc_host(&rpc_msg, &resp_msg);
                break;
            case fcode_get_processor_clk_div:
                msg_length = clock_control_get_processor_clk_div_rpc_host(&rpc_msg, &resp_msg);
                break;
            case fcode_get_switch_clk_div:
                msg_length = clock_control_get_switch_clk_div_rpc_host(&rpc_msg, &resp_msg);
                break;
            case fcode_get_processor_clock:
                msg_length = clock_control_get_processor_clk_rpc_host(&rpc_msg, &resp_msg);
                break;
            case fcode_get_ref_clock:
                msg_length = clock_control_get_ref_clk_rpc_host(&rpc_msg, &resp_msg);
                break;
            case fcode_get_switch_clock:
                msg_length = clock_control_get_switch_clk_rpc_host(&rpc_msg, &resp_msg);
                break;
            case fcode_set_node_pll_ratio:
                msg_length = clock_control_set_node_pll_ratio_rpc_host(&rpc_msg, &resp_msg);
                break;
            case fcode_get_node_pll_ratio:
                msg_length = clock_control_get_node_pll_ratio_rpc_host(&rpc_msg, &resp_msg);
                break;
            case fcode_scale_links:
                msg_length = clock_control_scale_links_rpc_host(&rpc_msg, &resp_msg);
                break;
            case fcode_reset_links:
                msg_length = clock_control_reset_links_rpc_host(&rpc_msg, &resp_msg);
                break;
            case fcode_get_local_lock:
                msg_length = clock_control_get_local_lock_rpc_host(&rpc_msg, &resp_msg);
                break;
            case fcode_release_local_lock:
                msg_length = clock_control_release_local_lock_rpc_host(&rpc_msg, &resp_msg);
                break;
            default:
                xassert(0); /* Unhandled fcode received */
                break;
        }

        rtos_osal_free(req_msg);

        /* send RPC response message to client */
        rtos_intertile_tx(intertile_ctx, intertile_port, resp_msg, msg_length);
        rtos_osal_free(resp_msg);
    }
}

__attribute__((fptrgroup("rtos_driver_rpc_host_start_fptr_grp")))
static void clock_control_rpc_start(
        rtos_driver_rpc_t *rpc_config)
{
    xassert(rpc_config->host_task_priority >= 0);

    for (int i = 0; i < rpc_config->remote_client_count; i++) {
        rtos_intertile_address_t *client_address = &rpc_config->client_address[i];

        xassert(client_address->port >= 0);

        rtos_osal_thread_create(
                NULL,
                "cc_rpc_thread",
                (rtos_osal_entry_function_t) clock_control_rpc_thread,
                client_address,
                RTOS_THREAD_STACK_SIZE(clock_control_rpc_thread),
                rpc_config->host_task_priority);
    }
}

void rtos_clock_control_rpc_config(
        rtos_clock_control_t *cc_ctx,
        unsigned intertile_port,
        unsigned host_task_priority)
{
    rtos_driver_rpc_t *rpc_config = cc_ctx->rpc_config;

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

void rtos_clock_control_rpc_client_init(
        rtos_clock_control_t *cc_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *host_intertile_ctx)
{
    cc_ctx->rpc_config = rpc_config;
    cc_ctx->set_ref_clk_div = clock_control_remote_set_ref_clk_div;
    cc_ctx->set_processor_clk_div = clock_control_remote_set_processor_clk_div;
    cc_ctx->set_switch_clk_div = clock_control_remote_set_switch_clk_div;
    cc_ctx->get_ref_clk_div = clock_control_remote_get_ref_clk_div;
    cc_ctx->get_processor_clk_div = clock_control_remote_get_processor_clk_div;
    cc_ctx->get_switch_clk_div = clock_control_remote_get_switch_clk_div;
    cc_ctx->get_processor_clock = clock_control_remote_get_processor_clock;
    cc_ctx->get_ref_clock = clock_control_remote_get_ref_clock;
    cc_ctx->get_switch_clock = clock_control_remote_get_switch_clock;
    cc_ctx->set_node_pll_ratio = clock_control_remote_set_node_pll_ratio;
    cc_ctx->get_node_pll_ratio = clock_control_remote_get_node_pll_ratio;
    cc_ctx->scale_links = clock_control_remote_scale_links;
    cc_ctx->reset_links = clock_control_remote_reset_links;
    cc_ctx->get_local_lock = clock_control_remote_lock;
    cc_ctx->release_local_lock = clock_control_remote_unlock;
    rpc_config->rpc_host_start = NULL;
    rpc_config->remote_client_count = 0;
    rpc_config->host_task_priority = -1;

    /* This must be configured later with rtos_clock_control_rpc_config() */
    rpc_config->host_address.port = -1;

    rpc_config->host_address.intertile_ctx = host_intertile_ctx;
    rpc_config->host_ctx_ptr = (void *) s_chan_in_word(host_intertile_ctx->c);
}

void rtos_clock_control_rpc_host_init(
        rtos_clock_control_t *cc_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *client_intertile_ctx[],
        size_t remote_client_count)
{
    cc_ctx->rpc_config = rpc_config;
    rpc_config->rpc_host_start = clock_control_rpc_start;
    rpc_config->remote_client_count = remote_client_count;

    /* This must be configured later with rtos_clock_control_rpc_config() */
    rpc_config->host_task_priority = -1;

    for (int i = 0; i < remote_client_count; i++) {
        rpc_config->client_address[i].intertile_ctx = client_intertile_ctx[i];
        s_chan_out_word(client_intertile_ctx[i]->c, (uint32_t) cc_ctx);

        /* This must be configured later with rtos_clock_control_rpc_config() */
        rpc_config->client_address[i].port = -1;
    }
}
