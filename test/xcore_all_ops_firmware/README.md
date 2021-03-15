# Model Inference application

This application will run inference on a TensorFlow Lite for Microcontrollers
model (.tflite) and input tensors provided by the XCOREDeviceInterpreter.

## xCORE

Building all targets for xCORE

    > mkdir build
    > cd build
    > cmake ../
    > make

## x86

Building x86 host target

    > mkdir build
    > cd build
    > cmake ../ -DX86=ON
    > make

