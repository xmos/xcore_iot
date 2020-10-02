# Independent tiles example application

This example application demonstrates how to build a project with a unique FreeRTOS kernel per tile.  The example implements a distributed computing setup, utilizing the intertile device to allow one kernel to perform a remote procedure call on the other tile.

## Prerequisites for building

[XMOS Toolchain 15.0.1](https://www.xmos.com/software/tools/) or newer.

Install [CMake](https://cmake.org/download/) version 3.14 or newer.

Set environment variable for the XMOS AIoT SDK:

    > export XMOS_AIOT_SDK_PATH=<path to>/aiot_sdk

## Building for xCORE

To build this project run:

    > make BOARD=XCORE-AI-EXPLORER -j

### Running the firmware

Running with hardware.

    > xrun --xscope bin/independent_tiles.xe
