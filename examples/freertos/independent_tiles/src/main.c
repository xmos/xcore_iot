// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "queue.h"

#include "rtos_printf.h"

#include <xcore/channel.h>

#include "app_conf.h"
#include "driver_instances.h"
#include "example_pipeline/example_pipeline.h"

#if appconfWIFI_TEST && appconfQSPI_TEST
#error Cannot test the QSPI when the WIFI test is enabled
#endif

#if appconfRPC_TEST
#include "rpc_test/rpc_test.h"
#endif

#if appconfINTERTILE_TEST
#include "intertile_stress_test/intertile_stress_test.h"
#endif

#if appconfGPIO_TEST
#include "gpio_test/gpio_test.h"
#endif

#if appconfWIFI_TEST
#include "wifi_test/wifi_test.h"
#include "fs_support.h"
#endif

#if appconfSW_MEM_TEST
#include "swmem_test/swmem_test.h"
#endif

#include "platform_init.h"

chanend_t other_tile_c;

void vApplicationMallocFailedHook(void) {
  rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
  for (;;)
    ;
}

void startup_task(void *arg) {
  uint32_t dac_configured;

  rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE,
              portGET_CORE_ID());

#if appconfSW_MEM_TEST && ON_TILE(SWMEM_TILE)
  swmem_test_run();
#endif

  platform_start();

#if ON_TILE(I2C_TILE)
  {
    int dac_init(rtos_i2c_master_t * i2c_ctx);

    if (dac_init(i2c_master_ctx) == 0) {
      rtos_printf("DAC initialization succeeded\n");
      dac_configured = 1;
    } else {
      rtos_printf("DAC initialization failed\n");
      dac_configured = 0;
    }
    chan_out_byte(other_tile_c, dac_configured);
  }
#else
  { dac_configured = chan_in_byte(other_tile_c); }
#endif

  chanend_free(other_tile_c);

  if (!dac_configured) {
    vTaskDelete(NULL);
  }

#if ON_TILE(PIPELINE_TILE)
  { example_pipeline_init(mic_array_ctx, i2s_ctx); }
#endif

#if appconfRPC_TEST
  { rpc_test_init(intertile1_ctx); }
#endif

#if appconfINTERTILE_TEST
  { intertile_stress_test_start(intertile1_ctx, intertile2_ctx); }
#endif

#if appconfGPIO_TEST && ON_TILE(GPIO_TILE)
  { gpio_test_start(gpio_ctx); }
#endif

#if appconfWIFI_TEST && ON_TILE(WIFI_TILE)
  {
    rtos_fatfs_init(qspi_flash_ctx);
    wifi_test_start(wifi_device_ctx, gpio_ctx);
  }
#endif

#if appconfQSPI_TEST && ON_TILE(QSPI_FLASH_TILE)
  {
    const char test_str[] = "hello, world\n";
    const int len = strlen(test_str) + 1;
    uint8_t data[len];
    int erase = 0;

    rtos_printf("The QSPI flash size is %u\n",
                rtos_qspi_flash_size_get(qspi_flash_ctx));

    rtos_qspi_flash_read(qspi_flash_ctx, data, 0, len);
    if (data[0] != 0xFF) {
      rtos_printf("First read: %s", data);
      erase = 1;
    } else {
      rtos_printf("First read appears empty\n");
    }

    rtos_qspi_flash_lock(qspi_flash_ctx);
    rtos_qspi_flash_erase(qspi_flash_ctx, 0, len);
    rtos_qspi_flash_write(qspi_flash_ctx, (const uint8_t *)test_str, 0, len);
    rtos_qspi_flash_read(qspi_flash_ctx, data, 0, len);
    rtos_printf("Second read: %s", data);

    if (erase) {
      rtos_printf("Starting chip erase\n");
      rtos_qspi_flash_erase(qspi_flash_ctx, 0,
                            rtos_qspi_flash_size_get(qspi_flash_ctx));
    }
    rtos_qspi_flash_unlock(qspi_flash_ctx);
  }
#endif

  vTaskDelete(NULL);
}

static void tile_common_init(chanend_t c) {
#if appconfSW_MEM_TEST && ON_TILE(SWMEM_TILE)
  swmem_test_init();
#endif

  other_tile_c = c;

  platform_init(other_tile_c);

  xTaskCreate((TaskFunction_t)startup_task, "startup_task",
              RTOS_THREAD_STACK_SIZE(startup_task), NULL,
              appconfSTARTUP_TASK_PRIORITY, NULL);

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