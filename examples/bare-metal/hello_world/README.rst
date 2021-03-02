###########
Hello World
###########

This example application is the `hello_world <https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/micro/examples/hello_world>`__ example from TensorFlow Lite for Microcontrollers.

*********************
Building the firmware
*********************

Make a directory for the build.

.. code-block:: console

    $ mkdir build
    $ cd build

Run cmake.

.. code-block:: console

    $ cmake ../
    $ make

To install, run:

.. code-block:: console

    $ make install

Running the firmware
====================

Run the following command:

.. code-block:: console

    $ xrun --io bin/hello_world.xe 

You should notice a wave pattern moving back and forth on the LEDs.

.. image:: images/leds.*
    :width: 200px
    :align: left

********************
Optimizing the model
********************

If the model is retrained, you will need to optimize it for xcore.ai.  

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

.. code-block:: console

    $ xformer.py --analyze model/model_quant.tflite model/model_xcore.tflite

The following command will generate a C source file that contains the TensorFlow Lite model as a character array:

.. code-block:: console

    $ convert_tflite_to_c_source.py --input model/model_xcore.tflite --header model.h --source model.c --variable-name g_model --include-guard TENSORFLOW_LITE_MICRO_EXAMPLES_HELLO_WORLD_MODEL_H_

******************
Training the model
******************

You may wish to retrain this model.  This should rarely be necessary. However, if you would like to learn more about how this model is trained, see: https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/micro/examples/hello_world/train

