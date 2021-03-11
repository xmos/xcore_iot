// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "rtos_osal.h"

#include "msc_disk_manager.h"
#include "tusb.h"

#define disk_lock(disk_ptr)     do {                                            \
                                    rtos_osal_mutex_get(&disk_ptr->mutex,       \
                                                       RTOS_OSAL_WAIT_FOREVER); \
                                } while(0)

#define disk_unlock(disk_ptr)   do {                                            \
                                    rtos_osal_mutex_put(&disk_ptr->mutex);      \
                                } while(0)

static disk_desc_t disks[MSC_MAX_DISKS] = {{0}};

bool add_disk(uint32_t lun,
              uint8_t *starting_addr,
              uint32_t block_count,
              uint16_t block_size,
              const char* vid,
              const char* pid,
              const char* rev,
              bool (*init)(disk_desc_t *),
              bool (*ready)(disk_desc_t *),
              bool (*start_stop)(disk_desc_t *, uint8_t, bool, bool),
              int32_t (*read)(disk_desc_t *, uint8_t *, uint32_t, uint32_t, uint32_t),
              int32_t (*write)(disk_desc_t *, const uint8_t *, uint32_t, uint32_t, uint32_t),
              int32_t (*scsi_command)(disk_desc_t *, uint8_t, uint8_t *, const uint8_t *, uint16_t),
              void *args)
{
    bool retval = false;

    do {
        if (lun < MSC_MAX_DISKS) {
            disk_desc_t *this_disk = &disks[lun];

            if ((strlen(vid) > 8) || (strlen(pid) > 16) || (strlen(rev) > 4)) {
                rtos_printf("vid, pid, or rev strings are too long\n");
                break;
            }

            this_disk->starting_addr = starting_addr;
            this_disk->block_count = block_count;
            this_disk->block_size = block_size;
            this_disk->vid = vid;
            this_disk->pid = pid;
            this_disk->rev = rev;
            this_disk->init = init;
            rtos_osal_mutex_create(&this_disk->mutex, "disk_lock", RTOS_OSAL_NOT_RECURSIVE);    // TODO: unique names for easier debug
            this_disk->ready = ready;
            this_disk->start_stop = start_stop;
            this_disk->read = read;
            this_disk->write = write;
            this_disk->scsi_command = scsi_command;
            this_disk->args = args;

            retval = (this_disk->init == NULL) ? true : this_disk->init(this_disk);
            this_disk->have_been_init = true;
        } else {
            configASSERT(0); /* Invalid lun */
        }
    } while (0);

    return retval;
}

__attribute__((weak))
uint8_t tud_msc_get_maxlun_cb(void)
{
    return (uint8_t)(MSC_MAX_DISKS);
}

__attribute__((weak))
void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
    if (lun < MSC_MAX_DISKS) {
        if (disks[lun].have_been_init == true) {
            disk_desc_t *this_disk = &disks[lun];

            disk_lock(this_disk);
            {
                memcpy(vendor_id  , this_disk->vid, strlen(this_disk->vid));
                memcpy(product_id , this_disk->pid, strlen(this_disk->pid));
                memcpy(product_rev, this_disk->rev, strlen(this_disk->rev));
            }
            disk_unlock(this_disk);
        } else {
            rtos_printf("Disk LUN[%d] not initialized\n", lun);
        }
    } else {
        memset(vendor_id, 0x00, sizeof(uint8_t)*8);
        memset(product_id, 0x00, sizeof(uint8_t)*16);
        memset(product_rev, 0x00, sizeof(uint8_t)*12);
    }
}

__attribute__((weak))
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    bool retval = false;

    if (lun < MSC_MAX_DISKS) {
        if (disks[lun].have_been_init == true) {
            disk_desc_t *this_disk = &disks[lun];

            disk_lock(this_disk);
            {
                retval = this_disk->ready(this_disk);
            }
            disk_unlock(this_disk);
        } else {
            configASSERT(0); /* Disk not initialized */
        }
    } else {
        configASSERT(0); /* Invalid lun */
    }

    return retval;
}

__attribute__((weak))
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
{
    if (lun < MSC_MAX_DISKS) {
        if (disks[lun].have_been_init == true) {
            disk_desc_t *this_disk = &disks[lun];

            disk_lock(this_disk);
            {
                *block_count = this_disk->block_count;
                *block_size  = this_disk->block_size;
            }
            disk_unlock(this_disk);
        } else {
            configASSERT(0); /* Disk not initialized */
        }
    } else {
        configASSERT(0); /* Invalid lun */
    }
}

__attribute__((weak))
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
    bool retval = true;

    if (lun < MSC_MAX_DISKS) {
        disk_desc_t *this_disk = &disks[lun];

        if (this_disk->have_been_init == true) {
            disk_lock(this_disk);
            {
                retval = this_disk->start_stop(this_disk, power_condition, start, load_eject);
            }
            disk_unlock(this_disk);
        } else {
            configASSERT(0); /* Disk not initialized */
        }
    } else {
        configASSERT(0); /* Invalid lun */
    }

    return retval;
}

__attribute__((weak))
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
    int32_t retval = 0;

    if (lun < MSC_MAX_DISKS) {
        disk_desc_t *this_disk = &disks[lun];

        if (this_disk->have_been_init == true) {
            disk_lock(this_disk);
            {
                retval = this_disk->read(this_disk, buffer, lba, offset, bufsize);
            }
            disk_unlock(this_disk);
        } else {
            configASSERT(0); /* Disk not initialized */
        }
    } else {
        configASSERT(0); /* Invalid lun */
    }

    return retval;
}

__attribute__((weak))
int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
    int32_t retval = 0;

    if (lun < MSC_MAX_DISKS) {
        disk_desc_t *this_disk = &disks[lun];

        if (this_disk->have_been_init == true) {
            disk_lock(this_disk);
            {
                retval = this_disk->write(this_disk, buffer, lba, offset, bufsize);
            }
            disk_unlock(this_disk);
        } else {
            configASSERT(0); /* Disk not initialized */
        }
    } else {
        configASSERT(0); /* Invalid lun */
    }

    return retval;
}

__attribute__((weak))
int32_t tud_msc_scsi_cb (uint8_t lun, uint8_t const scsi_cmd[16], void* buffer, uint16_t bufsize)
{
    int32_t retval = 0;

    if (lun < MSC_MAX_DISKS) {
        disk_desc_t *this_disk = &disks[lun];

        if (this_disk->have_been_init == true) {
            disk_lock(this_disk);
            {
                retval = this_disk->scsi_command(this_disk, lun, buffer, scsi_cmd, bufsize);
            }
            disk_unlock(this_disk);
        } else {
            configASSERT(0); /* Disk not initialized */
        }
    } else {
        configASSERT(0); /* Invalid lun */
    }

    return retval;
}
