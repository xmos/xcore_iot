// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <string.h>
#include <xcore/triggerable.h>
#include <xcore/assert.h>

#include "rtos_gpio.h"

#define INTERRUPT_DISABLED        0
#define INTERRUPT_ENABLED         1
#define INTERRUPT_DISABLE_PENDING 2

static const port_t gpio_port_lookup[RTOS_GPIO_TOTAL_PORT_CNT] = {
    XS1_PORT_1A, XS1_PORT_1B, XS1_PORT_1C, XS1_PORT_1D, XS1_PORT_1E, XS1_PORT_1F, XS1_PORT_1G,
    XS1_PORT_1H, XS1_PORT_1I, XS1_PORT_1J, XS1_PORT_1K, XS1_PORT_1L, XS1_PORT_1M, XS1_PORT_1N,
    XS1_PORT_1O, XS1_PORT_1P,
    XS1_PORT_4A, XS1_PORT_4B, XS1_PORT_4C, XS1_PORT_4D, XS1_PORT_4E, XS1_PORT_4F,
    XS1_PORT_8A, XS1_PORT_8B, XS1_PORT_8C, XS1_PORT_8D,
    XS1_PORT_16A, XS1_PORT_16B, XS1_PORT_16C, XS1_PORT_16D,
    XS1_PORT_32A, XS1_PORT_32B
};

DEFINE_RTOS_INTERRUPT_CALLBACK(rtos_gpio_isr, arg)
{
    rtos_gpio_isr_info_t *cb_arg = arg;
    uint32_t value;
    port_t p = gpio_port_lookup[cb_arg->port_id];
    rtos_gpio_t *ctx = cb_arg->ctx;
    void *isr_app_data;
    RTOS_GPIO_ISR_CALLBACK_ATTR rtos_gpio_isr_cb_t cb;
    int enabled = INTERRUPT_ENABLED;

    int state = rtos_osal_critical_enter();
    {
        value = port_in(p);
        isr_app_data = cb_arg->isr_app_data;
        cb = cb_arg->callback;
        if (cb_arg->enabled == INTERRUPT_DISABLE_PENDING) {
            triggerable_disable_trigger(p);
            enabled = INTERRUPT_DISABLED;
            cb_arg->enabled = INTERRUPT_DISABLED;
        }
    }
    rtos_osal_critical_exit(state);

    if (enabled) {
        cb(ctx, isr_app_data, cb_arg->port_id, value);
        port_set_trigger_value(p, value);
    }
}

static int port_valid(rtos_gpio_port_id_t port_id)
{
    return port_id > rtos_gpio_port_none && port_id < RTOS_GPIO_TOTAL_PORT_CNT;
}

__attribute__((fptrgroup("rtos_gpio_port_enable_fptr_grp")))
static void gpio_local_port_enable(rtos_gpio_t *ctx, rtos_gpio_port_id_t port_id)
{
    (void) ctx;

    xassert(port_valid(port_id));
    port_enable(gpio_port_lookup[port_id]);
    port_set_clock(gpio_port_lookup[port_id], XS1_CLKBLK_REF);
}

__attribute__((fptrgroup("rtos_gpio_port_in_fptr_grp")))
static uint32_t gpio_local_port_in(rtos_gpio_t *ctx, rtos_gpio_port_id_t port_id)
{
    uint32_t value;
    (void) ctx;

    xassert(port_valid(port_id));

    int state = rtos_osal_critical_enter();
    {
        value = port_peek(gpio_port_lookup[port_id]);
    }
    rtos_osal_critical_exit(state);

    return value;
}

__attribute__((fptrgroup("rtos_gpio_port_out_fptr_grp")))
static void gpio_local_port_out(rtos_gpio_t *ctx, rtos_gpio_port_id_t port_id, uint32_t value)
{
    (void) ctx;

    xassert(port_valid(port_id));

    int state = rtos_osal_critical_enter();
    {
        port_out(gpio_port_lookup[port_id], value);
    }
    rtos_osal_critical_exit(state);
}

