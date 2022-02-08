// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <string.h>
#include <xcore/triggerable.h>
#include <xcore/assert.h>

#include "rtos_interrupt.h"
#include "rtos_gpio.h"
#include "rtos_rpc.h"

RTOS_GPIO_ISR_CALLBACK_ATTR
static void rtos_gpio_rpc_host_isr(rtos_gpio_t *ctx, void *app_data, rtos_gpio_port_id_t port_id, uint32_t value)
{
    chanend_t rpc_interrupt_c = (chanend_t) app_data;
    chanend_out_byte(rpc_interrupt_c, port_id);
    chanend_out_word(rpc_interrupt_c, value);
    chanend_out_control_token(rpc_interrupt_c, XS1_CT_PAUSE);
}

DEFINE_RTOS_INTERRUPT_CALLBACK(rtos_gpio_rpc_client_isr, arg)
{
    rtos_gpio_t *gpio_ctx = arg;
    rtos_gpio_port_id_t port_id;
    uint32_t value;
    void *isr_app_data;
    RTOS_GPIO_ISR_CALLBACK_ATTR rtos_gpio_isr_cb_t cb;

    port_id = chanend_in_byte(gpio_ctx->rpc_interrupt_c[0]);
    value = chanend_in_word(gpio_ctx->rpc_interrupt_c[0]);

    int state = rtos_osal_critical_enter();
    {
        rtos_gpio_isr_info_t *isr_info = gpio_ctx->isr_info[port_id];
        isr_app_data = isr_info->isr_app_data;
        cb = isr_info->callback;
    }
    rtos_osal_critical_exit(state);

    cb(gpio_ctx, isr_app_data, port_id, value);
}

enum {
    fcode_port_enable,
    fcode_port_in,
    fcode_port_out,
    fcode_port_write_control_word,
    fcode_isr_callback_set,
    fcode_interrupt_enable,
    fcode_interrupt_disable
};

__attribute__((fptrgroup("rtos_gpio_port_enable_fptr_grp")))
static void gpio_remote_port_enable(
        rtos_gpio_t *gpio_ctx,
        rtos_gpio_port_id_t port_id)
{
    rtos_intertile_address_t *host_address = &gpio_ctx->rpc_config->host_address;
    rtos_gpio_t *host_ctx_ptr = gpio_ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(gpio_ctx),
            RPC_PARAM_TYPE(port_id),
            RPC_PARAM_LIST_END
    };

    rtos_osal_mutex_get(&gpio_ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_port_enable, rpc_param_desc,
            &host_ctx_ptr, &port_id);
    rtos_osal_mutex_put(&gpio_ctx->lock);
}

__attribute__((fptrgroup("rtos_gpio_port_in_fptr_grp")))
static uint32_t gpio_remote_port_in(
        rtos_gpio_t *gpio_ctx,
        rtos_gpio_port_id_t port_id)
{
    rtos_intertile_address_t *host_address = &gpio_ctx->rpc_config->host_address;
    rtos_gpio_t *host_ctx_ptr = gpio_ctx->rpc_config->host_ctx_ptr;
    uint32_t ret;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(gpio_ctx),
            RPC_PARAM_TYPE(port_id),
            RPC_PARAM_RETURN(uint32_t),
            RPC_PARAM_LIST_END
    };

    rtos_osal_mutex_get(&gpio_ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_port_in, rpc_param_desc,
            &host_ctx_ptr, &port_id, &ret);
    rtos_osal_mutex_put(&gpio_ctx->lock);

    return ret;
}

__attribute__((fptrgroup("rtos_gpio_port_out_fptr_grp")))
static void gpio_remote_port_out(
        rtos_gpio_t *gpio_ctx,
        rtos_gpio_port_id_t port_id,
        uint32_t value)
{
    rtos_intertile_address_t *host_address = &gpio_ctx->rpc_config->host_address;
    rtos_gpio_t *host_ctx_ptr = gpio_ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(gpio_ctx),
            RPC_PARAM_TYPE(port_id),
            RPC_PARAM_TYPE(value),
            RPC_PARAM_LIST_END
    };

    rtos_osal_mutex_get(&gpio_ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_port_out, rpc_param_desc,
            &host_ctx_ptr, &port_id, &value);
    rtos_osal_mutex_put(&gpio_ctx->lock);
}

