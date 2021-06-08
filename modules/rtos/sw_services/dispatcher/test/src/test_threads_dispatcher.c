// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <xcore/hwtimer.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "dispatcher.h"
#include "unity.h"
#include "unity_fixture.h"

#define QUEUE_THREAD_PRIORITY (configMAX_PRIORITIES - 2)

typedef struct test_work_arg {
  int count;
} test_work_arg_t;

typedef struct test_parallel_work_arg {
  int begin;
  int end;
  int count;
} test_parallel_work_arg;

static SemaphoreHandle_t mutex;

inline void look_busy(int milliseconds) {
  const TickType_t xDelay = milliseconds / portTICK_PERIOD_MS;
  vTaskDelay(xDelay);
}

DISPATCHER_JOB_ATTRIBUTE
void do_thread_limited_work(void *p) {
  test_work_arg_t *arg = (test_work_arg_t *)p;

  look_busy(100);

  xSemaphoreTake(mutex, portMAX_DELAY);
  arg->count++;
  xSemaphoreGive(mutex);
}

DISPATCHER_JOB_ATTRIBUTE
void do_thread_standard_work(void *p) {
  test_work_arg_t *arg = (test_work_arg_t *)p;

  look_busy(500);

  xSemaphoreTake(mutex, portMAX_DELAY);
  arg->count++;
  xSemaphoreGive(mutex);
}

DISPATCHER_JOB_ATTRIBUTE
void do_thread_extended_work(void *p) {
  test_work_arg_t *arg = (test_work_arg_t *)p;

  look_busy(1000);

  xSemaphoreTake(mutex, portMAX_DELAY);
  arg->count++;
  xSemaphoreGive(mutex);
}

DISPATCHER_JOB_ATTRIBUTE
void do_thread_parallel_work(void *p) {
  // NOTE: the "volatile" is needed here or the compiler may optimize this away
  test_parallel_work_arg volatile *arg = (test_parallel_work_arg volatile *)p;

  for (int i = arg->begin; i < arg->end; i++)
    arg->count++;
}

TEST_GROUP(threads_dispatcher);

TEST_SETUP(threads_dispatcher) { mutex = xSemaphoreCreateMutex(); }

TEST_TEAR_DOWN(threads_dispatcher) { vSemaphoreDelete(mutex); }

TEST(threads_dispatcher, test_wait_job) {
  dispatcher_t *disp;
  dispatch_job_t *job;
  test_work_arg_t arg;
  const int kQueueLength = 10;
  const int kThreadCount = 3;

  disp = dispatcher_create();
  dispatcher_thread_init(disp, kQueueLength, kThreadCount,
                         QUEUE_THREAD_PRIORITY);

  arg.count = 0;

  job = dispatch_job_create(do_thread_standard_work, &arg);
  dispatcher_job_add(disp, job);
  dispatcher_job_wait(disp, job);

  TEST_ASSERT_EQUAL_INT(1, arg.count);

  dispatcher_delete(disp);
}

TEST(threads_dispatcher, test_wait_group) {
  dispatcher_t *disp;
  dispatch_group_t *group;
  test_work_arg_t arg;
  const int kQueueLength = 10;
  const int kThreadCount = 3;
  const int kGroupLength = 3;

  disp = dispatcher_create();
  dispatcher_thread_init(disp, kQueueLength, kThreadCount,
                         QUEUE_THREAD_PRIORITY);

  group = dispatch_group_create(kGroupLength);

  arg.count = 0;

  // add jobs to group
  for (int i = 0; i < kGroupLength; i++) {
    dispatch_group_function_add(group, do_thread_standard_work, &arg);
  }

  dispatcher_group_add(disp, group);
  dispatcher_group_wait(disp, group);

  TEST_ASSERT_EQUAL_INT(kGroupLength, arg.count);

  dispatch_group_delete(group);
  dispatcher_delete(disp);
}

TEST(threads_dispatcher, test_mixed_durations1) {
  dispatcher_t *disp;
  dispatch_job_t *standard_job;
  dispatch_job_t **extended_jobs;
  test_work_arg_t arg;
  const int kQueueLength = 10;
  const int kThreadCount = 3;
  const int kMaxExtendedTaskCount = 3 + 1;
  int extended_job_count;

  for (int i = 0; i < kMaxExtendedTaskCount; i++) {
    disp = dispatcher_create();
    dispatcher_thread_init(disp, kQueueLength, kThreadCount,
                           QUEUE_THREAD_PRIORITY);

    extended_job_count = i + 1;

    extended_jobs = malloc(extended_job_count * sizeof(dispatch_job_t *));

    // create all the jobs
    for (int j = 0; j < extended_job_count; j++) {
      extended_jobs[j] = dispatch_job_create(do_thread_extended_work, &arg);
    }
    standard_job = dispatch_job_create(do_thread_standard_work, &arg);

    arg.count = 0;

    // add extended jobs
    for (int j = 0; j < extended_job_count; j++) {
      dispatcher_job_add(disp, extended_jobs[j]);
    }

    // add limited job
    dispatcher_job_add(disp, standard_job);

    // now wait for the standard job
    dispatcher_job_wait(disp, standard_job);
    if (extended_job_count < 3)
      TEST_ASSERT_EQUAL_INT(1, arg.count);
    else
      TEST_ASSERT_EQUAL_INT(3 + 1, arg.count);

    // now wait for the last added extended job
    dispatcher_job_wait(disp, extended_jobs[extended_job_count - 1]);
    TEST_ASSERT_EQUAL_INT(extended_job_count + 1, arg.count);

    // cleanup
    for (int j = 0; j < extended_job_count - 1; j++) {
      dispatch_job_delete(extended_jobs[j]);
    }
    dispatcher_delete(disp);
    free(extended_jobs);
  }
}

