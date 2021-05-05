// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include <string.h>

#include "dispatch.h"
#include "unity.h"
#include "unity_fixture.h"

typedef struct test_work_arg {
  int count;
} test_work_arg_t;

DISPATCH_TASK_FUNCTION
void do_dispatch_group_work(void *p) {
  test_work_arg_t *arg = (test_work_arg_t *)p;
  arg->count += 1;
}

TEST_GROUP(dispatch_group);

TEST_SETUP(dispatch_group) {}

TEST_TEAR_DOWN(dispatch_group) {}

TEST(dispatch_group, test_create) {
  dispatch_group_t *group;

  group = dispatch_group_create(3, false);
  TEST_ASSERT_NOT_NULL(group);

  dispatch_group_delete(group);
}

TEST(dispatch_group, test_perform_tasks) {
  const int kLength = 3;
  dispatch_group_t *group;
  dispatch_task_t *tasks[kLength];
  test_work_arg_t arg;

  arg.count = 0;

  group = dispatch_group_create(kLength, false);

  for (int i = 0; i < kLength; i++) {
    tasks[i] = dispatch_task_create(do_dispatch_group_work, &arg, false);
    dispatch_group_task_add(group, tasks[i]);
  }
  dispatch_group_perform(group);

  TEST_ASSERT_EQUAL_INT(kLength, arg.count);

  for (int i = 0; i < kLength; i++) {
    dispatch_task_delete(tasks[i]);
  }

  dispatch_group_delete(group);
}

TEST(dispatch_group, test_perform_functions) {
  const int kLength = 3;
  dispatch_group_t *group;
  dispatch_task_t *tasks[kLength];
  test_work_arg_t arg;

  arg.count = 0;

  group = dispatch_group_create(kLength, false);

  for (int i = 0; i < kLength; i++) {
    tasks[i] = dispatch_group_function_add(group, do_dispatch_group_work, &arg);
  }
  dispatch_group_perform(group);

  TEST_ASSERT_EQUAL_INT(kLength, arg.count);

  for (int i = 0; i < kLength; i++) {
    dispatch_task_delete(tasks[i]);
  }

  dispatch_group_delete(group);
}

TEST_GROUP_RUNNER(dispatch_group) {
  RUN_TEST_CASE(dispatch_group, test_create);
  RUN_TEST_CASE(dispatch_group, test_perform_tasks);
  RUN_TEST_CASE(dispatch_group, test_perform_functions);
}