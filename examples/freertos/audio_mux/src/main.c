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
#include "device_control.h"
#include "device_control_usb.h"
#include "app_control/app_control.h"
#include "usb_descriptors.h"

DEVICE_CONTROL_CALLBACK_ATTR
control_ret_t read_cmd(control_resid_t resid, control_cmd_t cmd, uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("Device control READ\n\t");

    rtos_printf("Servicer on tile %d received command %02x for resid %02x\n\t", THIS_XCORE_TILE, cmd, resid);
    rtos_printf("The command is requesting %d bytes\n\t", payload_len);
    for (int i = 0; i < payload_len; i++) {
        payload[i] = (cmd & 0x7F) + i;
    }
    rtos_printf("Raw bytes to be sent are:\n\t", payload_len);
    for (int i = 0; i < payload_len; i++) {
        rtos_printf("%02x ", payload[i]);
    }
    rtos_printf("\n\n");

    return CONTROL_SUCCESS;
}

DEVICE_CONTROL_CALLBACK_ATTR
control_ret_t write_cmd(control_resid_t resid, control_cmd_t cmd, const uint8_t *payload, size_t payload_len, void *app_data)
{
    rtos_printf("Device control WRITE\n\t");

    rtos_printf("Servicer on tile %d received command %02x for resid %02x\n\t", THIS_XCORE_TILE, cmd, resid);
    rtos_printf("The command has %d bytes\n\t", payload_len);
    rtos_printf("Bytes received are:\n\t", payload_len);
    for (int i = 0; i < payload_len; i++) {
        rtos_printf("%02x ", payload[i]);
    }
    rtos_printf("\n\n");

    return CONTROL_SUCCESS;
}

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

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    // TODO not Implemented
    (void) itf;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
  }
}

static void send_hid_report(uint8_t report_id)
{
#if HID_CONTROL
  // skip if hid is not ready yet
  if ( !tud_hid_ready() )
  {
    //printf("tud_hid_ready FALSE!!\n");
    return;
  }
  //printf("\n\n In send_hid_report now!!\n\n");

  //printf("In send_hid_report()\n");
  static int state = 0;
  switch(report_id)
  {
    case REPORT_ID_MOUSE:
    {
      int8_t delta;
      if(!state)
      {
        delta = 5;
        state = 1;
      }
      else
      {
        delta = -5;
        state = 0;
      }

      // no button, right + down, no scroll, no pan
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);
    }
    break;

    default: break;
  }
#endif
}

void hid_task(void)
{
      // Poll every 10ms
    vTaskDelay(pdMS_TO_TICKS(10));

    // Remote wakeup
    if ( tud_suspended())
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }
    else
    {
        //printf("in hid_task()\n");
        // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
        send_hid_report(REPORT_ID_MOUSE);
    }
}

static void hid_task_wrapper(void *arg) {
    while(1) {
        hid_task();
    }
}

#if (!RUN_EP0_VIA_PROXY)
void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

    audio_pipeline_init(NULL, NULL);
#if ON_TILE(0)
    xTaskCreate((TaskFunction_t) hid_task_wrapper,
                    "hid_task",
                    portTASK_STACK_DEPTH(hid_task_wrapper),
                    NULL,
                    appconfSTARTUP_TASK_PRIORITY,
                    NULL);
#endif

    static device_control_servicer_t servicer_ctx;
    control_ret_t dc_ret;

    for (;;) {
        #if ON_TILE(0)
        {
            control_resid_t resources[] = {0x3};

            rtos_printf("Will register a servicer now on tile %d\n", THIS_XCORE_TILE);

            dc_ret = app_control_servicer_register(&servicer_ctx,
                                                   resources, sizeof(resources));
            rtos_printf("Servicer registered now on tile %d\n", THIS_XCORE_TILE);

            for (;;) {
                device_control_servicer_cmd_recv(&servicer_ctx, read_cmd, write_cmd, NULL, RTOS_OSAL_WAIT_FOREVER);
            }
        }
        #endif
        #if ON_TILE(1)
        {
            control_resid_t resources[] = {0x33};

            rtos_printf("Will register a servicer now on tile %d\n", THIS_XCORE_TILE);

            dc_ret = app_control_servicer_register(&servicer_ctx,
                                                   resources, sizeof(resources));
            rtos_printf("Servicer registered now on tile %d\n", THIS_XCORE_TILE);

            for (;;) {
                device_control_servicer_cmd_recv(&servicer_ctx, read_cmd, write_cmd, NULL, RTOS_OSAL_WAIT_FOREVER);
            }
        }
        #endif
    }
    /*
     * TODO: Watchdog?
     */
}



