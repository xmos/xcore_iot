.. include:: ../../substitutions.rst

################################
FreeRTOS Application Programming
################################

*********
Rationale
*********

Traditionally, XCore multi-core processors have been programmed using the XC language. The XC language allows the programmer to statically place tasks on the available hardware cores and wire them together with channels to provide inter-process communication. Cores and channels are both hardware resources, and thus are limited. The XC language also exposes "events," which are unique to the XCore architecture and are a useful alternative to interrupts. The XC language, however, does not expose interrupts (which the XCore architecture itself does support), and indeed makes interrupts near impossible to use.

Using a combination of tasks statically placed on hardware cores, channels, and events, it is possible to write software with deterministic timing, and with very low latency between I/O and software, as well as between tasks.

While lower level software functions that require deterministic timing are made possible by XC and the XCore architecture, there often needs to be higher level application type software running alongside it. Unfortunately, what makes the lower level deterministic software possible can in fact be a burden for an application that does not require deterministic timing. The relatively low number of hardware cores, several of which often must be allocated solely to lower level deterministic tasks, a low maximum channel count (16 within a single tile), the synchronous nature of channels, and a lack of interrupts, all can make it very difficult to write larger application software, often requiring unusually complex design.

With all of this in mind, a symmetric multiprocessing (SMP) real time operating system (RTOS) can be utilized to both greatly reduce the complexity required for XCore application designs, as well as to preserve the hard real-time benefits provided by the XCore architecture for the lower level software functions that require it.

The remainder of this document assumes familiarity with real time operating systems in general. Familiarity with FreeRTOS specifically should not be required, but will be helpful. For current up to date documentation on FreeRTOS see the following links on the `FreeRTOS website <https://www.freertos.org/>`_.

- `Overview <https://www.freertos.org/RTOS.html>`_
- `Developer Documentation <https://www.freertos.org/features.html>`_
- `API <https://www.freertos.org/a00106.html>`_

************
SMP FreeRTOS
************

To support this new programming model for XCore, XMOS has extended the popular and free FreeRTOS kernel to support SMP. This allows for the kernel's scheduler to be started on any number of available XCore cores per tile. Once the scheduler is started, FreeRTOS threads are placed on cores dynamically at runtime, rather than statically at compile time. All the usual FreeRTOS rules for thread scheduling are followed, except that rather than only running the single highest priority thread that is ready at any given time, multiple threads may run simultaneously. The threads chosen to run are always the highest priority threads that are ready. When there are more threads of a single priority that are ready to run than the number of cores available, they are scheduled in a round robin fashion.

SMP Specific Considerations
===========================

Programming an application for a multiprocessor environment using an SMP RTOS is very similar to programming for a single processor environment using an RTOS. Most of the time, it is almost identical and the fact that there are multiple cores available to the threads is a detail that the programmer does not need to worry about. However, there are some differences that the programmer must take into account to avoid race conditions which do not exist when there is only a single processor core available to the RTOS.

It is possible for multiple threads to run simultaneously on different cores. This is obvious, and is the point of an SMP RTOS. But it may not be immediately obvious why this requires special consideration above what must already be considered when programming for a multi-threaded, but single processor, environment.

The first big difference that this introduces is that it is now possible for threads with different priority levels to run simultaneously. In a single core environment, when two threads with different priorities share a data structure, it is not necessary for the higher priority one to enter a critical section when using it. This is no longer true in an multiprocessor environment. Any instance where a thread assumes that a lower priority thread will not run is no longer valid when using an SMP RTOS.

The second big difference is with interrupt service routines (ISRs). In a single processor environment, ISRs cannot run simultaneously either with each other (although there is a similar issue for architectures that support interrupt priority levels and nesting) or with application threads. Of course all of this is possible in a multiprocessor environment. So there must be a way to ensure mutual exclusion for access to data structures that are shared both between multiple ISRs, as well as between ISRs and threads.

FreeRTOS already provides the macro functions taskENTER_CRITICAL_FROM_ISR() and taskEXIT_CRITICAL_FROM_ISR() for use with ports for architectures that support interrupt nesting. The SMP FreeRTOS port for XCore makes use of these and uses an XCore hardware lock under the hood. Be sure to remember to use these in ISRs around access to data that is shared with threads and requires mutual exclusion. The corresponding task version macro functions taskENTER_CRITICAL() and taskEXIT_CRITICAL() must be called by threads that access this shared data. The task version both disables interrupts on the calling core, as well as obtains the lock.

New Features
============

Two new APIs have been added to FreeRTOS to support SMP and XCore. Similar capability is also found in other RTOSes that support SMP.

