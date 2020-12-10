// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef RTOS_GPIO_H_
#define RTOS_GPIO_H_

#include <xcore/port.h>

#include "drivers/rtos/rpc/api/rtos_driver_rpc.h"

typedef enum {
    rtos_gpio_port_none = -1,
    rtos_gpio_port_1A,
    rtos_gpio_port_1B,
    rtos_gpio_port_1C,
    rtos_gpio_port_1D,
    rtos_gpio_port_1E,
    rtos_gpio_port_1F,
    rtos_gpio_port_1G,
    rtos_gpio_port_1H,
    rtos_gpio_port_1I,
    rtos_gpio_port_1J,
    rtos_gpio_port_1K,
    rtos_gpio_port_1L,
    rtos_gpio_port_1M,
    rtos_gpio_port_1N,
    rtos_gpio_port_1O,
    rtos_gpio_port_1P,
    rtos_gpio_port_4A,
    rtos_gpio_port_4B,
    rtos_gpio_port_4C,
    rtos_gpio_port_4D,
    rtos_gpio_port_4E,
    rtos_gpio_port_4F,
    rtos_gpio_port_8A,
    rtos_gpio_port_8B,
    rtos_gpio_port_8C,
    rtos_gpio_port_8D,
    rtos_gpio_port_16A,
    rtos_gpio_port_16B,
    rtos_gpio_port_16C,
    rtos_gpio_port_16D,
    rtos_gpio_port_32A,
    rtos_gpio_port_32B,
    RTOS_GPIO_TOTAL_PORT_CNT
} rtos_gpio_port_id_t;

#define RTOS_GPIO_ISR_CALLBACK_ATTR __attribute__((fptrgroup("rtos_gpio_isr_cb_fptr_grp")))

typedef struct rtos_gpio_struct rtos_gpio_t;

typedef void (*rtos_gpio_isr_cb_t)(rtos_gpio_t *ctx, void *app_data, rtos_gpio_port_id_t port_id, uint32_t value);

typedef struct {
    RTOS_GPIO_ISR_CALLBACK_ATTR rtos_gpio_isr_cb_t callback;
    void *isr_app_data;
    int enabled;
    rtos_gpio_port_id_t port_id;
    rtos_gpio_t *ctx;
} rtos_gpio_isr_info_t;

struct rtos_gpio_struct{
    rtos_driver_rpc_t *rpc_config;
    chanend_t rpc_interrupt_c;

    __attribute__((fptrgroup("rtos_gpio_port_enable_fptr_grp")))
    void (*port_enable)(rtos_gpio_t *, rtos_gpio_port_id_t);

    __attribute__((fptrgroup("rtos_gpio_port_in_fptr_grp")))
    uint32_t (*port_in)(rtos_gpio_t *, rtos_gpio_port_id_t);

    __attribute__((fptrgroup("rtos_gpio_port_out_fptr_grp")))
    void (*port_out)(rtos_gpio_t *, rtos_gpio_port_id_t, uint32_t);

    __attribute__((fptrgroup("rtos_gpio_isr_callback_set_fptr_grp")))
    void (*isr_callback_set)(rtos_gpio_t *, rtos_gpio_port_id_t, rtos_gpio_isr_cb_t, void *);

    __attribute__((fptrgroup("rtos_gpio_interrupt_enable_fptr_grp")))
    void (*interrupt_enable)(rtos_gpio_t *, rtos_gpio_port_id_t);

    __attribute__((fptrgroup("rtos_gpio_interrupt_disable_fptr_grp")))
    void (*interrupt_disable)(rtos_gpio_t *, rtos_gpio_port_id_t);

    rtos_gpio_isr_info_t *isr_info[RTOS_GPIO_TOTAL_PORT_CNT];

    /* BEGIN RTOS SPECIFIC MEMBERS. */
    SemaphoreHandle_t lock; /* Only used by RPC client */
};

#include "drivers/rtos/gpio/api/rtos_gpio_rpc.h"

