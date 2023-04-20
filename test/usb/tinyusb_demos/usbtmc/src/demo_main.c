/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "rtos_gpio.h"
#include "demo_main.h"
#include "tusb.h"
#include "usbtmc_app.h"

 /* Blink pattern
  * - 250 ms  : device not mounted
  * - 0 ms : device mounted
  * - 2500 ms : device is suspended
  */
enum  {
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

static TimerHandle_t blinky_timer_ctx = NULL;
static rtos_gpio_t *gpio_ctx = NULL;
static rtos_gpio_port_id_t button_port = 0;
static rtos_gpio_port_id_t led_port = 0;
static uint32_t led_val = 0;
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    if (blinky_timer_ctx != NULL) {
        xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
    }
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    if (blinky_timer_ctx != NULL) {
        xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), 0);
    }
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    if (blinky_timer_ctx != NULL) {
        xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_SUSPENDED), 0);
    }
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    if (blinky_timer_ctx != NULL) {
        xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
    }
}

void usbtmc_app_task(void)
{
    while(1) {
        usbtmc_app_task_iter();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

//--------------------------------------------------------------------+
// BLINKING TASK + Indicator pulse
//--------------------------------------------------------------------+
void set_led_state() {
    #if XCOREAI_EXPLORER
        rtos_gpio_port_out(gpio_ctx, led_port, led_val);
    #else
    #error No valid board was specified
    #endif
}

// called from USB context
void led_indicator_pulse(void) {
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(750), 0);
    led_val = 1;    // force LED on
    set_led_state();
}

void led_blinky_cb(TimerHandle_t xTimer)
{
    (void) xTimer;
    led_val ^= 1;
    set_led_state();
}

void create_tinyusb_demo(rtos_gpio_t *ctx, unsigned priority)
{
    if (gpio_ctx == NULL) {
        gpio_ctx = ctx;

        led_port = rtos_gpio_port(PORT_LEDS);
        rtos_gpio_port_enable(gpio_ctx, led_port);
        rtos_gpio_port_out(gpio_ctx, led_port, led_val);

#if XCOREAI_EXPLORER
        button_port = rtos_gpio_port(PORT_BUTTONS);
#else
#error No valid board was specified
#endif
        rtos_gpio_port_enable(gpio_ctx, button_port);

        blinky_timer_ctx = xTimerCreate("blinky",
                                        pdMS_TO_TICKS(blink_interval_ms),
                                        pdTRUE,
                                        NULL,
                                        led_blinky_cb);
        configASSERT(blinky_timer_ctx);
        xTimerStart(blinky_timer_ctx, 0);

        xTaskCreate((TaskFunction_t) usbtmc_app_task,
                    "usbtmc_app_task",
                    portTASK_STACK_DEPTH(usbtmc_app_task),
                    NULL,
                    priority,
                    NULL);
    }
}
