// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public License: Version 1.
#ifndef XCORE_DEVICE_MEMORY_H_
#define XCORE_DEVICE_MEMORY_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "rtos/drivers/qspi_flash/api/rtos_qspi_flash.h"

#ifdef _TIME_H_
#define _clock_defined
#endif

#define IS_RAM(a) (((uintptr_t)a >= 0x80000) && ((uintptr_t)a <= 0x100000))
#define IS_NOT_RAM(a) ((uintptr_t)a > 0x100000)
#define IS_EXTMEM(a) \
  (((uintptr_t)a >= 0x10000000) && (((uintptr_t)a <= 0x20000000)))
#define IS_SWMEM(a) \
  (((uintptr_t)a >= 0x40000000) && (((uintptr_t)a <= 0x80000000)))

void swmem_setup(rtos_qspi_flash_t *ctx, unsigned swmem_task_priority);
void memload(void *dest, void *src, size_t size);

#ifdef __cplusplus
}
#endif

#endif  // XCORE_DEVICE_MEMORY_H_
