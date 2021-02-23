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

#include "demo_main.h"
#include "tusb.h"

#include "rtos/drivers/qspi_flash/api/rtos_qspi_flash.h"
#include "msc_disk_manager.h"
#include "fs_support.h"

#define QSPI_FLASH_FILESYSTEM_START_ADDRESS 0x100000


//--------------------------------------------------------------------+
// LUN 1
//--------------------------------------------------------------------+
#define README0_CONTENTS \
"RAMDISK: This is tinyusb's MassStorage Class demo.\r\n\r\n\
If you find any bugs or get any questions, feel free to file an\r\n\
issue at github.com/hathach/tinyusb"

uint8_t ram_disk[16][512] =
{
  //------------- Block0: Boot Sector -------------//
  // byte_per_sector    = DISK_BLOCK_SIZE; fat12_sector_num_16  = DISK_BLOCK_NUM;
  // sector_per_cluster = 1; reserved_sectors = 1;
  // fat_num            = 1; fat12_root_entry_num = 16;
  // sector_per_fat     = 1; sector_per_track = 1; head_num = 1; hidden_sectors = 0;
  // drive_number       = 0x80; media_type = 0xf8; extended_boot_signature = 0x29;
  // filesystem_type    = "FAT12   "; volume_serial_number = 0x1234; volume_label = "TinyUSB 0  ";
  // FAT magic code at offset 510-511
  {
      0xEB, 0x3C, 0x90, 0x4D, 0x53, 0x44, 0x4F, 0x53, 0x35, 0x2E, 0x30, 0x00, 0x02, 0x01, 0x01, 0x00,
      0x01, 0x10, 0x00, 0x10, 0x00, 0xF8, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x29, 0x34, 0x12, 0x00, 0x00, 'T' , 'i' , 'n' , 'y' , 'U' ,
      'S' , 'B' , ' ' , '0' , ' ' , ' ' , 0x46, 0x41, 0x54, 0x31, 0x32, 0x20, 0x20, 0x20, 0x00, 0x00,

      // Zero up to 2 last bytes of FAT magic code
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA
  },

  //------------- Block1: FAT12 Table -------------//
  {
      0xF8, 0xFF, 0xFF, 0xFF, 0x0F // // first 2 entries must be F8FF, third entry is cluster end of readme file
  },

  //------------- Block2: Root Directory -------------//
  {
      // first entry is volume label
      'T' , 'i' , 'n' , 'y' , 'U' , 'S' , 'B' , ' ' , '0' , ' ' , ' ' , 0x08, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4F, 0x6D, 0x65, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      // second entry is readme file
      'R' , 'E' , 'A' , 'D' , 'M' , 'E' , '0' , ' ' , 'T' , 'X' , 'T' , 0x20, 0x00, 0xC6, 0x52, 0x6D,
      0x65, 0x43, 0x65, 0x43, 0x00, 0x00, 0x88, 0x6D, 0x65, 0x43, 0x02, 0x00,
      sizeof(README0_CONTENTS)-1, 0x00, 0x00, 0x00 // readme's files size (4 Bytes)
  },

  //------------- Block3: Readme Content -------------//
  README0_CONTENTS
};

void create_tinyusb_disks(void *args) {
    // add_disk(0,                             /* LUN */
    //          0x100000,                      /* starting address */
    //          1024,                          /* block count*/
    //          1024,                          /* block size*/
    //          "XMOS",                        /* vid */
    //          "Mass Storage",                /* pid */
    //          "1.0",                         /* rev */
    //          qspi_flash_disk_init,          /* init fptr */
    //          qspi_flash_disk_ready,         /* ready fptr */
    //          qspi_flash_disk_start_stop,    /* start_stop fptr */
    //          qspi_flash_disk_read,          /* read fptr */
    //          qspi_flash_disk_write,         /* write fptr */
    //          qspi_flash_disk_scsi_command,  /* scsi_command fptr */
    //          args                           /* pass qspi flash ctx as arg */
    //      );

     add_disk(0,                      /* LUN */
              (uint8_t*)ram_disk,     /* starting address */
              16,                     /* block count*/
              512,                    /* block size*/
              "TinyUSB",              /* vid */
              "Mass Storage",         /* pid */
              "1.0",                  /* rev */
              ram_disk_init,          /* init fptr */
              ram_disk_ready,         /* ready fptr */
              ram_disk_start_stop,    /* start_stop fptr */
              ram_disk_read,          /* read fptr */
              ram_disk_write,         /* write fptr */
              ram_disk_scsi_command,  /* scsi_command fptr */
              NULL                    /* pass no args */
          );
}










