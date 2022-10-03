#####
XLINK
#####

This example application demonstrates the `AN01024 <https://www.xmos.ai/file/an01024-xconnect-dynamic-configuration-demo-sw/>`_ application note in FreeRTOS on XCORE AI.

.. note::

  This example application required XTC Tools version 15.2.0 or newer.

**********************
Preparing the hardware
**********************

This example requires 2 XCORE-AI-EXPLORER boards, and a user provided device to act as an I2C slave.

To setup the board for testing, the following connections must be made:

.. list-table:: XCORE-AI-EXPLORER to XCORE-AI-EXPLORER Connections 2 Wire
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - BOARD 0
     - BOARD 1
   * - GND
     - GND
   * - X1D65
     - X1D66
   * - X1D66
     - X1D65
   * - X1D64
     - X1D67
   * - X1D67
     - X1D64
   * - X1D63
     - X1D68
   * - X1D68
     - X1D63
   * - X1D62
     - X1D69
   * - X1D69
     - X1D62
   * - X1D61
     - X1D70
   * - X1D70
     - X1D61

.. list-table:: XCORE-AI-EXPLORER to XCORE-AI-EXPLORER Connections 5 Wire Additions
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - BOARD 0
     - BOARD 1
   * - X1D63
     - X1D68
   * - X1D68
     - X1D63
   * - X1D62
     - X1D69
   * - X1D69
     - X1D62
   * - X1D61
     - X1D70
   * - X1D70
     - X1D61

.. list-table:: XCORE-AI-EXPLORER Board 0 to Host Connections
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - BOARD 0
     - Host
   * - GND
     - Host GND
   * - SCL
     - Host SCL
   * - SDA
     - Host SDA

*********************
Building the firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        make example_freertos_xlink_both

.. tab:: Windows

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        nmake example_freertos_xlink_both

********************
Running the firmware
********************

This application requires example_freertos_xlink_0.xe to be run on BOARD 0, IE, the board with a host I2C connection.

Use the following command to determine available device:

.. code-block:: console

    xrun --list-devices

From the xcore_sdk build folder run:

.. code-block:: console

    xrun --id 0 example_freertos_xlink_0.xe
        
In another console, from the xcore_sdk build folder run:

.. code-block:: console

    xrun --id 1 example_freertos_xlink_1.xe

BOARD 0 will send out status messages and communication details to slave address 0xC.

The data will contain an ID, followed by a 4 byte payload.  The payload is an int32, sent least significant byte first.

Payloads match to ID per the table below:

.. list-table:: XCORE-AI-EXPLORER to XCORE-AI-EXPLORER Connections 2 Wire
   :widths: 50 50
   :header-rows: 1
   :align: left

   * - ID
     - Payload
   * - 0x01
     - RX state
   * - 0x82
     - received data bytes in the last second
   * - 0x83
     - received control tokens in the last second
   * - 0x84
     - timeouts in the last second

.. note::
  
    Data rates are highly dependant on the electrical characteristics of the physical connection.  Refer to `xCONNECT Architecture <https://www.xmos.ai/file/xconnect-architecture/>`_ for more information.