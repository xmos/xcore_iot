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

/*
 * After device is enumerated in dfu mode run the following commands
 *
 * To transfer firmware from host to device:
 *
 * $ dfu-util -D [filename]
 *
 * To transfer firmware from device to host:
 *
 * $ dfu-util -U [filename]
 *
 */

/*
 * XMOS Specific
 *
 * To create a upgrade image use
 *
 * $ xflash --factory-version 15.0 --upgrade 0 [.xe file] -o [upgrade image filename]
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <xs1.h>

#include "FreeRTOS.h"
#include "rtos_gpio.h"
#include "demo_main.h"
#include "tusb.h"

#include "flash_boot_image.h"

#define FLASH_PAGE_SIZE     (4096)
#define FLASH_PAGE_COUNT    (32768)

static boot_image_manager_ctx_t bim_ctx;
static boot_image_manager_ctx_t* bim_ctx_ptr = &bim_ctx;

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

static void reboot();

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

static size_t total_len = 0;
static size_t bytes_avail = 0;
static uint32_t dn_base_addr = 0;
bool tud_dfu_firmware_valid_check_cb()
{
    uint8_t dummy;
    rtos_printf("Pass firmware validity check addr 0x%x size %u\n", dn_base_addr, total_len);
    set_rt_mode();
    boot_image_read(bim_ctx_ptr->app_data, 0, &dummy, 1);   // dummy read to ensure flash writes have completed
    reboot();
    return true;
}

void tud_dfu_req_dnload_data_cb(uint16_t wBlockNum, uint8_t* data, uint16_t length)
{
  // rtos_printf("Block[%u] Len[%u] Buffer\n", wBlockNum, length);

  if (dn_base_addr == 0)
  {
      uint32_t addr = 0;
      total_len = 0;
      int tmp = boot_image_locate_available_spot(bim_ctx_ptr, &addr, &bytes_avail);
      if(tmp == -1)
      {
          boot_image_t* imgptr = NULL;
          imgptr = boot_image_get_last_image(bim_ctx_ptr);
          addr = imgptr->startAddress;
          bytes_avail = bim_ctx_ptr->boot_partition_size - addr;
      }
      rtos_printf("Using addr 0x%x\nsize %u\n", addr, bytes_avail);
      dn_base_addr = addr;
  }

  if(length > 0)
  {
    unsigned cur_addr = dn_base_addr + (wBlockNum * bim_ctx_ptr->page_size);
    if((bytes_avail - total_len) >= length)
    {
        // rtos_printf("write %d at 0x%x\n", length, cur_addr);
        boot_image_write(bim_ctx_ptr->app_data, cur_addr, data, length);
        total_len += length;

        tud_dfu_dnload_complete();
    } else {
        rtos_printf("Insufficient space\n");
    }
  }
}

bool tud_dfu_device_data_done_check_cb()
{
  rtos_printf("Dummy device data done check... Returning true\n");
  return true;
}

void tud_dfu_abort_cb()
{
  rtos_printf("Host Aborted transfer\n");
}

uint16_t tud_dfu_req_upload_data_cb(uint16_t block_num, uint8_t* data, uint16_t length)
{
  memset(data, 0x00, length);
  uint32_t addr = block_num * FLASH_PAGE_SIZE;
  uint32_t endaddr;

#if 0
  // Test code which will just read out all of flash rather than a specific image
  endaddr = 0x800000;
#else
  boot_image_t* imgptr = NULL;
  imgptr = boot_image_get_last_image(bim_ctx_ptr);
  addr += imgptr->startAddress;
  endaddr = imgptr->startAddress + imgptr->size;
#endif
  return (addr >= endaddr) ? 0 : (uint16_t)boot_image_read(bim_ctx_ptr->app_data, addr, data, length);
}

static void reboot(void)
{
    rtos_printf("Reboot initiated by tile:0x%x\n", get_local_tile_id());
    write_sswitch_reg_no_ack(get_local_tile_id(), XS1_SSWITCH_WATCHDOG_COUNT_NUM, 0x10000);
    write_sswitch_reg_no_ack(get_local_tile_id(), XS1_SSWITCH_WATCHDOG_CFG_NUM, (1 << XS1_WATCHDOG_COUNT_ENABLE_SHIFT) | (1 << XS1_WATCHDOG_TRIGGER_ENABLE_SHIFT) );
    while(1) {;}
}

void tud_dfu_runtime_reboot_to_dfu_cb(void)
{
    set_dfu_mode();
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
#define OFF         ~(RED & GREEN)
    if(led_val) {
        rtos_gpio_port_out(gpio_ctx, led_port, OFF);
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
    boot_image_manager_init(bim_ctx_ptr, FLASH_PAGE_SIZE, FLASH_PAGE_COUNT, args->qspi_ctx);
    boot_image_build_table(bim_ctx_ptr);

    gpio_ctx = args->gpio_ctx;
    qspi_ctx =  args->qspi_ctx;

    if (gpio_ctx != NULL) {
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
