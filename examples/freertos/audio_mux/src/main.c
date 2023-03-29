// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <platform.h>
#include <xs1.h>
#include <xcore/channel.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "queue.h"

/* Library headers */
#include "src.h"

/* App headers */
#include "app_conf.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"
#include "usb_support.h"
#include "usb_audio.h"
#include "audio_pipeline.h"
#if RUN_EP0_VIA_PROXY
#include "rtos_ep0_proxy.h"
#endif

void audio_pipeline_input(void *input_app_data,
                        int32_t **input_audio_frames,
                        size_t ch_count,
                        size_t frame_count)
{
    (void) input_app_data;

#if appconfMIC_INPUT
    static int flushed;
    while (!flushed) {
        size_t received;
        received = rtos_mic_array_rx(mic_array_ctx,
                                     input_audio_frames,
                                     frame_count,
                                     0);
        if (received == 0) {
            rtos_mic_array_rx(mic_array_ctx,
                              input_audio_frames,
                              frame_count,
                              portMAX_DELAY);
            flushed = 1;
        }
    }

    /*
     * NOTE: ALWAYS receive the next frame from the PDM mics,
     * even if USB is the current mic source. The controls the
     * timing since usb_audio_recv() does not block and will
     * receive all zeros if no frame is available yet.
     */
    rtos_mic_array_rx(mic_array_ctx,
                      input_audio_frames,
                      frame_count,
                      portMAX_DELAY);
#endif

#if appconfUSB_INPUT
    /*
     * As noted above, this does not block.
     * and expects mic 0, mic 1
     */
    usb_audio_recv_blocking(intertile_ctx,
                   frame_count,
                   input_audio_frames,
                   ch_count);
#endif

#if appconfI2S_INPUT
    /* This shouldn't need to block given it shares a clock with the PDM mics */

    /* I2S provides sample channel format */
    int32_t tmp[appconfAUDIO_PIPELINE_FRAME_ADVANCE][appconfAUDIO_PIPELINE_CHANNELS];
    int32_t *tmpptr = (int32_t *)input_audio_frames;

    size_t rx_count =
    rtos_i2s_rx(i2s_ctx,
                (int32_t*) tmp,
                frame_count,
                portMAX_DELAY);
    xassert(rx_count == frame_count);

    for (int i=0; i<appconfAUDIO_PIPELINE_FRAME_ADVANCE; i++) {
        /* ref is first */
        *(tmpptr + i) = tmp[i][0];
        *(tmpptr + i + appconfAUDIO_PIPELINE_FRAME_ADVANCE) = tmp[i][1];
    }
#endif
}

int audio_pipeline_output(void *output_app_data,
                        int32_t **output_audio_frames,
                        size_t ch_count,
                        size_t frame_count)
{
    (void) output_app_data;

#if appconfI2S_OUTPUT
    /* I2S expects sample channel format */
    int32_t tmp[appconfAUDIO_PIPELINE_FRAME_ADVANCE][appconfAUDIO_PIPELINE_CHANNELS];
    int32_t *tmpptr = (int32_t *)output_audio_frames;
    for (int j=0; j<appconfAUDIO_PIPELINE_FRAME_ADVANCE; j++) {
        /* ASR output is first */
        tmp[j][0] = *(tmpptr+j);
        tmp[j][1] = *(tmpptr+j+appconfAUDIO_PIPELINE_FRAME_ADVANCE);
    }

    rtos_i2s_tx(i2s_ctx,
                (int32_t*) tmp,
                frame_count,
                portMAX_DELAY);
#endif

#if appconfUSB_OUTPUT
    usb_audio_send(intertile_ctx,
                frame_count,
                output_audio_frames,
                2);
#endif

    return AUDIO_PIPELINE_FREE_FRAME;
}

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    xassert(0);
    for(;;);
}

