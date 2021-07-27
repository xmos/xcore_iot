##########
Dispatcher
##########

The Dispatcher is a job queue for XCore that can simplify dispatching jobs. Two job worker configuration types are supported; threads or interrupt service routines (ISR). The thread worker configuration is the most flexible while the ISR worker configuration has the least runtime overhead.

This document covers the Dispatcher concepts and includes code snippets to demonstrate usage. A full dispatcher code example is provided at the end of this document.

**********
Data Types
**********

Jobs
====

The basic unit of work, or job, is the `dispatch_job_t`. The following code snippet demonstrates how to create a `dispatch_job_t`:

.. code-block:: c

    #include "dispatcher.h"

    DISPATCHER_JOB_ATTRIBUTE
    void test_job(void *p) {
        // do some work here
    }

    dispatch_job_t *job;
    int arg;

    job = dispatch_job_create(test_job, &arg);

    // dispatch job here...

    dispatch_job_delete(job);

Job functions must have the following signature:

.. code-block:: c

    void(void *)

To support assisted stack size calculation, job functions must have the "dispatcher_job" function pointer group attribute:

.. code-block:: c

    #define DISPATCHER_JOB_ATTRIBUTE __attribute__((fptrgroup("dispatcher_job")))

Job Groups
==========

Job groups are a convienient way to dispatch similar jobs and wait for the group to finish. This is often referred to as the `fork-join model <https://en.wikipedia.org/wiki/Fork%E2%80%93join_model>`_. The following code snippet demonstrates how to create a `dispatch_group_t`:

.. code-block:: c

    #include "dispatcher.h"

    DISPATCHER_JOB_ATTRIBUTE
    void test_job(void *p) {
        // do some work here
    }

    dispatch_group_t *group;
    dispatch_job_t *jobs[3];
    int arg;

    // create group of 3 jobs
    group = dispatch_group_create(3);

    for (int i = 0; i < 3; i++) {
        jobs[i] = dispatch_job_create(test_job, &arg);
        // add job to group
        dispatch_group_job_add(group, jobs[i]);
    }

    // dispatch group here...

    // free memory
    for (int i = 0; i < 3; i++) {
        dispatch_job_delete(jobs[i]);
    }

    dispatch_group_delete(group);

*********************
Worker Configurations
*********************

Two job worker configuration types are supported; threads or ISRs.

When deciding on a worker configuration, it is important to consider the duration and frequency of dispatched jobs. For jobs with runtime durations that exceed several milliseconds, the threads configuration is recommended. When the job length is below 1 millisecond, the performance of the thread configuration may begin to suffer and you should consider the ISR configuration. Be sure to fully test your application with both configurations if you have short duration jobs.

Threads
=======

The thread configuration is the most flexible because it functions like a typical job queue. Jobs are added to the queue while a pool of worker threads pop jobs off the queue and execute them in first in, first out (FIFO) order. The application developer specifies the length of the queue, the number of worker threads in the pool, and the priority of the worker threads.

While the queue is empty, the worker threads will not be scheduled for execution by the RTOS kernel. They will remain idle until a new job is added to the queue. When a new job is added to the queue, the RTOS kernel will then schedule the worker threads for execution. Each threads will check the queue, one of the threads will execute the new job, and the other threads will return to the idle state.

This process has some small but not zero overhead which is why, if your jobs execute in less than 1 millisecond, you may observe that your computation does not parallelize well and you may want to test with the ISR configuration. This is more true as the duration of jobs reduces to something even smaller.  A computation split up into jobs that execute in 50-100 microseconds will not parallelize at all unless the ISR worker configuration is used.

The following code snippet demonstrates how to create and initialize the thread worker dispatcher configuration.

.. code-block:: c

    #include "dispatcher.h"

    dispatcher_t *disp;
    
    disp = dispatcher_create();
    
    // initialize the dispatcher with a queue length of 10,
    // 4 worker threads, and a thread priority of 30
    dispatcher_thread_init(disp, 10, 4, 30);

    // use dispatcher here...

    dispatcher_delete(disp);

ISRs
====

The ISR configuration is the most lightweight because it does not function like a typical job queue. Instead, jobs are dispatched far more quickly to interrupt service routines which execute the job. Jobs can not be queued so an application must not add more jobs than ISRs. To configure the ISR workers, the application only specifies the cores where ISRs are executed. It is important for application performance that, when using the ISR worker configuration, that jobs execute very quickly because no other threads will be scheduled on an ISR worker's core until the job completes.

The following code snippet demonstrates how to create and initialize the ISR worker dispatcher configuration.

.. code-block:: c

    #include "dispatcher.h"

    dispatcher_t *disp;
    
    disp = dispatcher_create();
    
    // initialize the dispatcher with ISRs enabled on 
    // cores 1, 2, 3 & 5
    dispatcher_isr_init(disp, 0b00101110);

    // use dispatcher here...

    dispatcher_delete(disp);

************
Code Example
************

The following example code demonstrates how to create a dispatcher and add jobs that perform a matrix multiply. To build this and other dispatcher examples, see the `README.rst` in `examples/freertos/dispatcher`.

.. literalinclude:: ../../../examples/freertos/dispatcher/src/main.c
  :language: c
