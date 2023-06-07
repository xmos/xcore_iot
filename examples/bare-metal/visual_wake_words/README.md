# Explorer Board

This example application implements the `Visual Wake Words <https://blog.tensorflow.org/2019/10/visual-wake-words-with-tensorflow-lite_30.html>`__ CNN architecture.  The VWW model is trained to classify images to two classes (person/not-person) and serves as a popular use-case for microcontrollers.

This example demonstrates how to receive input data using `xscope`.

## CMake Targets

The following CMake targets are provided:

- example_bare_metal_vww
- run_example_bare_metal_vww
- flash_app_example_bare_metal_vww

## Deploying the Firmware & Optimizing the Model

See the Programming Guide for information on building and running the application as well as information on optimizing the TFLite quantized model for xcore using the XMOS AI Tools.  
