#################
Visual Wake Words
#################

This example application implements the `Visual Wake Words <https://blog.tensorflow.org/2019/10/visual-wake-words-with-tensorflow-lite_30.html>`__ CNN architecture.  The VWW model is trained to classify images to two classes (person/not-person) and serves as a popular use-case for microcontrollers.

This example demonstrates how to receive input data using `xscope`.

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

Now return back the application directory:

.. code-block:: console

    $ cd ..

Running the firmware
====================

Running with hardware

.. code-block:: console

    $ xrun --xscope-port localhost:10234 bin/vww.xe

Running with simulator

.. code-block:: console

    $ xsim --xscope "-realtime localhost:10234" bin/vww.xe

The firmware will now wait until a data is sent from a host application. Test images can be sent to the firmware using `xscope`.  Most RGB images should work.  The `test_image.py` script requires the following Python modules and should be run in a new terminal window:

.. code-block:: console

    $ pip install numpy
    $ pip install matplotlib
    $ pip install opencv-python

Sending a test image to the xcore.ai Explorer board using `xscope`. The `test_image.py` script can be found in the application directory:

.. code-block:: console

    $ ./test_image.py path/to/image

********************
Optimizing the model
********************

Unoptimized and optimized models are included with the example.

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

.. code-block:: console

    $ xformer.py --analyze -par 5 model/model_quant.tflite model/model_xcore.tflite

Converting flatbuffer to source file
====================================

The following unix command will generate a C source file that contains the TensorFlow Lite model as a char array

.. code-block:: console

    $ convert_tflite_to_c_source.py --input model/model_xcore.tflite --header inference_engine/src/vww.h --source inference_engine/src/vww.c --variable-name vww
