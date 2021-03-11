// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "rtos/drivers/gpio/api/rtos_gpio.h"
#include "demo_main.h"
#include "tusb.h"

#include "usb_descriptors.h"

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

static rtos_gpio_t *gpio_ctx = NULL;
static rtos_gpio_port_id_t button_port = 0;
static rtos_gpio_port_id_t led_port = 0;
static uint32_t led_val = 0;
static uint32_t blink_interval_ms;

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
      }else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
      }
    }
    break;

    case REPORT_ID_MOUSE:
    {
      int8_t const delta = 5;

      // no button, right + down, no scroll, no pan
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);
    }
    break;

    case REPORT_ID_CONSUMER_CONTROL:
    {
      // use to avoid send multiple consecutive zero report
      static bool has_consumer_key = false;

      if ( btn )
      {
        // volume down
        uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
        has_consumer_key = true;
      }else
      {
        // send empty key report (release key) if previously has key pressed
        uint16_t empty_key = 0;
        if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
        has_consumer_key = false;
      }
    }
    break;

    case REPORT_ID_GAMEPAD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_gamepad_key = false;

      hid_gamepad_report_t report =
      {
        .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
        .hat = 0, .buttons = 0
      };

      if ( btn )
      {
        report.hat = GAMEPAD_HAT_UP;
        report.buttons = GAMEPAD_BUTTON_A;
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

        has_gamepad_key = true;
      } else {
        report.hat = GAMEPAD_HAT_CENTERED;
        report.buttons = 0;
        if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        has_gamepad_key = false;
      }
    }
    break;

    default: break;
  }
}

void hid_task(void* args)
{
    (void) args;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));

        uint32_t buttons_val = rtos_gpio_port_in(gpio_ctx, button_port);
#if OSPREY_BOARD
        buttons_val = (~buttons_val) & 0x4;
#elif XCOREAI_EXPLORER || XCORE200_MIC_ARRAY
        buttons_val = (~buttons_val) & 0x1;
#endif

        // Remote wakeup
        if ( tud_suspended() && buttons_val )
        {
            // Wake up host if we are in suspend mode
            // and REMOTE_WAKEUP feature is enabled by host
            tud_remote_wakeup();
        }
        else
        {
            // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
            send_hid_report(REPORT_ID_KEYBOARD, buttons_val);
        }
    }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t itf, uint8_t const* report, uint8_t len)
{
    (void) itf;
    (void) len;

    uint8_t next_report_id = report[0] + 1;

    if (next_report_id < REPORT_ID_COUNT)
    {
        uint32_t buttons_val = rtos_gpio_port_in(gpio_ctx, button_port);
#if OSPREY_BOARD
        buttons_val = (~buttons_val) & 0x4;
#elif XCOREAI_EXPLORER || XCORE200_MIC_ARRAY
        buttons_val = (~buttons_val) & 0x1;
#endif

        send_hid_report(next_report_id, buttons_val);
    }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    // TODO not Implemented
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    // TODO set LED based on CAPLOCK, NUMLOCK etc...
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}

void led_blinky_task(void *args)
{
    (void) args;

    TickType_t last_wake_time;
    const TickType_t blink_freq = 10;

    last_wake_time = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(blink_interval_ms));

        led_val ^= 1;

#if OSPREY_BOARD
    #define RED         ~(1<<6)
    #define GREEN       ~(1<<7)
        if(led_val) {
            rtos_gpio_port_out(gpio_ctx, led_port, RED);
        } else {
            rtos_gpio_port_out(gpio_ctx, led_port, GREEN);
        }
    #elif XCOREAI_EXPLORER
        rtos_gpio_port_out(gpio_ctx, led_port, led_val);
    #elif XCORE200_MIC_ARRAY
        rtos_gpio_port_out(gpio_ctx, led_port, led_val << 2);
    #else
#error No valid board was specified
#endif
    }
}

void create_tinyusb_demo(rtos_gpio_t *ctx, unsigned priority)
{
    if (gpio_ctx == NULL) {
        gpio_ctx = ctx;

#if XCORE200_MIC_ARRAY
        led_port = rtos_gpio_port(PORT_LED10_TO_12);
#else
        led_port = rtos_gpio_port(PORT_LEDS);
#endif
        rtos_gpio_port_enable(gpio_ctx, led_port);
        rtos_gpio_port_out(gpio_ctx, led_port, led_val);

#if OSPREY_BOARD
        button_port = rtos_gpio_port(PORT_BUTTON);
#elif XCOREAI_EXPLORER
        button_port = rtos_gpio_port(PORT_BUTTONS);
#elif XCORE200_MIC_ARRAY
        button_port = rtos_gpio_port(PORT_BUT_A_TO_D);
#else
#error No valid board was specified
#endif
        rtos_gpio_port_enable(gpio_ctx, button_port);

        blink_interval_ms = BLINK_NOT_MOUNTED;
        xTaskCreate((TaskFunction_t) led_blinky_task,
                    "led_blinky_task",
                    portTASK_STACK_DEPTH(led_blinky_task),
                    NULL,
                    configMAX_PRIORITIES - 1,
                    NULL);

        xTaskCreate((TaskFunction_t) hid_task,
                    "hid_task",
                    portTASK_STACK_DEPTH(hid_task),
                    NULL,
                    priority,
                    NULL);
    }
}
