// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdlib.h>
#include <stdio.h>
#include <xcore/assert.h>
#include <xcore/hwtimer.h>
#include <xcore/minicache.h>

#include "example_code.h"
#include "print_info.h"

// increment 's' 16 * 24 = 384 times
#define UNROLLED_LOOP(s) \
{ \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
  s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; s += 1; \
}

__attribute__((section(".SwMem_code")))
int unrolled_loop() {
  int sum = 0;

  UNROLLED_LOOP(sum);

  return sum;
}

void example_code(void) {
  uint32_t elapsed_time;
  int result;

  debug_printf("\nUnrolled loop\n");
  elapsed_time = get_reference_time();
  result = unrolled_loop();
  elapsed_time = get_reference_time() - elapsed_time;
  print_info(elapsed_time);
}
