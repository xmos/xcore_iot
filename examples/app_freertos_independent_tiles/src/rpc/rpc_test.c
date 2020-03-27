// Copyright (c) 2020, XMOS Ltd, All rights reserved

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* Library headers */
#include <string.h>
#include "soc.h"

/* BSP/bitstream headers */
#include "bitstream_devices.h"
#include "intertile_driver.h"

/* App headers */
#include "app_conf.h"
#include "intertile_pipe_mgr.h"
#include "rpc.h"

enum {
    rpc_fcode_test_call_1,
    rpc_fcode_test_call_2,
    rpc_fcode_test_call_3,
};

static IntertilePipe_t rpc_pipe;

#if THIS_XCORE_TILE == 1
/* actual call_1() on the host */
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

int call_1_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    int a;
    uint8_t *buffer;
    int len;
    int ret;

    rpc_request_unmarshall(
            rpc_msg,
            &a, &buffer, &len, &ret);

    ret = call_1(a, buffer, len);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            a, buffer, len, ret);

    return msg_length;
}

static void rpc_host_task(void *arg)
{
    int msg_length;
    uint8_t *req_msg;
    uint8_t *resp_msg;
    rpc_msg_t rpc_msg;

    rpc_pipe = intertile_pipe( INTERTILE_CB_ID_0, 2 );

    while (xIntertilePipeManagerReady() == pdFALSE) {
        vTaskDelay(pdMS_TO_TICKS(1)); /* try again in 1ms */
    }

    for (;;) {
        /* receive RPC request message from client */
        msg_length = intertile_recv(rpc_pipe, &req_msg);

        rpc_request_parse(&rpc_msg, req_msg);

        switch(rpc_msg.fcode) {
        case rpc_fcode_test_call_1:
            msg_length = call_1_rpc_host(&rpc_msg, &resp_msg);
            break;
        default:
            configASSERT(0);
        }

        vReleaseIntertileBuffer(req_msg);

        /* send RPC response message to client */
        intertile_send(rpc_pipe, resp_msg, msg_length);
    }
}
#endif

#if THIS_XCORE_TILE == 0
int call_1(int a, uint8_t *buffer, int len)
{
    uint8_t *req_msg;
    uint8_t *resp_msg;
    rpc_msg_t rpc_msg;
    int msg_length;
    int ret;

    const rpc_param_desc_t rpc_param_desc_test_call_1[] = {
            RPC_PARAM_TYPE(a),
            RPC_PARAM_IN_BUFFER(buffer, len),
            RPC_PARAM_TYPE(len),
            RPC_PARAM_RETURN(int),
            RPC_PARAM_LIST_END
    };

    msg_length = rpc_request_marshall(
            &req_msg, rpc_fcode_test_call_1, rpc_param_desc_test_call_1,
            a, buffer, len, &ret);

    /* send RPC request message to host */
    intertile_send(rpc_pipe, req_msg, msg_length);

    /* receive RPC response message from host */
    msg_length = intertile_recv(rpc_pipe, &resp_msg);

    rpc_response_parse(&rpc_msg, resp_msg);
    rpc_response_unmarshall(
            &rpc_msg, rpc_param_desc_test_call_1,
            &a, buffer, &len, &ret);

    vReleaseIntertileBuffer(resp_msg);

    return ret;
}

static void rpc_client_test(void *arg)
{
    rpc_pipe = intertile_pipe( INTERTILE_CB_ID_0, 2 );

    while (xIntertilePipeManagerReady() == pdFALSE) {
        vTaskDelay(pdMS_TO_TICKS(1)); /* try again in 1ms */
    }

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
    }
}
#endif

void rpc_test_init()
{
    //rpc_pipe = intertile_pipe( 42, 0 );
//    rpc_pipe = intertile_pipe( INTERTILE_CB_ID_0 );

#if THIS_XCORE_TILE == 0
    xTaskCreate(rpc_client_test, "rpc_client_test", portTASK_STACK_DEPTH(rpc_client_test), NULL, 15, NULL);
#endif
#if THIS_XCORE_TILE == 1
    xTaskCreate(rpc_host_task, "rpc_host_task", portTASK_STACK_DEPTH(rpc_host_task), NULL, 15, NULL);
#endif
}
