// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <platform.h>
#include <xs1.h>

#include "FreeRTOS.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_conf.h"
#include "model_runner.h"
#include "person_detect_task.h"
#include "person_detect_model_data.h"
#include "person_detect_model_runner.h"

#ifdef OUTPUT_IMAGE_STREAM
#include "xscope.h"
#endif

#define IMAGE_SIZE (96 * 96)

typedef struct model_runner_args {
  QueueHandle_t input_queue;
  rtos_intertile_address_t *intertile_addr;
} model_runner_args_t;

typedef struct app_task_args {
  QueueHandle_t input_queue;
  rtos_intertile_address_t *intertile_addr;
  rtos_gpio_t *gpio_ctx;
} app_task_args_t;

#define TENSOR_ARENA_SIZE (1024 * 87)

static void person_detect_app_task(void *args) {
  app_task_args_t *targs = (app_task_args_t *)args;
  QueueHandle_t input_queue = targs->input_queue;
  rtos_intertile_address_t *adr = targs->intertile_addr;
  rtos_gpio_t *gpio_ctx = targs->gpio_ctx;
  uint8_t *img_buf = NULL;
  uint8_t *output_tensor;
  int output_tensor_len;
  uint8_t ai_img_buf[IMAGE_SIZE];

  rtos_gpio_port_id_t led_port = 0;
  uint32_t val = 0;
  int toggle = 0;

  led_port = rtos_gpio_port(PORT_LEDS);
  rtos_gpio_port_enable(gpio_ctx, led_port);
  rtos_gpio_port_out(gpio_ctx, led_port, val);

  while (1) {
    rtos_printf("Wait for next image...\n");
    xQueueReceive(input_queue, &img_buf, portMAX_DELAY);

    /* img_buf[i%2] contains the values we want to pass to the ai task */
    for (int i = 0; i < (IMAGE_SIZE * 2); i++) {
      if ((i % 2)) {
        ai_img_buf[i >> 1] = img_buf[i];
      }
    }
    vPortFree(img_buf);

#ifdef OUTPUT_IMAGE_STREAM
    taskENTER_CRITICAL();
    {
      xscope_bytes(INPUT_IMAGE, IMAGE_SIZE, (const unsigned char *)ai_img_buf);
    }
    taskEXIT_CRITICAL();
#endif

    rtos_intertile_tx(adr->intertile_ctx, adr->port, ai_img_buf, IMAGE_SIZE);

    output_tensor_len = rtos_intertile_rx(
        adr->intertile_ctx, adr->port, (void **)&output_tensor, portMAX_DELAY);
    rtos_printf("\nPerson score: %d\nNo person score: %d\n", output_tensor[0],
                output_tensor[1]);

#ifdef OUTPUT_IMAGE_STREAM
    taskENTER_CRITICAL();
    {
      xscope_bytes(OUTPUT_TENSOR, output_tensor_len,
                   (const unsigned char *)output_tensor);
    }
    taskEXIT_CRITICAL();
#endif

    val = ((toggle++) & 0x01) << 3;
    val |= (output_tensor[0] > output_tensor[1]);
    rtos_gpio_port_out(gpio_ctx, led_port, val);

    vPortFree(output_tensor);
  }
}

static void person_detect_runner_rx(void *args) {
  model_runner_args_t *targs = (model_runner_args_t *)args;
  QueueHandle_t q = targs->input_queue;
  rtos_intertile_address_t *adr = targs->intertile_addr;
  uint8_t *input_tensor;
  int input_tensor_len;

  while (1) {
    input_tensor_len = rtos_intertile_rx(adr->intertile_ctx, adr->port,
                                         (void **)&input_tensor, portMAX_DELAY);
    xQueueSend(q, &input_tensor, portMAX_DELAY);
  }
}

