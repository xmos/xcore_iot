# MobileNet V1 Example applications for classifying hotdogs

This example application implements a [MobileNet V1](https://arxiv.org/abs/1704.04861) CNN architecture trained on the [ImageNet](http://www.image-net.org/) dataset.  The model was trained using transfer learning on a new training set containing images of hotdog and not-hotdog food stuffs. Further explanation can be found here: https://www.youtube.com/watch?v=vIci3C4JkL0

- Classes = 2
- Alpha = 0.25
- Image size = 128x128

This example demonstrates how to place models in LPDDR and to recieve input data using `xscope`.  The application will attempt to classify an entity in the image and assign it to one of the following classes:

- hotdog
- not_hotdog


This example also demonstrates how to to build the inference engine into a static library.

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

### Running the firmware

Running with hardware

    > xrun --io --xscope --xscope-port localhost:10234 bin/hotdog_not_hotdog.xe

Running with simulator

    > xsim --xscope "-realtime localhost:10234" bin/hotdog_not_hotdog.xe

Test images can be sent to the firmware using `xscope`.  Most RGB images should work.  The `test_image.py` script requires the following Python modules:

    pip install numpy
    pip install matplotlib
    pip install opencv-python

Sending a test image to the xcore.ai Explorer board using `xscope`.

    > ./test_image.py path/to/image

## Optimizing Model

Unoptimized and optimized models are included with the example.

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

    > xformer.py --analyze -par 5 model/model_quant.tflite model/model_xcore.tflite

### Converting flatbuffer to Source File

The following unix command will generate a C source file that contains the TensorFlow Lite model as a char array

    > python ../../../tools/generate/convert_tflite_to_c_source.py --input model/model_xcore.tflite --header inference_engine/src/hotdog_not_hotdog.h --source inference_engine/src/hotdog_not_hotdog.c --variable-name hotdog_not_hotdog_model
