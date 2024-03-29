/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Copyright (c) 2023 XMOS LIMITED
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
#include "usb_descriptors.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
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
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), 0);
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_SUSPENDED), 0);
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  // Poll every 10ms
  vTaskDelay(pdMS_TO_TICKS(10));

  uint32_t btn = rtos_gpio_port_in(gpio_ctx, button_port);
  btn = (~btn) & 0x1;

  if ( tud_suspended() && btn )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }
  else
  {
    // keyboard interface
    if ( tud_hid_n_ready(ITF_NUM_KEYBOARD) )
    {
      // used to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      uint8_t const report_id = 0;
      uint8_t const modifier  = 0;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_ARROW_RIGHT;

        tud_hid_n_keyboard_report(ITF_NUM_KEYBOARD, report_id, modifier, keycode);
        has_keyboard_key = true;
      }else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_n_keyboard_report(ITF_NUM_KEYBOARD, report_id, modifier, NULL);
        has_keyboard_key = false;
      }
    }

    // mouse interface
    if ( tud_hid_n_ready(ITF_NUM_MOUSE) )
    {
      if ( btn )
      {
        uint8_t const report_id   = 0;
        uint8_t const button_mask = 0;
        uint8_t const veritical   = 0;
        uint8_t const horizontal  = 0;
        int8_t  const delta       = 5;

        tud_hid_n_mouse_report(ITF_NUM_MOUSE, report_id, button_mask, delta, delta, veritical, horizontal);
      }
    }
  }
}

// Invoked when received SET_PROTOCOL request
// protocol is either HID_PROTOCOL_BOOT (0) or HID_PROTOCOL_REPORT (1)
void tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol)
{
  (void) instance;
  (void) protocol;

  // nothing to do since we use the same compatible boot report for both Boot and Report mode.
  // TOOD set a indicator for user
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len)
{
  (void) instance;
  (void) report;
  (void) len;

  // nothing to do
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
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
  (void) report_id;

  // keyboard interface
  if (instance == ITF_NUM_KEYBOARD)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_type == HID_REPORT_TYPE_OUTPUT)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        rtos_gpio_port_out(gpio_ctx, led_port, 0);
      }else
      {
        // Caplocks Off: back to normal blink
        rtos_gpio_port_out(gpio_ctx, led_port, 1);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+

void led_blinky_cb(TimerHandle_t xTimer)
{
    (void) xTimer;
    led_val ^= 1;

#if XCOREAI_EXPLORER
    rtos_gpio_port_out(gpio_ctx, led_port, led_val);
#else
#error No valid board was specified
#endif
}

static void hid_task_wrapper(void *arg) {
    while(1) {
        hid_task();
    }
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
        xTimerStart(blinky_timer_ctx, 0);

        xTaskCreate((TaskFunction_t) hid_task_wrapper,
                    "hid_task",
                    portTASK_STACK_DEPTH(hid_task_wrapper),
                    NULL,
                    priority,
                    NULL);
    }
}
