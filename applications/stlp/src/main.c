// Copyright (c) 2020-2022 XMOS LIMITED. This Software is subject to the terms of the
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
#include "rtos_printf.h"
#include "device_control.h"
#include "src.h"

/* App headers */
#include "app_conf.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"
#include "app_control/app_control.h"
#include "usb_support.h"
#include "usb_audio.h"
#include "audio_pipeline.h"
#include "ww_model_runner/ww_model_runner.h"
#include "fs_support.h"

#include "gpio_test/gpio_test.h"

volatile int mic_from_usb = appconfMIC_SRC_DEFAULT;
volatile int aec_ref_source = appconfAEC_REF_DEFAULT;

void audio_pipeline_input(void *input_app_data,
                        int32_t **input_audio_frames,
                        size_t ch_count,
                        size_t frame_count)
{
    (void) input_app_data;
    int32_t **mic_ptr = (int32_t **)(input_audio_frames + (2 * frame_count));

    static int flushed;
    while (!flushed) {
        size_t received;
        received = rtos_mic_array_rx(mic_array_ctx,
                                     mic_ptr,
                                     frame_count,
                                     0);
        if (received == 0) {
            rtos_mic_array_rx(mic_array_ctx,
                              mic_ptr,
                              frame_count,
                              portMAX_DELAY);
            flushed = 1;
        }
    }

    app_control_ap_handler(NULL, 0);

    /*
     * NOTE: ALWAYS receive the next frame from the PDM mics,
     * even if USB is the current mic source. The controls the
     * timing since usb_audio_recv() does not block and will
     * receive all zeros if no frame is available yet.
     */
    rtos_mic_array_rx(mic_array_ctx,
                      mic_ptr,
                      frame_count,
                      portMAX_DELAY);

#if appconfUSB_ENABLED
    int32_t **usb_mic_audio_frame = NULL;

    if (mic_from_usb) {
        usb_mic_audio_frame = input_audio_frames;
    }

    /*
     * As noted above, this does not block.
     * and expects ref L, ref R, mic 0, mic 1
     */
    usb_audio_recv(intertile_ctx,
                   frame_count,
                   usb_mic_audio_frame,
                   ch_count);
#endif

#if appconfI2S_ENABLED
    if (!appconfUSB_ENABLED || aec_ref_source == appconfAEC_REF_I2S) {
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
    }
#endif
}

int audio_pipeline_output(void *output_app_data,
                        int32_t **output_audio_frames,
                        size_t ch_count,
                        size_t frame_count)
{
    (void) output_app_data;

#if appconfI2S_ENABLED
#if !appconfI2S_TDM_ENABLED
    /* I2S expects sample channel format */
    int32_t tmp[appconfAUDIO_PIPELINE_FRAME_ADVANCE][appconfAUDIO_PIPELINE_CHANNELS];
    int32_t *tmpptr = (int32_t *)output_audio_frames;
    for (int j=0; j<appconfAUDIO_PIPELINE_FRAME_ADVANCE; j++) {
        /* ASR output is first */
        tmp[j][0] = *(tmpptr+j);
        tmp[j][1] = *(tmpptr+j); // duplicate on ch 1
    }

    rtos_i2s_tx(i2s_ctx,
                (int32_t*) tmp,
                frame_count,
                portMAX_DELAY);
#else
    int32_t *tmpptr = (int32_t *)output_audio_frames;
    for (int i = 0; i < frame_count; i++) {
        /* output_audio_frames format is
         *   processed_audio_frame
         *   reference_audio_frame
         *   raw_mic_audio_frame
         */
        int32_t tdm_output[6];

        tdm_output[0] = *(tmpptr + i + (4 * frame_count)) & ~0x1;   // mic 0
        tdm_output[1] = *(tmpptr + i + (5 * frame_count)) & ~0x1;   // mic 1
        tdm_output[2] = *(tmpptr + i + (2 * frame_count)) & ~0x1;   // ref 0
        tdm_output[3] = *(tmpptr + i + (3 * frame_count)) & ~0x1;   // ref 1
        tdm_output[4] = *(tmpptr + i) | 0x1;                        // proc 0
        tdm_output[5] = *(tmpptr + i + frame_count) | 0x1;          // proc 1

        rtos_i2s_tx(i2s_ctx,
                    tdm_output,
                    appconfI2S_AUDIO_SAMPLE_RATE / appconfAUDIO_PIPELINE_SAMPLE_RATE,
                    portMAX_DELAY);
    }
#endif
#endif

#if appconfUSB_ENABLED
    usb_audio_send(intertile_ctx,
                frame_count,
                output_audio_frames,
                6);
#endif

#if appconfWW_ENABLED
    ww_audio_send(intertile_ctx,
                  frame_count,
                  (int32_t(*)[2])output_audio_frames);
#endif

    return AUDIO_PIPELINE_FREE_FRAME;
}

