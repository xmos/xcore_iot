.. include:: ../../substitutions.rst

.. _freertos-faq:

#############
FreeRTOS FAQs
#############

1. What is the memory overhead of the FreeRTOS kernel?

    The FreeRTOS kernel requires approximately 9kB of RAM.

2. How do I determine the number of words to allocate for use as a task's stack?

    Since tasks run within FreeRTOS, the RTOS stack requirement must be known at compile time.  In FreeRTOS applications on most other microcontrollers, the general practice is to create a task with a large amount of stack, use the FreeRTOS stack debug functions to determine the worst case runtime usage of stack, and then adjust the stack memory value accordingly.  The problem with this method is that the stack of any given thread varies greatly based on the functions that are called within, and thus a code or compiler optimization change result in the optimal task stack usage to have to be redetermined.  This issue results in many FreeRTOS applications being written in such a way that wastes memory, by providing task with way more stack than they should need.  Additionally, stack overflow bugs can remain hidden for a long time and even when bugs do manifest, the source can be difficult to pinpoint.

    The XTC Tools address this issue by creating a symbol that represents the maximum stack requirement of any function at compile time.  By using the `RTOS_THREAD_STACK_SIZE()` macro, for the stack words argument for creating a FreeRTOS task, it is guaranteed that the optimal stack requirement is used, provided that the function does not call function pointers nor can infinitely recurse.

    .. code-block:: C

        xTaskCreate((TaskFunction_t) example_task,
                    "example_task",
                    RTOS_THREAD_STACK_SIZE(example_task),
                    NULL,
                    EXAMPLE_TASK_PRIORITY,
                    NULL);

    If function pointers are used within a thread, then the application programmer must annotate the code with the appropriate function pointer group attribute.  For recursive functions, the only option is to specify the stack manually.  See `Appendix A - Guiding Stack Size Calculation <https://www.xmos.ai/documentation/XM-014363-PC-LATEST/html/prog-guide/quick-start/c-programming-guide/index.html>`_ in the XTC Tools documentation for more information.

3. Can I use xcore resources like channels, timers and hw_locks?

    You are free to use channels, ports, timers, etc… in your FreeRTOS applications.  However, some considerations need to be made.  The RTOS kernel knows about RTOS primitives.  For example, if RTOS thread A attempts to take a semaphore, the kernel is free to schedule other tasks in thread A’s place while thread A is waiting for some other task to give the semaphore.  The RTOS kernel does not know anything about xcore resources.  For example, if RTOS thread A attempts to `recv` on a channel, the kernel is **not** free to schedule other tasks in its place while thread A is waiting for some other task to send to the other end of the channel.  A developer should be aware that blocking calls on xcore resources will block a FreeRTOS thread.  This may be OK as long as it is carefully considered in the application design.  There are a variety of methods to handle the decoupling of xcore and RTOS resources.  These can be best seen in the various RTOS drivers, which wrap the realtime IO hardware imitation layer.

