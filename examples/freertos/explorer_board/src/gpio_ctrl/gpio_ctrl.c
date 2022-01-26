// Copyright 2019-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Library headers */

/* App headers */
#include "app_conf.h"
#include "example_pipeline/example_pipeline.h"
#include "platform/driver_instances.h"

RTOS_GPIO_ISR_CALLBACK_ATTR
static void button_callback(rtos_gpio_t *ctx, void *app_data, rtos_gpio_port_id_t port_id, uint32_t value)
{
    TaskHandle_t task = app_data;
    BaseType_t xYieldRequired = pdFALSE;

    value = (~value) & 0x3;

    xTaskNotifyFromISR(task, value, eSetValueWithOverwrite, &xYieldRequired);

    portYIELD_FROM_ISR(xYieldRequired);
}

static void volume_up( void )
{
    BaseType_t gain = 0;
    gain = audiopipeline_get_stage1_gain();
    if( gain != 0xFFFFFFFF )
    {
        gain++;
    }
    audiopipeline_set_stage1_gain( gain );
    // rtos_printf("volume up\n");
}

static void volume_down( void )
{
    BaseType_t gain = 0;
    gain = audiopipeline_get_stage1_gain();
    if( gain > 0 )
    {
        gain--;
    }
    audiopipeline_set_stage1_gain( gain );
    // rtos_printf("volume down\n");
}

void vVolumeUpCallback( TimerHandle_t pxTimer )
{
    volume_up();
}

void vVolumeDownCallback( TimerHandle_t pxTimer )
{
    volume_down();
}

void gpio_ctrl(void)
{
    uint32_t status;
    uint32_t buttons_val;
    uint32_t buttonA;
    uint32_t buttonB;
    TimerHandle_t volume_up_timer;
    TimerHandle_t volume_down_timer;

    const rtos_gpio_port_id_t button_port = rtos_gpio_port(PORT_BUTTONS);
    const rtos_gpio_port_id_t led_port = rtos_gpio_port(PORT_LEDS);
    rtos_printf("enable led port %d\n", button_port);
    rtos_gpio_port_enable(gpio_ctx_t0, led_port);
    rtos_printf("enable button port %d\n", led_port);
    rtos_gpio_port_enable(gpio_ctx_t0, button_port);

    rtos_printf("enable button isr\n");
    rtos_gpio_isr_callback_set(gpio_ctx_t0, button_port, button_callback, xTaskGetCurrentTaskHandle());
    rtos_gpio_interrupt_enable(gpio_ctx_t0, button_port);

    rtos_printf("enable button timers\n");
    volume_up_timer = xTimerCreate(
                            "vol_up",
                            pdMS_TO_TICKS(appconfGPIO_VOLUME_RAPID_FIRE_MS),
                            pdTRUE,
                            NULL,
                            vVolumeUpCallback );

    volume_down_timer = xTimerCreate(
                            "vol_down",
                            pdMS_TO_TICKS(appconfGPIO_VOLUME_RAPID_FIRE_MS),
                            pdTRUE,
                            NULL,
                            vVolumeDownCallback);

    for (;;) {
        xTaskNotifyWait(
                0x00000000UL,    /* Don't clear notification bits on entry */
                0xFFFFFFFFUL,    /* Reset full notification value on exit */
                &status,         /* Pass out notification value into status */
                portMAX_DELAY ); /* Wait indefinitely until next notification */

        buttons_val = rtos_gpio_port_in(gpio_ctx_t0, button_port);
        buttonA = ( buttons_val >> 0 ) & 0x01;
        buttonB = ( buttons_val >> 1 ) & 0x01;

        /* Turn on LEDS based on buttons */
        rtos_gpio_port_out(gpio_ctx_t0, led_port, buttons_val);

        /* Adjust volume based on LEDs */
        if( buttonA == 0 )   /* Up */
        {
            xTimerStart( volume_up_timer, 0 );
            volume_up();
            // rtos_printf("volume up start\n");
        }
        else
        {
            xTimerStop( volume_up_timer, 0 );
        }

        if( buttonB == 0 )   /* Down */
        {
            xTimerStart( volume_down_timer, 0 );
            volume_down();
            // rtos_printf("volume down start\n");
        }
        else
        {
            xTimerStop( volume_down_timer, 0 );
        }
    }
}

void gpio_ctrl_create(UBaseType_t priority)
{
    xTaskCreate((TaskFunction_t) gpio_ctrl,
                "gpio_ctrl",
                RTOS_THREAD_STACK_SIZE(gpio_ctrl),
                NULL,
                priority,
                NULL);
}