TEST(threads_dispatcher, test_mixed_durations2) {
  dispatcher_t *disp;
  dispatch_job_t *extended_job1;
  dispatch_job_t *extended_job2;
  dispatch_group_t *limited_group;
  test_work_arg_t extended_arg;
  test_work_arg_t limited_arg;
  const int kQueueLength = 10;
  const int kThreadCount = 3;

  disp = dispatcher_create();
  dispatcher_thread_init(disp, kQueueLength, kThreadCount,
                         QUEUE_THREAD_PRIORITY);

  limited_group = dispatch_group_create(3);

  dispatch_group_function_add(limited_group, do_thread_limited_work,
                              &limited_arg);
  dispatch_group_function_add(limited_group, do_thread_limited_work,
                              &limited_arg);
  dispatch_group_function_add(limited_group, do_thread_limited_work,
                              &limited_arg);

  extended_arg.count = 0;
  limited_arg.count = 0;

  // add first extended job
  extended_job1 =
      dispatcher_function_add(disp, do_thread_extended_work, &extended_arg);
  dispatcher_job_wait(disp, extended_job1);
  TEST_ASSERT_EQUAL_INT(1, extended_arg.count);

  // add limited job group, and second extended job
  dispatcher_group_add(disp, limited_group);
  extended_job2 =
      dispatcher_function_add(disp, do_thread_extended_work, &extended_arg);

  // now wait for the limited jobs
  dispatcher_group_wait(disp, limited_group);
  TEST_ASSERT_EQUAL_INT(3, limited_arg.count);

  // now wait for the second extended job
  dispatcher_job_wait(disp, extended_job2);
  TEST_ASSERT_EQUAL_INT(2, extended_arg.count);

  dispatch_group_delete(limited_group);
  dispatcher_delete(disp);
}

TEST(threads_dispatcher, test_parallel) {
  const int kThreadCount = 5;
  const int kQueueLength = 10;
  const int kGroupLength = 3;
  const int kGroupIterations = 30;
  int num_values = kGroupLength * 1000000;
  test_parallel_work_arg args[kThreadCount];
  hwtimer_t hwtimer;
  int single_thread_ticks;

  // do single thread timing
  args[0].count = 0;
  args[0].begin = 0;
  args[0].end = num_values;

  hwtimer = hwtimer_alloc();
  single_thread_ticks = hwtimer_get_time(hwtimer);

  // do the work in this (single) thread
  do_thread_parallel_work(&args[0]);

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
  dispatcher_thread_init(disp, kQueueLength, kThreadCount,
                         QUEUE_THREAD_PRIORITY);

  for (int iter = 0; iter < kGroupIterations; iter++) {
    // create the dispatch group
    group = dispatch_group_create(kGroupLength);

    // initialize kThreadCount jobs, add them to the group
    for (int i = 0; i < kGroupLength; i++) {
      args[i].count = 0;
      args[i].begin = i * num_values_in_chunk;
      args[i].end = args[i].begin + num_values_in_chunk;
      dispatch_group_function_add(group, do_thread_parallel_work, &args[i]);
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

    // now test that the multi thread was ~kThreadCount times faster
    float actual_speedup =
        (float)single_thread_ticks / (float)multi_thread_ticks;
    float expected_speedup = (float)kGroupLength;
    float delta = 0.2 * expected_speedup;

    // rtos_printf("single_thread_ticks=%d  multi_thread_ticks=%d\n",
    //             single_thread_ticks, multi_thread_ticks);
    TEST_ASSERT_FLOAT_WITHIN(delta, expected_speedup, actual_speedup);
  }

  dispatcher_delete(disp);
}

TEST_GROUP_RUNNER(threads_dispatcher) {
  RUN_TEST_CASE(threads_dispatcher, test_wait_job);
  RUN_TEST_CASE(threads_dispatcher, test_wait_group);
  RUN_TEST_CASE(threads_dispatcher, test_mixed_durations1);
  RUN_TEST_CASE(threads_dispatcher, test_mixed_durations2);
  RUN_TEST_CASE(threads_dispatcher, test_parallel);
}