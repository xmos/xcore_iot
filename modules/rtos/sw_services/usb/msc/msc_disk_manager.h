// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef MSC_DISK_MANAGER_H_
#define MSC_DISK_MANAGER_H_

#include <stdbool.h>

#include "rtos_osal.h"

#ifndef MSC_MAX_DISKS
#define MSC_MAX_DISKS 1
#endif /* MSC_MAX_DISKS */

/**
 * Typedef to the MSC disk instance struct.
 */
typedef struct disk_desc_struct disk_desc_t;

/**
 * Struct representing an MSC disk instance.
 *
 * The members in this struct should not be accessed directly.
 */
struct disk_desc_struct {
    uint8_t *starting_addr;
    uint32_t block_count;
    uint16_t block_size;
    const char* vid; /* 8 chars max */
    const char* pid; /* 16 chars max */
    const char* rev; /* 4 chars max */
    bool have_been_init;
    rtos_osal_mutex_t mutex;

    __attribute__((fptrgroup("disk_init_fptr_grp")))
    bool (*init)(disk_desc_t *);

    __attribute__((fptrgroup("disk_ready_fptr_grp")))
    bool (*ready)(disk_desc_t *);

    /* Callback to handle Start Stop Unit command
     * Start = 0 stopped power mode
     * Start = 1 active power mode
     * If load_eject is set and start == 0 unload the disk
     * If load_eject is set and start == 1 load the disk
     */
    __attribute__((fptrgroup("disk_start_stop_fptr_grp")))
    bool (*start_stop)(disk_desc_t *, uint8_t, bool, bool);

    /* Copy up to bufsize bytes into buffer
     * Returns number of copied bytes
     */
    __attribute__((fptrgroup("disk_read_fptr_grp")))
    int32_t (*read)(disk_desc_t *, uint8_t *, uint32_t, uint32_t, uint32_t);

    /* Copy bufsize bytes from buffer to disk
     * Returns number of written bytes
     */
    __attribute__((fptrgroup("disk_write_fptr_grp")))
    int32_t (*write)(disk_desc_t *, const uint8_t *, uint32_t, uint32_t, uint32_t);

    /* Callback to handle SCSI commands other than those handled internally
     * by TinyUSB.
     *
     * Returns number of bytes in response
     *
     * This will be called for SCSI commands except READ_CAPACITY10,
     * READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE,
     * READ10 and WRITE10.
     */
    __attribute__((fptrgroup("disk_scsi_command_fptr_grp")))
    int32_t (*scsi_command)(disk_desc_t *, uint8_t, uint8_t *, const uint8_t *, uint16_t);

    void *args;
};

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
              void *args);

bool ram_disk_init(disk_desc_t *disk_ctx);
bool ram_disk_ready(disk_desc_t *disk_ctx);
bool ram_disk_start_stop(disk_desc_t *disk_ctx, uint8_t power_condition, bool start, bool load_eject);
int32_t ram_disk_read(disk_desc_t *disk_ctx, uint8_t *buffer, uint32_t lba, uint32_t offset, uint32_t bufsize);
int32_t ram_disk_write(disk_desc_t *disk_ctx, const uint8_t *buffer, uint32_t lba, uint32_t offset, uint32_t bufsize);
int32_t ram_disk_scsi_command(disk_desc_t *disk_ctx, uint8_t lun, uint8_t *buffer, const uint8_t *scsi_cmd, uint16_t bufsize);

bool qspi_flash_disk_init(disk_desc_t *disk_ctx);
bool qspi_flash_disk_ready(disk_desc_t *disk_ctx);
bool qspi_flash_disk_start_stop(disk_desc_t *disk_ctx, uint8_t power_condition, bool start, bool load_eject);
int32_t qspi_flash_disk_read(disk_desc_t *disk_ctx, uint8_t *buffer, uint32_t lba, uint32_t offset, uint32_t bufsize);
int32_t qspi_flash_disk_write(disk_desc_t *disk_ctx, const uint8_t *buffer, uint32_t lba, uint32_t offset, uint32_t bufsize);
int32_t qspi_flash_disk_scsi_command(disk_desc_t *disk_ctx, uint8_t lun, uint8_t *buffer, const uint8_t *scsi_cmd, uint16_t bufsize);

#endif  // MSC_DISK_MANAGER_H_
