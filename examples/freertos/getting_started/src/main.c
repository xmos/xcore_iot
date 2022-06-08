// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xcore/chanend.h>

#include "FreeRTOS.h"
#include "task.h"

#include "rtos_printf.h"

#include "platform/platform_init.h"
#include "platform/driver_instances.h"

#define LED_PORT XS1_PORT_4C

void vApplicationMallocFailedHook(void) {
  rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
  for (;;)
    ;
}

void tile0_task(void *arg) {
  rtos_printf("Blinky task running from tile %d on core %d\n", THIS_XCORE_TILE,
              portGET_CORE_ID());

  for (;;) {
    rtos_gpio_port_out(gpio_ctx_t0, rtos_gpio_port(LED_PORT), 0x0001);
    rtos_printf("Hello from tile %d\n", THIS_XCORE_TILE);
    vTaskDelay(pdMS_TO_TICKS(500));
    rtos_gpio_port_out(gpio_ctx_t0, rtos_gpio_port(LED_PORT), 0x0000);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void tile1_task(void *arg) {
  rtos_printf("Blinky task running from tile %d on core %d\n", THIS_XCORE_TILE,
              portGET_CORE_ID());

  for (;;) {
    rtos_gpio_port_out(gpio_ctx_t1, rtos_gpio_port(LED_PORT), 0x0002);
    rtos_printf("Hello from tile %d\n", THIS_XCORE_TILE);
    vTaskDelay(pdMS_TO_TICKS(500));
    rtos_gpio_port_out(gpio_ctx_t1, rtos_gpio_port(LED_PORT), 0x0000);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

static void tile_common_init(chanend_t c) {
  platform_init(c);

#if ON_TILE(0)
  rtos_gpio_port_enable(gpio_ctx_t0, rtos_gpio_port(LED_PORT));
  xTaskCreate((TaskFunction_t)tile0_task, "tile0_task",
              RTOS_THREAD_STACK_SIZE(tile0_task), NULL,
              configMAX_PRIORITIES - 1, NULL);
#endif

#if ON_TILE(1)
  rtos_gpio_port_enable(gpio_ctx_t1, rtos_gpio_port(LED_PORT));
  xTaskCreate((TaskFunction_t)tile1_task, "tile1_task",
              RTOS_THREAD_STACK_SIZE(tile1_task), NULL,
              configMAX_PRIORITIES - 1, NULL);

#endif

  rtos_printf("Start scheduler on tile %d\n", THIS_XCORE_TILE);
  vTaskStartScheduler();
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {

  (void)c0;
  (void)c2;
  (void)c3;

  tile_common_init(c1);
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
  (void)c1;
  (void)c2;
  (void)c3;

  tile_common_init(c0);
}
#endif