XCore SDK Repository
====================

Summary
-------

The XCore SDK is comprised of the following components:

- FreeRTOS - Libraries to support FreeRTOS operation on xcore.ai.
- Code Examples - Examples showing a variety of operations based on bare-metal and FreeRTOS operation.
- AI Tools - Scripts, tools and libraries to convert TensorFlow Lite for Microcontroller models to a format targeting accelerated operations on the xcore.ai platform.
- Documentation - Getting started guides and example build and execution instructions.

The SDK is designed to be used in conjunction with the xcore-ai Explorer board. The example applications compile targeting this board. Further information about the Explorer board, and xcore.ai device are available to authorized parties on `www.xmos.ai <https://www.xmos.ai/>`_. Future releases will include other xcore.ai hardware platforms, targeting specific use case applications. The following sections detail required tools, describe the example applications, upcoming features and how to get support and provide feedback.

Installation
------------

Some dependent components are included as git submodules. These can be obtained by cloning this repository with the following command:

.. code-block:: console

    $ git clone --recurse-submodules https://github.com/xmos/xcore_sdk.git

Documentation
-------------

TODO: Include URL for hosted documentation
