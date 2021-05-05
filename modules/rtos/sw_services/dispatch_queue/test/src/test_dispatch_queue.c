// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#include <xcore/hwtimer.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "dispatch.h"
#include "unity.h"
#include "unity_fixture.h"

#define QUEUE_THREAD_STACK_SIZE (1024)  // more than enough
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

DISPATCH_TASK_FUNCTION
void do_limited_work(void *p) {
  test_work_arg_t *arg = (test_work_arg_t *)p;

  look_busy(100);

  xSemaphoreTake(mutex, portMAX_DELAY);
  arg->count++;
  xSemaphoreGive(mutex);
}

DISPATCH_TASK_FUNCTION
void do_standard_work(void *p) {
  test_work_arg_t *arg = (test_work_arg_t *)p;

  look_busy(500);

  xSemaphoreTake(mutex, portMAX_DELAY);
  arg->count++;
  xSemaphoreGive(mutex);
}

DISPATCH_TASK_FUNCTION
void do_extended_work(void *p) {
  test_work_arg_t *arg = (test_work_arg_t *)p;

  look_busy(1000);

  xSemaphoreTake(mutex, portMAX_DELAY);
  arg->count++;
  xSemaphoreGive(mutex);
}

DISPATCH_TASK_FUNCTION
void do_parallel_work(void *p) {
  // NOTE: the "volatile" is needed here or the compiler may optimize this away
  test_parallel_work_arg volatile *arg = (test_parallel_work_arg volatile *)p;

  for (int i = arg->begin; i < arg->end; i++) arg->count++;
}

TEST_GROUP(dispatch_queue);

TEST_SETUP(dispatch_queue) { mutex = xSemaphoreCreateMutex(); }

TEST_TEAR_DOWN(dispatch_queue) { vSemaphoreDelete(mutex); }

TEST(dispatch_queue, test_wait_task) {
  dispatch_queue_t *queue;
  dispatch_task_t *task;
  test_work_arg_t arg;
  const int kQueueLength = 10;
  const int kQueueThreadCount = 3;

  queue = dispatch_queue_create(kQueueLength, kQueueThreadCount,
                                QUEUE_THREAD_STACK_SIZE, QUEUE_THREAD_PRIORITY);

  arg.count = 0;

  task = dispatch_task_create(do_standard_work, &arg, true);
  dispatch_queue_task_add(queue, task);
  dispatch_queue_task_wait(queue, task);

  TEST_ASSERT_EQUAL_INT(1, arg.count);

  dispatch_queue_delete(queue);
}

TEST(dispatch_queue, test_wait_group) {
  dispatch_queue_t *queue;
  dispatch_group_t *group;
  test_work_arg_t arg;
  const int kQueueLength = 10;
  const int kQueueThreadCount = 3;
  const int kGroupLength = 3;

  queue = dispatch_queue_create(kQueueLength, kQueueThreadCount,
                                QUEUE_THREAD_STACK_SIZE, QUEUE_THREAD_PRIORITY);
  group = dispatch_group_create(kGroupLength, true);

  arg.count = 0;

  // add tasks to group
  for (int i = 0; i < kGroupLength; i++) {
    dispatch_group_function_add(group, do_standard_work, &arg);
  }

  dispatch_queue_group_add(queue, group);
  dispatch_queue_group_wait(queue, group);

  TEST_ASSERT_EQUAL_INT(kGroupLength, arg.count);

  dispatch_group_delete(group);
  dispatch_queue_delete(queue);
}

TEST(dispatch_queue, test_mixed_durations1) {
  dispatch_queue_t *queue;
  dispatch_task_t *standard_task;
  dispatch_task_t **extended_tasks;
  test_work_arg_t arg;
  const int kQueueLength = 10;
  const int kQueueThreadCount = 3;
  const int kMaxExtendedTaskCount = 3 + 1;
  int extended_task_count;

  for (int i = 0; i < kMaxExtendedTaskCount; i++) {
    queue =
        dispatch_queue_create(kQueueLength, kQueueThreadCount,
                              QUEUE_THREAD_STACK_SIZE, QUEUE_THREAD_PRIORITY);

    extended_task_count = i + 1;

    extended_tasks = malloc(extended_task_count * sizeof(dispatch_task_t *));

    // create all the tasks
    for (int j = 0; j < extended_task_count; j++) {
      extended_tasks[j] = dispatch_task_create(do_extended_work, &arg, true);
    }
    standard_task = dispatch_task_create(do_standard_work, &arg, true);

    arg.count = 0;

    // add extended tasks
    for (int j = 0; j < extended_task_count; j++) {
      dispatch_queue_task_add(queue, extended_tasks[j]);
    }

    // add limited task
    dispatch_queue_task_add(queue, standard_task);

    // now wait for the standard task
    dispatch_queue_task_wait(queue, standard_task);
    if (extended_task_count < 3)
      TEST_ASSERT_EQUAL_INT(1, arg.count);
    else
      TEST_ASSERT_EQUAL_INT(3 + 1, arg.count);

    // now wait for the last added extended task
    dispatch_queue_task_wait(queue, extended_tasks[extended_task_count - 1]);
    TEST_ASSERT_EQUAL_INT(extended_task_count + 1, arg.count);

    // cleanup
    for (int j = 0; j < extended_task_count - 1; j++) {
      dispatch_task_delete(extended_tasks[j]);
    }
    dispatch_queue_delete(queue);
    free(extended_tasks);
  }
}

