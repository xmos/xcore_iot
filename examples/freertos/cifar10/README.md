# CIFAR-10 example application

This example application implements a CNN architecture trained on the [CIFAR-10](https://www.cs.toronto.edu/~kriz/cifar.html) dataset.  This example demonstrates how to place models in SRAM, Flash or LPDDR.

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

A Python 3 script is provided that will generate one example image from each of the classes above. This script requires [Tensorflow](https://www.tensorflow.org/) and [Numpy](https://numpy.org/).  If you have already installed the XMOS AI Toolchain extensions then you have these requirements.  Alternatively, you can install using `pip`.  To generate the images run:

    > cd filesystem_support/test_inputs
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

### Using SRAM memory

Run cmake:

    > cmake ../ -DBOARD=XCORE-AI-EXPLORER
    > make

### Using external flash memory

To building with the model stored in flash, replace the call to cmake above with the following:

    > cmake ../ -DBOARD=XCORE-AI-EXPLORER -DUSE_SWMEM=1
    > make

To flash the model and example images, run the following commands:

    > cd filesystem_support
    > ./flash_image.sh -s

### Using external DDR memory

To building with the model stored in LPDDR, replace the call to cmake above with the following:

    > cmake ../ -DBOARD=XCORE-AI-EXPLORER -DUSE_EXTMEM=1
    > make

To flash the example images, run the following commands:

    > cd filesystem_support
    > ./flash_image.sh -f

### Running the firmware

Running with hardware.

    > xrun --xscope bin/cifar10.xe

## Optimizing Model

Unoptimized and optimized models are included with the example.

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

    > xformer.py --analyze -par 5 model/model_quant.tflite model/model_xcore.tflite

### Converting flatbuffer to Source File

The following command will generate a C source file that contains the TensorFlow Lite model as a character array:

    > python ../../../tools/ai_tools/third_party/tensorflow/tensorflow/lite/python/convert_file_to_c_source.py --input_tflite_file model/model_xcore.tflite --output_header_file inference_engine/src/cifar10_model.h --output_source_file inference_engine/src/cifar10_model.c --array_variable_name cifar10_model --include_guard CIFAR10_MODEL_H_

Note, the command above will overwrite `inference_engine/src/cifar10_model.c`.  In order to allow the model to be stored in flash or DDR, the file needs to be modified after the script creates it.  Add the following lines directly above the line that sets `cifar10_model[]`.

    #ifdef USE_SWMEM
    __attribute__((section(".SwMem_data")))
    #elif USE_EXTMEM
    __attribute__((section(".ExtMem_data")))
    #endif