inline rtos_gpio_port_id_t rtos_gpio_port(port_t p)
{
    switch (p) {
    case XS1_PORT_1A:
        return rtos_gpio_port_1A;
    case XS1_PORT_1B:
        return rtos_gpio_port_1B;
    case XS1_PORT_1C:
        return rtos_gpio_port_1C;
    case XS1_PORT_1D:
        return rtos_gpio_port_1D;
    case XS1_PORT_1E:
        return rtos_gpio_port_1E;
    case XS1_PORT_1F:
        return rtos_gpio_port_1F;
    case XS1_PORT_1G:
        return rtos_gpio_port_1G;
    case XS1_PORT_1H:
        return rtos_gpio_port_1H;
    case XS1_PORT_1I:
        return rtos_gpio_port_1I;
    case XS1_PORT_1J:
        return rtos_gpio_port_1J;
    case XS1_PORT_1K:
        return rtos_gpio_port_1K;
    case XS1_PORT_1L:
        return rtos_gpio_port_1L;
    case XS1_PORT_1M:
        return rtos_gpio_port_1M;
    case XS1_PORT_1N:
        return rtos_gpio_port_1N;
    case XS1_PORT_1O:
        return rtos_gpio_port_1O;
    case XS1_PORT_1P:
        return rtos_gpio_port_1P;
    case XS1_PORT_4A:
        return rtos_gpio_port_4A;
    case XS1_PORT_4B:
        return rtos_gpio_port_4B;
    case XS1_PORT_4C:
        return rtos_gpio_port_4C;
    case XS1_PORT_4D:
        return rtos_gpio_port_4D;
    case XS1_PORT_4E:
        return rtos_gpio_port_4E;
    case XS1_PORT_4F:
        return rtos_gpio_port_4F;
    case XS1_PORT_8A:
        return rtos_gpio_port_8A;
    case XS1_PORT_8B:
        return rtos_gpio_port_8B;
    case XS1_PORT_8C:
        return rtos_gpio_port_8C;
    case XS1_PORT_8D:
        return rtos_gpio_port_8D;
    case XS1_PORT_16A:
        return rtos_gpio_port_16A;
    case XS1_PORT_16B:
        return rtos_gpio_port_16B;
    case XS1_PORT_16C:
        return rtos_gpio_port_16C;
    case XS1_PORT_16D:
        return rtos_gpio_port_16D;
    case XS1_PORT_32A:
        return rtos_gpio_port_32A;
    case XS1_PORT_32B:
        return rtos_gpio_port_32B;
    default:
        return rtos_gpio_port_none;
    }
}

inline void rtos_gpio_port_enable(
        rtos_gpio_t *ctx,
        rtos_gpio_port_id_t port_id)
{
    return ctx->port_enable(ctx, port_id);
}

inline uint32_t rtos_gpio_port_in(
        rtos_gpio_t *ctx,
        rtos_gpio_port_id_t port_id)
{
    return ctx->port_in(ctx, port_id);
}

inline void rtos_gpio_port_out(
        rtos_gpio_t *ctx,
        rtos_gpio_port_id_t port_id,
        uint32_t value)
{
    return ctx->port_out(ctx, port_id, value);
}

inline void rtos_gpio_isr_callback_set(
        rtos_gpio_t *ctx,
        rtos_gpio_port_id_t port_id,
        rtos_gpio_isr_cb_t cb,
        void *app_data)
{
    return ctx->isr_callback_set(ctx, port_id, cb, app_data);
}

inline void rtos_gpio_interrupt_enable(
        rtos_gpio_t *ctx,
        rtos_gpio_port_id_t port_id)
{
    return ctx->interrupt_enable(ctx, port_id);
}

inline void rtos_gpio_interrupt_disable(
        rtos_gpio_t *ctx,
        rtos_gpio_port_id_t port_id)
{
    return ctx->interrupt_disable(ctx, port_id);
}

void rtos_gpio_start(
        rtos_gpio_t *ctx);

void rtos_gpio_init(
        rtos_gpio_t *ctx);

#endif /* RTOS_GPIO_H_ */
