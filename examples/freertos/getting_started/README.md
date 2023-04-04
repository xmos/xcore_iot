# FreeRTOS Getting Started Example

This is a simple multi-tile FreeRTOS example application. It starts the FreeRTOS scheduler running on both `tile[0]` and `tile[1]`.  `tile[0]` has two tasks running.  One prints "Hello from tile 0" to the console and the other task blinks the 4 LEDs on the Explorer board.  `tile[1]` has one task running that prints "Hello from tile 0" to the console.

## CMake Targets

The following CMake targets are provided:

- example_freertos_getting_started
- run_example_freertos_getting_started

## Deploying the Firmware

See the Programming Guide for information on building and running the application.
