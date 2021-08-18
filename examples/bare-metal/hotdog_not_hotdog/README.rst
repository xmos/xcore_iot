################################################
MobileNet V1 Application for Classifying Hotdogs
################################################

This example application implements a `MobileNet V1  <https://arxiv.org/abs/1704.04861>`__ CNN architecture trained on the `ImageNet <http://www.image-net.org/>`__ dataset.  The model was trained using transfer learning on a new training set containing images of hotdog and not-hotdog food stuffs. Further explanation can be found here: https://www.youtube.com/watch?v=vIci3C4JkL0

- Classes = 2
- Alpha = 0.25
- Image size = 128x128

This example demonstrates how to place models in LPDDR and to recieve input data using `xscope`.  The application will attempt to classify an entity in the image and assign it to one of the following classes:

- hotdog
- not_hotdog

This example also demonstrates how to to build the inference engine into a static library.

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

Running the firmware
====================

Running with hardware

.. code-block:: console

    $ xrun --xscope --xscope-port localhost:10234 bin/hotdog_not_hotdog.xe

Running with simulator

.. code-block:: console

    $ xsim --xscope "-realtime localhost:10234" bin/hotdog_not_hotdog.xe

Test images can be sent to the firmware using `xscope`.  Most RGB images should work.  The `test_image.py` script requires Python.  Ensure you have installed Python 3 and the XCore SDK Python requirements.

Sending a test image to the xcore.ai Explorer board using `xscope`.

.. code-block:: console

    $ ./test_image.py path/to/image

********************
Optimizing the model
********************

An unoptimized model is included with the example.

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

.. code-block:: console

    $ xformer.py --analyze -par 5 model/model_quant.tflite model/model_xcore.tflite

Generating the model runner
===========================

The following command will generate source files for a model runner as well as the TensorFlow Lite model as a character array that can be use by the runner:

.. code-block:: console

    $ generate_model_runner.py --input model/model_xcore.tflite --output model_runner --name hotdog_not_hotdog

Converting flatbuffer to source file
====================================

The following unix command will generate a C source file that contains the TensorFlow Lite model as a char array

.. code-block:: console

    $ convert_tflite_to_c_source.py --input model/model_xcore.tflite --header model_runner/hotdog_not_hotdog_model_data.h --source model_runner/hotdog_not_hotdog_model_data.c --variable-name hotdog_not_hotdog
