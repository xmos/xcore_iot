// Copyright 2021 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1
#include <stdio.h>
#include <xcore/hwtimer.h>

#include "FreeRTOS.h"
#include "task.h"

#include "dispatcher.h"

#define NUM_THREADS 4
#define ROWS 100 // must be a multiple of NUM_THREADS
#define COLUMNS 100

typedef struct worker_arg
{
  int start_row;
  int end_row;
} worker_arg_t;

static int input_mat1[ROWS][COLUMNS];
static int input_mat2[ROWS][COLUMNS];
static int output_mat[ROWS][COLUMNS];

void reset_matrices()
{
  for (int i = 0; i < ROWS; i++)
    for (int j = 0; j < COLUMNS; j++)
    {
      input_mat1[i][j] = 1;
      input_mat2[i][j] = 1;
      output_mat[i][j] = 0;
    }
}

int verify_output_matrix()
{
  int num_errors = 0;
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLUMNS; j++)
    {
      if (output_mat[i][j] != ROWS)
      {
        rtos_printf("Whoops! output_mat[%d][%d] equals %d, expected %d\n", i, j,
                    output_mat[i][j], ROWS);
        num_errors += 1;
      }
    }
  }

  return num_errors;
}

DISPATCHER_JOB_ATTRIBUTE
void do_matrix_multiply(void *p)
{
  worker_arg_t *arg = (worker_arg_t *)p;

  for (int i = arg->start_row; i < arg->end_row; i++)
    for (int j = 0; j < COLUMNS; j++)
      for (int k = 0; k < ROWS; k++)
        output_mat[i][j] += (input_mat1[i][k] * input_mat2[k][j]);
}

void matrix_multiply()
{
  dispatcher_t *disp;
  dispatch_group_t *group;
  dispatch_job_t *jobs[NUM_THREADS];
  worker_arg_t job_args[NUM_THREADS];

  reset_matrices();

  // create the dispatcher
  disp = dispatcher_create();

  // initialize the dispatcher
  dispatcher_thread_init(disp, NUM_THREADS, NUM_THREADS,
                         configMAX_PRIORITIES - 1);

  // create group
  group = dispatch_group_create(NUM_THREADS);

  int num_rows = ROWS / NUM_THREADS;
  for (int i = 0; i < NUM_THREADS; i++)
  {
    job_args[i].start_row = i * num_rows;
    job_args[i].end_row = job_args[i].start_row + num_rows;

    jobs[i] = dispatch_job_create(do_matrix_multiply, (void *)&job_args[i]);
    // add job to group
    dispatch_group_job_add(group, jobs[i]);
  }

  // dispatch the group
  dispatcher_group_add(disp, group);

  // wait in this thread for all jobs in the group to finish
  dispatcher_group_wait(disp, group);

  // verify the output matrix
  if (verify_output_matrix() == 0)
    rtos_printf("Congratulations, output matrix verified!\n");

  // free memory
  for (int i = 0; i < NUM_THREADS; i++)
  {
    dispatch_job_delete(jobs[i]);
  }

  dispatch_group_delete(group);
  dispatcher_delete(disp);

  vTaskDelete(NULL);
}

void vApplicationMallocFailedHook(void) { debug_printf("Malloc failed!\n"); }

void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
  xTaskCreate(matrix_multiply, "MatrixMultiplyExample", 1024, NULL,
              configMAX_PRIORITIES - 1, NULL);
  vTaskStartScheduler();
}
