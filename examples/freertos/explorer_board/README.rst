##############
Explorer Board
##############

This example application demonstrates various capabilities of the Explorer board using FreeRTOS.  The example uses lib_soc and various libraries to build FreeRTOS applications targeting xCORE.  The application uses I2C, I2S, SPI, flash, mic array, and GPIO devices.

The FreeRTOS application creates a single stage audio pipeline which applies a variable gain. The output audio is sent to the DAC and can be listened to via the 3.5mm audio jack. The audio gain can be adjusted via GPIO, where button A is volume up and button B is volume down.

*********************
Building the firmware
*********************

Run the following commands in the xcore_sdk root folder to build the explorer_board firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make explorer_board

.. tab:: Windows XTC Tools CMD prompt

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake explorer_board

From the xcore_sdk build folder, create the filesystem and flash the device with the following command:

.. tab:: Linux and MacOS

    .. code-block:: console

        $ make flash_fs_explorer_board

.. tab:: Windows

    .. code-block:: console

        $ make flash_fs_explorer_board

*********************
Running the firmware
*********************

From the xcore_sdk build folder run:

.. code-block:: console

    $ make run_explorer_board

*********************
Debugging the firmware with xgdb
*********************

From the xcore_sdk build folder run:

.. code-block:: console

    $ make debug_explorer_board
