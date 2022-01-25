##############
Explorer Board
##############

This example application demonstrates various capabilities of the Explorer board using FreeRTOS.  The example uses lib_soc and various libraries to build FreeRTOS applications targetting xCORE.  The application uses I2C, I2S, SPI, flash, mic array, and GPIO devices.

The FreeRTOS application creates a single stage audio pipeline which applies a variable gain. The output audio is sent to the DAC and can be listened to via the 3.5mm audio jack. The audio gain can be adjusted via GPIO, where button A is volume up and button B is volume down.

****************
Filesystem setup
****************

Before the demo can be run, the filesystem must be configured and flashed.

Note, macOS users will need to install `dosfstools`.

.. code-block:: console

    $ brew install dosfstools

Flash the filesystem by running the script:

.. code-block:: console

    $ ./flash_image.sh

*********************
Building the firmware
*********************

.. tab:: Linux and Mac

    Run cmake:

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make

.. tab:: Windows XTC Tools CMD prompt

    Run cmake:

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake

Running the firmware
====================

To run the demo navigate to the bin folder and use the command:

.. code-block:: console

    $ xrun --xscope bin/explorer_board.xe
