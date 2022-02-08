// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* Standard library headers */
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "rtos_intertile.h"
#include "rtos_osal.h"
#include "rtos_rpc.h"

int rpc_request_marshall_va(uint8_t **msg, int fcode, const rpc_param_desc_t param_desc[], va_list ap)
{
    int param_count;
    int param_total_length = 0;
    int i;
    int msg_length;
    uint8_t *msg_ptr;

    for (i = 0; param_desc[i].input || param_desc[i].output; i++) {
        if (param_desc[i].input) {
            param_total_length += param_desc[i].length;
        }
    }

    param_count = i;

    msg_length = sizeof(int) +                             /* Space for the function code */
                 sizeof(int) +                             /* Space for the parameter count */
                 sizeof(rpc_param_desc_t) * param_count +  /* Space for each parameter descriptor */
                 param_total_length;                       /* Space for the parameters themselves */

    *msg = rtos_osal_malloc(msg_length);
    msg_ptr = *msg;

    memcpy(msg_ptr, &fcode, sizeof(int));
    msg_ptr += sizeof(int);
    memcpy(msg_ptr, &param_count, sizeof(int));
    msg_ptr += sizeof(int);
    memcpy(msg_ptr, param_desc, sizeof(rpc_param_desc_t) * param_count);
    msg_ptr += sizeof(rpc_param_desc_t) * param_count;

    for (i = 0; i < param_count; i++) {
        void *arg_ptr = va_arg(ap, void *);

        if (param_desc[i].input) {
            memcpy(msg_ptr, arg_ptr, param_desc[i].length);
            msg_ptr += param_desc[i].length;
        }
    }

    return msg_length;
}

int rpc_request_marshall(uint8_t **msg, int fcode, const rpc_param_desc_t param_desc[], ...)
{
    int msg_length;
    va_list ap;

    va_start(ap, param_desc);
    msg_length = rpc_request_marshall_va(msg, fcode, param_desc, ap);
    va_end(ap);

    return msg_length;
}

void rpc_request_parse(rpc_msg_t *rpc_msg, uint8_t *msg_buf)
{
    rpc_msg->msg_buf = msg_buf;

    memcpy(&rpc_msg->fcode, msg_buf, sizeof(int));
    msg_buf += sizeof(int);
    memcpy(&rpc_msg->param_count, msg_buf, sizeof(int));
    msg_buf += sizeof(int);
    rpc_msg->param_desc = (rpc_param_desc_t *) msg_buf;
    msg_buf += sizeof(rpc_param_desc_t) * rpc_msg->param_count;
    rpc_msg->params = msg_buf;
}

void rpc_request_unmarshall_va(rpc_msg_t *rpc_msg, va_list ap)
{
    int i;
    uint8_t *params_ptr = rpc_msg->params;

    for (i = 0; i < rpc_msg->param_count; i++) {
        if (rpc_msg->param_desc[i].ptr) {
            void **arg_ptr = va_arg(ap, void **);

            if (rpc_msg->param_desc[i].input) {
                if (rpc_msg->param_desc[i].length > 0) {
                    *arg_ptr = params_ptr;
                } else {
                    *arg_ptr = NULL;
                }
                params_ptr += rpc_msg->param_desc[i].length;
            } else if (rpc_msg->param_desc[i].output) {
                if (rpc_msg->param_desc[i].length == 0) {
                    *arg_ptr = NULL;
                } else {
                    *arg_ptr = NULL+1; /* just something not NULL but still invalid */
                }
            }
        } else {
            void *arg_ptr = va_arg(ap, void *);

            if (rpc_msg->param_desc[i].input) {
                memcpy(arg_ptr, params_ptr, rpc_msg->param_desc[i].length);
                params_ptr += rpc_msg->param_desc[i].length;
            }
        }
    }
}

void rpc_request_unmarshall(rpc_msg_t *rpc_msg, ...)
{
    va_list ap;

    va_start(ap, rpc_msg);
    rpc_request_unmarshall_va(rpc_msg, ap);
    va_end(ap);
}

