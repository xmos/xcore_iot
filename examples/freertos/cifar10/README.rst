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

A Python 3 script is provided that will generate one example image from each of the classes above. This script requires `Tensorflow <https://www.tensorflow.org/>`__ and `Numpy <https://numpy.org/>`__.  If you have already installed the XMOS AI Toolchain extensions then you have these requirements.  Alternatively, you can install using `pip`.  To generate the images run:

.. code-block:: console

    $ cd filesystem_support/test_inputs
    $ ./make_test_tensors.py

For background information on the CIFAR-10 dataset, please read `Learning Multiple Layers of Features from Tiny Images <https://www.cs.toronto.edu/~kriz/learning-features-2009-TR.pdf>`__, Alex Krizhevsky, 2009.

****************
Supported Boards
****************

This example is supported on the XCORE-AI-EXPLORER board and the AiOT Board (OSPREY-BOARD). 
Set the $TARGET environment variable to the board that you are using. For example:

.. code-block:: console

    $ export TARGET=OSPREY-BOARD

The build and run commands shown below will then pick up the correct target automatically.
Please note: The external DDR memory options are only available on the XCORE-AI-EXPLORER board. 


*********************
Building the firmware
*********************

Make a directory for the build.

.. code-block:: console

    $ mkdir build
    $ cd build

Using SRAM memory
=================

Run cmake:

.. code-block:: console

    $ cmake ../ -DBOARD=$TARGET
    $ make

Using external flash memory
===========================

To building with the model stored in flash, replace the call to cmake above with the following:

.. code-block:: console

    $ cmake ../ -DBOARD=$TARGET -DUSE_SWMEM=1
    $ make

To flash the model and example images, run the following commands:

.. code-block:: console

    $ cd filesystem_support
    $ ./flash_image.sh -s $TARGET

Using external DDR memory
=========================

To building with the model stored in LPDDR, replace the call to cmake above with the following:

.. code-block:: console

    $ cmake ../ -DBOARD=XCORE-AI-EXPLORER -DUSE_EXTMEM=1
    $ make

To flash the example images, run the following commands:

.. code-block:: console

    $ cd filesystem_support
    $ ./flash_image.sh -f XCORE-AI-EXPLORER

Running the firmware
====================

Running with hardware.

.. code-block:: console

    $ xrun --xscope bin/$TARGET/cifar10.xe

********************
Optimizing the model
********************

Unoptimized and optimized models are included with the example.

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

.. code-block:: console

    $ xformer.py --analyze -par 5 model/model_quant.tflite model/model_xcore.tflite

Converting flatbuffer to source file
====================================

The following command will generate a C source file that contains the TensorFlow Lite model as a character array:

.. code-block:: console

    $ python ../../../tools/generate/convert_tflite_to_c_source.py --input model/model_xcore.tflite --header inference_engine/src/cifar10_model.h --source inference_engine/src/cifar10_model.c --variable-name cifar10_model
