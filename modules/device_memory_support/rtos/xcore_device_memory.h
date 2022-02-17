// Copyright 2021-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef XCORE_DEVICE_MEMORY_H_
#define XCORE_DEVICE_MEMORY_H_

#include <stddef.h>
#include <xs1.h>

#define IS_RAM(a)                    \
  (((uintptr_t)a >= XS1_RAM_BASE) && \
   ((uintptr_t)a <= (XS1_RAM_BASE + XS1_RAM_SIZE)))
#define IS_NOT_RAM(a) ((uintptr_t)a > XS1_RAM_BASE)
#define IS_EXTMEM(a)                    \
  (((uintptr_t)a >= XS1_EXTMEM_BASE) && \
   (((uintptr_t)a <= (XS1_EXTMEM_BASE + XS1_EXTMEM_SIZE))))
#define IS_SWMEM(a)                    \
  (((uintptr_t)a >= XS1_SWMEM_BASE) && \
   (((uintptr_t)a <= (XS1_SWMEM_BASE - 1 + XS1_SWMEM_SIZE))))

#ifdef __cplusplus
extern "C" {
#endif

#include "rtos_qspi_flash.h"

/**
 * Initialize and start the SwMem driver.
 *
 * @param[in]  ctx  RTOS QSPI flash driver context
 * @param[in]  swmem_task_priority RTOS task priority
 */
void swmem_setup(rtos_qspi_flash_t *ctx, unsigned swmem_task_priority);

/**
 * Load memory from the SwMem memory segment.
 *
 * @param[out] dest Pointer to the memory location to copy to
 * @param[in]  src  Pointer to the memory location to copy from
 * @param[in]  size Number of bytes to copy
 */
size_t swmem_load(void *dest, const void *src, size_t size);

#ifdef __cplusplus
}
#endif

#endif  // XCORE_DEVICE_MEMORY_H_
