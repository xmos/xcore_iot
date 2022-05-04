.. include:: ../../substitutions.rst

################################
FreeRTOS Application Programming
################################

This document is intended to help you become familiar with FreeRTOS application programming on XCore.

*********
Rationale
*********

Traditionally, XCore multi-core processors have been programmed using the XC language. The XC language allows the programmer to statically place tasks on the available hardware cores and wire them together with channels to provide inter-process communication. The XC language also exposes "events," which are unique to the XCore architecture and are a useful alternative to interrupts.

Using the XC language, it is possible to write dedicated application software with deterministic timing and very low latency between I/O and tasks.

While XC elegantly enables the intrinsic, unique capabilities of the XCore architecture, there often needs to be higher level application type software running alongside it. The programming model that makes the lower level deterministic software possible may not be best suited for many higher level parts of an application that do not require deterministic timing. Where strict real-time execution is not required, higher level abstractions can be used to manage finite hardware resources, and provide a more familiar programming environment.

A symmetric multiprocessing (SMP) real time operating system (RTOS) can be used to simplify XCore application designs, as well as to preserve the hard real-time benefits provided by the XCore architecture for the lower level software functions that require it.

This document assumes familiarity with real time operating systems in general. Familiarity with FreeRTOS specifically should not be required, but will be helpful. For current up to date documentation on FreeRTOS see the following links on the `FreeRTOS website <https://www.freertos.org/>`_.

- `Overview <https://www.freertos.org/RTOS.html>`_
- `Developer Documentation <https://www.freertos.org/features.html>`_
- `API <https://www.freertos.org/a00106.html>`_

************
SMP FreeRTOS
************

To support this new programming model for XCore, XMOS has extended the popular and free FreeRTOS kernel to support SMP (now upstreamed to Amazon Web Services). This allows for the kernel's scheduler to be started on any number of available XCore logical cores per tile, leaving the remaining free to support other program elements that combine to create complete systems. Once the scheduler is started, FreeRTOS threads are placed on cores dynamically at runtime, rather than statically at compile time. All the usual FreeRTOS rules for thread scheduling are followed, except that rather than only running the single highest priority thread that is ready at any given time, multiple threads may run simultaneously. The threads chosen to run are always the highest priority threads that are ready. When there are more threads of a single priority that are ready to run than the number of cores available, they are scheduled in a round robin fashion. Dynamic scheduling allows FreeRTOS to optimize physical core usage based on priority and availability at runtime, opening up the potential for using tile wide MIPs more efficiently than what could be manually specified in a static compile time setting.

One of XCore’s primary strengths is its guarantee of deterministic behavior and timing. RTOS threads can also benefit from this determinism provided by the XCore architecture. An RTOS thread with interrupts disabled and a high enough priority behaves just as a bare-metal thread. An SMP RTOS kernel does not need to preempt a high priority thread because it has many other cores to utilize to schedule lower priority threads. Using an SMP RTOS allows developers to concentrate on specific requirements of their application without worrying about what affect they might have on non-preemptable thread response times. Furthermore, modification of the program in the future is much easier because the developer does not have to worry about affecting existing responsiveness with changes in unrelated areas. The non-preemptable threads will not be effected by adding lower-priority functionality.

Another XCore strength is it's performance. xcore.ai provides lightning fast general purpose compute, AI acceleration, powerful DSP and instantaneous I/O control. RTOS threads can also benefit from the performance provided by the XCore architecture, allowing an application developer to dynamically shift performance usage from one application feature to another. If more general purpose compute is needed, simply make those tasks higher priority, if more AI acceleration suddenly is required, simply make those tasks higher priority. The same is true for DSP and I/O control.

See `Symmetric Multiprocessing (SMP) with FreeRTOS <https://freertos.org/symmetric-multiprocessing-introduction.html>`_ for information on SMP support in the FreeRTOS kernel and SMP specific considerations.

****************
AMP SMP FreeRTOS
****************

To further leverage the XCore hardware and the FreeRTOS programming model, XMOS provides support for asymmetric multiprocessing (AMP) per tile. Each XMOS chip contains at least two tiles, which consist of their own set of logical XCore cores, IO, memory space, and more. XMOS provides a build method and variety of software drivers to allow an application to be created that is an AMP system containing, multiple SMP FreeRTOS kernels.  More information on how this programming model works can be found in :doc:`Getting Started with FreeRTOS <getting_started>`.

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
  - Clock control

- XCore features

  - Intertile channel communication
  - Software defined memory (xcore.ai only)
  - Software defined L2 Cache (xcore.ai only)

- External parts

  - Silicon Labs WF200 series WiFi transceiver

These drivers are all found in the SDK under the path `modules/rtos/drivers <https://github.com/xmos/xcore_sdk/tree/develop/modules/rtos/drivers>`_.

Documentation on each of these drivers can be found under the :doc:`Modules/RTOS Drivers<../../reference/rtos_drivers/index>` section in the SDK documentation pages.

It is worth noting that most of these drivers utilize a lightweight RTOS abstraction layer, meaning that they are not dependent on FreeRTOS. Conceivably they should work on any SMP RTOS, provided an abstraction layer for it is provided. This abstraction layer is found under the path `modules/rtos/osal <https://github.com/xmos/xcore_sdk/tree/develop/modules/rtos/osal>`_. At the moment the only available SMP RTOS for XCore is the XMOS SMP FreeRTOS, but more may become available in the future.

*****************
Software Services
*****************

The SDK also includes some higher level RTOS compatible software services, some of which call the aforementioned drivers. These include, but are not necessarily limited to:

- DHCP server
- Dispatcher
- FAT filesystem
- HTTP parser
- JSON parser
- MQTT client
- SNTP client
- TLS
- USB stack
- WiFi connection manager

Documentation on several software services can be found under the :doc:`Tutorials/FreeRTOS/Software Services<../../reference/rtos_services/index>` section in the SDK documentation pages.

These services are all found in the SDK under the path `modules/rtos/sw_services <https://github.com/xmos/xcore_sdk/tree/develop/modules/rtos/sw_services>`_.

***************
Getting Started
***************

The :doc:`Getting Started with FreeRTOS <getting_started>` tutorial will guide you through writing a FreeRTOS application for XCore.
