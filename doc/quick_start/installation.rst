.. _sdk-installation-label:

############
Installation
############

*******************
System Requirements
*******************

Make sure your system meets the minumum :ref:`system requirements <sdk-system-requirements-label>` and that you have installed all necessary :ref:`prerequisites <sdk-prerequisites-label>`.

******************
Installation Steps
******************

Step 1. Cloning the SDK
=======================

Clone the XCore SDK repository with the following command:

.. code-block:: console

    $ git clone --recurse-submodules https://github.com/xmos/xcore_sdk.git

Step 2. Install Host Applications
=================================

The SDK includes utilities that run on the PC host.  Run the following command to build and install these utilities:

.. tab:: Linux and MacOS

    .. code-block:: console

        $ cmake -B build_host
        $ cd build_host
        $ sudo make install

    This command installs the applications at ``/opt/xmos/SDK/<sdk version>/bin/`` directory.  You may wish to append this directory to your ``PATH`` variable.

    .. code-block:: console

        $ export PATH=$PATH:/opt/xmos/SDK/<sdk_version>/bin/

.. tab:: Windows

    Windows users must run the x86 native tools command prompt from Visual Studio

    .. code-block:: console

        $ cmake -B build_host
        $ cd build_host
        $ make install

    This command installs the applications at ``<USERPROFILE>\.xmos\SDK\<sdk version>\bin\`` directory.  You may wish to add this directory to your ``PATH`` variable.


Optional Step 3. Install Python and Python Requirements
=======================================================

The SDK does not require installing Python, however, several example applications do utilize Python scripts.  To run these scripts, Python 3 is needed, we recommend and test with Python 3.8.  Install `Python <https://www.python.org/downloads/>`__ and install the dependencies using the following commands:

.. note:: You can also setup a Python virtual environment using Conda or other virtual environment tool.

Install pip if needed:

.. code-block:: console

    $ python -m pip install --upgrade pip

Then use `pip` to install the required modules.

.. code-block:: console

    $ pip install -r tools/install/requirements.txt

Start Your First Application
============================

Follow the :doc:`Tutorials <../tutorials/index>` and begin your first project.
    