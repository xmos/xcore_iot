##############
Explorer Board
##############

This example application demonstrates various capabilities of the Explorer board using FreeRTOS.  The example uses lib_soc and various libraries to build FreeRTOS applications targeting xCORE.  The application uses I2C, I2S, SPI, flash, mic array, and GPIO devices.

The FreeRTOS application creates a single stage audio pipeline which applies a variable gain. The output audio is sent to the DAC and can be listened to via the 3.5mm audio jack. The audio gain can be adjusted via GPIO, where button A is volume up and button B is volume down.

*********************
Building the firmware
*********************

Run the following commands to build the explorer_board firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make

.. tab:: Windows XTC Tools CMD prompt

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake

After building the firmware, create the filesystem and flash the device with the following commands:

.. tab:: Linux and MacOS

    .. code-block:: console

        $ cd filesystem_support
        $ ./flash_image.sh

.. tab:: Windows

    .. code-block:: console

        $ cd filesystem_support
        $ flash_image.bat

Running the firmware
====================

From the root folder of the explorer_board application run:

.. code-block:: console

    $ xrun --xscope bin/explorer_board.xe
