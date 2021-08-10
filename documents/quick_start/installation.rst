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

It is recommended that your `XCORE_SDK_PATH` not include spaces.  However, if this is not possible, you will need to enclose your path environment variable value in quotes.

.. code-block:: console

    $ export XCORE_SDK_PATH="<path with spaces to>/xcore_sdk"

.. note:: You can also add this export command to your ``.profile`` or ``.bash_profile`` script. This way the environment variable will be set every time a new terminal window is launched.  Windows users can add the XCORE_SDK_PATH to the System Properties.

Install Python and Python Requirements
======================================

The SDK does not require installing Python, however, several example applications do utilize Python scripts and require some Python modules.  To run these scripts, Python 3 is needed, we recommend and test with Python 3.8.  Install `Python <https://www.python.org/downloads/>`__ and install the dependencies using the following commands:

.. note:: You can also setup a Python virtual environment using Conda or other virtual environment tool.

Install pip if needed:

.. code-block:: console

    $ python -m pip install --upgrade pip

Then use `pip` to install the required modules.

.. code-block:: console

    $ pip install -r tools/install/requirements.txt
