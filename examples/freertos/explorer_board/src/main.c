// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCP.h"

/* Library headers */

/* App headers */
// #include "audio_pipeline.h"
// #include "network.h"
// #include "UDPCommandInterpreter.h"
// #include "thruput_test.h"
// #include "gpio_ctrl.h"
#include "app_conf.h"
#include "sntpd.h"
#include "fs_support.h"
// #include "tls_echo_demo.h"
// #include "tls_echo_server.h"
#include "tls_support.h"
// #include "http_demo.h"
// #include "mqtt_demo_client.h"
#include "mem_analysis.h"

#include "drivers/rtos/intertile/FreeRTOS/rtos_intertile.h"
#include "drivers/rtos/mic_array/FreeRTOS/rtos_mic_array.h"
#include "drivers/rtos/i2c/FreeRTOS/rtos_i2c_master.h"
#include "drivers/rtos/i2s/FreeRTOS/rtos_i2s_master.h"
#include "drivers/rtos/spi/FreeRTOS/rtos_spi_master.h"
#include "drivers/rtos/qspi_flash/FreeRTOS/rtos_qspi_flash.h"
#include "drivers/rtos/gpio/api/rtos_gpio.h"

static rtos_mic_array_t mic_array_ctx_s;
static rtos_i2c_master_t i2c_master_ctx_s;
static rtos_i2s_master_t i2s_master_ctx_s;
static rtos_spi_master_t spi_master_ctx_s;
static rtos_spi_master_device_t wifi_device_ctx_s;
static rtos_qspi_flash_t qspi_flash_ctx_s;
static rtos_gpio_t gpio_ctx_s;
static rtos_intertile_t intertile_ctx_s;
static rtos_intertile_t intertile2_ctx_s;

static rtos_mic_array_t *mic_array_ctx = &mic_array_ctx_s;
static rtos_i2c_master_t *i2c_master_ctx = &i2c_master_ctx_s;
static rtos_i2s_master_t *i2s_master_ctx = &i2s_master_ctx_s;
static rtos_spi_master_t *spi_master_ctx = &spi_master_ctx_s;
static rtos_spi_master_device_t *wifi_device_ctx = &wifi_device_ctx_s;
static rtos_qspi_flash_t *qspi_flash_ctx = &qspi_flash_ctx_s;
static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;
static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;
static rtos_intertile_t *intertile2_ctx = &intertile2_ctx_s;

chanend_t other_tile_c;

#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
#if XCOREAI_EXPLORER
#if ON_TILE(0)
__attribute__((section(".ExtMem_data")))
#endif
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif
#endif

void vApplicationDaemonTaskStartup( void )
{
    #if ON_TILE(0)
    {
        #if 0
        /* Initialize filesystem  */
    	filesystem_init();

        /* Initialize TLS  */
    	tls_platform_init();

        /* Create TLS echo demo */
    	tls_echo_demo_create( appconfTLS_ECHO_TASK_PRIORITY );

        /* Create TLS echo demo */
    	tls_echo_server_create( appconfTLS_ECHO_SERVER_PRIORITY );

        /* Create HTTP demo */
    	http_demo_create( appconfHTTP_TASK_PRIORITY );

        /* Create audio pipeline */
        audio_pipeline_create( appconfAUDIO_PIPELINE_TASK_PRIORITY );

        /* Create MQTT demo*/
        mqtt_demo_create( appconfMQTT_TASK_PRIORITY );
        #endif

        /* Create SNTPD */
        sntp_create( appconfSNTPD_TASK_PRIORITY );
    }
    #endif

    /* Create heap analysis task */
    mem_analysis_create( "heap" );

    vTaskDelete(NULL);
}

void vApplicationCoreInitHook(BaseType_t xCoreID)
{
#if ON_TILE(0)
    rtos_printf("Initializing tile 0, core %d on core %d\n", xCoreID, portGET_CORE_ID());
#endif

#if ON_TILE(1)
    rtos_printf("Initializing tile 1 core %d on core %d\n", xCoreID, portGET_CORE_ID());

    switch (xCoreID) {

    case 0:
        rtos_mic_array_interrupt_init(mic_array_ctx);
    }

#endif
}

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    for(;;);
}


#if ON_TILE(0)
void main_tile0(chanend_t c)
{
    // board_tile0_init(c, intertile_ctx, intertile2_ctx, mic_array_ctx, i2s_master_ctx, i2c_master_ctx, spi_master_ctx, qspi_flash_ctx, wifi_device_ctx, gpio_ctx);
    //     /* Create UDP CLI */
    //     vStartUDPCommandInterpreterTask( portTASK_STACK_DEPTH(vUDPCommandInterpreterTask), appconfCLI_UDP_PORT, appconfCLI_TASK_PRIORITY );
    //
    //     /* Create the thruput test */
    //     thruput_test_create( appconfTHRUPUT_TEST_TASK_PRIORITY );
    //
    //     /* Create the gpio control task */
    //     gpio_ctrl_create( appconfGPIO_TASK_PRIORITY );
    //
    //     /* Initialize WiFi */
    //     initalize_wifi();

    other_tile_c = c;

    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
                RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
                1,
                NULL);

    rtos_printf("start scheduler on tile 0\n");
    vTaskStartScheduler();

    return;
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c)
{
    // board_tile1_init(c, intertile_ctx, intertile2_ctx, mic_array_ctx, i2s_master_ctx, i2c_master_ctx, spi_master_ctx, qspi_flash_ctx, wifi_device_ctx, gpio_ctx);

    other_tile_c = c;

    xTaskCreate((TaskFunction_t) vApplicationDaemonTaskStartup,
                "vApplicationDaemonTaskStartup",
                RTOS_THREAD_STACK_SIZE(vApplicationDaemonTaskStartup),
                NULL,
                1,
                NULL);

    rtos_printf("start scheduler on tile 1\n");
    vTaskStartScheduler();

    return;
}
#endif
