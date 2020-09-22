# CIFAR-10 example application

This example application implements a CNN architecture trained on the [CIFAR-10](https://www.cs.toronto.edu/~kriz/cifar.html) dataset.  This example demonstrates how to place models in SRAM, Flash or LPDDR.

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

A Python 3 script is provided that will generate one example image from each of the classes above. This script requires [Tensorflow](https://www.tensorflow.org/) and [Numpy](https://numpy.org/).  If you have already installed the XMOS AI Toolchain extenstions then you have these requirements.  Alternatively, you can install using `pip`.  To generate the images run:

    > cd test_inputs
    > ./make_test_tensors.py

For background information on the CIFAR-10 dataset, please read [Learning Multiple Layers of Features from Tiny Images](https://www.cs.toronto.edu/~kriz/learning-features-2009-TR.pdf), Alex Krizhevsky, 2009.


## Prerequisites for building

[XMOS Toolchain 15.0.1](https://www.xmos.com/software/tools/) or newer.

Install [CMake](https://cmake.org/download/) version 3.14 or newer.

Set environment variable for the XMOS AIoT SDK:

    > export XMOS_AIOT_SDK_PATH=<path to>/aiot_sdk

## Building for xCORE

Make a directory for the build.

    > mkdir build
    > cd build

Run cmake:

    > cmake ../
    > make

To install, run:

    > make install

### Using external memory

To building with the model stored in flash, replace the call to cmake above with the following:

    > cmake ../ -DUSE_SWMEM=ON

To flash the model, run the following commands:

    > cd bin
    > xobjdump --strip cifar10.xe
    > xobjdump --split cifar10.xb
    > xflash --write-all image_n0c0.bin --target XCORE-AI-EXPLORER

To building with the model stored in LPDDR, replace the call to cmake above with the following:

    > cmake ../ -DUSE_EXTMEM=ON

No additional steps are necessary to copy the model into LPDDR.

### Running the firmware

Running with the xCORE simulator.

    > xsim --xscope "-offline trace.xmt" --args bin/cifar10.xe test_inputs/horse.bin

Running with hardware.

    > xrun --io --xscope --args bin/cifar10.xe test_inputs/horse.bin

## Building for x86

TO build for x86, run:

    > cmake ../ -DXCORE=OFF
    > make install

Run the following command on the host PC:

    > ./bin/cifar10 test_inputs/horse.bin

## Optimizing Model

Unoptimized and optimized models are included with the example.

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

    > xformer.py --analyze -par 5 model/model.tflite model/model_xcore.tflite

### Converting flatbuffer to Source File

The following command will generate a C source file that contains the TensorFlow Lite model as a character array:

    > python ../../../third_party/tensorflow/tensorflow/lite/python/convert_file_to_c_source.py --input_tflite_file ../../models/CIFAR-10/debug/arm_benchmark/models/model_xcore.tflite --output_header_file src/cifar10_model.h --output_source_file src/cifar10_model.c --array_variable_name cifar10_model --include_guard CIFAR10_MODEL_H_

The following command will generate a C source file that contains the TensorFlow Lite model as a character array:

    > python ../../../../ai_tools/third_party/tensorflow/tensorflow/lite/python/convert_file_to_c_source.py --input_tflite_file model/model_xcore.tflite --output_header_file inference_engine/src/cifar10_model.h --output_source_file inference_engine/src/cifar10_model.c --array_variable_name cifar10_model --include_guard CIFAR10_MODEL_H_


Note, the command above will overwrite `inference_engine/src/cifar10_model.c`.  In order to allow the model to be stored in flash or DDR, the file needs to be modified after the script creates it.  Add the following lines directly above the line that sets `cifar10_model[]`.

    #ifdef USE_SWMEM
    __attribute__((section(".SwMem_data")))
    #elif USE_EXTMEM
    __attribute__((section(".ExtMem_data")))
    #endif