// // Invoked to determine max LUN
// uint8_t tud_msc_get_maxlun_cb(void)
// {
//     return (uint8_t)(sizeof(disks)/sizeof(disk_desc_t));
// }
//
// // Invoked when received SCSI_CMD_INQUIRY
// // Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
// void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
// {
//     memcpy(vendor_id  , disks[lun].vid, strlen(disks[lun].vid));
//     memcpy(product_id , disks[lun].pid, strlen(disks[lun].pid));
//     memcpy(product_rev, disks[lun].rev, strlen(disks[lun].rev));
// }
//
// // Invoked when received Test Unit Ready command.
// // return true allowing host to read/write this LUN e.g SD card inserted
// bool tud_msc_test_unit_ready_cb(uint8_t lun)
// {
//     return disks[lun].ready(disks[lun]);
// }
//
// // Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// // Application update block count and block size
// void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
// {
//   *block_count = disks[lun].block_count;
//   *block_size  = disks[lun].block_size;
// }
//
// // Invoked when received Start Stop Unit command
// // - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// // - Start = 1 : active mode, if load_eject = 1 : load disk storage
// bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
// {
//   (void) lun;
//   (void) power_condition;
//
//   if ( load_eject )
//   {
//     if (start)
//     {
//       // load disk storage
//     }else
//     {
//       // unload disk storage
//     }
//   }
//
//   return true;
// }
//
// // Callback invoked when received READ10 command.
// // Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
// int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
// {
//     if (lun == 0) {
//         extern rtos_qspi_flash_t *ff_qspi_flash_ctx;
//         uint32_t disk_block_size = disks[lun].block_size;
//
//         if( ff_qspi_flash_ctx != NULL )
//         {
//             rtos_qspi_flash_read(
//                 ff_qspi_flash_ctx,
//                 (uint8_t*)buffer,
//                 (unsigned)(QSPI_FLASH_FILESYSTEM_START_ADDRESS + (lba * disk_block_size) + offset),
//                 (size_t)bufsize);
//         } else {
//             configASSERT(0);
//         }
//     } else if (lun == 1) {
//         uint8_t const* addr = ram_disk[lba] + offset;
//         memcpy(buffer, addr, bufsize);
//     } else {
//         rtos_printf("Unexpected lun read\n");
//         configASSERT(0);
//     }
//
//   return bufsize;
// }
//
// // Callback invoked when received WRITE10 command.
// // Process data in buffer to disk's storage and return number of written bytes
// int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
// {
//   // uint8_t* addr = (lun ? msc_disk1[lba] : msc_disk0[lba])  + offset;
//   // memcpy(addr, buffer, bufsize);
//
//   return bufsize;
// }
//
// // Callback invoked when received an SCSI command not in built-in list below
// // - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// // - READ10 and WRITE10 has their own callbacks
// int32_t tud_msc_scsi_cb (uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize)
// {
//   // read10 & write10 has their own callback and MUST not be handled here
//
//   void const* response = NULL;
//   uint16_t resplen = 0;
//
//   // most scsi handled is input
//   bool in_xfer = true;
//
//   switch (scsi_cmd[0])
//   {
//     case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
//       // Host is about to read/write etc ... better not to disconnect disk
//       resplen = 0;
//     break;
//
//     case SCSI_CMD_START_STOP_UNIT:
//       // Host try to eject/safe remove/poweroff us. We could safely disconnect with disk storage, or go into lower power
//       /* scsi_start_stop_unit_t const * start_stop = (scsi_start_stop_unit_t const *) scsi_cmd;
//         // Start bit = 0 : low power mode, if load_eject = 1 : unmount disk storage as well
//         // Start bit = 1 : Ready mode, if load_eject = 1 : mount disk storage
//         start_stop->start;
//         start_stop->load_eject;
//        */
//        resplen = 0;
//     break;
//
//
//     default:
//       // Set Sense = Invalid Command Operation
//       tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
//
//       // negative means error -> tinyusb could stall and/or response with failed status
//       resplen = -1;
//     break;
//   }
//
//   // return resplen must not larger than bufsize
//   if ( resplen > bufsize ) resplen = bufsize;
//
//   if ( response && (resplen > 0) )
//   {
//     if(in_xfer)
//     {
//       memcpy(buffer, response, resplen);
//     }else
//     {
//       // SCSI output
//     }
//   }
//
//   return resplen;
// }
