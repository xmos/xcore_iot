.. _freertos-faq:

.. include:: ../../substitutions.rst

#############
FreeRTOS FAQs
#############

1. What is the memory overhead of the FreeRTOS kernel?

    The FreeRTOS kernel requires approximately 9kB of RAM.

2. Can I use XCore resources like channels, timers and hw_locks?

    You are free to use channels, ports, timers, etc… in your FreeRTOS applications.  However, some considerations need to be made.  The RTOS kernel knows about RTOS primitives.  For example, if RTOS thread A attempts to take a semaphore, the kernel is free to schedule other tasks in thread A’s place while thread A is waiting for some other task to give the semaphore.  The RTOS kernel does not know anything about XCore resources.  For example, if RTOS thread A attempts to `recv` on a channel, the kernel is **not** free to schedule other tasks in its place while thread A is waiting for some other task to send to the other end of the channel.  A developer should be aware that blocking calls on XCore resources will block a FreeRTOS thread.  This may be OK as long as it is carefully considered in the application design.  There are a variety of methods to handle the decoupling of XCore and RTOS resources.  These can be best seen in the various RTOS drivers, which wrap the realtime IO hardware imitation layer.

