###
DFU
###

This example application demonstrates a method to add DFU to a FreeRTOS application on XCORE. Additionally, it provides an example usage of the alternate loader image selection. This application demonstrates a factory image and an upgrade image. The user can force the factory image to be loaded on boot by pressing a button at boot, otherwise the upgrade image will be used by default. The user can use USB DFU to replace the upgrade image.

******************
Preparing the host
******************

This application supports any host host application that is capable of USB DFU Class V1.1.

The application was verified using dfu-util.

Installation instructions for respective operating system can be found `here <https://dfu-util.sourceforge.net/>`__

If on Linux the user may need to add the USB device to their udev rules.  This example defaults to Vendor ID 0xCAFE with Product ID 0x4000.

If on Windows the user may need to use a tool such as Zadig to install USB drivers.

******************************************
Deploying the firmware with Linux or macOS
******************************************

=====================
Building the firmware
=====================

Run the following commands in the root folder to build the firmware:

.. code-block:: console

    cmake -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    make example_freertos_dfu_v1
    make example_freertos_dfu_v2
    make example_freertos_dfu_v3
    make create_loader_object_example_freertos_dfu_v1

======================
Preparing the hardware
======================

It is recommended to begin from an erased flash.  To erase flash run:

.. code-block:: console

    make erase_all_example_freertos_dfu_v1

This target will use ``xflash`` to erase the flash of the device specified by the provided target XN file.

After building the firmware and erasing the flash, the factory image must be flashed.  From the build folder run:

    make flash_app_example_freertos_dfu_v1

This target will use ``xflash`` to flash the application as a factory image with a boot partition size specified in ``dfu.cmake``.

The board may then be power cycled and will boot up the application.

.. code-block:: console

    make create_upgrade_img_example_freertos_dfu_v2
    make create_upgrade_img_example_freertos_dfu_v3

This target will use ``xflash`` to create an upgrade image for the specified target.

====================
Running the firmware
====================

After flashed, the factory image will run by default.  The user may opt to manually run via ``xrun`` to see debug messages.

.. code-block:: console

    make run_example_freertos_dfu_v1

================================
Debugging the firmware with xgdb
================================

From the build folder run:

.. code-block:: console

    make debug_example_freertos_dfu_v1

***********************************
Deploying the firmware with Windows
***********************************

=====================
Building the firmware
=====================

Run the following commands in the root folder to build the firmware:

.. code-block:: console

    cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=xmos_cmake_toolchain/xs3a.cmake
    cd build
    nmake example_freertos_dfu_v1
    nmake example_freertos_dfu_v2
    nmake example_freertos_dfu_v3
    nmake create_loader_object_example_freertos_dfu_v1

======================
Preparing the hardware
======================

It is recommended to begin from an erased flash.  To erase flash run:

.. code-block:: console

    nmake erase_all_example_freertos_dfu_v1

This target will use ``xflash`` to erase the flash of the device specified by the provided target XN file.

After building the firmware and erasing the flash, the factory image must be flashed.  From the build folder run:

.. code-block:: console

    nmake flash_app_example_freertos_dfu_v1

This target will use ``xflash`` to flash the application as a factory image with a boot partition size specified in ``dfu.cmake``.

The board may then be power cycled and will boot up the application.

.. code-block:: console

    nmake create_upgrade_img_example_freertos_dfu_v2
    nmake create_upgrade_img_example_freertos_dfu_v3

This target will use ``xflash`` to create an upgrade image for the specified target.

====================
Running the firmware
====================

After flashed, the factory image will run by default.  The user may opt to manually run via ``xrun`` to see debug messages.

From the build folder run:

.. code-block:: console

    nmake run_example_freertos_dfu_v1

================================
Debugging the firmware with xgdb
================================

From the build folder run:

.. code-block:: console

    nmake debug_example_freertos_dfu_v1

******************************
Upgrading the firmware via DFU
******************************

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

From the build folder, the upgrade image can be written by running:

.. code-block:: console

    dfu-util -e -d 4000 -a 1 -D example_freertos_dfu_v2_upgrade.bin

After updating the upgrade image it may be necessary to unplug the USB device to initiate a host re-enumeration.

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


******************************
Using Alternate Loader
******************************

An application can override the default loader behavior to control which image is loaded during the second stage bootloader. This example checks the state of button A at boot. If pressed, all upgrade images are rejected, forcing loading of the factory image. If not pressed, the image selection will use normal behavior.

This example can only be run from flash, and not xrun nor xgdb.

==========================================
Deploying the firmware with Linux or macOS
==========================================

From the build folder run:

.. code-block:: console

    make example_freertos_dfu_loader_flash

===================================
Deploying the firmware with Windows
===================================

From the build folder run:

.. code-block:: console

    nmake example_freertos_dfu_loader_flash

The user can power cycle or press the RESET button to force reboot. If button A is pressed at boot, the factory image will be loader. If button A is not pressed at boot, the highest valid image will be loaded.

Once booted the running image can be checked with the dfu-util tool. The output of this command will very based on which image is running.
The factory image in this example is example_freertos_dfu_v1.

To check the running version run:

.. code-block:: console

    dfu-util -l

For the factory image, the output should contain:

.. code-block:: console

    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=2, name="DFU dev DATAPARTITION v1", serial="123456"
    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=1, name="DFU dev UPGRADE v1", serial="123456"
    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=0, name="DFU dev FACTORY v1", serial="123456"

For the default upgrade image, the output should contain:

.. code-block:: console

    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=2, name="DFU dev DATAPARTITION v2", serial="123456"
    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=1, name="DFU dev UPGRADE v2", serial="123456"
    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=0, name="DFU dev FACTORY v2", serial="123456"

From the build folder, the upgrade image can be changed by running:

.. code-block:: console

    dfu-util -e -d 4000 -a 1 -D example_freertos_dfu_v3_upgrade.bin

This will replace the upgrade image with the v3 binary. After this, the output of `dfu-util -l` should yield:

.. code-block:: console

    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=2, name="DFU dev DATAPARTITION v3", serial="123456"
    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=1, name="DFU dev UPGRADE v3", serial="123456"
    Found DFU: [cafe:4000] ver=0100, devnum=53, cfg=1, intf=0, path="3-4.1", alt=0, name="DFU dev FACTORY v3", serial="123456"
