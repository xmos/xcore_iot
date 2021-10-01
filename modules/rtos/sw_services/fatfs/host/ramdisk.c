// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <stdlib.h>
#include <string.h>
#include "ramdisk.h"

static BYTE *ramdisk;
static size_t ramdisk_size;
static size_t ramdisk_sector_size;

BYTE *RAM_disk_raw(size_t *size)
{
    *size = ramdisk_size;
    return ramdisk;
}

DSTATUS RAM_disk_status(void)
{
    if (ramdisk != NULL) {
        return 0;
    } else {
        return STA_NOINIT;
    }
}

DSTATUS RAM_disk_initialize(size_t size,
                            size_t sector_size)
{
    if (ramdisk == NULL) {
        ramdisk_size = ((size + (sector_size - 1)) / sector_size) * sector_size;

        ramdisk = calloc(ramdisk_size, sizeof(BYTE));

        if (ramdisk != NULL) {
            ramdisk_sector_size = sector_size;
            return 0;
        } else {
            ramdisk_size = 0;
            ramdisk_sector_size = 0;
            return STA_NOINIT;
        }
    } else {
        return 0;
    }
}

DRESULT RAM_disk_read(BYTE *buff,     /* Data buffer to store read data */
                      LBA_t sector,   /* Start sector in LBA */
                      UINT count)     /* Number of sectors to read */
{
    if (ramdisk == NULL) {
        return RES_NOTRDY;
    }

    memcpy(buff,
           ramdisk + sector * ramdisk_sector_size,
           count * ramdisk_sector_size);

    return RES_OK;
}

DRESULT RAM_disk_write(const BYTE *buff,   /* Data to be written */
                       LBA_t sector,       /* Start sector in LBA */
                       UINT count)         /* Number of sectors to write */
{
    if (ramdisk == NULL) {
        return RES_NOTRDY;
    }

    memcpy(ramdisk + sector * ramdisk_sector_size,
           buff,
           count * ramdisk_sector_size);

    return RES_OK;
}

DRESULT RAM_disk_ioctl(BYTE cmd,       /* Control code */
                       void *buff)     /* Buffer to send/receive control data */
{
    DRESULT res;

    if (ramdisk == NULL) {
        return RES_NOTRDY;
    }

    switch (cmd) {
    case CTRL_SYNC:
        res = RES_OK;
        break;

    case GET_SECTOR_COUNT:
        *(LBA_t *)buff = ramdisk_size / ramdisk_sector_size;
        res = RES_OK;
        break;

    case GET_SECTOR_SIZE:
        *(WORD *)buff = (WORD) ramdisk_sector_size;
        res = RES_OK;
        break;

    case GET_BLOCK_SIZE:
        *(DWORD *)buff = 1;
        res = RES_OK;
        break;

    case CTRL_TRIM:
        res = RES_OK;
        break;

    default:
        res = RES_PARERR;
        break;
    }

    return res;
}

