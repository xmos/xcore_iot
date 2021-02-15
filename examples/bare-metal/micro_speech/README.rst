############
Micro Speech
############

This example application is the `micro_speech <https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/micro/examples/micro_speech>`__ example from TensorFlow Lite for Microcontrollers.

****************
Supported Boards
****************

This example is supported on the XCORE-AI-EXPLORER and OSPREY-BOARD boards.
Set the $TARGET environment variable to the board that you are using. For example:

.. code-block:: console

    $ export TARGET=XCORE-AI-EXPLORER

or 

.. code-block:: console

    $ export TARGET=OSPREY-BOARD

The build and run commands shown below will then pick up the correct target automatically.

.. note::

    The external DDR memory options are only available on the XCORE-AI-EXPLORER board.

*********************
Building the firmware
*********************

Make a directory for the build.

.. code-block:: console

    $ mkdir build
    $ cd build

Run cmake.

.. code-block:: console

    $ cmake ../ -DBOARD=$TARGET
    $ make

To install, run:

.. code-block:: console

    $ make install

Running the firmware
====================

Run the following command:

.. code-block:: console

    $ xrun --io bin/$TARGET/micro_speech.xe

You should notice console output, which will update based on the model result.

.. code-block:: console

    Heard silence (204) @368ms
    Heard silence (167) @544ms
    Heard no (162) @6864ms

Running the test firmware
=========================

The test application loads model input features that are defined in static arrays.  One feature array is provided for "yes", and another for "no". See,

.. code-block:: cpp

    #include "tensorflow/lite/micro/examples/micro_speech/micro_features/no_micro_features_data.h"
    #include "tensorflow/lite/micro/examples/micro_speech/micro_features/yes_micro_features_data.h"

Run the following command:

.. code-block:: console

    $ xrun --io bin/$TARGET/micro_speech_test.xe

You should notice console output

.. code-block:: console

    Testing TestInvoke
    Ran successfully

    1/1 tests passed
    ~~~ALL TESTS PASSED~~~

********************
Optimizing the model
********************

If the model is retrained, you will need to optimize it for xcore.ai.

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

.. code-block:: console

    $ xformer.py --analyze model/model_quant.tflite model/model_xcore.tflite

**NOTE: Due to a limitation, the micro_speech model is left unoptimized.  This limitation will be eliminated by a pending software enhancement.**

Converting flatbuffer to source file
====================================

The following command will generate a C source file that contains the TensorFlow Lite model as a character array:

.. code-block:: console

    $ convert_tflite_to_c_source.py --input model/model_xcore.tflite --header model.h --source model.c --variable-name g_model --include-guard TENSORFLOW_LITE_MICRO_EXAMPLES_MICRO_SPEECH_MODEL_H_

******************
Training the model
******************

You may wish to retrain this model.  This should rarely be necessary. However, if you would like to learn more about how this model is trained, see: https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/micro/examples/micro_speech/train

