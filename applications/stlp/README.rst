============================
Avona Voice Reference Design
============================

This is the XMOS Avona voice reference design

****************** 
Supported Hardware
****************** 

This example is supported on the XCORE-AI-EXPLORER board.

***** 
Setup
***** 

This example requires the xcore_sdk and Amazon Wakeword.

Set the environment variable XCORE_SDK_PATH to the root of the xcore_sdk and set the environment variable WW_PATH to the root of the Amazon Wakeword.

.. tab:: Linux and MacOS

    .. code-block:: console

        $ export XCORE_SDK_PATH=/path/to/sdk
        $ export WW_PATH=/path/to/wakeword
        
.. tab:: Windows XTC Tools CMD prompt

    .. code-block:: console
    
        $ set XCORE_SDK_PATH=C:\path\to\sdk\
        $ set WW_PATH=C:\path\to\wakeword\

It is recommended that your `XCORE_SDK_PATH` and WW_PATH not include spaces.  However, if this is not possible, you will need to enclose your path environment variable value in quotes.

.. code-block:: console

    $ export XCORE_SDK_PATH="<path with spaces to>/xcore_sdk"

.. note:: Linux and MacOS users can add this export command to your ``.profile`` or ``.bash_profile`` script. This way the environment variable will be set every time a new terminal window is launched.  Windows users can add the environmet variables to the System Properties.

*********************
Building the Firmware
*********************

Run the following commands to build the avona firmware:

.. tab:: Linux and MacOS

    .. code-block:: console
    
        $ cmake -B build -DMULTITILE_BUILD=1 -DUSE_WW=amazon -DBOARD=XCORE-AI-EXPLORER -DXE_BASE_TILE=0 -DOUTPUT_DIR=bin
        $ cd build
        $ make -j
        
.. tab:: Windows XTC Tools CMD prompt

    .. code-block:: console
    
        $ cmake -G "NMake Makefiles" -B build -DMULTITILE_BUILD=1 -DUSE_WW=amazon -DBOARD=XCORE-AI-EXPLORER -DXE_BASE_TILE=0 -DOUTPUT_DIR=bin
        $ cd build
        $ nmake

After building the firmware, you need to create the filesystem, which includes the wakeword models, and flash the device with the following commands:

Flash the filesystem with the following command:

.. tab:: Linux and MacOS

    .. code-block:: console

        $ cd filesystem_support
        $ ./flash_image.sh

.. tab:: Windows

    .. code-block:: console

        $ cd filesystem_support
        $ flash_image.bat

********************
Running the Firmware
********************

From the root folder of the avona application run:

    .. code-block:: console

        $ xrun --xscope bin/sw_avona.xe
