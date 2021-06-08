// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <xcore/hwtimer.h>

#include "FreeRTOS.h"

#include "dispatcher.h"
#include "unity.h"
#include "unity_fixture.h"

typedef struct test_work_arg {
  int core_flags[8];
} test_work_arg_t;

typedef struct test_parallel_work_arg {
  int begin;
  int end;
  int count;
} test_parallel_work_arg;

DISPATCHER_JOB_ATTRIBUTE
void do_isr_standard_work(void *p) {
  test_work_arg_t *arg = (test_work_arg_t *)p;

  arg->core_flags[rtos_core_id_get()] = 1;
}

DISPATCHER_JOB_ATTRIBUTE
void do_isr_parallel_work(void *p) {
  // NOTE: the "volatile" is needed here or the compiler may optimize this away
  test_parallel_work_arg volatile *arg = (test_parallel_work_arg volatile *)p;

  for (int i = arg->begin; i < arg->end; i++)
    arg->count++;
}

TEST_GROUP(isr_dispatcher);

TEST_SETUP(isr_dispatcher) {}

TEST_TEAR_DOWN(isr_dispatcher) {}

TEST(isr_dispatcher, test_wait_job) {
  dispatcher_t *disp;
  dispatch_job_t *job;
  test_work_arg_t arg;
  const uint32_t kCoreMap = 0b00000100;

  arg.core_flags[2] = 0;

  disp = dispatcher_create();
  dispatcher_isr_init(disp, kCoreMap);

  job = dispatch_job_create(do_isr_standard_work, &arg);
  dispatcher_job_add(disp, job);
  dispatcher_job_wait(disp, job);

  TEST_ASSERT_EQUAL_INT(1, arg.core_flags[2]);

  dispatcher_delete(disp);
}

TEST(isr_dispatcher, test_wait_group) {
  dispatcher_t *disp;
  dispatch_group_t *group;
  test_work_arg_t arg;
  const uint32_t kCoreMap = 0b11001000;
  const int kGroupLength = 3;

  disp = dispatcher_create();
  dispatcher_isr_init(disp, kCoreMap);

  group = dispatch_group_create(kGroupLength);

  // add jobs to group
  for (int i = 0; i < kGroupLength; i++) {
    dispatch_group_function_add(group, do_isr_standard_work, &arg);
  }

  dispatcher_group_add(disp, group);
  dispatcher_group_wait(disp, group);

  TEST_ASSERT_EQUAL_INT(1, arg.core_flags[7]);
  TEST_ASSERT_EQUAL_INT(1, arg.core_flags[6]);
  TEST_ASSERT_EQUAL_INT(1, arg.core_flags[3]);

  dispatch_group_delete(group);
  dispatcher_delete(disp);
}

TEST(isr_dispatcher, test_parallel) {
  const int kISRCount = 5;
  const uint32_t kCoreMap = 0b00111110;
  const int kGroupLength = 3;
  int num_values = kGroupLength * 1000;
  test_parallel_work_arg args[kISRCount];
  hwtimer_t hwtimer;
  int single_thread_ticks;

  // do single thread timing
  args[0].count = 0;
  args[0].begin = 0;
  args[0].end = num_values;

  hwtimer = hwtimer_alloc();
  single_thread_ticks = hwtimer_get_time(hwtimer);

  // do the work in this (single) thread
  do_isr_parallel_work(&args[0]);

  single_thread_ticks = hwtimer_get_time(hwtimer) - single_thread_ticks;
  hwtimer_free(hwtimer);

  TEST_ASSERT_EQUAL_INT(num_values, args[0].count);

  // do multi thread timing
  dispatcher_t *disp;
  dispatch_group_t *group;
  int num_values_in_chunk = num_values / kGroupLength;
  int multi_thread_ticks;

  // create the dispatcher
  disp = dispatcher_create();
  dispatcher_isr_init(disp, kCoreMap);

  // create the dispatch group
  group = dispatch_group_create(kGroupLength);

  // initialize kISRCount jobs, add them to the group
  for (int i = 0; i < kGroupLength; i++) {
    args[i].count = 0;
    args[i].begin = i * num_values_in_chunk;
    args[i].end = args[i].begin + num_values_in_chunk;
    dispatch_group_function_add(group, do_isr_parallel_work, &args[i]);
  }

  hwtimer = hwtimer_alloc();
  multi_thread_ticks = hwtimer_get_time(hwtimer);

  // add group to dispatcher
  dispatcher_group_add(disp, group);
  // wait for all jobs in the group to finish executing
  dispatcher_group_wait(disp, group);

  multi_thread_ticks = hwtimer_get_time(hwtimer) - multi_thread_ticks;
  hwtimer_free(hwtimer);

  for (int i = 0; i < kGroupLength; i++) {
    TEST_ASSERT_EQUAL_INT(num_values_in_chunk, args[i].count);
  }

  dispatch_group_delete(group);

  // now test that the multi thread was ~kISRCount times faster
  float actual_speedup = (float)single_thread_ticks / (float)multi_thread_ticks;
  float expected_speedup = (float)kGroupLength;
  float delta = 0.2 * expected_speedup;

  // rtos_printf("single_thread_ticks=%d  multi_thread_ticks=%d\n",
  //             single_thread_ticks, multi_thread_ticks);
  TEST_ASSERT_FLOAT_WITHIN(delta, expected_speedup, actual_speedup);

  dispatcher_delete(disp);
}

TEST_GROUP_RUNNER(isr_dispatcher) {
  RUN_TEST_CASE(isr_dispatcher, test_wait_job);
  RUN_TEST_CASE(isr_dispatcher, test_wait_group);
  RUN_TEST_CASE(isr_dispatcher, test_parallel);
}