// Copyright (c) 2021, XMOS Ltd, All rights reserved

#ifndef THREADING_ALT_H_
#define THREADING_ALT_H_

#include "FreeRTOS.h"
#include "semphr.h"

typedef struct mbedtls_threading_mutex_t
{
    SemaphoreHandle_t mutex;
    char is_valid;
} mbedtls_threading_mutex_t;

#endif /* THREADING_ALT_H_ */
