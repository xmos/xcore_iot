###############
Getting Started
###############

********
Overview
********

The XCore SDK is comprised the following components:

- AI Toolchain Extensions - Scripts, tools and libraries to convert TensorFlow Lite for Microcontroller models to a format targetting accelerated operations on the xcore.ai platform.
- FreeRTOS - Libraries to support FreeRTOS operation on xcore.ai.
- Examples - Examples showing a variety of operations based on bare-metal and FreeRTOS operation.
- Documentation - Getting started guides and example build and execution instructions.

The SDK is designed to be used in conjunction with the xcore-ai Explorer board. The example applications compile targeting this board. Further information about the Explorer board, and xcore.ai device are available to authorised parties on www.xmos.ai. Future releases will include other xcore.ai hardware platforms, targeting specific use case applications. The following sections detail required tools, describe the example applications, upcoming features and how to get support and provide feedback.

 .. _sdk-installation-label:

************
Installation
************

System Requirements
===================

Mke sure your system meets the minumum :ref:`system requirements <sdk-system-requirements-label>` and that you have installed all necessary :ref:`prerequisites <sdk-prerequisites-label>`.

Cloning the SDK
===============

Clone the XCore SDK repository with the following commands:

.. code-block:: console

    $ git clone https://github.com/xmos/xcore_sdk.git
    $ cd xcore_sdk
    $ git submodule update --init --recursive

Set up Environment Variable
===========================

.. code-block:: console

    $ export XCORE_SDK_PATH=<path to>/xcore_sdk

.. note:: You can also add this export command to your ``.profile`` or ``.bash_profile`` script. This way the environment variable will be set every time a new terminal window is launched.

.. _sdk-setup-virtual-environment-label:

Set up Virtual Environment
==========================

To install, follow these steps:

**Step 1. Create a Conda environment**

.. code-block:: console

    $ conda create --prefix xcore_sdk_env python=3.8

.. _sdk-activate-virtual-environment-label:

Activate the environment

.. code-block:: console

    $ conda activate xcore_sdk_env

.. note:: You may need to specify the fully-qualified path to your environment.

.. note:: You can also add an alias to your ``.profile`` or ``.bash_profile`` script to make activating the environment easier. Add a line similar to ``alias xcore_sdk_env='conda activate xcore_sdk_env'``.  The next time you launch a console, the command ``xcore_sdk_env`` will activate the environment.

**Step 2. Install**

The following command will install all libraries and Python modules.

.. code-block:: console

    $ ./install.sh
