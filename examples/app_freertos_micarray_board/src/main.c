// Copyright (c) 2019, XMOS Ltd, All rights reserved

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCP.h"

/* Library headers */

/* App headers */
#include "audio_pipeline.h"
#include "network.h"
#include "queue_to_tcp_stream.h"
#include "UDPCommandInterpreter.h"
#include "thruput_test.h"
#include "gpio_ctrl.h"
#include "app_conf.h"

eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase,
                                            uint32_t ulIPAddress )
{
    debug_printf("DHCP phase %d\n", eDHCPPhase);
    if (eDHCPPhase == eDHCPPhasePreRequest) {
        ulIPAddress = FreeRTOS_ntohl(ulIPAddress);
        debug_printf("%d.%d.%d.%d\n", (ulIPAddress >> 24) & 0xff, (ulIPAddress >> 16) & 0xff, (ulIPAddress >> 8) & 0xff, (ulIPAddress >> 0) & 0xff);

    }

    return eDHCPContinue;
}

void soc_tile0_main(
        int tile)
{
    QueueHandle_t ap_output_queue;

    ap_output_queue = xQueueCreate(2, sizeof(void *));

    /* Create audio pipeline */
    audio_pipeline_create( ap_output_queue, appconfAUDIO_PIPELINE_TASK_PRIORITY );

    /* Create queue to tcp task */
    queue_to_tcp_stream_create( ap_output_queue, appconfQUEUE_TO_TCP_TASK_PRIORITY );

    /* Create UDP CLI */
    vStartUDPCommandInterpreterTask( portTASK_STACK_DEPTH(vUDPCommandInterpreterTask), appconfCLI_UDP_PORT, appconfCLI_TASK_PRIORITY );

    /* Create the thruput test */
    thruput_test_create( appconfTHRUPUT_TEST_TASK_PRIORITY );

    gpio_ctrl_create(appconfTHRUPUT_TEST_TASK_PRIORITY);

    /* Initialize FreeRTOS IP*/
    initalize_FreeRTOS_IP();

    vTaskStartScheduler();
}


void vApplicationMallocFailedHook(void)
{
    debug_printf("Malloc failed!\n");
    //configASSERT(0);
}