static void person_detect_task_runner(void *args) {
  model_runner_args_t *targs = (model_runner_args_t *)args;
  QueueHandle_t q = targs->input_queue;
  rtos_intertile_address_t *adr = targs->intertile_addr;
  size_t req_size = 0;
  uint8_t *interpreter_buf = NULL;
  int8_t *input_buffer = NULL;
  size_t input_size = 0;
  int8_t *output_buffer = NULL;
  size_t output_size = 0;
  model_runner_t *model_runner_ctx = NULL;
  uint8_t *tensor_arena = NULL;
  uint8_t *input_tensor;
  dispatcher_t *dispatcher;

  tensor_arena = pvPortMalloc(TENSOR_ARENA_SIZE);

  model_runner_init(tensor_arena, TENSOR_ARENA_SIZE);
  dispatcher = dispatcher_create();
  dispatcher_thread_init(dispatcher, appconfDISPATCHER_LENGTH,
                         appconfDISPATCHER_THREAD_COUNT,
                         appconfDISPATCHER_THREAD_PRIORITY);

  req_size = model_runner_buffer_size_get();
  interpreter_buf = pvPortMalloc(req_size);
  model_runner_ctx = pvPortMalloc(sizeof(model_runner_t));

  person_detect_model_runner_create(model_runner_ctx, interpreter_buf);
  model_runner_dispatcher_create(model_runner_ctx, dispatcher);
  if (model_runner_allocate(model_runner_ctx, person_detect_model_data) != 0) {
    rtos_printf("Invalid model provided!\n");

    vPortFree(tensor_arena);
    vPortFree(interpreter_buf);
    vPortFree(model_runner_ctx);

    vTaskDelete(NULL);
  }

  input_buffer = model_runner_input_buffer_get(model_runner_ctx);
  input_size = model_runner_input_size_get(model_runner_ctx);
  rtos_printf("image size if %d\n", input_size);
  output_buffer = model_runner_output_buffer_get(model_runner_ctx);
  output_size = model_runner_output_size_get(model_runner_ctx);

  while (1) {
    rtos_printf("Wait for input tensor...\n");
    xQueueReceive(q, &input_tensor, portMAX_DELAY);

    memcpy(input_buffer, input_tensor, input_size);
    vPortFree(input_tensor);

    rtos_printf("Running inference...\n");
    model_runner_invoke(model_runner_ctx);
    // model_runner_profiler_summary_print(model_runner_ctx);

    rtos_intertile_tx(adr->intertile_ctx, adr->port, output_buffer,
                      output_size);
  }
}

void person_detect_app_task_create(rtos_intertile_address_t *intertile_addr,
                                   rtos_gpio_t *gpio_ctx, unsigned priority,
                                   QueueHandle_t input_queue) {
  if (gpio_ctx != NULL) {
    app_task_args_t *args = pvPortMalloc(sizeof(app_task_args_t));

    configASSERT(args);

    args->input_queue = input_queue;
    args->intertile_addr = intertile_addr;
    args->gpio_ctx = gpio_ctx;

    xTaskCreate((TaskFunction_t)person_detect_app_task, "person_detect_app",
                RTOS_THREAD_STACK_SIZE(person_detect_app_task), args, priority,
                NULL);
  } else {
    rtos_printf("Invalid gpio ctx provided\n");
  }
}

void person_detect_model_runner_task_create(
    rtos_intertile_address_t *intertile_addr, unsigned priority) {
  model_runner_args_t *args = pvPortMalloc(sizeof(model_runner_args_t));
  QueueHandle_t input_queue = xQueueCreate(1, sizeof(int32_t *));

  configASSERT(args);
  configASSERT(input_queue);

  args->input_queue = input_queue;
  args->intertile_addr = intertile_addr;

  xTaskCreate((TaskFunction_t)person_detect_task_runner, "person_detect", 500,
              args, priority, NULL);

  xTaskCreate((TaskFunction_t)person_detect_runner_rx, "person_detect_rx",
              RTOS_THREAD_STACK_SIZE(person_detect_runner_rx), args,
              priority - 1, NULL);
}
