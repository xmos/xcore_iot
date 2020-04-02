// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef RPC_H_
#define RPC_H_

#include <stdint.h>

#include "intertile_pipe_mgr.h"

#define RPC_PARAM_TYPE(param)                 {sizeof(typeof(param)),              0, 1, 0}
#define RPC_PARAM_RETURN(type)                {sizeof(type),                       0, 0, 1}
#define RPC_PARAM_IN_BUFFER(param, length)    {sizeof(typeof(param[0])) * length,  1, 1, 0}
#define RPC_PARAM_OUT_BUFFER(param, length)   {sizeof(typeof(param[0])) * length,  1, 0, 1}
#define RPC_PARAM_INOUT_BUFFER(param, length) {sizeof(typeof(param[0])) * length,  1, 1, 1}
#define RPC_PARAM_LIST_END                    {0,                                  0, 0, 0}

typedef struct {
    uint32_t length  : 24;
    uint8_t  ptr     :  1;
    uint8_t  input   :  1;
    uint8_t  output  :  1;
} rpc_param_desc_t;

typedef struct {
    int fcode;
    int param_count;
    rpc_param_desc_t *param_desc;
    void *params;
    void *msg_buf;
} rpc_msg_t;

int rpc_request_marshall_va(uint8_t **msg, int fcode, const rpc_param_desc_t param_desc[], va_list ap);
int rpc_request_marshall(uint8_t **msg, int fcode, const rpc_param_desc_t param_desc[], ...);

void rpc_request_parse(rpc_msg_t *rpc_msg, uint8_t *msg_buf);
void rpc_request_unmarshall_va(rpc_msg_t *rpc_msg, va_list ap);
void rpc_request_unmarshall(rpc_msg_t *rpc_msg, ...);

int rpc_response_marshall_va(uint8_t **msg, const rpc_msg_t *rpc_msg, va_list ap);
int rpc_response_marshall(uint8_t **msg, const rpc_msg_t *rpc_msg, ...);

void rpc_response_parse(rpc_msg_t *rpc_msg, uint8_t *msg_buf);
void rpc_response_unmarshall_va(const rpc_msg_t *rpc_msg, const rpc_param_desc_t param_desc[], va_list ap);
void rpc_response_unmarshall(const rpc_msg_t *rpc_msg, const rpc_param_desc_t param_desc[], ...);

void rpc_client_call_generic(IntertilePipe_t rpc_pipe, int fcode, const rpc_param_desc_t param_desc[], ...);

#endif /* RPC_H_ */
