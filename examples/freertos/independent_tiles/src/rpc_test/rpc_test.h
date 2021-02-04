// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

#ifndef RPC_TEST_H_
#define RPC_TEST_H_

#include "rtos/drivers/rpc/api/rtos_rpc.h"

void rpc_test_init(rtos_intertile_t *intertile_ctx);
int call_1(int a, uint8_t *buffer, int len);

#endif /* RPC_TEST_H_ */
