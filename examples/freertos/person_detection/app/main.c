// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"

/* Library headers */
#include "rtos/drivers/intertile/api/rtos_intertile.h"
#include "rtos/drivers/gpio/api/rtos_gpio.h"
#include "rtos/drivers/spi/api/rtos_spi_master.h"
#include "rtos/drivers/i2c/api/rtos_i2c_master.h"

/* App headers */
#include "app_conf.h"
#include "board_init.h"
#include "spi_camera.h"
#include "person_detect_task.h"

#if ON_TILE(0)
static rtos_i2c_master_t i2c_master_ctx_s;
static rtos_spi_master_t spi_master_ctx_s;
static rtos_spi_master_device_t ov_device_ctx_s;
static rtos_gpio_t gpio_ctx_s;

static rtos_i2c_master_t *i2c_master_ctx = &i2c_master_ctx_s;
static rtos_spi_master_t *spi_master_ctx = &spi_master_ctx_s;
static rtos_spi_master_device_t *ov_device_ctx = &ov_device_ctx_s;
static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;
#endif

static rtos_intertile_t intertile_ctx_s;
static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;

static rtos_intertile_address_t person_detect_addr_s;
static rtos_intertile_address_t *person_detect_addr = &person_detect_addr_s;

void vApplicationMallocFailedHook( void )
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    for(;;);
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char* pcTaskName )
{
    rtos_printf("\nStack Overflow!!! %d %s!\n", THIS_XCORE_TILE, pcTaskName);
    configASSERT(0);
}

void vApplicationCoreInitHook(BaseType_t xCoreID)
{
#if ON_TILE(0)
    rtos_printf("Initializing tile 0, core %d on core %d\n", xCoreID, portGET_CORE_ID());
#endif

#if ON_TILE(1)
    rtos_printf("Initializing tile 1 core %d on core %d\n", xCoreID, portGET_CORE_ID());
#endif
}

void vApplicationDaemonTaskStartup(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    rtos_intertile_start(
            intertile_ctx);

    person_detect_addr->intertile_ctx = intertile_ctx;
    person_detect_addr->port = PERSON_DETECT_PORT;

    #if ON_TILE(0)
    {
        rtos_printf("Starting GPIO driver\n");
        rtos_gpio_start(gpio_ctx);

        rtos_printf("Starting SPI driver\n");
        rtos_spi_master_start(spi_master_ctx, configMAX_PRIORITIES-1);

        rtos_printf("Starting i2c driver\n");
        rtos_i2c_master_start(i2c_master_ctx);

        QueueHandle_t qcam2ai = xQueueCreate( 1, sizeof( uint8_t* ) );

        if( create_spi_camera_to_queue( ov_device_ctx, i2c_master_ctx, appconfSPI_CAMERA_TASK_PRIORITY, qcam2ai )
            == pdTRUE )
        {
            rtos_printf("Starting person detect app task\n");
            person_detect_app_task_create(person_detect_addr, gpio_ctx, appconfPERSON_DETECT_TASK_PRIORITY, qcam2ai);
        }
        else
        {
            debug_printf("Camera setup failed...\n");
        }
    }
    #endif

    #if ON_TILE(1)
    {
        rtos_printf("Starting model runner task\n");
        person_detect_model_runner_task_create(person_detect_addr, appconfPERSON_DETECT_TASK_PRIORITY);
    }
    #endif

    vTaskDelete(NULL);
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    board_tile0_init(c1, intertile_ctx, i2c_master_ctx, spi_master_ctx, ov_device_ctx, gpio_ctx);
    (void) c2;
    (void) c3;

    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
                RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("start scheduler on tile 0\n");
    vTaskStartScheduler();

    return;
}

/* Workaround.
 * extmem builds will fail to load since tile 1 was not given control in the tile 0 build
 * TODO: Find a better solution
 */
#if USE_EXTMEM
__attribute__((section(".ExtMem_code")))
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    return;
}
#endif
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    board_tile1_init(c0, intertile_ctx);
    (void) c1;
    (void) c2;
    (void) c3;

    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
                RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("start scheduler on tile 1\n");
    vTaskStartScheduler();

    return;
}
#endif
