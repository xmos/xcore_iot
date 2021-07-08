// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <platform.h>
#include <xs1.h>
#include <xcore/hwtimer.h>

#include "FreeRTOS.h"
#include "task.h"

#include "dispatch.h"
#include "rtos_printf.h"

#define NUM_THREADS 4
#define ROWS 100  // must be a multiple of NUM_THREADS
#define COLUMNS 100

typedef struct worker_arg {
  int start_row;
  int end_row;
} worker_arg_t;

static int input_mat1[ROWS][COLUMNS];
static int input_mat2[ROWS][COLUMNS];
static int output_mat[ROWS][COLUMNS];

void vApplicationMallocFailedHook(void) {
  rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
  for (;;)
    ;
}

void vApplicationDaemonTaskStartup(void* arg) { vTaskDelete(NULL); }

void reset_matrices() {
  for (int i = 0; i < ROWS; i++)
    for (int j = 0; j < COLUMNS; j++) {
      input_mat1[i][j] = 1;
      input_mat2[i][j] = 1;
      output_mat[i][j] = 0;
    }
}

void verify_output() {
  for (int i = 0; i < ROWS; i++)
    for (int j = 0; j < COLUMNS; j++) {
      if (output_mat[i][j] != ROWS) {
        printf("Whoops! output_mat[%d][%d] equals %d, expected %d\n", i, j,
               output_mat[i][j], ROWS);
      }
    }
}

DISPATCH_TASK_FUNCTION
void do_matrix_multiply(void* p) {
  worker_arg_t* arg = (worker_arg_t*)p;

  for (int i = arg->start_row; i < arg->end_row; i++)
    for (int j = 0; j < COLUMNS; j++)
      for (int k = 0; k < ROWS; k++)
        output_mat[i][j] += (input_mat1[i][k] * input_mat2[k][j]);
  return;
}

static int single_thread_mat_mul() {
  reset_matrices();

  worker_arg_t arg;
  hwtimer_t hwtimer;
  int ticks;

  arg.start_row = 0;
  arg.end_row = ROWS;

  hwtimer = hwtimer_alloc();
  ticks = hwtimer_get_time(hwtimer);

  do_matrix_multiply(&arg);

  ticks = hwtimer_get_time(hwtimer) - ticks;
  hwtimer_free(hwtimer);

  verify_output();

  return ticks;
}

static int multi_thread_mat_mul() {
  dispatch_queue_t* queue;
  dispatch_group_t* group;
  worker_arg_t args[NUM_THREADS];
  int queue_length = NUM_THREADS;
  int queue_thread_count = NUM_THREADS;
  hwtimer_t hwtimer;
  int ticks;

  reset_matrices();

  // create the dispatch queue
  queue = dispatch_queue_create(queue_length, queue_thread_count, 1024, 0);

  // create the dispatch group
  group = dispatch_group_create(queue_thread_count, true);

  // initialize NUM_THREADS tasks, add them to the group
  int num_rows = ROWS / NUM_THREADS;
  for (int i = 0; i < NUM_THREADS; i++) {
    args[i].start_row = i * num_rows;
    args[i].end_row = args[i].start_row + num_rows;

    dispatch_group_function_add(group, do_matrix_multiply, &args[i]);
  }

  hwtimer = hwtimer_alloc();
  ticks = hwtimer_get_time(hwtimer);

  // add group to dispatch queue
  dispatch_queue_group_add(queue, group);
  // wait for all tasks in the group to finish executing
  dispatch_queue_group_wait(queue, group);

  ticks = hwtimer_get_time(hwtimer) - ticks;
  hwtimer_free(hwtimer);

  verify_output();

  // delete the dispatch group and queue
  dispatch_group_delete(group);
  dispatch_queue_delete(queue);

  return ticks;
}

static void mat_mul(void* unused) {
  int single_thread_duration;
  int multi_thread_duration;

  single_thread_duration = single_thread_mat_mul();
  multi_thread_duration = multi_thread_mat_mul();

  rtos_printf("single thread duration = %d ticks\n", single_thread_duration);
  rtos_printf("multiply thread duration = %d ticks\n", multi_thread_duration);
  printf("speedup = %0.1fx\n",
         (float)single_thread_duration / (float)multi_thread_duration);

  for (;;)
    ;
}

void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
  (void)c0;
  (void)c1;
  (void)c2;
  (void)c3;

  xTaskCreate(mat_mul, "mat_mul", 1024 * 16, NULL, configMAX_PRIORITIES - 1,
              NULL);

  rtos_printf("start scheduler on tile 0\n");
  vTaskStartScheduler();

  return;
}
