.. _sdk-installation:

############
Installation
############

.. _sdk-system-requirements:

*************
Prerequisites
*************

`XTC Tools 15.2.1 <https://www.xmos.com/software/tools/>`_ or newer and `CMake 3.21 <https://cmake.org/download/>`_ or newer are required for building the example applications.  If necessary, download and follow the installation instructions for those components.

=======
Windows
=======

A standard C/C++ compiler is required to build applications for the host PC.  Windows users may use `Build Tools for Visual Studio <https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170#download-and-install-the-tools>`__ command-line interface.

Host build should also work using other Windows GNU development environments like GNU Make, MinGW or Cygwin.

libusb
------

The DFU example requires `dfu-util <https://dfu-util.sourceforge.net/>`_ which requires ``libusb v1.0``. ``libusb`` requires the installation of a driver for use on a Windows host. Driver installation should be done using a third-party installation tool like `Zadig <https://zadig.akeo.ie/>`_.

=====
macOS
=====

A standard C/C++ compiler is required to build applications for the host PC.  Mac users may use the Xcode command-line tools.

.. _sdk-install-steps:

*************
Install Steps
*************

Follow the following steps to install and setup the SDK:

=======================
Step 1. Cloning the SDK
=======================

Clone the XCORE SDK repository with the following command:

.. code-block:: console

    git clone --recurse-submodules git@github.com:xmos/xcore_sdk.git

=================================
Step 2. Install Host Applications
=================================

The SDK includes utilities that run on the PC host.  Run the following command to build and install these utilities:

Linux and MacOS
---------------

.. code-block:: console

    cmake -B build_host
    cd build_host
    sudo make install

This command installs the applications at ``/opt/xmos/SDK/<sdk version>/bin/`` directory.  You may wish to append this directory to your ``PATH`` variable.

.. code-block:: console

    export PATH=$PATH:/opt/xmos/SDK/<sdk_version>/bin/

Some host applications require that the location of ``xscope_endpoint.so`` be added to your ``LD_LIBRARY_PATH`` environment variable.  This environment variable will be set if you run the host application in the XTC Tools command-line environment.  For more information see `Configuring the command-line environment <https://www.xmos.ai/documentation/XM-014363-PC-LATEST/html/tools-guide/install-configure/getting-started.html>`__.   

Or, you may prefer to set this environment variable manually.

.. code-block:: console

    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<path-to-XTC-Tools>/lib

Windows
-------

Windows users must run the x86 native tools command prompt from Visual Studio

.. code-block:: console

    cmake  -G "NMake Makefiles" -B build_host
    cd build_host
    nmake install

This command installs the applications at ``<USERPROFILE>\.xmos\SDK\<sdk version>\bin\`` directory.  You may wish to add this directory to your ``PATH`` variable.

Some host applications require that the location of ``xscope_endpoint.dll`` be added to your PATH. This environment variable will be set if you run the host application in the XTC Tools command-line environment.  For more information see `Configuring the command-line environment <https://www.xmos.ai/documentation/XM-014363-PC-LATEST/html/tools-guide/install-configure/getting-started.html>`__.

=======================================================
Optional Step 3. Install Python and Python Requirements
=======================================================

The SDK does not require installing Python, however, several example applications do utilize Python scripts.  To run these scripts, Python 3 is needed, we recommend and test with Python 3.8.  Install `Python <https://www.python.org/downloads/>`__ and install the dependencies using the following commands:

.. note:: 
    
    You can also setup a Python virtual environment using Conda or other virtual environment tool.

Install ``pip`` if needed:

.. code-block:: console

    python -m pip install --upgrade pip

Then use ``pip`` to install the required modules.

.. code-block:: console

    pip install -r tools/install/requirements.txt

==================================
Build & Run Your First Application
==================================

Once your have installed the SDK, the next step is to :ref:`build and run your first xcore application. <sdk-tutorials>`
    
