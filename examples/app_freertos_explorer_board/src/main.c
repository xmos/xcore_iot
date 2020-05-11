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
#include "queue_to_i2s.h"
#include "UDPCommandInterpreter.h"
#include "thruput_test.h"
#include "gpio_ctrl.h"
#include "app_conf.h"

void soc_tile0_main(
        int tile)
{
    QueueHandle_t ap_output_queue0;
    QueueHandle_t ap_output_queue1;

    ap_output_queue0 = xQueueCreate(2, sizeof(void *));
    ap_output_queue1 = xQueueCreate(2, sizeof(void *));

    /* Create audio pipeline */
    audio_pipeline_create( ap_output_queue0, ap_output_queue1, appconfAUDIO_PIPELINE_TASK_PRIORITY );

    /* Create queue to tcp task */
    queue_to_tcp_stream_create( ap_output_queue0, appconfQUEUE_TO_TCP_TASK_PRIORITY );

    /* Create queue to i2s task */
    queue_to_i2s_create( ap_output_queue1, appconfQUEUE_TO_I2S_TASK_PRIORITY );

    /* Create UDP CLI */
    vStartUDPCommandInterpreterTask( portTASK_STACK_DEPTH(vUDPCommandInterpreterTask), appconfCLI_UDP_PORT, appconfCLI_TASK_PRIORITY );

    /* Create the thruput test */
    thruput_test_create( appconfTHRUPUT_TEST_TASK_PRIORITY );

    /* Create the gpio control task */
    gpio_ctrl_create( appconfGPIO_TASK_PRIORITY );

    /* Initialize WiFi */
    initalize_wifi();

    vTaskStartScheduler();
}


void vApplicationMallocFailedHook(void)
{
    debug_printf("Malloc failed!\n");
    //configASSERT(0);
}
