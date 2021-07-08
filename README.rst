XCore SDK Repository
===================

Summary
-------

The XCore SDK and is intended to enable basic evaluation of xcore.ai functionality and perfomance. It comprises the following components:

- FreeRTOS - Libraries to support FreeRTOS operation.
- Code Examples - Examples showing a variety of operations based on bare-metal and FreeRTOS operation.
- AI Tools - Scripts, tools and libraries to convert TensorFlowLite for Microcontroller models to a format targetting accelerated operations.
- Documentation - Getting started guides and example build and execution instructions.

The XCore SDK is designed to be used in conjunction with the xcore-ai Explorer board. The example
applications compile targeting this board. Further information about the Explorer board, and the xcore.ai
device are available to authorised parties on `www.xmos.ai <https://www.xmos.ai/>`_.

Installation
------------

Some dependent components are included as git submodules. These can be obtained by cloning this repository with the following commands:

.. code-block:: console

    $ git clone https://github.com/xmos/xcore_sdk.git
    $ cd xcore_sdk
    $ git submodule update --init --recursive

Documentation
-------------

See the `Installation Guide <https://github.com/xmos/xcore_sdk/blob/develop/documents/quick_start/installation.rst>`_ for instructions on installing the SDK.

Contributing
------------

See the `CONTRIBUTING.rst <https://github.com/xmos/xcore_sdk/blob/develop/CONTRIBUTING.rst>`_ for information on building example applications, running tests, and guidelines for adding code.


