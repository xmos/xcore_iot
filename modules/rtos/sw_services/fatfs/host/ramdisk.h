// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef RAMDISK_H_
#define RAMDISK_H_

#include <stddef.h>
#include "ff.h"
#include "diskio.h"

BYTE *RAM_disk_raw(size_t *size);

DSTATUS RAM_disk_status(void);

DSTATUS RAM_disk_initialize(size_t size,
                            size_t sector_size);

DRESULT RAM_disk_read(BYTE *buff,
                      LBA_t sector,
                      UINT count);

DRESULT RAM_disk_write(const BYTE *buff,
                       LBA_t sector,
                       UINT count);

DRESULT RAM_disk_ioctl(BYTE cmd,
                       void *buff);


#endif /* RAMDISK_H_ */
