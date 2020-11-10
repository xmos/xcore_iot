###############
Getting Started
###############

********
Overview
********

The AIoT is comprised the following components:

- AI Toolchain Extensions - Scripts, tools and libraries to convert TensorFlow Lite for Microcontroller models to a format targetting accelerated operations on the xcore.ai platform.
- FreeRTOS - Libraries to support FreeRTOS operation on xcore.ai.
- Examples - Examples showing a variety of operations based on bare-metal and FreeRTOS operation.
- Documentation - Getting started guides and example build and execution instructions.

The AIoT SDK is designed to be used in conjunction with the xcore-ai Explorer board. The example applications compile targeting this board. Further information about the Explorer board, and xcore.ai device are available to authorised parties on www.xmos.ai. Future releases will include other xcore.ai hardware platforms, targeting specific use case applications. The following sections detail required tools, describe the example applications, upcoming features and how to get support and provide feedback.

 .. _aiot-sdk-installation-label:

************
Installation
************

Cloning the SDK
===============

Clone the AIoT SDK repository with the following command:

.. code-block:: console

    $ git clone --recurse-submodules https://github.com/xmos/aiot_sdk.git

Set up Environment Variable
===========================

.. code-block:: console

    $ export XMOS_AIOT_SDK_PATH=<path to>/aiot_sdk

You can also add this export command to your ``.profile`` or ``.bash_profile`` script. This way the environment variable will be set in a new terminal window.
