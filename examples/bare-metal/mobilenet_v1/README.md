# MobileNet V1 Example applications

This example application implements a [MobileNet V1](https://arxiv.org/abs/1704.04861) CNN architecture trained on the [ImageNet](http://www.image-net.org/) dataset.  This example demonstrates how to place models in LPDDR and to recieve input data using `xscope`.  The application will attempt to classify an entity in the image and assign it to one of the following classes:

- tench
- goldfish
- great_white_shark
- tiger_shark
- hammerhead
- electric_ray
- stingray
- cock
- hen
- ostrich

## Prerequisites for building

[XMOS Toolchain 15.0.1](https://www.xmos.com/software/tools/) or newer.

Install [CMake](https://cmake.org/download/) version 3.10 or newer.

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

    > xrun --io --xscope --xscope-port localhost:10234 bin/mobilenet_v1.xe

Running with simulator

    > xsim --xscope "-realtime localhost:10234" bin/mobilenet_v1.xe

Test images can be sent to the firmware using `xscope`.  Most RGB images should work.  The `test_image.py` script requires the following Python modules:

    pip install numpy
    pip install matplotlib
    pip install opencv-python

Sending a test image to the xcore.ai Explorer board using `xscope`.

    > ./test_image.py path/to/image

## Optimizing Model

Unoptimized and optimized models are included with the example.

First, be sure you have installed the XMOS AI Toolchain extensions.  If installed, you can optimize your model with the following command:

    > xformer.py --analyze -par 5 model/model.tflite model/model_xcore.tflite

### Converting flatbuffer to Source File

The following unix command will generate a C source file that contains the TensorFlow Lite model as a char array

    > python ../../../tools/ai_tools/third_party/tensorflow/tensorflow/lite/python/convert_file_to_c_source.py --input_tflite_file model/model_xcore.tflite --output_header_file inference_engine/src/mobilenet_v1.h --output_source_file inference_engine/src/mobilenet_v1.c --array_variable_name mobilenet_v1_model --include_guard MOBILENET_V1_MODEL_H_

Note, the command above will overwrite `mobilenet_v1.c`.  In order to allow the model to be stored in flash or DDR, the file needs to be modified after the script creates it.  Add the following lines directly above the line that sets `mobilenet_v1_model[]`.

    // NOTE: The following model was generated with:
    //        classes = 10
    //        alpha = 0.50
    //        image size 128x128
    __attribute__((section(".ExtMem_data")))