RTOS_I2S_APP_SEND_FILTER_CALLBACK_ATTR
size_t i2s_send_upsample_cb(rtos_i2s_t *ctx, void *app_data, int32_t *i2s_frame, size_t i2s_frame_size, int32_t *send_buf, size_t samples_available)
{
    static int i;
    static int32_t src_data[2][SRC_FF3V_FIR_TAPS_PER_PHASE] __attribute__((aligned(8)));

    xassert(i2s_frame_size == 2);

    switch (i) {
    case 0:
        i = 1;
        if (samples_available >= 2) {
            i2s_frame[0] = src_us3_voice_input_sample(src_data[0], src_ff3v_fir_coefs[2], send_buf[0]);
            i2s_frame[1] = src_us3_voice_input_sample(src_data[1], src_ff3v_fir_coefs[2], send_buf[1]);
            return 2;
        } else {
            i2s_frame[0] = src_us3_voice_input_sample(src_data[0], src_ff3v_fir_coefs[2], 0);
            i2s_frame[1] = src_us3_voice_input_sample(src_data[1], src_ff3v_fir_coefs[2], 0);
            return 0;
        }
    case 1:
        i = 2;
        i2s_frame[0] = src_us3_voice_get_next_sample(src_data[0], src_ff3v_fir_coefs[1]);
        i2s_frame[1] = src_us3_voice_get_next_sample(src_data[1], src_ff3v_fir_coefs[1]);
        return 0;
    case 2:
        i = 0;
        i2s_frame[0] = src_us3_voice_get_next_sample(src_data[0], src_ff3v_fir_coefs[0]);
        i2s_frame[1] = src_us3_voice_get_next_sample(src_data[1], src_ff3v_fir_coefs[0]);
        return 0;
    default:
        xassert(0);
        return 0;
    }
}

RTOS_I2S_APP_RECEIVE_FILTER_CALLBACK_ATTR
size_t i2s_send_downsample_cb(rtos_i2s_t *ctx, void *app_data, int32_t *i2s_frame, size_t i2s_frame_size, int32_t *receive_buf, size_t sample_spaces_free)
{
    static int i;
    static int64_t sum[2];
    static int32_t src_data[2][SRC_FF3V_FIR_NUM_PHASES][SRC_FF3V_FIR_TAPS_PER_PHASE] __attribute__((aligned (8)));

    xassert(i2s_frame_size == 2);

    switch (i) {
    case 0:
        i = 1;
        sum[0] = src_ds3_voice_add_sample(0, src_data[0][0], src_ff3v_fir_coefs[0], i2s_frame[0]);
        sum[1] = src_ds3_voice_add_sample(0, src_data[1][0], src_ff3v_fir_coefs[0], i2s_frame[1]);
        return 0;
    case 1:
        i = 2;
        sum[0] = src_ds3_voice_add_sample(sum[0], src_data[0][1], src_ff3v_fir_coefs[1], i2s_frame[0]);
        sum[1] = src_ds3_voice_add_sample(sum[1], src_data[1][1], src_ff3v_fir_coefs[1], i2s_frame[1]);
        return 0;
    case 2:
        i = 0;
        if (sample_spaces_free >= 2) {
            receive_buf[0] = src_ds3_voice_add_final_sample(sum[0], src_data[0][2], src_ff3v_fir_coefs[2], i2s_frame[0]);
            receive_buf[1] = src_ds3_voice_add_final_sample(sum[1], src_data[1][2], src_ff3v_fir_coefs[2], i2s_frame[1]);
            return 2;
        } else {
            (void) src_ds3_voice_add_final_sample(sum[0], src_data[0][2], src_ff3v_fir_coefs[2], i2s_frame[0]);
            (void) src_ds3_voice_add_final_sample(sum[1], src_data[1][2], src_ff3v_fir_coefs[2], i2s_frame[1]);
            return 0;
        }
    default:
        xassert(0);
        return 0;
    }
}

void i2s_rate_conversion_enable(void)
{
#if !appconfI2S_TDM_ENABLED
    rtos_i2s_send_filter_cb_set(i2s_ctx, i2s_send_upsample_cb, NULL);
#endif
    rtos_i2s_receive_filter_cb_set(i2s_ctx, i2s_send_downsample_cb, NULL);
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

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

#if ON_TILE(1)
    gpio_test(gpio_ctx_t0);
#endif

#if ON_TILE(1)
    app_control_ap_servicer_register();
#endif
    audio_pipeline_init(NULL, NULL);

#if ON_TILE(FS_TILE_NO)
    rtos_fatfs_init(qspi_flash_ctx);
#endif

#if appconfWW_ENABLED && ON_TILE(WW_TILE_NO)
    ww_task_create(appconfWW_TASK_PRIORITY);
#endif

    mem_analysis();
    /*
     * TODO: Watchdog?
     */
}

void vApplicationMinimalIdleHook(void)
{
    rtos_printf("idle hook on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());
    asm volatile("waiteu");
}

static void tile_common_init(chanend_t c)
{
    control_ret_t ctrl_ret;

    platform_init(c);
    chanend_free(c);

    ctrl_ret = app_control_init();
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
