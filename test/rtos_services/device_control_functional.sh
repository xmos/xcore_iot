#!/bin/sh


# This script is one part of a functional test for the device_control sw_service using the
# device_control application. It ensures the board receives a particular command.


# Establish working directories
# sdk root dir
SDK_ROOT_DIR=$(git rev-parse --show-toplevel)

# host app dir
HOST_APP_DIR=$SDK_ROOT_DIR/build_host/examples/freertos/device_control/host


# Set up a clean log
REPORT="device_control_test.log"
rm -f ${REPORT} 


# Run firmware on the board
cd $SDK_ROOT_DIR
cd build
make run_example_freertos_device_control &
sleep 20 # allow firmware to run before firing host app


# Run host app command, log it
HOST_APP_CMD="$HOST_APP_DIR/example_freertos_device_control_host -g version"
($HOST_APP_CMD 2>&1 | tee -a ${REPORT})


# Cleanup
pkill -P $$
wait # for it