TEST(dispatch_queue, test_mixed_durations2) {
  dispatch_queue_t *queue;
  dispatch_task_t *extended_task1;
  dispatch_task_t *extended_task2;
  dispatch_group_t *limited_group;
  test_work_arg_t extended_arg;
  test_work_arg_t limited_arg;
  const int kQueueLength = 10;
  const int kQueueThreadCount = 3;

  queue = dispatch_queue_create(kQueueLength, kQueueThreadCount,
                                QUEUE_THREAD_STACK_SIZE, QUEUE_THREAD_PRIORITY);
  limited_group = dispatch_group_create(3, true);

  dispatch_group_function_add(limited_group, do_limited_work, &limited_arg);
  dispatch_group_function_add(limited_group, do_limited_work, &limited_arg);
  dispatch_group_function_add(limited_group, do_limited_work, &limited_arg);

  extended_arg.count = 0;
  limited_arg.count = 0;

  // add first extended task
  extended_task1 =
      dispatch_queue_function_add(queue, do_extended_work, &extended_arg, true);
  dispatch_queue_task_wait(queue, extended_task1);
  TEST_ASSERT_EQUAL_INT(1, extended_arg.count);

  // add limited task group, and second extended task
  dispatch_queue_group_add(queue, limited_group);
  extended_task2 =
      dispatch_queue_function_add(queue, do_extended_work, &extended_arg, true);

  // now wait for the limited tasks
  dispatch_queue_group_wait(queue, limited_group);
  TEST_ASSERT_EQUAL_INT(3, limited_arg.count);

  // now wait for the second extended task
  dispatch_queue_task_wait(queue, extended_task2);
  TEST_ASSERT_EQUAL_INT(2, extended_arg.count);

  dispatch_group_delete(limited_group);
  dispatch_queue_delete(queue);
}

TEST(dispatch_queue, test_parallel) {
  const int kThreadCount = 4;
  int num_values = kThreadCount * 1000000;
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
  do_parallel_work(&args[0]);

  single_thread_ticks = hwtimer_get_time(hwtimer) - single_thread_ticks;
  hwtimer_free(hwtimer);

  TEST_ASSERT_EQUAL_INT(num_values, args[0].count);

  // do multi thread timing
  const int kQueueLength = 4;
  dispatch_queue_t *queue;
  dispatch_group_t *group;
  int num_values_in_chunk = num_values / kThreadCount;
  int multi_thread_ticks;

  // create the dispatch queue
  queue = dispatch_queue_create(kQueueLength, kThreadCount,
                                QUEUE_THREAD_STACK_SIZE, QUEUE_THREAD_PRIORITY);

  // create the dispatch group
  group = dispatch_group_create(kThreadCount, true);

  // initialize kThreadCount tasks, add them to the group
  for (int i = 0; i < kThreadCount; i++) {
    args[i].count = 0;
    args[i].begin = i * num_values_in_chunk;
    args[i].end = args[i].begin + num_values_in_chunk;
    dispatch_group_function_add(group, do_parallel_work, &args[i]);
  }

  hwtimer = hwtimer_alloc();
  multi_thread_ticks = hwtimer_get_time(hwtimer);

  // add group to dispatch queue
  dispatch_queue_group_add(queue, group);
  // wait for all tasks in the group to finish executing
  dispatch_queue_group_wait(queue, group);

  multi_thread_ticks = hwtimer_get_time(hwtimer) - multi_thread_ticks;
  hwtimer_free(hwtimer);

  for (int i = 0; i < kThreadCount; i++) {
    TEST_ASSERT_EQUAL_INT(num_values_in_chunk, args[i].count);
  }

  // delete the dispatch group and queue
  dispatch_group_delete(group);
  dispatch_queue_delete(queue);

  // now test that the multi thread was ~kThreadCount times faster
  float speedup = (float)single_thread_ticks / (float)multi_thread_ticks;
  TEST_ASSERT_FLOAT_WITHIN(0.1, (float)kThreadCount, speedup);
}

TEST_GROUP_RUNNER(dispatch_queue) {
  RUN_TEST_CASE(dispatch_queue, test_wait_task);
  RUN_TEST_CASE(dispatch_queue, test_wait_group);
  RUN_TEST_CASE(dispatch_queue, test_mixed_durations1);
  RUN_TEST_CASE(dispatch_queue, test_mixed_durations2);
  RUN_TEST_CASE(dispatch_queue, test_parallel);
}