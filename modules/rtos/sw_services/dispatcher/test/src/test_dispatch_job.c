// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include "dispatcher.h"
#include "unity.h"
#include "unity_fixture.h"

typedef struct test_work_arg {
  int zero;
  int one;
} test_work_arg_t;

DISPATCHER_JOB_ATTRIBUTE
void do_dispatch_job_work(void *p) {
  test_work_arg_t *arg = (test_work_arg_t *)p;
  arg->zero = 0;
  arg->one = 1;
}

DISPATCHER_JOB_ATTRIBUTE
void undo_dispatch_job_work(void *p) {
  test_work_arg_t *arg = (test_work_arg_t *)p;
  arg->zero = 1;
  arg->one = 0;
}

TEST_GROUP(dispatch_job);

TEST_SETUP(dispatch_job) {}

TEST_TEAR_DOWN(dispatch_job) {}

TEST(dispatch_job, test_create) {
  dispatch_job_t *ob;

  ob = dispatch_job_create(do_dispatch_job_work, NULL);
  TEST_ASSERT_NOT_NULL(ob);

  dispatch_job_delete(ob);
}

TEST(dispatch_job, test_perform) {
  dispatch_job_t *ob;
  test_work_arg_t arg;

  ob = dispatch_job_create(do_dispatch_job_work, &arg);

  dispatch_job_perform(ob);
  TEST_ASSERT_EQUAL_INT(0, arg.zero);
  TEST_ASSERT_EQUAL_INT(1, arg.one);

  dispatch_job_delete(ob);
}

TEST_GROUP_RUNNER(dispatch_job) {
  RUN_TEST_CASE(dispatch_job, test_create);
  RUN_TEST_CASE(dispatch_job, test_perform);
}