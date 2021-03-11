// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#define DEBUG_UNIT MSC_RAMDISK

#include <stdio.h>

#include "rtos_osal.h"
#include "msc_disk_manager.h"
#include "tusb.h"

__attribute__((fptrgroup("disk_init_fptr_grp"))) __attribute__((weak))
bool ram_disk_init(disk_desc_t *disk_ctx)
{
    rtos_printf("ram_disk default init callback\n");
    return true;
}

__attribute__((fptrgroup("disk_ready_fptr_grp"))) __attribute__((weak))
bool ram_disk_ready(disk_desc_t *disk_ctx)
{
    rtos_printf("ram_disk default ready callback\n");
    return true;
}

__attribute__((fptrgroup("disk_start_stop_fptr_grp"))) __attribute__((weak))
bool ram_disk_start_stop(disk_desc_t *disk_ctx, uint8_t power_condition, bool start, bool load_eject)
{
    rtos_printf("ram_disk default start_stop callback\n");
    return true;
}

__attribute__((fptrgroup("disk_read_fptr_grp"))) __attribute__((weak))
int32_t ram_disk_read(disk_desc_t *disk_ctx, uint8_t *buffer, uint32_t lba, uint32_t offset, uint32_t bufsize)
{
    rtos_printf("ram_disk default read callback\n");
    uint8_t const* addr = disk_ctx->starting_addr + (lba * disk_ctx->block_size) + offset;
    memcpy(buffer, addr, bufsize);
    return bufsize;
}

__attribute__((fptrgroup("disk_write_fptr_grp"))) __attribute__((weak))
int32_t ram_disk_write(disk_desc_t *disk_ctx, const uint8_t *buffer, uint32_t lba, uint32_t offset, uint32_t bufsize)
{
    rtos_printf("ram_disk default write callback\n");
    uint8_t *addr = disk_ctx->starting_addr + (lba * disk_ctx->block_size) + offset;
    memcpy(addr, buffer, bufsize);
    return bufsize;
}

__attribute__((fptrgroup("disk_scsi_command_fptr_grp"))) __attribute__((weak))
int32_t ram_disk_scsi_command(disk_desc_t *disk_ctx, uint8_t lun, uint8_t *buffer, const uint8_t *scsi_cmd, uint16_t bufsize)
{
    rtos_printf("ram_disk default scsi command callback\n");
    uint16_t resplen = 0;

#if (CFG_TUD_MSC == 1)
    switch (scsi_cmd[0]) {
        case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        case SCSI_CMD_START_STOP_UNIT:
            resplen = 0;
        break;

        default:
            // Set Sense = Invalid Command Operation
            tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

            // negative means error -> tinyusb could stall and/or response with failed status
            resplen = -1;
            break;
    }
#endif /* CFG_TUD_MSC */

    return resplen;
}
