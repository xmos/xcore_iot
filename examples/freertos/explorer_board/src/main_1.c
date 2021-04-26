// Copyright 2021 XMOS LIMITED.
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
#include "rtos/drivers/intertile/api/rtos_intertile.h"
#include "rtos/drivers/mic_array/api/rtos_mic_array.h"
#include "rtos/drivers/i2c/api/rtos_i2c_master.h"
#include "rtos/drivers/i2s/api/rtos_i2s.h"

/* App headers */
#include "app_conf.h"
#include "board_init.h"
#include "gpio_ctrl.h"
#include "example_pipeline/example_pipeline.h"
#include "mem_analysis.h"

#if ON_TILE(1)

static rtos_mic_array_t mic_array_ctx_s;
static rtos_i2c_master_t i2c_master_ctx_s;
static rtos_i2s_t i2s_ctx_s;
static rtos_intertile_t intertile_ctx_s;
static rtos_gpio_t gpio_ctx_s;

static rtos_mic_array_t *mic_array_ctx = &mic_array_ctx_s;
static rtos_i2c_master_t *i2c_master_ctx = &i2c_master_ctx_s;
static rtos_i2s_t *i2s_ctx = &i2s_ctx_s;
static rtos_intertile_t *intertile_ctx = &intertile_ctx_s;
static rtos_gpio_t *gpio_ctx = &gpio_ctx_s;

chanend_t other_tile_c;

#if( configAPPLICATION_ALLOCATED_HEAP == 1 )
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif

void vApplicationDaemonTaskStartup( void )
{
    uint32_t dac_configured;
    rtos_intertile_start(intertile_ctx);

    rtos_i2c_master_rpc_config(i2c_master_ctx, I2C_MASTER_RPC_PORT, I2C_MASTER_RPC_HOST_TASK_PRIORITY);
    rtos_gpio_rpc_config(gpio_ctx, GPIO_RPC_PORT, GPIO_RPC_HOST_TASK_PRIORITY);

    /* Create heap analysis task */
    // mem_analysis_create( "heap" );

    vTaskDelay(pdMS_TO_TICKS(1000));

    int dac_init(rtos_i2c_master_t *i2c_ctx);

    if (dac_init(i2c_master_ctx) == 0) {
        rtos_printf("DAC initialization succeeded\n");
        dac_configured = 1;
    } else {
        rtos_printf("DAC initialization failed\n");
        dac_configured = 0;
    }

    if (!dac_configured) {
        vTaskDelete(NULL);
    }

    const int pdm_decimation_factor = rtos_mic_array_decimation_factor(
            appconfPDM_CLOCK_FREQUENCY,
            appconfPIPELINE_AUDIO_SAMPLE_RATE);

    rtos_printf("Starting mic array driver\n");
    rtos_mic_array_start(
            mic_array_ctx,
            pdm_decimation_factor,
            rtos_mic_array_third_stage_coefs(pdm_decimation_factor),
            rtos_mic_array_fir_compensation(pdm_decimation_factor),
            1.2 * MIC_DUAL_FRAME_SIZE,
            0);

    rtos_printf("Starting i2s driver\n");
    rtos_i2s_start(
            i2s_ctx,
            rtos_i2s_mclk_bclk_ratio(appconfAUDIO_CLOCK_FREQUENCY, appconfPIPELINE_AUDIO_SAMPLE_RATE),
            I2S_MODE_I2S,
            0,
            1.2 * MIC_DUAL_FRAME_SIZE,
            0);

    /* Create the gpio control task */
    gpio_ctrl_create(gpio_ctx, appconfGPIO_TASK_PRIORITY);

    /* Create audio pipeline */
    example_pipeline_init(mic_array_ctx, i2s_ctx, intertile_ctx, INTERTILE_AUDIOPIPELINE_PORT);
    remote_cli_gain_init(intertile_ctx, CLI_RPC_PROCESS_COMMAND_PORT, CLI_RPC_PROCESS_COMMAND_TASK_PRIORITY);

    vTaskDelete(NULL);
}

void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    board_tile1_init(c0, intertile_ctx, mic_array_ctx, i2s_ctx, i2c_master_ctx, gpio_ctx);
    (void) c1;
    (void) c2;
    (void) c3;

    other_tile_c = c0;

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

#endif /* ON_TILE(1) */
