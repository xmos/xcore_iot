.. include:: ../../substitutions.rst

.. _freertos-common_issues:

######################
FreeRTOS Common Issues
######################

****************
Task Stack Space
****************

One easy to make mistake in FreeRTOS, is not providing enough stack space for a created task.  A vast amount of questions exist online around how to select the FreeRTOS stack size, which the most common answer being to create the task with more than enough stack, force the worst case stack condition (not always trivial), and then use the FreeRTOS debug function `uxTaskGetStackHighWaterMark()` to  determine how much you can decrease the stack.  This method leaves plenty of room for error and must be done during runtime, and therefore on a build by build basis.  The static analysis tools provided by The XTC Tools greatly simplify this process since they calculate the exact stack required for a given function call.  The macro `RTOS_THREAD_STACK_SIZE` will return the `nstackwords` symbol for a given thread plus the additional space required for the kernel ISRs.  Using this macro for every task create will ensure that there is appropriate stack space for each thread, and thus no stack overflow.

.. code-block:: C

    xTaskCreate((TaskFunction_t) task_foo,
                "foo",
                RTOS_THREAD_STACK_SIZE(task_foo),
                NULL,
                configMAX_PRIORITIES-1,
                NULL);
