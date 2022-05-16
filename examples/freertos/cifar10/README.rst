########
CIFAR-10
########

This example application implements a CNN architecture trained on the `CIFAR-10 <https://www.cs.toronto.edu/~kriz/cifar.html>`__ dataset.  The example reads a set of test images from a filesystem in flash.  The FreeRTOS kernel manages filesystem IO and sends example images to the AI device that implements the CIFAR-10 model.  The application will attempt to classify an entity in the image and assign it to one of the following classes:

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

Run the following commands in the xcore_sdk root folder to build the firmware:

.. tab:: Linux and Mac

    .. code-block:: console

        $ cmake -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ make example_freertos_cifar10

.. tab:: Windows

    .. code-block:: console

        $ cmake -G "NMake Makefiles" -B build -DCMAKE_TOOLCHAIN_FILE=tools/xmos_cmake_toolchain/xs3a.cmake
        $ cd build
        $ nmake example_freertos_cifar10


***********************
Setting up the hardware
***********************

.. note::
   The host applications are required to create the filesystem.  See the SDK Installation instructions for more information.

Before running the firmware, the filesystem containing the images must be flashed.  After the images have been generated, by following the instructions above:

.. tab:: Linux and Mac

    .. code-block:: console

        $ make flash_fs_example_freertos_cifar10

.. tab:: Windows

    .. code-block:: console

        $ nmake flash_fs_example_freertos_cifar10


********************
Running the firmware
********************

Running with hardware.


.. tab:: Linux and Mac

    .. code-block:: console

        $ make run_example_freertos_cifar10

.. tab:: Windows

    .. code-block:: console

        $ nmake run_example_freertos_cifar10

********************
Optimizing the model
********************

An unoptimized, quantized model is included with the example.

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

.. code-block:: console

    $ xcore-opt --xcore-flash-image-file=filesystem_support/model.bin -o model/model_xcore.tflite model/model_quant.tflite

Converting flatbuffer to source file
====================================

The following unix command will generate a C source file that contains the TensorFlow Lite model as a char array.

.. code-block:: console

    $ python <path-to-sdk>/tools/tflite_micro/convert_tflite_to_c_source.py --input model/model_xcore.tflite --header src/image_classifier/cifar10_model_data.h --source src/image_classifier/cifar10_model_data.c --variable-name cifar10