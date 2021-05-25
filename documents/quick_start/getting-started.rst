###############
Getting Started
###############

********
Overview
********

The XCore SDK is comprised the following components:

- FreeRTOS - Libraries to support FreeRTOS operation on xcore.ai.
- Code Examples - Examples showing a variety of operations based on bare-metal and FreeRTOS operation.
- AI Toolchain - Scripts, tools and libraries to convert TensorFlow Lite for Microcontroller models to a format targetting accelerated operations on the xcore.ai platform.
- Documentation - Getting started guides and example build and execution instructions.

The SDK is designed to be used in conjunction with the xcore-ai Explorer board. The example applications compile targeting this board. Further information about the Explorer board, and xcore.ai device are available to authorised parties on www.xmos.ai. Future releases will include other xcore.ai hardware platforms, targeting specific use case applications. The following sections detail required tools, describe the example applications, upcoming features and how to get support and provide feedback.

 .. _sdk-installation-label:

************
Installation
************

System Requirements
===================

Make sure your system meets the minumum :ref:`system requirements <sdk-system-requirements-label>` and that you have installed all necessary :ref:`prerequisites <sdk-prerequisites-label>`.

Cloning the SDK
===============

Clone the XCore SDK repository with the following commands:

.. code-block:: console

    $ git clone https://github.com/xmos/xcore_sdk.git
    $ cd xcore_sdk
    $ git submodule update --init --recursive

Set up Environment Variable
===========================

All code examples require that an environment variable be set that specifies the fully-qualified path to the xcore_sdk directory. Set the following environment variable:

.. code-block:: console

    $ export XCORE_SDK_PATH=<path to>/xcore_sdk

.. note:: You can also add this export command to your ``.profile`` or ``.bash_profile`` script. This way the environment variable will be set every time a new terminal window is launched.
