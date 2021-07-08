// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include "FreeRTOS.h"
#include "task.h"

#include "dispatcher.h"

DISPATCHER_JOB_ATTRIBUTE
void test_job(void *arg) {
  rtos_printf("Hello World from job %d\n", *(int *)arg);
}

void hello_world() {
  dispatcher_t *disp;
  dispatch_group_t *group;
  dispatch_job_t *jobs[3];
  int job_args[3] = {1, 2, 3};

  // create the dispatcher
  disp = dispatcher_create();

  // initialize the dispatcher with a queue length of 10,
  // 4 worker threads, and a thread priority of
  // (configMAX_PRIORITIES - 1)
  dispatcher_thread_init(disp, 10, 4, configMAX_PRIORITIES - 1);

  // create group of 3 jobs
  group = dispatch_group_create(3);

  for (int i = 0; i < 3; i++) {
    jobs[i] = dispatch_job_create(test_job, (void *)&job_args[i]);
    // add job to group
    dispatch_group_job_add(group, jobs[i]);
  }

  // dispatch the group
  dispatcher_group_add(disp, group);

  // perform other code here if desired

  // wait in this thread for all jobs in the group to finish
  dispatcher_group_wait(disp, group);

  // free memory
  for (int i = 0; i < 3; i++) {
    dispatch_job_delete(jobs[i]);
  }

  dispatch_group_delete(group);
  dispatcher_delete(disp);
}

void vApplicationMallocFailedHook(void) { debug_printf("Malloc failed!\n"); }

int main(void) {
  xTaskCreate(hello_world, "HelloWorldExample", 1024, NULL,
              configMAX_PRIORITIES - 1, NULL);
  vTaskStartScheduler();
  return 0;
}