static void tile_common_init(chanend_t c)
{
    platform_init(c);
    chanend_free(c);

    control_ret_t ctrl_ret = app_control_init();
    xassert(ctrl_ret == CONTROL_SUCCESS);
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

    xTaskCreate((TaskFunction_t) hid_task_wrapper, // Task for handling the HID endpoint
                    "hid_task",
                    portTASK_STACK_DEPTH(hid_task_wrapper),
                    NULL,
                    appconfSTARTUP_TASK_PRIORITY,
                    NULL);

    int dummy = 0;
    rtos_intertile_tx(intertile_ctx, appconfUSB_MANAGER_SYNC_PORT, &dummy, sizeof(dummy));
#else
    printf("Call ep0_proxy_start on tile %d\n", THIS_XCORE_TILE);
    
    ep0_proxy_start(configMAX_PRIORITIES-1);

    int ret = 0;
    rtos_intertile_rx_len(intertile_ctx, appconfUSB_MANAGER_SYNC_PORT, RTOS_OSAL_WAIT_FOREVER);
    rtos_intertile_rx_data(intertile_ctx, &ret, sizeof(ret));
#endif
#endif
    static device_control_servicer_t servicer_ctx;
    control_ret_t dc_ret;

    for (;;) {
        #if ON_TILE(0)
        {
            control_resid_t resources[] = {0x3};

            rtos_printf("Will register a servicer now on tile %d\n", THIS_XCORE_TILE);

            dc_ret = app_control_servicer_register(&servicer_ctx,
                                                   resources, sizeof(resources));
            rtos_printf("Servicer registered now on tile %d\n", THIS_XCORE_TILE);

            for (;;) {
                device_control_servicer_cmd_recv(&servicer_ctx, read_cmd, write_cmd, NULL, RTOS_OSAL_WAIT_FOREVER);
            }
        }
        #endif
        #if ON_TILE(1)
        {
            control_resid_t resources[] = {0x33};

            rtos_printf("Will register a servicer now on tile %d\n", THIS_XCORE_TILE);

            dc_ret = app_control_servicer_register(&servicer_ctx,
                                                   resources, sizeof(resources));
            rtos_printf("Servicer registered now on tile %d\n", THIS_XCORE_TILE);

            for (;;) {
                device_control_servicer_cmd_recv(&servicer_ctx, read_cmd, write_cmd, NULL, RTOS_OSAL_WAIT_FOREVER);
            }
        }
        #endif
    }
}

void tile_common_init_tile1(chanend_t c)
{
    // Open new cross tile paths tile1 c_ep0_proxy <-> tile 0 c_ep0_proxy using existing channel c0 <-> c1
    chanend_t c_ep0_proxy;
    c_ep0_proxy = chanend_alloc();
    chan_out_word(c, c_ep0_proxy);
    chanend_set_dest(c_ep0_proxy, chan_in_word(c));

    chanend_t c_ep_hid_proxy;
    c_ep_hid_proxy = chanend_alloc();
    chan_out_word(c, c_ep_hid_proxy);
    chanend_set_dest(c_ep_hid_proxy, chan_in_word(c));

    chanend_t c_ep0_proxy_xfer_complete;
    c_ep0_proxy_xfer_complete = chanend_alloc();
    chan_out_word(c, c_ep0_proxy_xfer_complete);
    chanend_set_dest(c_ep0_proxy_xfer_complete, chan_in_word(c));

    //printf("Calling tile_common_init_tile1() on tile %d, c_ep0_proxy = %ld\n", THIS_XCORE_TILE, c_ep0_proxy);
    printf("In tile_common_init_tile1()\n");
    platform_init(c);
    chanend_free(c);

    control_ret_t ctrl_ret = app_control_init();
    xassert(ctrl_ret == CONTROL_SUCCESS);

#if appconfUSB_ENABLED
    printf("Calling usb_audio_init()\n");
    usb_audio_init(intertile_ctx, appconfUSB_AUDIO_TASK_PRIORITY);
#endif
#if appconfUSB_ENABLED
    usb_manager_init(c_ep0_proxy, c_ep_hid_proxy, c_ep0_proxy_xfer_complete);
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
volatile channel_t channel_ep_out[RTOS_USB_ENDPOINT_COUNT_MAX];
volatile channel_t channel_ep_in[RTOS_USB_ENDPOINT_COUNT_MAX];

DECLARE_JOB(_XUD_Main, (chanend_t, chanend_t, chanend_t, XUD_BusSpeed_t, XUD_PwrConfig));
DECLARE_JOB(tile_common_init_tile0, (chanend_t, chanend_t, chanend_t));

void tile_common_init_tile0(chanend_t c, chanend_t c_ep0_out, chanend_t c_ep0_in)
{
    chanend_t c_ep0_proxy;
    c_ep0_proxy = chanend_alloc();
    chanend_set_dest(c_ep0_proxy, chan_in_word(c));
    chan_out_word(c, c_ep0_proxy);

    chanend_t c_ep_hid_proxy;
    c_ep_hid_proxy = chanend_alloc();
    chanend_set_dest(c_ep_hid_proxy, chan_in_word(c));
    chan_out_word(c, c_ep_hid_proxy);
    

    chanend_t c_ep0_proxy_xfer_complete;
    c_ep0_proxy_xfer_complete = chanend_alloc();
    chanend_set_dest(c_ep0_proxy_xfer_complete, chan_in_word(c));
    chan_out_word(c, c_ep0_proxy_xfer_complete);

    platform_init(c);
    chanend_free(c);

    control_ret_t ctrl_ret = app_control_init();
    xassert(ctrl_ret == CONTROL_SUCCESS);

#if appconfUSB_ENABLED
    ep0_proxy_init(c_ep0_out, c_ep0_in, c_ep0_proxy, c_ep_hid_proxy, c_ep0_proxy_xfer_complete);
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

    channel_ep_out[0] = chan_alloc(); // Allocate EP0 channels since EP0 proxy uses one of these to signal _XUD_Main when it's safe to start XUD_Main
    channel_ep_in[0] = chan_alloc();

    printf("Calling tile_common_init_tile0() on tile %d\n", THIS_XCORE_TILE);

    PAR_JOBS(        
        PJOB(_XUD_Main, (channel_ep_out[0].end_a, channel_ep_in[0].end_a, 0, XUD_SPEED_HS, XUD_PWR_BUS)),
        PJOB(tile_common_init_tile0, (c1, channel_ep_out[0].end_b, channel_ep_in[0].end_b))

    );
}
#endif
#endif
