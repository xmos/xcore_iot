// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include <string.h>

#include "dispatcher.h"
#include "unity.h"
#include "unity_fixture.h"

typedef struct test_work_arg {
  int count;
} test_work_arg_t;

DISPATCHER_JOB_ATTRIBUTE
void do_dispatch_group_work(void *p) {
  test_work_arg_t *arg = (test_work_arg_t *)p;
  arg->count += 1;
}

TEST_GROUP(dispatch_group);

TEST_SETUP(dispatch_group) {}

TEST_TEAR_DOWN(dispatch_group) {}

TEST(dispatch_group, test_create) {
  dispatch_group_t *group;

  group = dispatch_group_create(3);
  TEST_ASSERT_NOT_NULL(group);

  dispatch_group_delete(group);
}

TEST(dispatch_group, test_perform_jobs) {
  const int kLength = 3;
  dispatch_group_t *group;
  dispatch_job_t *jobs[kLength];
  test_work_arg_t arg;

  arg.count = 0;

  group = dispatch_group_create(kLength);

  for (int i = 0; i < kLength; i++) {
    jobs[i] = dispatch_job_create(do_dispatch_group_work, &arg);
    dispatch_group_job_add(group, jobs[i]);
  }
  dispatch_group_perform(group);

  TEST_ASSERT_EQUAL_INT(kLength, arg.count);

  for (int i = 0; i < kLength; i++) {
    dispatch_job_delete(jobs[i]);
  }

  dispatch_group_delete(group);
}

TEST(dispatch_group, test_perform_functions) {
  const int kLength = 3;
  dispatch_group_t *group;
  dispatch_job_t *jobs[kLength];
  test_work_arg_t arg;

  arg.count = 0;

  group = dispatch_group_create(kLength);

  for (int i = 0; i < kLength; i++) {
    jobs[i] = dispatch_group_function_add(group, do_dispatch_group_work, &arg);
  }
  dispatch_group_perform(group);

  TEST_ASSERT_EQUAL_INT(kLength, arg.count);

  for (int i = 0; i < kLength; i++) {
    dispatch_job_delete(jobs[i]);
  }

  dispatch_group_delete(group);
}

TEST_GROUP_RUNNER(dispatch_group) {
  RUN_TEST_CASE(dispatch_group, test_create);
  RUN_TEST_CASE(dispatch_group, test_perform_jobs);
  RUN_TEST_CASE(dispatch_group, test_perform_functions);
}