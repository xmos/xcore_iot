// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"

/* Library headers */
#include "rtos/drivers/intertile/api/rtos_intertile.h"
#include "rtos/drivers/qspi_flash/api/rtos_qspi_flash.h"

/* App headers */
#include "app_conf.h"
#include "board_init.h"
#include "cifar10_task.h"
#include "fs_support.h"

static rtos_intertile_t intertile_ctx_s;
static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;

static rtos_intertile_address_t cifar10_addr_s;
static rtos_intertile_address_t *cifar10_addr = &cifar10_addr_s;

static rtos_qspi_flash_t qspi_flash_ctx_s;
static rtos_qspi_flash_t *qspi_flash_ctx = &qspi_flash_ctx_s;

#if USE_SWMEM
#if ON_TILE(1)
#include "rtos/drivers/swmem/api/rtos_swmem.h"
#include "xcore_device_memory.h"
#endif
#endif

void vApplicationMallocFailedHook(void) {
  rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
  for (;;)
    ;
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
  rtos_printf("\nStack Overflow!!! %d %s!\n", THIS_XCORE_TILE, pcTaskName);
  configASSERT(0);
}

void vApplicationCoreInitHook(BaseType_t xCoreID) {
#if ON_TILE(0)
  rtos_printf("Initializing tile 0, core %d on core %d\n", xCoreID,
              portGET_CORE_ID());
#endif

#if ON_TILE(1)
  rtos_printf("Initializing tile 1 core %d on core %d\n", xCoreID,
              portGET_CORE_ID());
#endif
}

void vApplicationDaemonTaskStartup(void *arg) {
  rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE,
              portGET_CORE_ID());

  rtos_intertile_start(intertile_ctx);

  rtos_qspi_flash_rpc_config(qspi_flash_ctx, QSPI_RPC_PORT,
                             QSPI_RPC_HOST_TASK_PRIORITY);

  cifar10_addr->intertile_ctx = intertile_ctx;
  cifar10_addr->port = CIFAR10_PORT;

#if ON_TILE(0)
  {
    rtos_printf("Starting QSPI flash driver\n");
    rtos_qspi_flash_start(qspi_flash_ctx, configMAX_PRIORITIES - 1);

    rtos_printf("Starting filesystem\n");
    rtos_fatfs_init(qspi_flash_ctx);

    cifar10_app_task_create(cifar10_addr, appconfCIFAR10_TASK_PRIORITY);
  }
#endif

#if ON_TILE(1)
  {
#if USE_SWMEM
    rtos_printf("Starting swmem task\n");
    swmem_setup(qspi_flash_ctx, appconfSWMEM_TASK_PRIORITY);
#endif
    rtos_printf("Starting model runner task\n");
    cifar10_model_runner_task_create(cifar10_addr,
                                     appconfCIFAR10_TASK_PRIORITY);
  }
#endif

  vTaskDelete(NULL);
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
  (void)c0;
  board_tile0_init(c1, intertile_ctx, qspi_flash_ctx);
  (void)c2;
  (void)c3;

  xTaskCreate((TaskFunction_t)vApplicationDaemonTaskStartup,
              "vApplicationDaemonTaskStartup",
              RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup), NULL,
              appconfSTARTUP_TASK_PRIORITY, NULL);

  rtos_printf("start scheduler on tile 0\n");
  vTaskStartScheduler();

  return;
}

/* Workaround.
 * extmem builds will fail to load since tile 1 was not given control in the
 * tile 0 build
 * TODO: Find a better solution
 */
#if USE_EXTMEM
__attribute__((section(".ExtMem_code"))) void main_tile1(chanend_t c0,
                                                         chanend_t c1,
                                                         chanend_t c2,
                                                         chanend_t c3) {
  return;
}
#endif
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
  board_tile1_init(c0, intertile_ctx, qspi_flash_ctx);
  (void)c1;
  (void)c2;
  (void)c3;

  xTaskCreate((TaskFunction_t)vApplicationDaemonTaskStartup,
              "vApplicationDaemonTaskStartup",
              RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup), NULL,
              appconfSTARTUP_TASK_PRIORITY, NULL);

  rtos_printf("start scheduler on tile 1\n");
  vTaskStartScheduler();

  return;
}
#endif
