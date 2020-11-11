# TensorFlow person detection example application

This example demonstrates how to integrate the xCORE FreeRTOS port with the TensorFlow person detection example application.  This application places the model in LPDDR.

The example uses an [ArduCam 2MP Plus OV2640 Mini Module SPI camera](https://www.arducam.com/product/arducam-2mp-spi-camera-b0067-arduino/) to capture a 96x96 8 bit grayscale image.  The application will attempt to classify whether the image contains a person or does not contain a person.

When a person is detected LED 0 turns on.  When a person is not detected LED 0 turns off.  LED 3 toggles after each inference.  Additionally, a Python 3 script is provided to output image and output tensor pairs to the host machine.  This script requires [Numpy](https://numpy.org/) and [Matplotlib](https://matplotlib.org/).

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

    > cmake ../ -DBOARD=XCORE-AI-EXPLORER
    > make

## Hardware Setup

### Materials Required:
  * [ArduCam 2MP Plus OV2640 Mini Module SPI camera](https://www.arducam.com/product/arducam-2mp-spi-camera-b0067-arduino/)
  * 0.100" female to female jumper wires
  * 0.100" (2.54mm) throughhole header pins
  * Soldering tools and supplies

To connect the Arducam to the Explorer Board use the schematic and following table for reference:

| ArduCam  | Explorer Board |
|:--------:|:--------------:|
| CS       | J14 : X0D00    |
| MOSI     | J6 : MOSI      |
| MISO     | J6 : MISO      |
| SCK      | J6 : CLK       |
| GND      | J5 : GND       |
| VCC      | TP8 : 3V3      |
| SDA      | TP19 : SDA_IOL |
| SCL      | TP18 : SCL_IOL |

## Running the firmware

This demo can be run with only GPIO or GPIO and host output.

Running with GPIO only:

    > xrun --xscope ../bin/XCORE-AI-EXPLORER/person_detection.xe

Running with GPIO and host:

    > xrun --xscope --xscope-port localhost:10234 ../bin/XCORE-AI-EXPLORER/person_detection.xe

In a second terminal:

    > python image_viewer.py

Once the host script connects to the xscope server the image and associated output tensor values will be displayed.

Example Person Detected:

![person](images/person.png)

Example No Person Detected:

![not_person](images/not_person.png)

## Training

You may wish to retrain this model.  This should rarely be necessary. However, if you would like to learn more about how this model is trained, see: https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/micro/examples/person_detection/training_a_model.md

### Optimizing Model

If the model is retrained, you will need to optimize it for xcore.ai.  Refer to the documentation in ai_tools/doc for information on this process.