int rpc_response_marshall_va(uint8_t **msg, const rpc_msg_t *rpc_msg, va_list ap)
{
    int param_total_length = 0;
    int i;
    int msg_length;
    uint8_t *msg_ptr;

    for (i = 0; i < rpc_msg->param_count; i++) {
        if (rpc_msg->param_desc[i].output) {
            param_total_length += rpc_msg->param_desc[i].length;
        }
    }

    msg_length = sizeof(int) +                             /* Space for the function code */
                 param_total_length;                       /* Space for the parameters themselves */

    *msg = rtos_osal_malloc(msg_length);
    msg_ptr = *msg;

    memcpy(msg_ptr, &rpc_msg->fcode, sizeof(int));
    msg_ptr += sizeof(int);

    for (i = 0; i < rpc_msg->param_count; i++) {
        int64_t arg64;
        void *arg_ptr;
        int32_t arg32;
        int16_t arg16;
        int8_t arg8;

        if (rpc_msg->param_desc[i].ptr) {
            arg_ptr = va_arg(ap, void *);
        } else {
            switch (rpc_msg->param_desc[i].length) {
            case sizeof(int8_t):
                arg8 = va_arg(ap, int32_t);
                arg_ptr = &arg8;
                break;
            case sizeof(int16_t):
                arg16 = va_arg(ap, int32_t);
                arg_ptr = &arg16;
                break;
            case sizeof(int32_t):
                arg32 = va_arg(ap, int32_t);
                arg_ptr = &arg32;
                break;
            case sizeof(int64_t):
                arg64 = va_arg(ap, int64_t);
                arg_ptr = &arg64;
                break;
            default:
                xassert(0);
            }
        }
        if (rpc_msg->param_desc[i].output) {
            memcpy(msg_ptr, arg_ptr, rpc_msg->param_desc[i].length);
            msg_ptr += rpc_msg->param_desc[i].length;
        }
    }

    return msg_length;
}

int rpc_response_marshall(uint8_t **msg, const rpc_msg_t *rpc_msg, ...)
{
    int msg_length;
    va_list ap;

    va_start(ap, rpc_msg);
    msg_length = rpc_response_marshall_va(msg, rpc_msg, ap);
    va_end(ap);

    return msg_length;
}

void rpc_response_parse(rpc_msg_t *rpc_msg, uint8_t *msg_buf)
{
    rpc_msg->msg_buf = msg_buf;

    memcpy(&rpc_msg->fcode, msg_buf, sizeof(int));
    msg_buf += sizeof(int);
    rpc_msg->params = msg_buf;
}

void rpc_response_unmarshall_va(const rpc_msg_t *rpc_msg, const rpc_param_desc_t param_desc[], va_list ap)
{
    int i;
    uint8_t *params_ptr = rpc_msg->params;

    for (i = 0; param_desc[i].input || param_desc[i].output; i++) {
        void *arg_ptr = va_arg(ap, void *);

        if (param_desc[i].output) {
            memcpy(arg_ptr, params_ptr, param_desc[i].length);
            params_ptr += param_desc[i].length;
        }
    }
}

void rpc_response_unmarshall(const rpc_msg_t *rpc_msg, const rpc_param_desc_t param_desc[], ...)
{
    va_list ap;

    va_start(ap, param_desc);
    rpc_response_unmarshall_va(rpc_msg, param_desc, ap);
    va_end(ap);
}

void rpc_client_call_generic(rtos_intertile_t *intertile_ctx, uint8_t port, int fcode, const rpc_param_desc_t param_desc[], ...)
{
    uint8_t *req_msg;
    uint8_t *resp_msg;
    rpc_msg_t rpc_msg;
    int msg_length;
    va_list ap_init, ap;

    va_start(ap_init, param_desc);

    va_copy(ap, ap_init);
    msg_length = rpc_request_marshall_va(
            &req_msg, fcode, param_desc,
            ap);
    va_end(ap);

    /* send RPC request message to host */
    rtos_intertile_tx(intertile_ctx, port, req_msg, msg_length);
    rtos_osal_free(req_msg);

    /* receive RPC response message from host */
    msg_length = rtos_intertile_rx(intertile_ctx, port, (void **) &resp_msg, RTOS_OSAL_WAIT_FOREVER);
    if (msg_length == 0) {
        /* TODO: What to do here? */
        xassert(0);
    }

    rpc_response_parse(&rpc_msg, resp_msg);

    xassert(rpc_msg.fcode == fcode);

    va_copy(ap, ap_init);
    rpc_response_unmarshall_va(
            &rpc_msg, param_desc,
            ap);
    va_end(ap);

    rtos_osal_free(resp_msg);

    va_end(ap_init);
}
