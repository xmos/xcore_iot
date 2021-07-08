.. _sdk-installation-label:

############
Installation
############

*******************
System Requirements
*******************

Make sure your system meets the minumum :ref:`system requirements <sdk-system-requirements-label>` and that you have installed all necessary :ref:`prerequisites <sdk-prerequisites-label>`.

*******
Install
*******

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
