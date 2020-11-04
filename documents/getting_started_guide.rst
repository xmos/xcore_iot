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

************
Installation
************

System Requirements
===================

The AIoT SDK is officially supported on the following platforms:

- MacOS 10.13 +
- Linux CentOS 5.8 or Ubuntu 12.04 LTS

The tools also work on many other versions of Linux, including Fedora 30 +.

*Windows 10 is not currently supported.  However, support for Windows is expected for initial product release*

Prerequisites
=============

`Python 3.6 <https://www.python.org/downloads/>`_ + is required, however, we recommend you setup an `Anaconda <https://www.anaconda.com/products/individual/>`_ environment before installing.  If necessary, download and follow Anaconda's installation instructions.

`xTIMEComposer 15.0.1 + <https://www.xmos.com/software/tools/>`_ and `CMake 3.14 <https://cmake.org/download/>`_ + are required for building the example applications.  If necessary, download and follow the installation instructions for those components.

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

********************
Example Applications
********************

Several example applications are included to illustrate the fundamental tool flow and provide a starting point for basic evaluation. The examples do not seek to exhibit the potential of the platform, and are purposely basic to provide instruction. 

.. toctree::
   :maxdepth: 2

   examples/index

****************************************
Support, Feedback or Further Information
****************************************

Please contact your Field Application Engineer or Engineering point of contact for support and to request
features and provide feedback.

