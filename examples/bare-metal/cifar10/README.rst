########
CIFAR-10
########

This example application implements a CNN architecture trained on the `CIFAR-10 <https://www.cs.toronto.edu/~kriz/cifar.html>`__ dataset.  This example demonstrates how to place models in SRAM, Flash or LPDDR.  And, this example demonstrates how to benchmark inference duration.

The example reads a test image specified on the command line over JTAG.  Input images must be raw RGB buffers with size 32x32x3.  Any image with the proper shape should work.  The application will attempt to classify an entity in the image and assign it to one of the following classes:

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

A Python 3 script is provided that will generate one example image from each of the classes above. This script requires `Tensorflow <https://www.tensorflow.org/>`__ and `Numpy <https://numpy.org/>`__.  If you have already installed the XMOS AI Toolchain extensions then you have these requirements.  Alternatively, you can install using `pip`.  To generate the images run:

.. code-block:: console

    $ cd test_inputs
    $ ./make_test_tensors.py

For background information on the CIFAR-10 dataset, please read `Learning Multiple Layers of Features from Tiny Images <https://www.cs.toronto.edu/~kriz/learning-features-2009-TR.pdf>`__, Alex Krizhevsky, 2009.

*********************
Building the firmware
*********************

Make a directory for the build.

.. code-block:: console

    $ mkdir build
    $ cd build

Run cmake:

.. code-block:: console

    $ cmake ../
    $ make

To install, run:

.. code-block:: console

    $ make install

Using external memory
=====================

To building with the model stored in flash, replace the call to cmake above with the following:

.. code-block:: console

    $ cmake ../ -DUSE_SWMEM=1

To flash the model, run the following commands:

.. code-block:: console

    $ cd bin
    $ xobjdump --strip cifar10.xe
    $ xobjdump --split cifar10.xb
    $ xflash --write-all image_n0c0.swmem --target XCORE-AI-EXPLORER

To building with the model stored in LPDDR, replace the call to cmake above with the following:

.. code-block:: console

    $ cmake ../ -DUSE_EXTMEM=1

No additional steps are necessary to copy the model into LPDDR.

Running the firmware
====================

Running with the xCORE simulator.

.. code-block:: console

    $ xsim --xscope "-offline trace.vcd" --args bin/cifar10.xe test_inputs/horse.bin

Running with hardware.

.. code-block:: console

    $ xrun --xscope --args bin/cifar10.xe test_inputs/horse.bin

********************
Optimizing the model
********************

Unoptimized and optimized models are included with the example.

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

.. code-block:: console

    $ xformer.py --analyze -par 5 model/model_quant.tflite model/model_xcore.tflite


********************
Optimizing the model
********************

Generating the model runner
===========================

The following command will generate source files for a model runner as well as the TensorFlow Lite model as a character array that can be use by the runner:

.. code-block:: console

    $ generate_model_runner.py --input model/model_xcore.tflite --output model_runner --name cifar10

Converting flatbuffer to source file
====================================

If you do not want to regenerate the model runner, the following command will generate ony the C source file that contains the TensorFlow Lite model as a character array:

.. code-block:: console

    $ convert_tflite_to_c_source.py --input model/model_xcore.tflite --header model_runner/src/cifar10_model_data.h --source model_runner/src/cifar10_model_data.c --variable-name cifar10
