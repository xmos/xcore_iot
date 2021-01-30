// Copyright (c) 2021, XMOS Ltd, All rights reserved

#include <platform.h>
#include <xs1.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs_support.h"
#include "app_conf.h"
#include "cifar10_task.h"
#include "cifar10_model_data.h"
#include "cifar10_model_runner.h"
#include "model_runner/api/model_runner.h"

#define TENSOR_ARENA_SIZE 58000

static int argmax(const int8_t *A, const int N) {
  int m = 0;

  for (int i = 1; i < N; i++) {
    if (A[i] > A[m]) {
      m = i;
    }
  }

  return m;
}

static const char* test_input_files[] = {
	"airplane.bin",
    "bird.bin",
    "cat.bin",
    "deer.bin",
    "frog.bin",
    "horse.bin",
    "truck.bin"
};

static void cifar10_task_app(void *args)
{
    rtos_intertile_address_t *adr = (rtos_intertile_address_t*)args;
    FIL current_file;
    unsigned int file_size;
    uint8_t *data = NULL;
    FRESULT result;
    unsigned int bytes_read = 0;
    uint8_t *output_tensor;
    int output_tensor_len;
    char classification[12] = {0};

    while(1)
    {
        for( int i=0; i<( sizeof( test_input_files )/sizeof( test_input_files[0] ) ); i++ )
        {
            if( get_file(test_input_files[i], &current_file, &file_size ) == 0 )
            {
                rtos_printf("Failed to load file %s\n", test_input_files[i]);
                continue;
            }

		    data = pvPortMalloc( sizeof( unsigned char ) * file_size );

            configASSERT( data != NULL); /* Failed to allocate memory for file data */

            result = f_read( &current_file, data, file_size, &bytes_read );

            rtos_intertile_tx(adr->intertile_ctx,
                              adr->port,
                              data,
                              file_size);

            vPortFree(data);

            output_tensor_len = rtos_intertile_rx(adr->intertile_ctx,
                                                  adr->port,
                                                  (void**) &output_tensor,
                                                  portMAX_DELAY);

            switch (argmax(output_tensor, 10)) {
            case 0:
                snprintf(classification, 9, "Airplane");
                break;
            case 1:
                snprintf(classification, 11, "Automobile");
                break;
            case 2:
                snprintf(classification, 5, "Bird");
                break;
            case 3:
                snprintf(classification, 4, "Cat");
                break;
            case 4:
                snprintf(classification, 5, "Deer");
                break;
            case 5:
                snprintf(classification, 4, "Dog");
                break;
            case 6:
                snprintf(classification, 5, "Frog");
                break;
            case 7:
                snprintf(classification, 6, "Horse");
                break;
            case 8:
                snprintf(classification, 5, "Ship");
                break;
            case 9:
                snprintf(classification, 6, "Truck");
                break;
            default:
                break;
            }
            rtos_printf("Classification of file %s is %s\n", test_input_files[i], classification);
            vPortFree(output_tensor);
        }
        rtos_printf("All files complete.  Repeating in 5 seconds...");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}


static void cifar10_task_runner(void *args)
{
    rtos_intertile_address_t *adr = (rtos_intertile_address_t*)args;
    size_t req_size = 0;
    uint8_t *interpreter_buf = NULL;
    int8_t *input_buffer = NULL;
    size_t input_size = 0;
    int8_t *output_buffer = NULL;
    size_t output_size = 0;
    model_runner_t *model_runner_ctx = NULL;
    uint8_t *tensor_arena = NULL;
    uint8_t *input_tensor;
    int input_tensor_len;

    tensor_arena = pvPortMalloc(TENSOR_ARENA_SIZE);

    model_runner_init(tensor_arena, TENSOR_ARENA_SIZE);

    req_size = model_runner_buffer_size_get();
    interpreter_buf = pvPortMalloc(req_size);
    model_runner_ctx = pvPortMalloc(sizeof(model_runner_t));

    cifar10_model_runner_create(model_runner_ctx, interpreter_buf);
    model_runner_allocate(model_runner_ctx, cifar10_model_data);
    input_buffer = model_runner_input_buffer_get(model_runner_ctx);
    input_size = model_runner_input_size_get(model_runner_ctx);
    output_buffer = model_runner_output_buffer_get(model_runner_ctx);
    output_size = model_runner_output_size_get(model_runner_ctx);

    while(1)
    {
        input_tensor_len = rtos_intertile_rx(adr->intertile_ctx,
                                              adr->port,
                                              (void**) &input_tensor,
                                              portMAX_DELAY);

        memcpy(input_buffer, input_tensor, input_size);
        vPortFree(input_tensor);
        printf("Running inference...\n");

        model_runner_invoke(model_runner_ctx);

        model_runner_profiler_summary_print(model_runner_ctx);

        rtos_intertile_tx(adr->intertile_ctx,
                          adr->port,
                          output_buffer,
                          output_size);
    }
}

void cifar10_app_task_create(
        rtos_intertile_address_t *intertile_addr,
        unsigned priority)
{
    xTaskCreate((TaskFunction_t) cifar10_task_app,
                "cifar10",
                RTOS_THREAD_STACK_SIZE(cifar10_task_app),
                intertile_addr,
                priority,
                NULL);
}

void cifar10_model_runner_task_create(
        rtos_intertile_address_t *intertile_addr,
        unsigned priority)
{
    xTaskCreate((TaskFunction_t) cifar10_task_runner,
                "cifar10",
                1000,
                intertile_addr,
                priority,
                NULL);
}