- The first allows a FreeRTOS thread to be excluded from any number of cores. This is done with a core exclusion mask. This supports various scenarios.

  - One common scenario is having a task that fully utilizes the XCore architecture and requires deterministic execution. Most FreeRTOS applications, however, require a `timer interrupt that runs periodically <https://www.freertos.org/implementation/a00011.html>`_, typically once every 1 or 10 milliseconds. The XCore SMP FreeRTOS port always places this timer interrupt on core 0 [#]_. When execution of this interrupt's service routine breaks the timing assumption made by tasks that require deterministic execution, and it is not feasible to disable interrupts around its critical sections, then it can make sense to exclude these tasks from core 0.

  - Another scenario is when there are two or more "legacy" threads written with the assumption that they are running in a single core environment. It is common to find that the higher priority threads will often not enter a critical section when modifying data structures shared with lower priority threads, as it is not possible for the lower priority threads to preempt the higher priority threads. While this is still true in an SMP environment, it is possible that the lower priority thread can run simultaneously in another core. Therefore, additional protection must be added (see the discussion above about this). When it is not possible to modify the code to add this protection, for example when the functions are part of a third party library, then it can make sense to lock all of these threads to a single core, ensuring that they do not run simultaneously.

  The two new functions to support this are:

  .. code-block:: C

    void vTaskCoreExclusionSet( const TaskHandle_t xTask, UBaseType_t uxCoreExclude )

  This function sets the specified thread's core exclusion mask. Each bit position represents the corresponding core number, supporting up to 32 cores. Subsequent to the call, the task will be prevented from running on any core whose corresponding bit in the mask is set to 1.

  .. code-block:: C

    UBaseType_t vTaskCoreExclusionGet( const TaskHandle_t xTask )

  This function returns the specified thread's current core exclusion mask.

- The second new feature allows preemption to be disabled at runtime on a per thread basis. Global preemption may still be disabled at compile time with the configuration option configUSE_TASK_PREEMPTION_DISABLE.

  This allows threads to ensure that they are never preempted by another lower or same priority task. This can be useful for tasks that require deterministic execution but that do not necessarily need to be run at the highest priority level. For example, a thread that spends much of the time blocked in a waiting state, but once woken up and running must not be interrupted. Disabling interrupts within these tasks may also be required, but by additionally disabling preemption the scheduler will not even attempt to preempt it, ensuring that other threads continue running as they should.

  The two new functions to support this are:

  .. code-block:: C

    void vTaskPreemptionDisable( const TaskHandle_t xTask )

  This function disables preemption for the specified thread.

  .. code-block:: C

    void vTaskPreemptionEnable( const TaskHandle_t xTask )

  This function enables preemption for the specified thread.

Aside from the above additions, the API is identical between the official FreeRTOS kernel and XMOS's SMP FreeRTOS. Any code that has been written for single core FreeRTOS should compile and work under SMP FreeRTOS. Just be aware of the single core assumption that is occasionally made and account for it as necessary.

************
RTOS Drivers
************

To help ease development of XCore applications using an SMP RTOS, XMOS provides several SMP RTOS compatible drivers. These include, but are not necessarily limited to:

- Common I/O interfaces

  - GPIO
  - |I2C|
  - |I2S|
  - PDM microphones
  - QSPI flash
  - SPI
  - USB

- XCore features

  - Intertile channel communication
  - Software defined memory (xcore.ai only)

- External parts

  - Silicon Labs WF200 series WiFi transceiver

These drivers are all found in the SDK under the path `modules/rtos/drivers <https://github.com/xmos/xcore_sdk/tree/develop/modules/rtos/drivers>`_.

Documentation on each of these drivers can be found under the :doc:`References/RTOS Drivers<../../reference/rtos_drivers/index>` section in the SDK documentation pages.

It is worth noting that these drivers utilize a lightweight RTOS abstraction layer, meaning that they are not dependent on FreeRTOS. Conceivably they should work on any SMP RTOS, provided an abstraction layer for it is provided. This abstraction layer is found under the path `modules/rtos/osal <https://github.com/xmos/xcore_sdk/tree/develop/modules/rtos/osal>`_. At the moment the only available SMP RTOS for XCore is the XMOS SMP FreeRTOS, but more may become available in the future.

*****************
Software Services
*****************

XMOS also includes some higher level RTOS compatible software services, some of which call the aforementioned drivers. These include, but are not necessarily limited to:

- DHCP server
- Dispatcher
- FAT filesystem
- HTTP parser
- JSON parser
- MQTT
- SNTP client
- TLS
- USB stack
- WiFi connection manager

Documentation on several software services can be found under the :doc:`Tutorails/FreeRTOS/Software Services<../../reference/rtos_services/index>` section in the SDK documentation pages.

These services are all found in the SDK under the path `modules/rtos/sw_services <https://github.com/xmos/xcore_sdk/tree/develop/modules/rtos/sw_services>`_.

***************
Getting Started
***************

The :doc:`Getting Started with FreeRTOS <getting_started>` tutorial will guide you through writing a FreeRTOS application for XCore.

-------------------------------------------------------------------------------

.. rubric:: Footnotes

.. [#] This is not necessarily core 0 as returned by get_logical_core_id() found in xs1.h. SMP FreeRTOS maintains its own core ID numbering for the cores that it resides on. For the SMP RTOS core ID value, use rtos_core_id_get() instead.
