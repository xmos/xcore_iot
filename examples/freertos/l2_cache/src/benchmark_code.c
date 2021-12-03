// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdlib.h>
#include <stdio.h>
#include <xcore/assert.h>
#include <xcore/hwtimer.h>
#include <xcore/minicache.h>

#include "benchmark_code.h"
#include "benchmark_data.h"
#include "swmem_macros.h"
#include "print_info.h"

// increment 's' 16 * 24 = 384 times
# define UNROLLED_LOOP(s) \
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

XCORE_CODE_SECTION_ATTRIBUTE
int unrolled_loop() {
  int sum = 0;

  UNROLLED_LOOP(sum);

  return sum;
}

XCORE_CODE_SECTION_ATTRIBUTE
int dot_product(int a[], int b[], unsigned len)
{
  int c = 0;
  for(int k = 0; k < len; k++){
    c += (a[k]*b[k]);
  }
  return c;
}

//////////////////////////////////////////////////////////////////

void benchmark_sequential_data_throughput(void) {
  volatile int computed_sum = 0;

  uint32_t elapsed_time;

  debug_printf("\nSequential data throughput\n");

  elapsed_time = get_reference_time();

  elapsed_time = get_reference_time();
  for (int i = 0; i < data_array_len; i++) {
    computed_sum += data_array[i];
  }
  elapsed_time = get_reference_time() - elapsed_time;
  xassert(computed_sum == 2147450880); // Check computed_sum

  print_info(elapsed_time);
}

//////////////////////////////////////////////////////////////////

#define PREFETCH(ADDR)    minicache_prefetch(ADDR)

void benchmark_sequential_data_throughput_with_prefetch(void) {

  debug_printf("\nSequential data timing (with prefetch)\n");

  uint32_t elapsed_time;
  int computed_sum = 0;
  const unsigned pages = (data_array_size + 255) >> 8;
  //////////// TIMED ////////////
  elapsed_time = get_reference_time();
  PREFETCH( &data_array[0] );
  for(int page = 0; page < pages; page++){
    const int start_dex = page * 64;
    const int end_dex = (page+1) * 64;
    PREFETCH( &data_array[end_dex] );

    for(int i = start_dex; i < end_dex; i++){
      computed_sum += data_array[i];
    }
  }
  elapsed_time = get_reference_time() - elapsed_time;
  //////////////////////////////
  xassert(computed_sum == 2147450880); // Check computed_sum
  print_info(elapsed_time);
}


//////////////////////////////////////////////////////////////////


void benchmark_random_data_throughput(void) {
  volatile int computed_sum = 0;
  uint32_t elapsed_time;
  int index;

  debug_printf("\nRandom data throughput\n");

  srand(0);
  elapsed_time = get_reference_time();
  for (int i = 0; i < data_array_len; i++) {
    index = rand() % data_array_len; // for random
    int val = data_array[index];
    computed_sum += val;
  }
  elapsed_time = get_reference_time() - elapsed_time;

  print_info(elapsed_time);
}


//////////////////////////////////////////////////////////////////


void benchmark_code(void) {

  debug_printf("\nUnrolled loop\n");

  uint32_t elapsed_time;
  int result;

  elapsed_time = get_reference_time();
  result = unrolled_loop();
  elapsed_time = get_reference_time() - elapsed_time;

  print_info(elapsed_time);
}
