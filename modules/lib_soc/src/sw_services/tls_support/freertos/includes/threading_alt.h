// Copyright (c) 2020, XMOS Ltd, All rights reserved

#ifndef THREADING_ALT_H_
#define THREADING_ALT_H_

#include "FreeRTOS.h"
#include "semphr.h"

typedef struct mbedtls_threading_mutex_t
{
    SemaphoreHandle_t mutex;
    char is_valid;
} mbedtls_threading_mutex_t;

void mbedtls_threading_set_alt( void ( * mutex_init )( mbedtls_threading_mutex_t * ),
							    void ( * mutex_free )( mbedtls_threading_mutex_t * ),
							    int ( * mutex_lock )( mbedtls_threading_mutex_t * ),
							    int ( * mutex_unlock )( mbedtls_threading_mutex_t * ) );

#endif /* THREADING_ALT_H_ */
