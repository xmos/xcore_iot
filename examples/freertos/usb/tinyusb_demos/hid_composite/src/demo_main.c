// Copyright 2021-2022 XMOS LIMITED. This Software is subject to the terms of the
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "rtos_gpio.h"
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

  //printf("send_hid_report(): report_id = %d\n", report_id);
  switch(report_id)
  {
    case REPORT_ID_MOUSE:
    {
      int8_t const delta = 5;

      // no button, right + down, no scroll, no pan
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);
    }
    break;

    default: break;
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
      // Poll every 10ms
    vTaskDelay(pdMS_TO_TICKS(100));

    uint32_t buttons_val = rtos_gpio_port_in(gpio_ctx, button_port);
#if XCOREAI_EXPLORER
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
        //printf("in hid_task()\n");
        // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
        send_hid_report(REPORT_ID_MOUSE, buttons_val);
    }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t itf, uint8_t const* report, uint8_t len)
{
    return;
    (void) itf;
    (void) len;

    uint8_t next_report_id = report[0] + 1;
    printf("tud_hid_report_complete_cb(): next_report_id = %d\n", next_report_id);

    if (next_report_id < REPORT_ID_COUNT)
    {
        uint32_t buttons_val = rtos_gpio_port_in(gpio_ctx, button_port);
#if XCOREAI_EXPLORER
        buttons_val = (~buttons_val) & 0x1;
#endif

        send_hid_report(next_report_id, buttons_val);
    }
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

void led_blinky_task(void *args)
{
    (void) args;

    TickType_t last_wake_time;

    last_wake_time = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(blink_interval_ms));

        led_val ^= 1;

#if XCOREAI_EXPLORER
        rtos_gpio_port_out(gpio_ctx, led_port, led_val);
#else
#error No valid board was specified
#endif
    }
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

        blink_interval_ms = BLINK_NOT_MOUNTED;
        xTaskCreate((TaskFunction_t) led_blinky_task,
                    "led_blinky_task",
                    portTASK_STACK_DEPTH(led_blinky_task),
                    NULL,
                    configMAX_PRIORITIES - 1,
                    NULL);

        xTaskCreate((TaskFunction_t) hid_task_wrapper,
                    "hid_task",
                    portTASK_STACK_DEPTH(hid_task_wrapper),
                    NULL,
                    priority,
                    NULL);
    }
}
