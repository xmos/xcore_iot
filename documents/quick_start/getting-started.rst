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

Clone the AIoT SDK repository with the following commands:

.. code-block:: console

    $ git clone https://github.com/xmos/aiot_sdk.git
    $ cd aiot_sdk
    $ git submodule update --init --recursive modules
    
Set up Environment Variable
===========================

.. code-block:: console

    $ export XMOS_AIOT_SDK_PATH=<path to>/aiot_sdk

You can also add this export command to your ``.profile`` or ``.bash_profile`` script. This way the environment variable will be set every time a new terminal window is launched.

Set up Virtual Environment
==========================

To install, follow these steps:

**Step 1. Create a Conda environment**

.. code-block:: console

    $ conda create --prefix xmos_env python=3.6

Activate the environment

.. code-block:: console

    $ conda activate xmos_env

.. note:: You may need to specify the fully-qualified path to your environment.

**Step 2. Install**

The following command will install all libraries and Python modules.

.. code-block:: console

    $ ./install.sh