__attribute__((fptrgroup("rtos_gpio_port_write_control_word_fptr_grp")))
static void gpio_remote_port_write_control_word(
        rtos_gpio_t *gpio_ctx,
        rtos_gpio_port_id_t port_id,
        uint32_t value)
{
    rtos_intertile_address_t *host_address = &gpio_ctx->rpc_config->host_address;
    rtos_gpio_t *host_ctx_ptr = gpio_ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(gpio_ctx),
            RPC_PARAM_TYPE(port_id),
            RPC_PARAM_TYPE(value),
            RPC_PARAM_LIST_END
    };

    rtos_osal_mutex_get(&gpio_ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_port_write_control_word, rpc_param_desc,
            &host_ctx_ptr, &port_id, &value);
    rtos_osal_mutex_put(&gpio_ctx->lock);
}

__attribute__((fptrgroup("rtos_gpio_isr_callback_set_fptr_grp")))
static void gpio_remote_isr_callback_set(
        rtos_gpio_t *gpio_ctx,
        rtos_gpio_port_id_t port_id,
        rtos_gpio_isr_cb_t cb,
        void *app_data)
{
    rtos_intertile_address_t *host_address = &gpio_ctx->rpc_config->host_address;
    rtos_gpio_t *host_ctx_ptr = gpio_ctx->rpc_config->host_ctx_ptr;
    chanend_t host_rpc_interrupt_c = chanend_get_dest(gpio_ctx->rpc_interrupt_c[0]);

    xassert(host_address->port >= 0);

    int state = rtos_osal_critical_enter();
    {
        if (gpio_ctx->isr_info[port_id] == NULL) {
            gpio_ctx->isr_info[port_id] = rtos_osal_malloc(sizeof(rtos_gpio_isr_info_t));
        }

        gpio_ctx->isr_info[port_id]->callback = cb;
        gpio_ctx->isr_info[port_id]->isr_app_data = app_data;
    }
    rtos_osal_critical_exit(state);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(gpio_ctx),
            RPC_PARAM_TYPE(port_id),
            RPC_PARAM_TYPE(host_rpc_interrupt_c),
            RPC_PARAM_LIST_END
    };

    rtos_osal_mutex_get(&gpio_ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_isr_callback_set, rpc_param_desc,
            &host_ctx_ptr, &port_id, &host_rpc_interrupt_c);
    rtos_osal_mutex_put(&gpio_ctx->lock);
}

__attribute__((fptrgroup("rtos_gpio_interrupt_enable_fptr_grp")))
static void gpio_remote_interrupt_enable(
        rtos_gpio_t *gpio_ctx,
        rtos_gpio_port_id_t port_id)
{
    rtos_intertile_address_t *host_address = &gpio_ctx->rpc_config->host_address;
    rtos_gpio_t *host_ctx_ptr = gpio_ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(gpio_ctx),
            RPC_PARAM_TYPE(port_id),
            RPC_PARAM_LIST_END
    };

    rtos_osal_mutex_get(&gpio_ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_interrupt_enable, rpc_param_desc,
            &host_ctx_ptr, &port_id);
    rtos_osal_mutex_put(&gpio_ctx->lock);
}

__attribute__((fptrgroup("rtos_gpio_interrupt_disable_fptr_grp")))
static void gpio_remote_interrupt_disable(
        rtos_gpio_t *gpio_ctx,
        rtos_gpio_port_id_t port_id)
{
    rtos_intertile_address_t *host_address = &gpio_ctx->rpc_config->host_address;
    rtos_gpio_t *host_ctx_ptr = gpio_ctx->rpc_config->host_ctx_ptr;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(gpio_ctx),
            RPC_PARAM_TYPE(port_id),
            RPC_PARAM_LIST_END
    };

    rtos_osal_mutex_get(&gpio_ctx->lock, RTOS_OSAL_WAIT_FOREVER);
    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, fcode_interrupt_disable, rpc_param_desc,
            &host_ctx_ptr, &port_id);
    rtos_osal_mutex_put(&gpio_ctx->lock);
}

