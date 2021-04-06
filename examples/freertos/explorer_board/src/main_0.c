// Copyright 2019-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

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
#include "rtos/drivers/i2c/api/rtos_i2c_master.h"
#include "rtos/drivers/i2s/api/rtos_i2s.h"
#include "rtos/drivers/intertile/api/rtos_intertile.h"
#include "rtos/drivers/mic_array/api/rtos_mic_array.h"
#include "rtos/drivers/qspi_flash/api/rtos_qspi_flash.h"
#include "rtos/drivers/spi/api/rtos_spi_master.h"

/* App headers */
#include "app_conf.h"
#include "board_init.h"
#include "UDPCommandInterpreter.h"
#include "thruput_test.h"
#include "sntpd.h"
#include "fs_support.h"
#include "tls_echo_demo.h"
#include "tls_echo_server.h"
#include "tls_support.h"
#include "http_demo.h"
#include "mqtt_demo_client.h"
#include "mem_analysis.h"
#include "network_setup.h"

#include "queue_to_tcp_stream.h"
#include "example_pipeline/example_pipeline.h"

#if ON_TILE(0)

static rtos_i2c_master_t i2c_master_ctx_s;
static rtos_spi_master_t spi_master_ctx_s;
static rtos_spi_master_device_t wifi_device_ctx_s;
static rtos_qspi_flash_t qspi_flash_ctx_s;
static rtos_gpio_t gpio_ctx_s;
static rtos_intertile_t intertile_ctx_s;

static rtos_i2c_master_t *i2c_master_ctx = &i2c_master_ctx_s;
static rtos_spi_master_t *spi_master_ctx = &spi_master_ctx_s;
static rtos_spi_master_device_t *wifi_device_ctx = &wifi_device_ctx_s;
static rtos_qspi_flash_t *qspi_flash_ctx = &qspi_flash_ctx_s;
static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;
static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;

chanend_t other_tile_c;

#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
#if XCOREAI_EXPLORER
__attribute__((section(".ExtMem_data")))
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif
#endif

void vApplicationDaemonTaskStartup( void )
{
    rtos_printf("Starting intertile driver\n");
    rtos_intertile_start(intertile_ctx);

    /* Initialize rpc config  */
    rtos_i2c_master_rpc_config(i2c_master_ctx, I2C_MASTER_RPC_PORT, I2C_MASTER_RPC_HOST_TASK_PRIORITY);
    rtos_gpio_rpc_config(gpio_ctx, GPIO_RPC_PORT, GPIO_RPC_HOST_TASK_PRIORITY);

    /* Initialize drivers  */
    rtos_printf("Starting GPIO driver\n");
    rtos_gpio_start(gpio_ctx);

    rtos_printf("Starting SPI driver\n");
    rtos_spi_master_start(spi_master_ctx, configMAX_PRIORITIES-1);

    rtos_printf("Starting QSPI flash driver\n");
    rtos_qspi_flash_start(qspi_flash_ctx, configMAX_PRIORITIES-1);

    rtos_printf("Starting i2c driver\n");
    rtos_i2c_master_start(i2c_master_ctx);

    /* Initialize filesystem  */
    rtos_fatfs_init(qspi_flash_ctx);

    /* Initialize WiFi */
    wifi_start(wifi_device_ctx, gpio_ctx);

    /* Create intertile audio frame receiver */
    intertile_pipeline_to_tcp_create( intertile_ctx, INTERTILE_AUDIOPIPELINE_PORT, INTERTILE_AUDIOPIPELINE_TASK_PRIORITY );

    /* Initialize TLS  */
	tls_platform_init();

    /* Create the thruput test */
    thruput_test_create( appconfTHRUPUT_TEST_TASK_PRIORITY );

    /* Create SNTPD */
    sntp_create( appconfSNTPD_TASK_PRIORITY );

    /* Create UDP CLI */
    vStartUDPCommandInterpreterTask( portTASK_STACK_DEPTH( vUDPCommandInterpreterTask ), appconfCLI_UDP_PORT, appconfCLI_TASK_PRIORITY );
    vInitializeUDPIntertile( intertile_ctx, CLI_RPC_PROCESS_COMMAND_PORT );

    /* Create TLS echo demo */
	tls_echo_demo_create( appconfTLS_ECHO_TASK_PRIORITY );

    /* Create TLS echo demo */
	tls_echo_server_create( appconfTLS_ECHO_SERVER_PRIORITY );

    /* Create HTTP demo */
	http_demo_create( appconfHTTP_TASK_PRIORITY );

    /* Create MQTT demo*/
    mqtt_demo_create( gpio_ctx, appconfMQTT_TASK_PRIORITY );

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
                configMINIMAL_STACK_SIZE*6,
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("start scheduler on tile 0\n");
    vTaskStartScheduler();

    return;
}

#endif /* ON_TILE(0) */
