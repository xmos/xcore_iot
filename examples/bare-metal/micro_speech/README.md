# Micro Speech example application

This example application is the [micro_speech](https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/micro/examples/micro_speech) example from TensorFlow Lite for Microcontrollers.

## Prerequisites for building

[XMOS Toolchain 15.0.1](https://www.xmos.com/software/tools/) or newer.

Install [CMake](https://cmake.org/download/) version 3.14 or newer.

Set environment variable for the XMOS AIoT SDK:

    > export XMOS_AIOT_SDK_PATH=<path to>/aiot_sdk

## Building for xCORE

Make a directory for the build.

    > mkdir build
    > cd build

Run cmake.

    > cmake ../
    > make

To install, run:

    > make install

## Running the firmware

Run the following command:

    > xrun --io bin/micro_speech.xe

You should notice console output, which will update based on the model result.

    Heard silence (204) @368ms
    Heard silence (167) @544ms
    Heard no (162) @6864ms

## Running the test firmware

The test application loads model input features that are defined in static arrays.  One feature array is provided for "yes", and another for "no". See,

    #include "tensorflow/lite/micro/examples/micro_speech/micro_features/no_micro_features_data.h"
    #include "tensorflow/lite/micro/examples/micro_speech/micro_features/yes_micro_features_data.h"

Run the following command:

    > xrun --io bin/micro_speech_test.xe

You should notice console output

    Testing TestInvoke
    Ran successfully

    1/1 tests passed
    ~~~ALL TESTS PASSED~~~

## Training

You may wish to retrain this model.  This should rarely be necessary. However, if you would like to learn more about how this model is trained, see: https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/micro/examples/micro_speech/train

### Optimizing Model

If the model is retrained, you will need to optimize it for xcore.ai.

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

    > xformer.py --analyze model/model_quant.tflite model/model_xcore.tflite

**NOTE: Due to a limitation, the micro_speech model is left unoptimized.  This limitation will be eliminated by a pending software enhancement.**

The following command will generate a C source file that contains the TensorFlow Lite model as a character array:

    > python ../../../tools/ai_tools/third_party/tensorflow/tensorflow/lite/python/convert_file_to_c_source.py --input_tflite_file model/model_xcore.tflite --output_header_file model.h --output_source_file model.c --array_variable_name g_model --include_guard TENSORFLOW_LITE_MICRO_EXAMPLES_MICRO_SPEECH_MODEL_H_