static int gpio_port_enable_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_gpio_t *gpio_ctx;
    rtos_gpio_port_id_t port_id;

    rpc_request_unmarshall(
            rpc_msg,
            &gpio_ctx, &port_id);

    rtos_gpio_port_enable(gpio_ctx, port_id);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            gpio_ctx, port_id);

    return msg_length;
}

static int gpio_port_in_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_gpio_t *gpio_ctx;
    rtos_gpio_port_id_t port_id;
    uint32_t ret;

    rpc_request_unmarshall(
            rpc_msg,
            &gpio_ctx, &port_id, &ret);

    ret = rtos_gpio_port_in(gpio_ctx, port_id);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            gpio_ctx, port_id, ret);

    return msg_length;
}

static int gpio_port_out_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_gpio_t *gpio_ctx;
    rtos_gpio_port_id_t port_id;
    uint32_t value;

    rpc_request_unmarshall(
            rpc_msg,
            &gpio_ctx, &port_id, &value);

    rtos_gpio_port_out(gpio_ctx, port_id, value);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            gpio_ctx, port_id, value);

    return msg_length;
}

static int gpio_port_write_control_word_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_gpio_t *gpio_ctx;
    rtos_gpio_port_id_t port_id;
    uint32_t value;

    rpc_request_unmarshall(
            rpc_msg,
            &gpio_ctx, &port_id, &value);

    rtos_gpio_write_control_word(gpio_ctx, port_id, value);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            gpio_ctx, port_id, value);

    return msg_length;
}

static int gpio_isr_callback_set_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_gpio_t *gpio_ctx;
    rtos_gpio_port_id_t port_id;
    chanend_t host_rpc_interrupt_c;

    rpc_request_unmarshall(
            rpc_msg,
            &gpio_ctx, &port_id, &host_rpc_interrupt_c);

    rtos_gpio_isr_callback_set(gpio_ctx, port_id, rtos_gpio_rpc_host_isr, (void *) host_rpc_interrupt_c);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            gpio_ctx, port_id, host_rpc_interrupt_c);

    return msg_length;
}

static int gpio_interrupt_enable_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_gpio_t *gpio_ctx;
    rtos_gpio_port_id_t port_id;

    rpc_request_unmarshall(
            rpc_msg,
            &gpio_ctx, &port_id);

    rtos_gpio_interrupt_enable(gpio_ctx, port_id);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            gpio_ctx, port_id);

    return msg_length;
}

static int gpio_interrupt_disable_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_gpio_t *gpio_ctx;
    rtos_gpio_port_id_t port_id;

    rpc_request_unmarshall(
            rpc_msg,
            &gpio_ctx, &port_id);

    rtos_gpio_interrupt_disable(gpio_ctx, port_id);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            gpio_ctx, port_id);

    return msg_length;
}

static void gpio_rpc_thread(rtos_intertile_address_t *client_address)
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
        case fcode_port_enable:
            msg_length = gpio_port_enable_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_port_in:
            msg_length = gpio_port_in_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_port_out:
            msg_length = gpio_port_out_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_port_write_control_word:
            msg_length = gpio_port_write_control_word_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_isr_callback_set:
            msg_length = gpio_isr_callback_set_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_interrupt_enable:
            msg_length = gpio_interrupt_enable_rpc_host(&rpc_msg, &resp_msg);
            break;
        case fcode_interrupt_disable:
            msg_length = gpio_interrupt_disable_rpc_host(&rpc_msg, &resp_msg);
            break;
        }

        rtos_osal_free(req_msg);

        /* send RPC response message to client */
        rtos_intertile_tx(intertile_ctx, intertile_port, resp_msg, msg_length);
        rtos_osal_free(resp_msg);
    }
}

