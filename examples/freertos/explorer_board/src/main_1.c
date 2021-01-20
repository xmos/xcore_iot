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

/* App headers */
#include "app_conf.h"
#include "board_init.h"
#include "gpio_ctrl.h"
#include "example_pipeline/example_pipeline.h"
#include "mem_analysis.h"

#if ON_TILE(1)

static rtos_mic_array_t mic_array_ctx_s;
static rtos_i2c_master_t i2c_master_ctx_s;
static rtos_i2s_master_t i2s_master_ctx_s;
static rtos_intertile_t intertile_ctx_s;
static rtos_gpio_t gpio_ctx_s;

static rtos_mic_array_t *mic_array_ctx = &mic_array_ctx_s;
static rtos_i2c_master_t *i2c_master_ctx = &i2c_master_ctx_s;
static rtos_i2s_master_t *i2s_master_ctx = &i2s_master_ctx_s;
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
    mem_analysis_create( "heap" );

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
    /*
     * FIXME: It is necessary that these two tasks run on cores that are uninterrupted.
     * Therefore they must not run on core 0. It is currently difficult to guarantee this
     * as there is no support for core affinity.
     *
     * Setting I2C_TILE to 0 so that this task does not block above and remains on core 0
     * here will, however, ensure that these two tasks run on the two highest cores.
     */
    const int pdm_decimation_factor = rtos_mic_array_decimation_factor(
            appconfPDM_CLOCK_FREQUENCY,
            EXAMPLE_PIPELINE_AUDIO_SAMPLE_RATE);

    rtos_printf("Starting mic array driver\n");
    rtos_mic_array_start(
            mic_array_ctx,
            pdm_decimation_factor,
            rtos_mic_array_third_stage_coefs(pdm_decimation_factor),
            rtos_mic_array_fir_compensation(pdm_decimation_factor),
            1.2 * MIC_DUAL_FRAME_SIZE,
            configMAX_PRIORITIES-1);

    rtos_printf("Starting i2s driver\n");
    rtos_i2s_master_start(
            i2s_master_ctx,
            rtos_i2s_master_mclk_bclk_ratio(appconfAUDIO_CLOCK_FREQUENCY, EXAMPLE_PIPELINE_AUDIO_SAMPLE_RATE),
            I2S_MODE_I2S,
            1.2 * MIC_DUAL_FRAME_SIZE,
            configMAX_PRIORITIES-1);

    /* Create the gpio control task */
    gpio_ctrl_create(gpio_ctx, appconfGPIO_TASK_PRIORITY);

    /* Create audio pipeline */
    example_pipeline_init(mic_array_ctx, i2s_master_ctx);

    vTaskDelete(NULL);
}

void vApplicationCoreInitHook(BaseType_t xCoreID)
{
    rtos_printf("Initializing tile 1 core %d on core %d\n", xCoreID, portGET_CORE_ID());

    switch (xCoreID) {

    case 0:
        rtos_mic_array_interrupt_init(mic_array_ctx);
    }
}

void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    board_tile1_init(c0, intertile_ctx, mic_array_ctx, i2s_master_ctx, i2c_master_ctx, gpio_ctx);
    (void) c1;
    (void) c2;
    (void) c3;

    other_tile_c = c0;

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

#endif /* ON_TILE(1) */
