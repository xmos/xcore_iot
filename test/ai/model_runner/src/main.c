// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdlib.h>
#include <string.h>

/* System headers */
#include <platform.h>
#include <xs1.h>

#include "FreeRTOS.h"
#include "task.h"

#include "rtos/drivers/qspi_flash/api/rtos_qspi_flash.h"
#include "rtos/drivers/swmem/api/rtos_swmem.h"
#include "xcore_device_memory.h"
#include "model_runner.h"
#include "dispatcher.h"

#include "app_conf.h"
#include "test_model_data.h"
#include "test_model_runner.h"

#if USE_SWMEM
static rtos_qspi_flash_t qspi_flash_ctx_s;
static rtos_qspi_flash_t *qspi_flash_ctx = &qspi_flash_ctx_s;
#endif

dispatcher_t *dispatcher_setup() {
  dispatcher_t *dispatcher;

  rtos_printf("Setting up dispatcher\n");
  dispatcher = dispatcher_create();
#if appconfUSE_ISR_DISPATCHER
  dispatcher_isr_init(dispatcher, appconfDISPATCHER_CORE_MAP);
#elif appconfUSE_THREAD_DISPATCHER
  dispatcher_thread_init(dispatcher, appconfDISPATCHER_LENGTH,
                         appconfDISPATCHER_THREAD_COUNT,
                         appconfDISPATCHER_THREAD_PRIORITY);
#endif

  return dispatcher;
}

void dispatcher_teardown(dispatcher_t *queue) {
  rtos_printf("Tearing down dispatcher\n");
  dispatcher_delete(queue);
}

void model_runner_task(void *arg) {
  size_t req_size = 0;
  uint8_t *tensor_arena = NULL;
  uint8_t *interpreter_buf = NULL;
  int8_t *input_buffer;
  size_t input_size;
  int8_t *output_buffer;
  size_t output_size;
  model_runner_t *model_runner_ctx = NULL;

  // initialize model runner global state
  tensor_arena = pvPortMalloc(appconfTENSOR_ARENA_SIZE);
  memset(tensor_arena, 0, appconfTENSOR_ARENA_SIZE);
  model_runner_init(tensor_arena, appconfTENSOR_ARENA_SIZE);

  req_size = model_runner_buffer_size_get();
  interpreter_buf = pvPortMalloc(req_size);
  model_runner_ctx = pvPortMalloc(sizeof(model_runner_t));

  // create dispatch queue
  dispatcher_t *dispatcher = dispatcher_setup();

  // setup model runner
  test_model_runner_create(model_runner_ctx, NULL);
  model_runner_dispatcher_create(model_runner_ctx, dispatcher);
  model_runner_allocate(model_runner_ctx, test_model_data);
  input_buffer = model_runner_input_buffer_get(model_runner_ctx);
  input_size = model_runner_input_size_get(model_runner_ctx);
  output_buffer = model_runner_output_buffer_get(model_runner_ctx);
  output_size = model_runner_output_size_get(model_runner_ctx);

  // set input & output tensors to all zeros
  memset(input_buffer, 0, input_size);
  memset(output_buffer, 0, output_size);

  // Run inference, and report any error
  rtos_printf("Running inference...\n");
  model_runner_invoke(model_runner_ctx);

  // print output buffer
  rtos_printf("Output buffer:");
  for (size_t i = 0; i < output_size; i++) {
    rtos_printf(" %d", (int)output_buffer[i]);
  }
  rtos_printf("\n");

  // print profiler report
  rtos_printf("Profiler report:\n");
  model_runner_profiler_summary_print(model_runner_ctx);

  dispatcher_teardown(dispatcher);
  vPortFree(interpreter_buf);
  vPortFree(model_runner_ctx);
  vPortFree(tensor_arena);

  _Exit(0);
}

void vApplicationMallocFailedHook(void) {
  rtos_printf("Malloc Failed!");
  // configASSERT(0);
  for (;;)
    ;
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
  rtos_printf("Stack Overflow! %s", pcTaskName);
  // configASSERT(0);
  for (;;)
    ;
}

void vApplicationDaemonTaskStartup(void *arg) {
#if USE_SWMEM

  rtos_qspi_flash_init(qspi_flash_ctx, XS1_CLKBLK_2, PORT_SQI_CS, PORT_SQI_SCLK,
                       PORT_SQI_SIO,

                       /** Derive QSPI clock from the 600 MHz xcore clock **/
                       qspi_io_source_clock_xcore,

                       /** Full speed clock configuration **/
                       5, // 600 MHz / (2*5) -> 60 MHz,
                       1, qspi_io_sample_edge_rising, 0,

                       /** SPI read clock configuration **/
                       12, // 600 MHz / (2*12) -> 25 MHz
                       0, qspi_io_sample_edge_falling, 0,

                       qspi_flash_page_program_1_1_1);

  rtos_printf("Starting QSPI flash driver\n");
  rtos_qspi_flash_start(qspi_flash_ctx, configMAX_PRIORITIES - 1);

  rtos_printf("Starting SwMem task\n");
  swmem_setup(qspi_flash_ctx, appconfSWMEM_TASK_PRIORITY);
#endif
  rtos_printf("Starting model runner task\n");
  xTaskCreate((TaskFunction_t)model_runner_task, "model_runner_task",
              appconfMODEL_RUNNER_TASK_STACK_SIZE, NULL,
              appconfMODEL_RUNNER_TASK_PRIORITY, NULL);
  vTaskDelete(NULL);
}

int main(void) {
  xTaskCreate((TaskFunction_t)vApplicationDaemonTaskStartup,
              "vApplicationDaemonTaskStartup",
              RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup), NULL,
              appconfSTARTUP_TASK_PRIORITY, NULL);

  vTaskStartScheduler();
  return 0;
}