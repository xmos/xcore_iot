// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef APP_COMMON_H_
#define APP_COMMON_H_

// #include "l2_cache_config.h"

#ifndef __ASSEMBLER__

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include <xcore/_support/xcore_common.h>
#include <xcore/_support/xcore_macros.h>

#define WORD_ALIGNED  __attribute__((aligned(4)))
#define DWORD_ALIGNED  __attribute__((aligned(8)))

#define THREAD_STACK_SIZE(thread_entry) \
    ({ uint32_t stack_size; \
       asm volatile ( "ldc %0, " #thread_entry ".nstackwords" : "=r"(stack_size) ); \
        stack_size; })


static inline void* STACK_BASE(void * const __mem_base, size_t const __words) _XCORE_NOTHROW
{
  int *stack_top;
  int *stack_buf = __mem_base;
  stack_top = &(stack_buf[__words - 1]);
  stack_top = (int *) ((uint32_t) stack_top & ~(_XCORE_STACK_ALIGN_REQUIREMENT - 1));
  /* Check the alignment of the calculated top of stack is correct. */
  assert(((uint32_t) stack_top & (_XCORE_STACK_ALIGN_REQUIREMENT - 1)) == 0UL);
  return stack_top;
}

#endif // ! __ASSEMBLER__
#endif //APP_COMMON_H_
