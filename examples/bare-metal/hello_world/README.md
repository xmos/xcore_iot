# Hello World example application

This example application is the [hello_world](https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/micro/examples/hello_world) example from TensorFlow Lite for Microcontrollers.

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

    > xrun -io bin/hello_world.xe 

You should notice a wave pattern moving back and forth on the LEDs.

![](images/leds.gif)

## Training

You may wish to retrain this model.  This should rarely be necessary. However, if you would like to learn more about how this model is trained, see: https://github.com/tensorflow/tensorflow/tree/master/tensorflow/lite/micro/examples/hello_world/train

### Optimizing Model

If the model is retrained, you will need to optimize it for xcore.ai.  

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

    > xformer.py --analyze model/model.tflite model/model_xcore.tflite

The following command will generate a C source file that contains the TensorFlow Lite model as a character array:

    > python ../../../../ai_tools/third_party/tensorflow/tensorflow/lite/python/convert_file_to_c_source.py --input_tflite_file model/model_xcore.tflite --output_header_file model.h --output_source_file model.c --array_variable_name g_model --include_guard TENSORFLOW_LITE_MICRO_EXAMPLES_HELLO_WORLD_MODEL_H_