static void mem_analysis(void)
{
	for (;;) {
		rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

void vApplicationMinimalIdleHook(void)
{
    //rtos_printf("idle hook on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());
    asm volatile("waiteu");
}

#if (!RUN_EP0_VIA_PROXY)
void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

    audio_pipeline_init(NULL, NULL);

    mem_analysis();
    /*
     * TODO: Watchdog?
     */
}



static void tile_common_init(chanend_t c)
{
    platform_init(c);
    chanend_free(c);

#if appconfUSB_ENABLED && ON_TILE(USB_TILE_NO)
    usb_audio_init(intertile_ctx, appconfUSB_AUDIO_TASK_PRIORITY);
#endif

    xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                RTOS_THREAD_STACK_SIZE(startup_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("start scheduler on tile %d\n", THIS_XCORE_TILE);
    vTaskStartScheduler();
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    (void) c2;
    (void) c3;

    tile_common_init(c1);
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c1;
    (void) c2;
    (void) c3;

    tile_common_init(c0);
}
#endif

#endif /* RUN_EP0_VIA_PROXY */


#if RUN_EP0_VIA_PROXY
void startup_task(void *arg)
{
    platform_start();

    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

#if appconfUSB_ENABLED
#if ON_TILE(EP0_TILE_NO)
    printf("Call usb_manager_start() on tile %d\n", THIS_XCORE_TILE);
    usb_manager_start(configMAX_PRIORITIES-1);
#else
    printf("Call ep0_proxy_start on tile %d\n", THIS_XCORE_TILE);
    
    ep0_proxy_start(configMAX_PRIORITIES-1);
#endif
#endif
    mem_analysis();
}

void tile_common_init_tile1(chanend_t c)
{
    // Open new cross tile paths tile1 c_ep0_proxy <-> tile 0 c_ep0_proxy using existing channel c0 <-> c1
    chanend_t c_ep0_proxy;
    c_ep0_proxy = chanend_alloc();
    chan_out_word(c, c_ep0_proxy);
    chanend_set_dest(c_ep0_proxy, chan_in_word(c));

    chanend_t c_ep0_proxy_xfer_complete;
    c_ep0_proxy_xfer_complete = chanend_alloc();
    chan_out_word(c, c_ep0_proxy_xfer_complete);
    chanend_set_dest(c_ep0_proxy_xfer_complete, chan_in_word(c));

    //printf("Calling tile_common_init_tile1() on tile %d, c_ep0_proxy = %ld\n", THIS_XCORE_TILE, c_ep0_proxy);
    printf("In tile_common_init_tile1()\n");
    platform_init(c);
    chanend_free(c);
#if appconfUSB_ENABLED
    printf("Calling usb_audio_init()\n");
    usb_audio_init(intertile_ctx, appconfUSB_AUDIO_TASK_PRIORITY);
#endif
#if appconfUSB_ENABLED
    usb_manager_init(c_ep0_proxy, c_ep0_proxy_xfer_complete);
#endif
    xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                RTOS_THREAD_STACK_SIZE(startup_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("start scheduler on tile %d\n", THIS_XCORE_TILE);
    vTaskStartScheduler();
}

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c1;
    (void) c2;
    (void) c3;

    tile_common_init_tile1(c0);
}
#endif




volatile uint32_t noEpOut = 0;
volatile uint32_t noEpIn = 0;
volatile XUD_EpType epTypeTableOut[RTOS_USB_ENDPOINT_COUNT_MAX];
volatile XUD_EpType epTypeTableIn[RTOS_USB_ENDPOINT_COUNT_MAX];

DECLARE_JOB(_XUD_Main, (chanend_t, chanend_t, chanend_t, XUD_BusSpeed_t, XUD_PwrConfig));
DECLARE_JOB(tile_common_init_tile0, (chanend_t, chanend_t, chanend_t));

void tile_common_init_tile0(chanend_t c, chanend_t c_ep0_out, chanend_t c_ep0_in)
{
    chanend_t c_ep0_proxy;
    c_ep0_proxy = chanend_alloc();
    chanend_set_dest(c_ep0_proxy, chan_in_word(c));
    chan_out_word(c, c_ep0_proxy);

    chanend_t c_ep0_proxy_xfer_complete;
    c_ep0_proxy_xfer_complete = chanend_alloc();
    chanend_set_dest(c_ep0_proxy_xfer_complete, chan_in_word(c));
    chan_out_word(c, c_ep0_proxy_xfer_complete);

    platform_init(c);
    chanend_free(c);


#if appconfUSB_ENABLED
    ep0_proxy_init(c_ep0_out, c_ep0_in, c_ep0_proxy, c_ep0_proxy_xfer_complete);
#endif
    

    xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                RTOS_THREAD_STACK_SIZE(startup_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("start scheduler on tile %d\n", THIS_XCORE_TILE);
    vTaskStartScheduler();
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    (void) c2;
    (void) c3;

    // Allocate EP0 endpoint channels
    channel_t channel_ep0_out;
    channel_t channel_ep0_in;

    channel_ep0_out = chan_alloc();
    channel_ep0_in = chan_alloc();

    printf("Calling tile_common_init_tile0() on tile %d\n", THIS_XCORE_TILE);

    PAR_JOBS(        
        PJOB(_XUD_Main, (channel_ep0_out.end_a, channel_ep0_in.end_a, 0, XUD_SPEED_HS, XUD_PWR_BUS)),
        PJOB(tile_common_init_tile0, (c1, channel_ep0_out.end_b, channel_ep0_in.end_b))

    );
}
#endif
#endif
