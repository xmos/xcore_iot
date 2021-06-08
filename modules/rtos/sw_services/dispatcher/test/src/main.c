// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"

#include "unity.h"
#include "unity_fixture.h"

void vApplicationMallocFailedHook(void) {
  debug_printf("Malloc failed!\n");
  exit(1);
}

static void RunTests(void *unused) {
  RUN_TEST_GROUP(dispatch_job);
  RUN_TEST_GROUP(dispatch_group);
  RUN_TEST_GROUP(threads_dispatcher);
  RUN_TEST_GROUP(isr_dispatcher);
  UnityEnd();
  exit(Unity.TestFailures);
}

int main(int argc, const char *argv[]) {
  UnityGetCommandLineOptions(argc, argv);
  UnityBegin(argv[0]);

  xTaskCreate(RunTests, "RunTests", 1024 * 16, NULL, configMAX_PRIORITIES - 1,
              NULL);
  vTaskStartScheduler();
  // we never reach here because vTaskStartScheduler never returns
  return (int)Unity.TestFailures;
}