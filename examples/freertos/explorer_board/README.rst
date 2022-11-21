##############
Explorer Board
##############

This example application demonstrates various capabilities of the Explorer board using FreeRTOS. The application uses I2C, I2S, SPI, UART, flash, mic array, and GPIO devices.

The FreeRTOS application creates a single stage audio pipeline which applies a variable gain. The output audio is sent to the DAC and can be listened to via the 3.5mm audio jack. The audio gain can be adjusted via GPIO, where button A is volume up and button B is volume down.

**********************
Preparing the hardware
**********************

The UART loopback section of the demo requires that a jumper cable be connected
between X1D36 and X1D39. This connects the Tx pin to the Rx pin.

*********************
Building the firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        make example_freertos_explorer_board

.. tab:: Windows

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        nmake example_freertos_explorer_board

.. note::
   The host applications are required to create the filesystem.  See the SDK Installation instructions for more information.

From the xcore_sdk build folder, create the filesystem and flash the device with the following command:

.. tab:: Linux and Mac

    .. code-block:: console

        make flash_app_example_freertos_explorer_board

.. tab:: Windows

    .. code-block:: console

        nmake flash_app_example_freertos_explorer_board

********************
Running the firmware
********************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make run_example_freertos_explorer_board

.. tab:: Windows

    .. code-block:: console

        nmake run_example_freertos_explorer_board


********************************
Debugging the firmware with xgdb
********************************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make debug_example_freertos_explorer_board

.. tab:: Windows

    .. code-block:: console

        nmake debug_example_freertos_explorer_board
