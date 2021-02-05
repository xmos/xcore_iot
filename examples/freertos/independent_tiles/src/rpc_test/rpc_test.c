// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* Library headers */
#include <string.h>

#include "rpc_test.h"

enum {
    rpc_fcode_test_call_1,
    rpc_fcode_test_call_2,
};

static rtos_intertile_t *intertile_ctx;
static const uint8_t intertile_port = 0;

#if ON_TILE(1)
/*
 * actual call_1() on the host.
 * Demonstrates an in/out buffer.
 * Can also be used to demonstrate
 * and input only buffer.
 */
int call_1(int a, uint8_t *buffer, int len)
{
    int ret;

    rtos_printf("call_1() on host called with:\n");
    rtos_printf("\ta: %d\n", a);
    rtos_printf("\tbuffer: ");
    for (int i = 0; i < len; i++) {
        rtos_printf("%02x ", buffer[i]);
    }
    rtos_printf("\n");
    rtos_printf("\tlen: %d\n", len);

    ret = a;

    for (int i = 0; i < len; i++) {
        ret += buffer[i];
        buffer[i] = i;
    }

    rtos_printf("call_1() on host returning: %d\n", ret);
    rtos_printf("\tbuffer: ");
    for (int i = 0; i < len; i++) {
        rtos_printf("%02x ", buffer[i]);
    }
    rtos_printf("\n\n");

    return ret;
}

/*
 * actual call_2() on the host.
 * Demonstrates an output only buffer.
 */
int call_2(uint8_t *buffer, int len)
{
    static int retlen = 5;

    if (retlen > len) {
        retlen = len;
    }

    for (int i = 0; i < retlen; i++) {
        buffer[i] = retlen+i;
    }

    return retlen++;
}
#endif
#if ON_TILE(0)
/* The client version of call_1() */
int call_1(int a, uint8_t *buffer, int len)
{
    int ret;

    const rpc_param_desc_t rpc_param_desc_test_call_1[] = {
            RPC_PARAM_TYPE(a),
            RPC_PARAM_INOUT_BUFFER(buffer, len),
            RPC_PARAM_TYPE(len),
            RPC_PARAM_RETURN(int),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            intertile_ctx, intertile_port, rpc_fcode_test_call_1, rpc_param_desc_test_call_1,
            &a, buffer, &len, &ret);

    return ret;
}

/* The client version of call_2() */
int call_2(uint8_t *buffer, int len)
{
    int ret;

    const rpc_param_desc_t rpc_param_desc_test_call_2[] = {
            RPC_PARAM_OUT_BUFFER(buffer, len),
            RPC_PARAM_TYPE(len),
            RPC_PARAM_RETURN(int),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            intertile_ctx, intertile_port, rpc_fcode_test_call_2, rpc_param_desc_test_call_2,
            buffer, &len, &ret);

    return ret;
}
#endif

#if ON_TILE(1)
static int call_1_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    int a;
    uint8_t *buffer;
    int len;
    int ret;

    /* Here, because the buffer is an input, the buffer
    pointer gets set to point at the data in the request
    message. */
    rpc_request_unmarshall(
            rpc_msg,
            &a, &buffer, &len, &ret);

    ret = call_1(a, buffer, len);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            a, buffer, len, ret);

    return msg_length;
}

static int call_2_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    uint8_t *buffer;
    int len;
    int ret;

    /* Here, because the buffer is output only, the buffer
    pointer will not get set. */
    rpc_request_unmarshall(
            rpc_msg,
            &buffer, &len, &ret);

    /* Instead allocate a buffer with the length parameter
    before calling, as this is what would be done by
    the client. Alternatively this could be allocated
    statically or on the stack if the length is known
    at compile time. */
    buffer = pvPortMalloc(len);

    ret = call_2(buffer, len);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            buffer, len, ret);

    /* The data from buffer has been copied into the response
    message. Since buffer was allocated on the heap, free
    it now. */
    vPortFree(buffer);

    return msg_length;
}

static void rpc_host_task(void *arg)
{
    int msg_length;
    uint8_t *req_msg;
    uint8_t *resp_msg;
    rpc_msg_t rpc_msg;

    intertile_ctx = arg;

    for (;;) {
        /* receive RPC request message from client */
        msg_length = rtos_intertile_rx(intertile_ctx, intertile_port, (void **) &req_msg, portMAX_DELAY);

        rpc_request_parse(&rpc_msg, req_msg);

        switch(rpc_msg.fcode) {
        case rpc_fcode_test_call_1:
            msg_length = call_1_rpc_host(&rpc_msg, &resp_msg);
            break;
        case rpc_fcode_test_call_2:
            msg_length = call_2_rpc_host(&rpc_msg, &resp_msg);
            break;
        default:
            configASSERT(0);
        }

        vPortFree(req_msg);

        /* send RPC response message to client */
        rtos_intertile_tx(intertile_ctx, intertile_port, resp_msg, msg_length);
        vPortFree(resp_msg);
    }
}
#endif

#if ON_TILE(0)
static void rpc_client_test(void *arg)
{
    intertile_ctx = arg;

    int ret;
    uint8_t buffer[32] = {0};

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        rtos_printf("call_1() on client calling with:\n");
        rtos_printf("\ta: %d\n", 6);
        rtos_printf("\tbuffer: ");
        for (int i = 0; i < 32; i++) {
            rtos_printf("%02x ", buffer[i]);
        }
        rtos_printf("\n");
        rtos_printf("\tlen: %d\n\n", 32);

        ret = call_1(6, buffer, 32);

        rtos_printf("call_1() on client returned: %d\n", ret);
        rtos_printf("\tbuffer: ");
        for (int i = 0; i < 32; i++) {
            rtos_printf("%02x ", buffer[i]);
        }
        rtos_printf("\n\n");

        ret = call_2(buffer, 32);
        rtos_printf("call_2() on client returned: %d\n", ret);
        rtos_printf("\tbuffer: ");
        for (int i = 0; i < ret; i++) {
            rtos_printf("%02x ", buffer[i]);
        }
        rtos_printf("\n\n");
    }
}
#endif

void rpc_test_init(rtos_intertile_t *intertile_ctx)
{
#if ON_TILE(0)
    xTaskCreate(rpc_client_test, "rpc_client_test", portTASK_STACK_DEPTH(rpc_client_test), intertile_ctx, 15, NULL);
#endif
#if ON_TILE(1)
    xTaskCreate(rpc_host_task, "rpc_host_task", portTASK_STACK_DEPTH(rpc_host_task), intertile_ctx, 15, NULL);
#endif
}
