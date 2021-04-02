// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef XCORE_DEVICE_MEMORY_H_
#define XCORE_DEVICE_MEMORY_H_

#include <stddef.h>
#include <stdint.h>
#include <xs1.h>

#define STRINGIFY(NAME) #NAME
#define GET_STACKWORDS(DEST, NAME) \
  asm("ldc %[__dest], " STRINGIFY(NAME) ".nstackwords" : [ __dest ] "=r"(DEST))
#define GET_STACKSIZE(DEST, NAME)                        \
  {                                                      \
    size_t _stack_words;                                 \
    asm("ldc %[__dest], " STRINGIFY(NAME) ".nstackwords" \
        : [ __dest ] "=r"(_stack_words));                \
    DEST = (_stack_words + 2) * 4;                       \
  }

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

/**
 * Connect to the flash memory and setup the SwMem minicache fill handling
 */
void swmem_setup();

/**
 * Disconnect from the flash memory and tear down the SwMem handling
 */
void swmem_teardown();

/**
 * SwMem minicache fill event handler
 *
 * Start this task after calling swmem_setup() and before accessing the SwMem
 * memory segment and before calling swmem_load().
 */
void swmem_handler(void *ignored);

/**
 * Load memory from an external memory segment.
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
