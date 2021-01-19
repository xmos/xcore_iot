// Copyright (c) 2021, XMOS Ltd, All rights reserved

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
#include "drivers/rtos/intertile/FreeRTOS/rtos_intertile.h"
#include "drivers/rtos/mic_array/FreeRTOS/rtos_mic_array.h"
#include "drivers/rtos/i2c/FreeRTOS/rtos_i2c_master.h"
#include "drivers/rtos/i2s/FreeRTOS/rtos_i2s_master.h"
#include "drivers/rtos/spi/FreeRTOS/rtos_spi_master.h"
#include "drivers/rtos/qspi_flash/FreeRTOS/rtos_qspi_flash.h"
#include "drivers/rtos/gpio/api/rtos_gpio.h"

/* App headers */
#include "app_conf.h"
#include "board_init.h"
// #include "network.h"
// #include "UDPCommandInterpreter.h"
// #include "thruput_test.h"
// #include "gpio_ctrl.h"
#include "sntpd.h"
#include "fs_support.h"
// #include "tls_echo_demo.h"
// #include "tls_echo_server.h"
#include "tls_support.h"
// #include "http_demo.h"
// #include "mqtt_demo_client.h"
#include "mem_analysis.h"

#if ON_TILE(0)

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
__attribute__((section(".ExtMem_data")))
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif
#endif

void vApplicationDaemonTaskStartup( void )
{
    uint32_t dac_configured;
    rtos_intertile_start(intertile_ctx);

    rtos_i2c_master_rpc_config(i2c_master_ctx, I2C_MASTER_RPC_PORT, I2C_MASTER_RPC_HOST_TASK_PRIORITY);

    // rtos_printf("Starting GPIO driver\n");
    // rtos_gpio_start(gpio_ctx);
    //
    // rtos_printf("Starting SPI driver\n");
    // rtos_spi_master_start(spi_master_ctx, configMAX_PRIORITIES-1);
    //
    // rtos_printf("Starting QSPI flash driver\n");
    // rtos_qspi_flash_start(qspi_flash_ctx, configMAX_PRIORITIES-1);

    rtos_printf("Starting i2c driver\n");
    rtos_i2c_master_start(i2c_master_ctx);

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

        /* Create MQTT demo*/
        mqtt_demo_create( appconfMQTT_TASK_PRIORITY );

        /* Create UDP CLI */
        vStartUDPCommandInterpreterTask( portTASK_STACK_DEPTH(vUDPCommandInterpreterTask), appconfCLI_UDP_PORT, appconfCLI_TASK_PRIORITY );

        /* Create the thruput test */
        thruput_test_create( appconfTHRUPUT_TEST_TASK_PRIORITY );

        /* Create the gpio control task */
        gpio_ctrl_create( appconfGPIO_TASK_PRIORITY );

        /* Initialize WiFi */
        initalize_wifi();
        #endif

    /* Create SNTPD */
    sntp_create( appconfSNTPD_TASK_PRIORITY );

    /* Create heap analysis task */
    mem_analysis_create( "heap" );

    vTaskDelete(NULL);
}

void vApplicationCoreInitHook(BaseType_t xCoreID)
{
    rtos_printf("Initializing tile 0, core %d on core %d\n", xCoreID, portGET_CORE_ID());
}


void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    board_tile0_init(c1, intertile_ctx, i2c_master_ctx, spi_master_ctx, qspi_flash_ctx, wifi_device_ctx, gpio_ctx);
    (void) c2;
    (void) c3;

    other_tile_c = c1;

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

#endif /* ON_TILE(0) */
