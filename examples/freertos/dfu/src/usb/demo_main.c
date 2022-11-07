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
#include "timers.h"
#include "platform/driver_instances.h"
#include "demo_main.h"
#include "tusb.h"

/*
 * After device is enumerated in dfu mode run the following commands
 *
 * To transfer firmware from host to device (best to test with text file)
 *
 * $ dfu-util -d cafe -a 0 -D [filename]
 * $ dfu-util -d cafe -a 1 -D [filename]
 *
 * To transfer firmware from device to host:
 *
 * $ dfu-util -d cafe -a 0 -U [filename]
 * $ dfu-util -d cafe -a 1 -U [filename]
 *
 */

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+
const char* upload_image[2]=
{
  "Hello world from TinyUSB DFU! - Partition 0",
  "Hello world from TinyUSB DFU! - Partition 1"
};

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
static rtos_gpio_port_id_t led_port = 0;
static uint32_t led_val = 0;
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;


static void reboot(void);

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  if(blinky_timer_ctx != NULL)
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  if(blinky_timer_ctx != NULL)
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), 0);
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
  if(blinky_timer_ctx != NULL)
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_SUSPENDED), 0);
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  if(blinky_timer_ctx != NULL)
    xTimerChangePeriod(blinky_timer_ctx, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

//--------------------------------------------------------------------+
// DFU callbacks
// Note: alt is used as the partition number, in order to support multiple partitions like FLASH, EEPROM, etc.
//--------------------------------------------------------------------+

static size_t total_len = 0;
static size_t bytes_avail = 0;
static uint32_t dn_base_addr = 0;

// Invoked right before tud_dfu_download_cb() (state=DFU_DNBUSY) or tud_dfu_manifest_cb() (state=DFU_MANIFEST)
// Application return timeout in milliseconds (bwPollTimeout) for the next download/manifest operation.
// During this period, USB host won't try to communicate with us.
uint32_t tud_dfu_get_timeout_cb(uint8_t alt, uint8_t state)
{
  if ( state == DFU_DNBUSY )
  {
    // For this example
    // - Atl0 Flash is fast : 10  ms
    // - Alt1 EEPROM is slow: 100 ms
    return (alt == 0) ? 10 : 100;
  }
  else if (state == DFU_MANIFEST)
  {
    // since we don't buffer entire image and do any flashing in manifest stage
    return 0;
  }

  return 0;
}

// Invoked when received DFU_DNLOAD (wLength>0) following by DFU_GETSTATUS (state=DFU_DNBUSY) requests
// This callback could be returned before flashing op is complete (async).
// Once finished flashing, application must call tud_dfu_finish_flashing()
void tud_dfu_download_cb(uint8_t alt, uint16_t block_num, uint8_t const* data, uint16_t length)
{
  // (void) alt;
  // (void) block_num;

  rtos_printf("Received Alt %d BlockNum %d of length %d\n", alt, block_num, length);

  // if (dn_base_addr == 0)
  // {
  //     uint32_t addr = 0;
  //     total_len = 0;
  //     int tmp = boot_image_locate_available_spot(bim_ctx_ptr, &addr, &bytes_avail);
  //     if(tmp == -1)
  //     {
  //         boot_image_t* imgptr = NULL;
  //         imgptr = boot_image_get_last_image(bim_ctx_ptr);
  //         addr = imgptr->startAddress;
  //         bytes_avail = bim_ctx_ptr->boot_partition_size - addr;
  //     }
  //     rtos_printf("Using addr 0x%x\nsize %u\n", addr, bytes_avail);
  //     dn_base_addr = addr;
  // }

  // if(length > 0)
  // {
  //     unsigned cur_addr = dn_base_addr + (block_num * bim_ctx_ptr->page_size);
  //     if((bytes_avail - total_len) >= length)
  //     {
  //         // rtos_printf("write %d at 0x%x\n", length, cur_addr);
  //         boot_image_write(bim_ctx_ptr->app_data, cur_addr, data, length);
  //         total_len += length;
  //     } else {
  //         rtos_printf("Insufficient space\n");
  //     }
  // }

  // flashing op for download complete without error
  tud_dfu_finish_flashing(DFU_STATUS_OK);
}

// Invoked when download process is complete, received DFU_DNLOAD (wLength=0) following by DFU_GETSTATUS (state=Manifest)
// Application can do checksum, or actual flashing if buffered entire image previously.
// Once finished flashing, application must call tud_dfu_finish_flashing()
void tud_dfu_manifest_cb(uint8_t alt)
{
  (void) alt;
  rtos_printf("Download completed, enter manifestation\n");

  // flashing op for manifest is complete without error
  // Application can perform checksum, should it fail, use appropriate status such as errVERIFY.
  tud_dfu_finish_flashing(DFU_STATUS_OK);

    //   rtos_printf("Pass firmware validity check addr 0x%x size %u\n", dn_base_addr, total_len);
    // set_rt_mode();
    // boot_image_read(bim_ctx_ptr->app_data, 0, &dummy, sizeof(dummy));   // dummy read to ensure flash writes have completed
    // reboot();
}

// Invoked when received DFU_UPLOAD request
// Application must populate data with up to length bytes and
// Return the number of written bytes
uint16_t tud_dfu_upload_cb(uint8_t alt, uint16_t block_num, uint8_t* data, uint16_t length)
{
  (void) block_num;
  (void) length;

  uint16_t const xfer_len = (uint16_t) strlen(upload_image[alt]);
  memcpy(data, upload_image[alt], xfer_len);

  return xfer_len;
  
//     (void) alt;

//     memset(data, 0x00, length);
//     uint32_t addr = block_num * FLASH_PAGE_SIZE;
//     uint32_t endaddr;

// #if 0
//     // Test code which will just read out all of flash rather than a specific image
//     endaddr = 0x800000;
// #else
//     boot_image_t* imgptr = NULL;
//     imgptr = boot_image_get_last_image(bim_ctx_ptr);
//     addr += imgptr->startAddress;
//     endaddr = imgptr->startAddress + imgptr->size;
// #endif
//     return (addr >= endaddr) ? 0 : (uint16_t)boot_image_read(bim_ctx_ptr->app_data, addr, data, length);


}

// Invoked when the Host has terminated a download or upload transfer
void tud_dfu_abort_cb(uint8_t alt)
{
  (void) alt;
  rtos_printf("Host aborted transfer\n");
  // set_rt_mode();
  // boot_image_read(bim_ctx_ptr->app_data, 0, &dummy, sizeof(dummy));   // dummy read to ensure flash writes have completed
}

// Invoked when a DFU_DETACH request is received
void tud_dfu_detach_cb(void)
{
  rtos_printf("Host detach, we should probably reboot\n");
    // set_rt_mode();
    reboot();
}

void tud_dfu_runtime_reboot_to_dfu_cb(void)
{
    rtos_printf("Host detach, reboot\n");
    // set_dfu_mode();
    reboot();
}

static void reboot(void)
{
    rtos_printf("Reboot initiated by tile:0x%x\n", get_local_tile_id());
    write_sswitch_reg_no_ack(get_local_tile_id(), XS1_SSWITCH_WATCHDOG_COUNT_NUM, 0x10000);
    write_sswitch_reg_no_ack(get_local_tile_id(), XS1_SSWITCH_WATCHDOG_CFG_NUM, (1 << XS1_WATCHDOG_COUNT_ENABLE_SHIFT) | (1 << XS1_WATCHDOG_TRIGGER_ENABLE_SHIFT) );
    while(1) {;}
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+

void led_blinky_cb(TimerHandle_t xTimer)
{
    (void) xTimer;
    led_val ^= 1;

    rtos_gpio_port_out(gpio_ctx_t0, led_port, led_val);
}

void create_tinyusb_demo(unsigned priority)
{
    led_port = rtos_gpio_port(PORT_LEDS);
    rtos_gpio_port_enable(gpio_ctx_t0, led_port);
    rtos_gpio_port_out(gpio_ctx_t0, led_port, led_val);

    blinky_timer_ctx = xTimerCreate("blinky",
                                    pdMS_TO_TICKS(blink_interval_ms),
                                    pdTRUE,
                                    NULL,
                                    led_blinky_cb);
    xTimerStart(blinky_timer_ctx, 0);
}
