############
Introduction
############

The XCORE SDK is a collection of C/C++ software libraries designed to simplify and accelerate application development on xcore processors. It is composed of the following components:

- :ref:`Peripheral IO <fwk_io-peripherals>` libraries including; UART, I2C, I2S, SPI, QSPI, PDM microphones, and USB. These libraries support bare-metal and RTOS application development.
- Libraries core to :ref:`DSP applications <fwk_core-optimized>`, including :ref:`vectorized math <fwk_core-optimized>`.  These libraries support bare-metal and RTOS application development. 
- Voice processing libraries including; adaptive echo cancellation, adaptive gain control, noise suppression, interference cancellation (IC), and voice activity detection. These libraries support bare-metal and RTOS application development.
- Libraries that enable `multi-core FreeRTOS development <https://www.freertos.org/symmetric-multiprocessing-introduction.html>`__ on xcore including a wide array of :ref:`RTOS drivers and middleware <fwk_rtos-framework>`.
- Code Examples - Examples showing a variety of xcore features based on :ref:`bare-metal <sdk-baremetal-code-examples>` and :ref:`FreeRTOS <sdk-freertos-code-examples>` programming.
- Documentation - Getting started guides, references and API guides.

The SDK is designed to be used in conjunction with the xcore.ai Explorer board and the Voice Reference evaluation kit. The example applications compile targeting these boards. Further information about the Explorer board, the Voice Reference evaluation kit, and xcore.ai devices is available to on `www.xmos.ai <https://www.xmos.ai/>`__.

***************
Getting Started
***************

Fast-forward to the :ref:`Tutorials <sdk-tutorials>` for information on :ref:`system requirements <sdk-system-requirements>`, :ref:`installing <sdk-installation>` and :ref:`using the XCORE SDK <sdk-tutorials>`.   