__attribute__((fptrgroup("rtos_driver_rpc_host_start_fptr_grp")))
static void gpio_rpc_start(
        rtos_driver_rpc_t *rpc_config)
{
    xassert(rpc_config->host_task_priority >= 0);

    for (int i = 0; i < rpc_config->remote_client_count; i++) {

        rtos_intertile_address_t *client_address = &rpc_config->client_address[i];

        xassert(client_address->port >= 0);

        rtos_osal_thread_create(
                NULL,
                "gpio_rpc_thread",
                (rtos_osal_entry_function_t) gpio_rpc_thread,
                client_address,
                RTOS_THREAD_STACK_SIZE(gpio_rpc_thread),
                rpc_config->host_task_priority);
    }
}

void rtos_gpio_rpc_config(
        rtos_gpio_t *gpio_ctx,
        unsigned intertile_port,
        unsigned host_task_priority)
{
    rtos_driver_rpc_t *rpc_config = gpio_ctx->rpc_config;

    if (rpc_config->remote_client_count == 0) {
        /* This is a client */
        rpc_config->host_address.port = intertile_port;

        rtos_osal_mutex_create(&gpio_ctx->lock, "gpio_rpc_lock", RTOS_OSAL_NOT_RECURSIVE);
    } else {
        for (int i = 0; i < rpc_config->remote_client_count; i++) {
            rpc_config->client_address[i].port = intertile_port;
        }
        rpc_config->host_task_priority = host_task_priority;
    }
}

void rtos_gpio_rpc_client_init(
        rtos_gpio_t *gpio_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *host_intertile_ctx)
{
    gpio_ctx->rpc_config = rpc_config;
    gpio_ctx->port_enable = gpio_remote_port_enable;
    gpio_ctx->port_in = gpio_remote_port_in;
    gpio_ctx->port_out = gpio_remote_port_out;
    gpio_ctx->port_write_control_word = gpio_remote_port_write_control_word;
    gpio_ctx->isr_callback_set = gpio_remote_isr_callback_set;
    gpio_ctx->interrupt_enable = gpio_remote_interrupt_enable;
    gpio_ctx->interrupt_disable = gpio_remote_interrupt_disable;
    rpc_config->rpc_host_start = NULL;
    rpc_config->remote_client_count = 0;
    rpc_config->host_task_priority = -1;

    /* This must be configured later with rtos_gpio_rpc_config() */
    rpc_config->host_address.port = -1;

    rpc_config->host_address.intertile_ctx = host_intertile_ctx;
    rpc_config->host_ctx_ptr = (void *) s_chan_in_word(host_intertile_ctx->c);

    memset(gpio_ctx->isr_info, 0, sizeof(gpio_ctx->isr_info));
    gpio_ctx->rpc_interrupt_c[0] = chanend_alloc();
    s_chan_out_word(host_intertile_ctx->c, gpio_ctx->rpc_interrupt_c[0]);
    chanend_set_dest(gpio_ctx->rpc_interrupt_c[0], s_chan_in_word(host_intertile_ctx->c));
    triggerable_setup_interrupt_callback(gpio_ctx->rpc_interrupt_c[0], gpio_ctx, RTOS_INTERRUPT_CALLBACK(rtos_gpio_rpc_client_isr));
    triggerable_enable_trigger(gpio_ctx->rpc_interrupt_c[0]);
}

void rtos_gpio_rpc_host_init(
        rtos_gpio_t *gpio_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *client_intertile_ctx[],
        size_t remote_client_count)
{
    gpio_ctx->rpc_config = rpc_config;
    rpc_config->rpc_host_start = gpio_rpc_start;
    rpc_config->remote_client_count = remote_client_count;

    /* This must be configured later with rtos_gpio_rpc_config() */
    rpc_config->host_task_priority = -1;

    for (int i = 0; i < remote_client_count; i++) {
        rpc_config->client_address[i].intertile_ctx = client_intertile_ctx[i];
        s_chan_out_word(client_intertile_ctx[i]->c, (uint32_t) gpio_ctx);

        gpio_ctx->rpc_interrupt_c[i] = chanend_alloc();
        chanend_set_dest(gpio_ctx->rpc_interrupt_c[i], s_chan_in_word(client_intertile_ctx[i]->c));
        s_chan_out_word(client_intertile_ctx[i]->c, gpio_ctx->rpc_interrupt_c[i]);

        /* This must be configured later with rtos_gpio_rpc_config() */
        rpc_config->client_address[i].port = -1;
    }
}
