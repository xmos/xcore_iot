########
CIFAR-10
########

This example application implements a CNN architecture trained on the `CIFAR-10 <https://www.cs.toronto.edu/~kriz/cifar.html>`__ dataset.  This example demonstrates how to place models in SRAM, Flash or LPDDR.

The example reads a set of test images from a filesystem in flash.  The FreeRTOS kernel manages filesystem IO and sends example images to the AI device that implements the CIFAR-10 model.  The application will attempt to classify an entity in the image and assign it to one of the following classes:

- airplane
- automobile
- bird
- cat
- deer
- dog
- frog
- horse
- ship
- truck

The resulting output tensor is returned to FreeRTOS for the application to handle.

A Python script is provided that will generate one example image from each of the classes above.  Ensure you have installed Python 3 and the XCore SDK Python requirements.

To generate the images run:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cd filesystem_support/test_inputs
        $ ./make_test_tensors.py
        $ cd ../..

.. tab:: Windows

    .. code-block:: console

        $ cd filesystem_support\test_inputs
        $ python3 make_test_tensors.py
        $ cd..\..

For background information on the CIFAR-10 dataset, please read `Learning Multiple Layers of Features from Tiny Images <https://www.cs.toronto.edu/~kriz/learning-features-2009-TR.pdf>`__, Alex Krizhevsky, 2009.

****************
Supported Boards
****************

This example is supported on the XCORE-AI-EXPLORER board.

.. note::

    The external DDR memory options are only available on the XCORE-AI-EXPLORER board.

*********************
Building the firmware
*********************

Building SRAM memory configuration
==================================

Run the following commands in the xcore_sdk root folder to build the cifar10 firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make example_freertos_cifar10_sram

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake example_freertos_cifar10_sram


Building external flash memory configuration
============================================

.. note::

    There is no Windows support for using external flash memory.

To build with the model stored in flash, replace the call to make above with the following:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make example_freertos_cifar10_swmem


Building external DDR memory configuration
==========================================

If your board supports LPDDR, you may also place your neural network in the external DDR memory.

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make example_freertos_cifar10_extmem

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake example_freertos_cifar10_extmem

***********************
Setting up the hardware
***********************

Before running the firmware, the filesystem containing the images must be flashed.  After the images have been generated, by following the instructions above:

Flashing SRAM memory configuration
==================================

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make flash_fs_example_freertos_cifar10_sram

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake flash_fs_example_freertos_cifar10_sram


Flashing external flash memory configuration
============================================

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make flash_fs_example_freertos_cifar10_swmem


Flashing external DDR memory configuration
==========================================

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make flash_fs_example_freertos_cifar10_extmem

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake flash_fs_example_freertos_cifar10_extmem

********************
Running the firmware
********************

Running with hardware.


Running SRAM memory configuration
=================================

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make run_example_freertos_cifar10_sram

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake run_example_freertos_cifar10_sram


Running external flash memory configuration
===========================================

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make run_example_freertos_cifar10_swmem


Running external DDR memory configuration
=========================================

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build
        $ cd build
        $ make run_example_freertos_cifar10_extmem

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build
        $ cd build
        $ nmake run_example_freertos_cifar10_extmem
