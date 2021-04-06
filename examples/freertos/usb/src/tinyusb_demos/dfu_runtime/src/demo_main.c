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

/* After device is enumerated, run following command
 *
 * $ dfu-util -l
 *
 * It should be able to list our device as in Runtime mode. Then run
 *
 * $ dfu-util -e
 *
 * This will send DETTACH command to put device into bootloader. Since this example
 * is minimal, it doesn't actually go into DFU mode but rather change the LED blinking
 * pattern to fast rate as indicator.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "rtos/drivers/gpio/api/rtos_gpio.h"
#include "demo_main.h"
#include "tusb.h"

/* Blink pattern
 * - 1000 ms : device should reboot
 * - 250 ms  : device not mounted
 * - 0 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
    BLINK_DFU_MODE = 100,
    BLINK_NOT_MOUNTED = 250,
    BLINK_MOUNTED = 1000,
    BLINK_SUSPENDED = 2500,
};

static TimerHandle_t blinky_timer_ctx = NULL;
static rtos_gpio_t *gpio_ctx = NULL;
static rtos_gpio_port_id_t led_port = 0;
static uint32_t led_val = 0;
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
static rtos_qspi_flash_t *qspi_ctx = NULL;
//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    if(blinky_timer_ctx != NULL) xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    if(blinky_timer_ctx != NULL) xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), 0);
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    if(blinky_timer_ctx != NULL) xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_SUSPENDED), 0);
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    if(blinky_timer_ctx != NULL) xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

//--------------------------------------------------------------------+
// Class callbacks
//--------------------------------------------------------------------+
uint8_t tud_dfu_runtime_init_attrs_cb()
{
    return (uint8_t) DFU_FUNC_ATTR_CAN_DOWNLOAD_BITMASK | DFU_FUNC_ATTR_MANIFESTATION_TOLERANT_BITMASK | DFU_FUNC_ATTR_CAN_UPLOAD_BITMASK | DFU_FUNC_ATTR_WILL_DETACH_BITMASK;
}

uint8_t tud_dfu_mode_init_attrs_cb()
{
    return (uint8_t) DFU_FUNC_ATTR_CAN_DOWNLOAD_BITMASK | DFU_FUNC_ATTR_MANIFESTATION_TOLERANT_BITMASK | DFU_FUNC_ATTR_CAN_UPLOAD_BITMASK | DFU_FUNC_ATTR_WILL_DETACH_BITMASK;
}

bool tud_dfu_mode_firmware_valid_check_cb()
{
    return true;
}

void tud_dfu_mode_req_dnload_data_cb(uint16_t wBlockNum, uint8_t* data, uint16_t length)
{
  rtos_printf("Block[%u] Len[%u] Buffer:\n", wBlockNum, length);
  for(int i=0; i<11; i++) {
    rtos_printf("%c", data[i] );
  }
  rtos_printf("\n");
}

void tud_dfu_mode_get_poll_timeout_cb(uint8_t *ms_timeout)
{
    *(ms_timeout+0) = 0;
}

void tud_dfu_mode_start_poll_timeout_cb(uint8_t *ms_timeout)
{
  uint32_t delay = ms_timeout[2] << 8 | ms_timeout[1] << 1 | ms_timeout[0];
  rtos_printf("start poll timeout\n");
  vTaskDelay(pdMS_TO_TICKS(delay)); // TODO: this delay should not be here
  rtos_printf("timeout done\n");
  tud_dfu_mode_poll_timeout_done();
}

bool tud_dfu_mode_device_data_done_check_cb()
{
  rtos_printf("Dummy device data done check... Returning true\n");
  return true;
}

void tud_dfu_mode_abort_cb()
{
  rtos_printf("Host Aborted transfer\n");
}

const char test_string[] = "This is an upload test.\nHello world!\n";
static int test_send = 1;
uint16_t tud_dfu_mode_req_upload_data_cb(uint16_t block_num, uint8_t* data, uint16_t length)
{
  if (test_send == 0) {
    test_send = 1;
    return 0;
  } else {
    memcpy(data, &test_string, sizeof(test_string));
    test_send = 0;
    return length;
  }
}

dfu_protocol_type_t dfu_init_in_mode_cb()
{
  return DFU_PROTOCOL_DFU;
}

static void reboot()
{
    // TODO reset other tiles too
    unsigned pll_ctrl_val[1];

    for (unsigned i = 0; i < 1; i++) {
        read_sswitch_reg(get_local_tile_id(), XS1_SSWITCH_PLL_CTL_NUM, &pll_ctrl_val[i]);
        read_sswitch_reg(get_local_tile_id(), XS1_SSWITCH_PLL_CTL_NUM, &pll_ctrl_val[i]);

        //debug_printf("tile %d pll config: %x\n", i, pll_ctrl_val[i]);

        /* ensure the reset and hold bits are cleared */
        pll_ctrl_val[i] &= 0x8FFFFFFF;
    }

    /* reset the local node last */
    write_sswitch_reg(get_local_tile_id(), XS1_SSWITCH_PLL_CTL_NUM, pll_ctrl_val[0]);
    write_sswitch_reg(get_local_tile_id(), XS1_SSWITCH_PLL_CTL_NUM, pll_ctrl_val[0]);

    while (1);
}

void tud_dfu_runtime_reboot_to_dfu_cb()
{
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_DFU_MODE), 0);
    set_dfu_mode();
    reboot();
}

void tud_dfu_mode_reboot_to_rt_cb()
{
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_DFU_MODE), 0);
    set_rt_mode();
    reboot();
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+

void led_blinky_cb(TimerHandle_t xTimer)
{
    (void) xTimer;
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
#else
#error No valid board was specified
#endif
}

void create_tinyusb_demo(demo_args_t *args, unsigned priority)
{
    gpio_ctx = args->gpio_ctx;
    qspi_ctx =  args->qspi_ctx;

    if (gpio_ctx == NULL) {
        led_port = rtos_gpio_port(PORT_LEDS);
        rtos_gpio_port_enable(gpio_ctx, led_port);
        rtos_gpio_port_out(gpio_ctx, led_port, led_val);

        blinky_timer_ctx = xTimerCreate("blinky",
                                        pdMS_TO_TICKS(blink_interval_ms),
                                        pdTRUE,
                                        NULL,
                                        led_blinky_cb);
        xTimerStart(blinky_timer_ctx, 0);
    }
}
