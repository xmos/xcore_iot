// Copyright 2020 XMOS LIMITED. This Software is subject to the terms of the 
// XMOS Public License: Version 1

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCP.h"

/* Library headers */

/* App headers */
#include "network.h"
#include "app_conf.h"
#include "sntpd.h"
#include "fs_support.h"
#include "tls_support.h"
#include "mqtt_demo_client.h"
#include "mem_analysis.h"

#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
#if XCOREAI_EXPLORER
__attribute__((section(".ExtMem_data")))
#endif
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif

void soc_tile0_main(
        int tile)
{
    /* Initialize WiFi */
    initalize_wifi();

    vTaskStartScheduler();
}

void vApplicationDaemonTaskStartupHook( void )
{
    /* Initialize filesystem  */
	filesystem_init();

    /* Initialize TLS  */
	tls_platform_init();

    /* Create SNTPD */
    sntp_create( appconfSNTPD_TASK_PRIORITY );

    /* Create MQTT demo*/
    mqtt_demo_create( appconfMQTT_TASK_PRIORITY );

    /* Create heap analysis task */
    mem_analysis_create( "heap" );
}

void vApplicationMallocFailedHook(void)
{
    debug_printf("Malloc failed!\n");
    //configASSERT(0);
}
