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

This example is supported on the XCORE-AI-EXPLORER board.
Set the $TARGET environment variable to the board that you are using. For example:

.. code-block:: console

    $ export TARGET=XCORE-AI-EXPLORER

The build and run commands shown below will then pick up the correct target automatically.

.. note::

    The external DDR memory options are only available on the XCORE-AI-EXPLORER board.

*********************
Building the firmware
*********************

Using SRAM memory
=================

Run make:

.. code-block:: console

    $ make BOARD=$TARGET

To flash the example images, run the following commands:

.. code-block:: console

    $ cd filesystem_support
    $ ./flash_image.sh -f

Using external flash memory
===========================

To build with the model stored in flash, replace the call to make above with the following:

.. code-block:: console

    $ make BOARD=$TARGET USE_SWMEM=1

To flash the model and example images, run the following commands:

.. code-block:: console

    $ cd filesystem_support
    $ ./flash_image.sh -s

Using external DDR memory
=========================

If your board supports LPDDR, you may also place your neural network in the external DDR memory.  Currently, only the Explorer Board supports LPDDR.

To build with the model stored in LPDDR, replace the call to make above with the following:

.. code-block:: console

    $ make BOARD=$TARGET USE_EXTMEM=1

To flash the example images, run the following commands:

.. code-block:: console

    $ cd filesystem_support
    $ ./flash_image.sh -f

Running the firmware
====================

Running with hardware.

.. code-block:: console

    $ xrun --xscope bin/cifar10.xe

********************
Optimizing the model
********************

Unoptimized and optimized models are included with the example.

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

.. code-block:: console

    $ xformer.py --analyze -par 5 model/model_quant.tflite model/model_xcore.tflite

Generating the model runner
===========================

The following command will generate source files for a model runner as well as the TensorFlow Lite model as a character array that can be use by the runner:

.. code-block:: console

    $ generate_model_runner.py --input model/model_xcore.tflite --output src/model_runner --name cifar10

Converting flatbuffer to source file
====================================

The following command will generate a C source file that contains the TensorFlow Lite model as a character array:

.. code-block:: console

    $ convert_tflite_to_c_source.py --input model/model_xcore.tflite --header inference_engine/src/cifar10_model.h --source inference_engine/src/cifar10_model.c --variable-name cifar10_model
