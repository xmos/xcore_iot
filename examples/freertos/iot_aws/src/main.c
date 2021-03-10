// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCP.h"

/* Library headers */
#include "rtos/drivers/gpio/api/rtos_gpio.h"
#include "rtos/drivers/intertile/api/rtos_intertile.h"
#include "rtos/drivers/mic_array/api/rtos_mic_array.h"
#include "rtos/drivers/qspi_flash/api/rtos_qspi_flash.h"
#include "rtos/drivers/spi/api/rtos_spi_master.h"

/* App headers */
#include "app_conf.h"
#include "board_init.h"
#include "fs_support.h"
#include "mem_analysis.h"
#include "mqtt_demo_client.h"
#include "network_setup.h"
#include "sntpd.h"
#include "tls_support.h"

static rtos_intertile_t intertile_ctx_s;
static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;

#if ON_TILE(0)
static rtos_spi_master_t spi_master_ctx_s;
static rtos_spi_master_device_t wifi_device_ctx_s;
static rtos_qspi_flash_t qspi_flash_ctx_s;
static rtos_gpio_t gpio_ctx_s;

static rtos_spi_master_t *spi_master_ctx = &spi_master_ctx_s;
static rtos_spi_master_device_t *wifi_device_ctx = &wifi_device_ctx_s;
static rtos_qspi_flash_t *qspi_flash_ctx = &qspi_flash_ctx_s;
static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;
#endif

void vApplicationMallocFailedHook(void) {
  rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
  for (;;)
    ;
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
  rtos_printf(
      "\n****************************\nStack OF %d "
      "%s!\n****************************\\n",
      THIS_XCORE_TILE, pcTaskName);
  for (;;)
    ;
}

void vApplicationDaemonTaskStartup(void) {
  rtos_printf("Starting intertile driver\n");
  rtos_intertile_start(intertile_ctx);

#if ON_TILE(0)
  /* Initialize drivers  */
  rtos_printf("Starting GPIO driver\n");
  rtos_gpio_start(gpio_ctx);

  rtos_printf("Starting SPI driver\n");
  rtos_spi_master_start(spi_master_ctx, configMAX_PRIORITIES - 1);

  rtos_printf("Starting QSPI flash driver\n");
  rtos_qspi_flash_start(qspi_flash_ctx, configMAX_PRIORITIES - 1);

  /* Initialize filesystem  */
  rtos_fatfs_init(qspi_flash_ctx);

  /* Initialize WiFi */
  wifi_start(wifi_device_ctx, gpio_ctx);

  /* Initialize TLS  */
  tls_platform_init();

  /* Create SNTPD */
  sntp_create(appconfSNTPD_TASK_PRIORITY);

  /* Create MQTT demo*/
  mqtt_demo_create(gpio_ctx, appconfMQTT_TASK_PRIORITY);

  /* Create heap analysis task */
  mem_analysis_create("heap");
#endif

  vTaskDelete(NULL);
}

void vApplicationCoreInitHook(BaseType_t xCoreID) {
  rtos_printf("Initializing tile 0, core %d on core %d\n", xCoreID,
              portGET_CORE_ID());
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
  (void)c0;
  board_tile0_init(c1, intertile_ctx, spi_master_ctx, qspi_flash_ctx,
                   wifi_device_ctx, gpio_ctx);
  (void)c2;
  (void)c3;

  xTaskCreate((TaskFunction_t)vApplicationDaemonTaskStartup,
              "vApplicationDaemonTaskStartup", configMINIMAL_STACK_SIZE * 6,
              NULL, appconfSTARTUP_TASK_PRIORITY, NULL);

  rtos_printf("start scheduler on tile 0\n");
  vTaskStartScheduler();

  return;
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
  board_tile1_init(c0, intertile_ctx);
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