__attribute__((fptrgroup("rtos_gpio_port_write_control_word_fptr_grp")))
static void gpio_local_port_write_control_word(rtos_gpio_t *ctx, rtos_gpio_port_id_t port_id, uint32_t value)
{
    (void) ctx;

    xassert(port_valid(port_id));

    int state = rtos_osal_critical_enter();
    {
        port_write_control_word(gpio_port_lookup[port_id], value);
    }
    rtos_osal_critical_exit(state);
}

__attribute__((fptrgroup("rtos_gpio_isr_callback_set_fptr_grp")))
static void gpio_local_isr_callback_set(rtos_gpio_t *ctx, rtos_gpio_port_id_t port_id, rtos_gpio_isr_cb_t cb, void *app_data)
{
    xassert(port_valid(port_id));

    int state = rtos_osal_critical_enter();
    {
        if (ctx->isr_info[port_id] == NULL) {
            ctx->isr_info[port_id] = rtos_osal_malloc(sizeof(rtos_gpio_isr_info_t));
            ctx->isr_info[port_id]->ctx = ctx;
            ctx->isr_info[port_id]->port_id = port_id;
            ctx->isr_info[port_id]->enabled = INTERRUPT_DISABLED;

            triggerable_setup_interrupt_callback(gpio_port_lookup[port_id], ctx->isr_info[port_id], RTOS_INTERRUPT_CALLBACK(rtos_gpio_isr));
        }

        ctx->isr_info[port_id]->callback = cb;
        ctx->isr_info[port_id]->isr_app_data = app_data;
    }
    rtos_osal_critical_exit(state);
}

__attribute__((fptrgroup("rtos_gpio_interrupt_enable_fptr_grp")))
static void gpio_local_interrupt_enable(rtos_gpio_t *ctx, rtos_gpio_port_id_t port_id)
{
    uint32_t value;
    port_t p;

    xassert(port_valid(port_id));
    p = gpio_port_lookup[port_id];

    int state = rtos_osal_critical_enter();
    {
        if (ctx->isr_info[port_id] != NULL && !ctx->isr_info[port_id]->enabled) {
            value = port_peek(p);
            port_set_trigger_in_not_equal(p, value);
            triggerable_enable_trigger(p);
        }
        ctx->isr_info[port_id]->enabled = INTERRUPT_ENABLED;
    }
    rtos_osal_critical_exit(state);
}

__attribute__((fptrgroup("rtos_gpio_interrupt_disable_fptr_grp")))
static void gpio_local_interrupt_disable(rtos_gpio_t *ctx, rtos_gpio_port_id_t port_id)
{
    xassert(port_valid(port_id));

    int state = rtos_osal_critical_enter();
    {
        if (ctx->isr_info[port_id] != NULL && ctx->isr_info[port_id]->enabled) {
            ctx->isr_info[port_id]->enabled = INTERRUPT_DISABLE_PENDING;
        }
    }
    rtos_osal_critical_exit(state);
}

void rtos_gpio_start(
        rtos_gpio_t *ctx)
{
    if (ctx->rpc_config != NULL && ctx->rpc_config->rpc_host_start != NULL) {
        ctx->rpc_config->rpc_host_start(ctx->rpc_config);
    }
}

void rtos_gpio_init(
        rtos_gpio_t *ctx)
{
    memset(ctx->isr_info, 0, sizeof(ctx->isr_info));

    ctx->port_enable = gpio_local_port_enable;
    ctx->port_in = gpio_local_port_in;
    ctx->port_out = gpio_local_port_out;
    ctx->port_write_control_word = gpio_local_port_write_control_word;
    ctx->isr_callback_set = gpio_local_isr_callback_set;
    ctx->interrupt_enable = gpio_local_interrupt_enable;
    ctx->interrupt_disable = gpio_local_interrupt_disable;
}
