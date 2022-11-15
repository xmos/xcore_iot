##############
DFU
##############

This example application demonstrates a method to add DFU to a FreeRTOS application on XCORE.

**********************
Preparing the host
**********************

This application supports any host host application that is capable of USB DFU Class V1.1.

The application was verified using dfu-util.

Installation instructions for respective operating system can be found `here <https://dfu-util.sourceforge.net/>`__

If on Linux/Mac the user may need to add the USB device to their udev rules.  This example defaults to Vendor ID 0xCAFE with Product ID 0x4000.

*********************
Building the firmware
*********************

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        make example_freertos_dfu_v1
        make example_freertos_dfu_v2

.. tab:: Windows

    .. code-block:: console

        cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
        cd build
        nmake example_freertos_dfu_v1
        nmake example_freertos_dfu_v2

**********************
Preparing the hardware
**********************

It is recommended to begin from an erased flash.  To erase flash run:

.. code-block:: console

    xflash --erase-all --target=XCORE-AI-EXPLORER

After building the firmware and erasing the flash, the factory image must be flashed.  From the xcore_sdk build folder run:

.. code-block:: console

    xflash --quad-spi-clock 50MHz --factory example_freertos_dfu_v1.xe --boot-partition-size 0x100000

The board may then be power cycled and will boot up the application.

Next, the user make create an upgrade image.  From the xcore_sdk build folder run:

.. code-block:: console

    xflash --factory-version <FACTORY_MAJOR_VER>.<FACTORY_MINOR_VER> --upgrade 0 example_freertos_dfu_v2.xe -o example_freertos_dfu_v2_upgrade.bin

OR
.. code-block:: console

    make create_upgrade_img_example_freertos_dfu_v1

This will create an upgrade image file.

Populating the version of tools you are running.

********************
Running the firmware
********************

After flashed, the factory image will run by default.  The user may opt to manually run via xrun to see debug messages.

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make run_example_freertos_dfu_v1

.. tab:: Windows

    .. code-block:: console

        nmake run_example_freertos_dfu_v1

********************
Upgrading the firmware via DFU
********************

Once the application is running, a USB DFU v1.1 tool can be used to perform various actions.  This example will demonstrate with dfu-util commands.

MacOS users may need to sudo the following commands.

To verify the device is running run:

.. code-block:: console

    dfu-util -l

The output of this command will very based on which image is running.
For example_freertos_dfu_v1, the output should contain:

.. code-block:: console

    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=2, name="DFU dev DATAPARTITION v1", serial="123456"
    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=1, name="DFU dev UPGRADE v1", serial="123456"
    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=0, name="DFU dev FACTORY v1", serial="123456"

For example_freertos_dfu_v2, the output should contain:

.. code-block:: console

    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=2, name="DFU dev DATAPARTITION v2", serial="123456"
    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=1, name="DFU dev UPGRADE v2", serial="123456"
    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=0, name="DFU dev FACTORY v2", serial="123456"

The factory image can be read back by running:

.. code-block:: console

    dfu-util -e -d 4000 -a 0 -U readback_factory_img.bin

From the xcore_sdk build folder, the upgrade image can be written by running:

.. code-block:: console

    dfu-util -e -d 4000 -a 1 -D example_freertos_dfu_v2_upgrade.bin

The upgrade image can be read back by running:

.. code-block:: console

    dfu-util -e -d 4000 -a 1 -U readback_upgrade_img.bin

The data partition image can be read back by running:

.. code-block:: console

    dfu-util -e -d 4000 -a 2 -U readback_data_partition_img.bin

The data partition image can be written by running:

.. code-block:: console

    dfu-util -e -d 4000 -a 2 -D readback_data_partition_img.bin

If running the application with the run_example_freertos_dfu_v1 target, information is printed to verify behavior.

Initially, the debug prints will contain:

.. code-block:: console

    DFU Image Info
    Factory:
        Addr:0x1C70
        Size:103108
        Version:0
    Upgrade:
        Addr:0x1B000
        Size:0
        Version:0
    Data Partition
        Addr:0x100000
    First word at data partition start is: 0xFFFFFFFF

After writing an upgrade image the debug prints will contain:

.. code-block:: console

    DFU Image Info
    Factory:
        Addr:0x1C70
        Size:103108
        Version:0
    Upgrade:
        Addr:0x1B000
        Size:103108
        Version:0
    Data Partition
        Addr:0x100000
    First word at data partition start is: 0xFFFFFFFF

The debug prints include the value of the first word at the start of the data partition.  Writing a text file containing "XMOS" will result in:

.. code-block:: console

    DFU Image Info
    Factory:
        Addr:0x1C70
        Size:103108
        Version:0
    Upgrade:
        Addr:0x1B000
        Size:103108
        Version:0
    Data Partition
        Addr:0x100000
    First word at data partition start is: 0x534F4D58

********************************
Debugging the firmware with xgdb
********************************

From the xcore_sdk build folder run:

.. tab:: Linux and Mac

    .. code-block:: console

        make debug_example_freertos_dfu_v1

.. tab:: Windows

    .. code-block:: console

        nmake debug_example_freertos_dfu_v1
